```yaml
# example configuration:

wolf_scom:
  id: wolf_scom_1

sensor:
  - platform: wolf_scom
    wolf_scom_id: wolf_scom_1
    target_flow_temperature:
      name: Target Flow Temperature
    operation_mode:
      name: Operation mode
    outside_temperature:
      name: Outside temperature
    dwt_outside_temperature:
      name: DWT Outside temperature
    flow_temperature:
      name: Flow Temperature
    water_temperature:
      name: Water Temperature
    target_water_temperature:
      name: Target Water Temperature
    error_code:
      name: Error code
    rpm:
      name: Fan RPM
    boiler_status:
      name: Boiler Status

binary_sensor:
  - platform: wolf_scom
    wolf_scom_id: wolf_scom_1
    heating:
      name: Heating
    hot_water:
      name: Hot Water
    heat:
      name: Heat
    pump:
      name: Pump
    three_way_valve:
      name: Three Way Valve
```

===== Sect 53: DWT -> Boiler =====
SCOM 6E: Target Flow Temperature: °C
SCOM 72: Heating On: 0/1
SCOM 74: Hot Water On: 0/1
SCOM 67: Operation Mode: 0-20
SCOM 71: Outside Temperature: °C
===== Sect 44: Boiler -> DWT =====
SCOM 6C: Outside Temperature: °C
SCOM 68: Flow Temperature: °C
SCOM 6A: Water Temperature: °C
SCOM 6F: Target Water Temperature: °C
SCOM 65: Error Code: 0
SCOM A0: RPM: rps
SCOM 70: Boiler Status: 8 dez
SCOM 70:b0: Burner On: 0
SCOM 70:b1: 3-Way Valve: Hot Water: 0
SCOM 70:b2: Pump: 0