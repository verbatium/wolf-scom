#include "esphome/core/log.h"
#include "wolf_scom.h"

namespace esphome {
namespace wolf_scom {

static const char *TAG = "wolf_scom.component";

void WolfScom::setup() {
    ESP_LOGCONFIG(TAG, "Setting up WolfScom sensors...");
}

void WolfScom::loop() {
    // Считываем данные из UART, если они доступны
    while (available()) {
      uint8_t byte = read();
      buffer_.push_back(byte);

      // Проверяем, достаточно ли данных в буфере для анализа
      if (buffer_.size() >= 8) {
        // Пытаемся найти и обработать пакет в буфере
        if (process_packet()) {
          // Если пакет успешно обработан, удаляем его из буфера
          buffer_.erase(buffer_.begin(), buffer_.begin() + 8);
        } else {
          // Если пакет не валиден, удаляем первый байт и продолжаем
          buffer_.erase(buffer_.begin());
        }
      }
    }
}

bool WolfScom::process_packet() {
    // Проверяем, что у нас есть хотя бы 8 байт
    if (buffer_.size() < 8) {
      return false;
    }

    uint8_t *buffer = buffer_.data();
    // Проверка пакета от контроллера к устройству (0x53 0x53)
    if (buffer[0] == 0x53 && buffer[1] == 0x53) {
      uint8_t command = buffer[2];
      uint8_t value = buffer[4];

      // Проверка соответствия данных
      if (command == buffer[3] && value == buffer[5] && buffer[6] == 0x06 && value == buffer[7]) {
        switch (command) {
          case 0x6E:
            if (this->target_flow_temperature_sensor != nullptr) this->target_flow_temperature_sensor->publish_state(value);
            break;
          case 0x72:
            if (this->heating_day_sensor != nullptr) this->heating_day_sensor->publish_state(value);
            break;
          case 0x74:
            if (this->hot_water_day_sensor != nullptr) this->hot_water_day_sensor->publish_state(value);
            break;
          case 0x67:
            if (this->operation_mode_sensor != nullptr) this->operation_mode_sensor->publish_state(value);
            break;
          case 0x71:
            if (this->dwt_outside_temperature_sensor != nullptr) this->dwt_outside_temperature_sensor->publish_state(value);
            break;
          default:
            ESP_LOGW("uart_protocol_sensor", "Unknown command: 0x%02X", command);
            break;
        }
        return true; // Пакет успешно обработан
      }
    }
    // Проверка пакета от устройства к контроллеру (0x44 0x44)
    else if (buffer[0] == 0x44 && buffer[1] == 0x44) {
      uint8_t command = buffer[2];
      uint8_t value = buffer[5];

      // Проверка соответствия данных
      if (command == buffer[3] && command == buffer[4] && buffer[7] == 0x06) {
        switch (command) {
          case 0x6C:
            if (this->outside_temperature_sensor != nullptr) this->outside_temperature_sensor->publish_state(value);
            break;
          case 0x68:
            if (this->flow_temperature_sensor != nullptr) this->flow_temperature_sensor->publish_state(value);
            break;
          case 0x6A:
            if (this->water_temperature_sensor != nullptr) this->water_temperature_sensor->publish_state(value);
            break;
          case 0x6F:
            if (this->target_water_temperature_sensor != nullptr) this->target_water_temperature_sensor->publish_state(value);
            break;
          case 0x65:
            if (this->error_code_sensor != nullptr) this->error_code_sensor->publish_state(value);
            break;
          case 0xA0:
            if (this->rpm_sensor != nullptr) this->rpm_sensor->publish_state(value);
            break;
          case 0x70:
            if (this->boiler_status_sensor != nullptr) this->boiler_status_sensor->publish_state(value);
            if (this->pump_sensor != nullptr) this->pump_sensor->publish_state(value && 1);
            break;
          default:
            ESP_LOGW("uart_protocol_sensor", "Unknown command: 0x%02X", command);
            break;
        }
        return true; // Пакет успешно обработан
      }
    }

    return false; // Пакет не валиден или не полный
  }

void WolfScom::dump_config(){
    ESP_LOGCONFIG(TAG, "Wolf SCOM component");
}

}  // namespace empty_uart_component
}  // namespace esphome