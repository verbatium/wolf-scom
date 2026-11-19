#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include <optional>

#include "scom_frame.h"
#include "scom_master.h"
#include "scom_publish.h"

namespace esphome {
    namespace wolf_scom {
        struct UartPort : scom::BytePort {
          explicit UartPort(uart::UARTDevice &device) : device_(device) {}
          void write(uint8_t byte) override { device_.write_byte(byte); }

         private:
          uart::UARTDevice &device_;
        };

        class WolfScom : public uart::UARTDevice, public Component, public scom::Listener {
         public:
          void set_target_flow_temperature_sensor(sensor::Sensor *sensor) { target_flow_temperature_sensor = sensor; }
          void set_heating_day_sensor(binary_sensor::BinarySensor *sensor) { heating_day_sensor = sensor; }
          void set_hot_water_day_sensor(binary_sensor::BinarySensor *sensor) { hot_water_day_sensor = sensor; }
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

          void set_master(bool master) { master_ = master; }

          void setup() override;
          void loop() override;
          void dump_config() override;

          void on_frame(const scom::Frame &frame) override;
          void on_failure(scom::FailReason reason, uint8_t cmd) override;
          void on_trace(const std::vector<scom::TraceEntry> &trace) override;
          void on_refused() override;
         protected:
          bool master_ = false;
          bool refused_ = false;
          std::optional<UartPort> uart_port_;
          std::optional<scom::Master> master_core_;
          scom::StreamParser parser_;
          scom::Publisher publisher_;

          void feed_passive(uint8_t byte);
          void forward(const scom::Publication &publication);

          sensor::Sensor *target_flow_temperature_sensor = nullptr;
          binary_sensor::BinarySensor *heating_day_sensor = nullptr;
          binary_sensor::BinarySensor *hot_water_day_sensor = nullptr;
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