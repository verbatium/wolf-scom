#include "esphome/core/log.h"
#include "wolf_scom.h"

namespace esphome {
namespace wolf_scom {

static const char *TAG = "wolf_scom.component";

namespace {
void publish_to(sensor::Sensor *target, float value) {
  if (target != nullptr) target->publish_state(value);
}

void publish_to(binary_sensor::BinarySensor *target, float value) {
  if (target != nullptr) target->publish_state(value != 0.0f);
}
}  // namespace

void WolfScom::setup() {
    ESP_LOGCONFIG(TAG, "Setting up WolfScom sensors...");
    if (master_) {
      uart_port_.emplace(*this);
      master_core_.emplace(*uart_port_, *this, scom::Master::Config{/*loopback=*/true});
    }
}

void WolfScom::on_frame(const scom::Frame &) {}
void WolfScom::on_failure(scom::FailReason, uint8_t) {}
void WolfScom::on_trace(const std::vector<scom::TraceEntry> &) {}
void WolfScom::on_refused() {}

void WolfScom::loop() {
    while (available()) {
      uint8_t byte = read();
      if (master_core_) {
        master_core_->on_rx(byte, millis());
      } else {
        feed_passive(byte);
      }
    }
    if (master_core_) master_core_->tick(millis());
}

void WolfScom::feed_passive(uint8_t byte) {
    auto frame = parser_.push(byte);
    if (!frame) return;
    if (!scom::Publisher::is_known_command(*frame)) {
      ESP_LOGW("uart_protocol_sensor", "Unknown command: 0x%02X", frame->cmd);
    }
    for (const scom::Publication &publication : publisher_.on_frame(*frame)) {
      forward(publication);
    }
}

void WolfScom::forward(const scom::Publication &publication) {
    const float value = publication.value;
    switch (publication.sensor) {
      case scom::Sensor::FlowTemperature: publish_to(flow_temperature_sensor, value); break;
      case scom::Sensor::OutsideTemperature: publish_to(outside_temperature_sensor, value); break;
      case scom::Sensor::WaterTemperature: publish_to(water_temperature_sensor, value); break;
      case scom::Sensor::TargetWaterTemperature: publish_to(target_water_temperature_sensor, value); break;
      case scom::Sensor::ErrorCode: publish_to(error_code_sensor, value); break;
      case scom::Sensor::Rpm: publish_to(rpm_sensor, value); break;
      case scom::Sensor::TargetFlowTemperature: publish_to(target_flow_temperature_sensor, value); break;
      case scom::Sensor::HeatingDay: publish_to(heating_day_sensor, value); break;
      case scom::Sensor::HotWaterDay: publish_to(hot_water_day_sensor, value); break;
      case scom::Sensor::OperationMode: publish_to(operation_mode_sensor, value); break;
      case scom::Sensor::DwtOutsideTemperature: publish_to(dwt_outside_temperature_sensor, value); break;
      case scom::Sensor::BoilerStatus: publish_to(boiler_status_sensor, value); break;
      case scom::Sensor::Pump: publish_to(pump_sensor, value); break;
      case scom::Sensor::Heat: publish_to(heat_sensor, value); break;
      case scom::Sensor::ThreeWayValve: publish_to(three_way_valve_sensor, value); break;
      case scom::Sensor::Count: break;
    }
}

void WolfScom::dump_config(){
    ESP_LOGCONFIG(TAG, "Wolf SCOM component");
}

}  // namespace wolf_scom
}  // namespace esphome
