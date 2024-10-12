#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
namespace esphome {
    namespace wolf_scom {
        class WolfScom : public uart::UARTDevice, public Component  {
         public:
          WolfScom(UARTComponent *parent) : UARTDevice(parent) {}

          void set_target_flow_temperature_sensor(sensor::Sensor *sensor) { target_flow_temperature_sensor = sensor; }
          void set_heating_day_sensor(sensor::Sensor *sensor) { heating_day_sensor = sensor; }
          void set_hot_water_day_sensor(sensor::Sensor *sensor) { hot_water_day_sensor = sensor; }
          void set_operation_mode_sensor(sensor::Sensor *sensor) { operation_mode_sensor = sensor; }
          void set_outside_temperature_sensor(sensor::Sensor *sensor) { outside_temperature_sensor = sensor; }
          void set_dwt_outside_temperature_sensor(sensor::Sensor *sensor) { dwt_outside_temperature_sensor = sensor; }
          void set_flow_temperature_sensor(sensor::Sensor *sensor) { flow_temperature_sensor = sensor; }
          void set_water_temperature_sensor(sensor::Sensor *sensor) { water_temperature_sensor = sensor; }
          void set_target_water_temperature_sensor(sensor::Sensor *sensor) { target_water_temperature_sensor = sensor; }
          void set_error_code_sensor(sensor::Sensor *sensor) { error_code_sensor = sensor; }
          void set_rpm_sensor(sensor::Sensor *sensor) { rpm_sensor = sensor; }
          void set_boiler_status_sensor(sensor::Sensor *sensor) { boiler_status_sensor = sensor; }
          void set_pump_sensor(binary_sensor::BinarySensor *sensor) { pump_sensor = sensor; }
          void set_heat_sensor(binary_sensor::BinarySensor *sensor) { heat_sensor = sensor; }
          void set_three_way_valve_sensor(binary_sensor::BinarySensor *sensor) { three_way_valve_sensor = sensor; }

          // Устанавливаем тип и единицы измерения для сенсоров температуры
          //target_flow_temperature_sensor->set_unit_of_measurement("°C");
          //target_flow_temperature_sensor->set_device_class("temperature");

          void setup() override;
          void loop() override;
          void dump_config() override;

         protected:
          std::vector<uint8_t> buffer_;

          bool process_packet();

          sensor::Sensor *target_flow_temperature_sensor = nullptr;
          sensor::Sensor *heating_day_sensor = nullptr;
          sensor::Sensor *hot_water_day_sensor = nullptr;
          sensor::Sensor *operation_mode_sensor = nullptr;
          sensor::Sensor *outside_temperature_sensor = nullptr;
          sensor::Sensor *dwt_outside_temperature_sensor = nullptr;
          sensor::Sensor *flow_temperature_sensor = nullptr;
          sensor::Sensor *water_temperature_sensor = nullptr;
          sensor::Sensor *target_water_temperature_sensor = nullptr;
          sensor::Sensor *error_code_sensor = nullptr;
          sensor::Sensor *rpm_sensor = nullptr;
          sensor::Sensor *boiler_status_sensor = nullptr;
          binary_sensor::BinarySensor *pump_sensor = nullptr;
          binary_sensor::BinarySensor *heat_sensor = nullptr;
          binary_sensor::BinarySensor *three_way_valve_sensor = nullptr;
        };
    }
}