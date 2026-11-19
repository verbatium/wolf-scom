# Wolf SCOM protocol (DWT ↔ boiler)

Notes from passive sniffing of the SCOM bus between a Wolf **DWT** room/weather controller and a
**Wolf GG-2EK-S / GU-2EK-S** (non-condensing) gas boiler.

Raw capture: [`scom-capture-2026-11-19.log`](scom-capture-2026-11-19.log) (~2 min, 1619 bytes).

## Physical layer

- 2-wire bus that carries both **power (≈18 V DC, supplied by the boiler)** and data. The DWT is powered from the bus.
- UART framing: **2400 baud**, 8N1.
- Interface used: an **M-Bus slave transceiver module** (TSS721-like chip, two optocouplers, pins
  `GBUS VB PF TX RX VDD VS`, MCU side `GND RXD TXD VIN`). It receives the bus voltage modulation and can
  transmit by current modulation — the same way a bus-powered slave device (like the DWT) talks.
- ESP32-C3: RX = GPIO20, TX = GPIO21. The component currently only reads.

## Roles

- **The DWT initiates every exchange; the boiler only answers.** Without a DWT (or something emulating it) the bus is silent.
- `0x53` (`'S'`) starts a **write** (DWT → boiler), `0x44` (`'D'`) starts a **read** (boiler → DWT).
- Every byte is **echoed** by the other side, and `0x06` is an ACK.
  Bytes are spaced ~25–50 ms apart while one byte at 2400 baud takes ~4 ms, i.e. the exchange looks
  byte-by-byte: send a byte, wait for its echo, send the next one.

## Frame formats (8 bytes as seen on the bus)

Write (DWT sets a value in the boiler):

```
53 53  CC CC  VV VV  06  VV
S  S'  cmd    value  ACK value
```

Read (DWT queries a value from the boiler):

```
44 44  CC CC CC  VV VV  06
D  D'  cmd       value  ACK
```

`'` = echo. Open question: which side sends the trailing `VV` of a write frame and the third `CC` of a
read frame — log timestamps are taken on the receiving client and are too coarse to tell. To be verified
when testing transmission.

## Poll cycle

The DWT repeats the same 12 frames every **~7.8 s**:

| # | Frame | Dir | Cmd | Meaning | Example value |
|---|-------|-----|-----|---------|---------------|
| 1 | `53 53 6E 6E 27 27 06 27` | write | `6E` | target flow temperature, °C | `27` = 39 °C |
| 2 | `53 53 72 72 01 01 06 01` | write | `72` | heating enabled (0/1) | 1 |
| 3 | `53 53 74 74 00 00 06 00` | write | `74` | hot water (DHW) enabled (0/1) | 0 |
| 4 | `44 44 6C 6C 6C 05 05 06` | read  | `6C` | outside temperature (boiler sensor), °C, signed | 5 °C |
| 5 | `53 53 71 71 05 05 06 05` | write | `71` | outside temperature as used by DWT, °C, signed | 5 °C |
| 6 | `44 44 68 68 68 1F 1F 06` | read  | `68` | flow temperature, °C | 31 °C |
| 7 | `44 44 6A 6A 6A 20 20 06` | read  | `6A` | DHW / tank temperature, °C | 32 °C |
| 8 | `44 44 6F 6F 6F 23 23 06` | read  | `6F` | DHW setpoint (set by the boiler knob), °C | 35 °C |
| 9 | `44 44 65 65 65 00 00 06` | read  | `65` | error code | 0 |
| 10 | `44 44 A0 A0 A0 00 00 06` | read | `A0` | fan speed, **revolutions per second** | 0 |
| 11 | `44 44 70 70 70 0C 0C 06` | read | `70` | status bits | `0x0C` |
| 12 | `53 53 67 67 14 14 06 14` | write | `67` | operation mode (0–20) | 20 |

### Status byte `0x70`

| Bit | Meaning |
|-----|---------|
| 0 | burner on |
| 1 | 3-way valve in DHW position |
| 2 | heating pump on |
| 3 | unknown (set most of the time) |

Observed values: `0x0C` (pump, idle), `0x0D` (pump + burner), `0x0E` (pump + DHW valve), `0x0F` (DHW charging, burner on), `0x08`.

## Error codes (GG-2EK-S / GU-2EK-S manual)

| Code | Meaning |
|------|---------|
| 1 | flow temperature exceeded STB limit (110 °C) |
| 4 | no ignition |
| 5 | flame loss during stabilisation / soft start |
| 6 | DHW temperature exceeded STW (95 °C) |
| 7 | flue gas temperature exceeded (GU-2EK-S) |
| 8 | flue damper does not switch |
| 11 | flame simulation |
| 12 | flow sensor faulty |
| 13 | flue gas sensor faulty (GU-2EK-S) |
| 14 | adapter plug fuse / cables damaged |
| 15 | outside sensor faulty |
| 16 | return sensor faulty |
| 17 | **modulation current** deviates > 15 mA from nominal for > 10 s |
| 22 | lack of combustion air (GG-2EK-S) |
| 23 | differential pressure switch faulty (GG-2EK-S) |
| 25 | flue gas fan speed out of range (GG-2EK-S) |
| 41 | return temperature 25 K above flow (flow switch); may appear briefly on DHW → heating switch-over |

## Controlling the boiler (not implemented yet)

To charge a buffer tank only two writes are needed: `72` (heating on/off) and `6E` (target flow
temperature). Emulating the DWT means taking over the whole poll cycle, so the DWT must be
disconnected — two masters on the bus would collide.

Suggested test sequence:

1. DWT disconnected, ESP sends a single read `44 … 68` and checks that the boiler answers.
2. Verify echo/ACK handling and timing.
3. Writes `72` / `6E`.

Boiler safety functions (STB, flame supervision, error lockouts) do not depend on the bus.

## How the capture was made

`uart:` debug in the ESPHome config:

```yaml
uart:
  debug:
    direction: BOTH
    after:
      timeout: 15ms
    sequence:
      - lambda: UARTDebug::log_hex(direction, bytes, ' ');
```

Timestamps in the log are from `esphome logs` on the receiving side, not from the device.
