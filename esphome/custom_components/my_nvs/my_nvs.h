#pragma once

#include "esphome.h"
#include "nvs_flash.h"

namespace esphome {
namespace my_nvs {

class MyNVSComponent : public Component {
 public:
  void setup() override {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      nvs_flash_erase();
      ret = nvs_flash_init();
    }
    if (ret == ESP_OK) {
      ESP_LOGI("my_nvs", "NVS initialized successfully.");
    } else {
      ESP_LOGE("my_nvs", "NVS init failed: %s", esp_err_to_name(ret));
    }
  }

  void loop() override {
    // Optional: logic that needs to run every loop
  }
};

}  // namespace my_nvs
}  // namespace esphome
