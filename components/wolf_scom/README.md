```yaml
# example configuration:

wolf_scom:
  id: wolf_scom_1

sensor:
  - platform: wolf_scom
    wolf_scom_id: wolf_scom_1
      - targetFlowTemperature:
        name: Target Flow Temperature
      - operationMode:
        name: Operation Mode
      - outsideTemperatureDwt:
        name: Outside Temperature Dwt
      - outsideTemperature:
        name: Outside Temperature
      - flowTemperature:
        name: Flow Temperature
      - waterTemperature:
        name: Water Temperature
      - targetWaterTemperature:
        name: Target Water Temperature
      - errorCode:
        name: Error Code
      - rpm:
        name: RPM
      - boilerStatus:
        name: Boiler Status
binary_sensor:
  - platform: wolf_scom
    wolf_scom_id: wolf_scom_1
      - heating:
        name: Heating
      - hotWater:
        name: Hot Water
      - burner:
        name: Burner
      - threeWayValve:
        name: 3-Way Valve
      - pump:
        name: Pump
```

===== Sect 53: DWT -> Boiler =====
SCOM 6E: Target Flow Temperature: 5 °C
SCOM 72: Heating On: 0/1
SCOM 74: Hot Water On: 0/1
SCOM 67: Operation Mode: 0-20
SCOM 71: Outside Temperature: 7 °C
===== Sect 44: Boiler -> DWT =====
SCOM 6C: Outside Temperature: 7 °C
SCOM 68: Flow Temperature: 33 °C
SCOM 6A: Water Temperature: 49 °C
SCOM 6F: Target Water Temperature: 55 °C
SCOM 65: Error Code: 0
SCOM A0: RPM: 0 rps
SCOM 70: Boiler Status: 8 dez
SCOM 70:b0: Burner On: 0
SCOM 70:b1: 3-Way Valve: Hot Water: 0
SCOM 70:b2: Pump: 0