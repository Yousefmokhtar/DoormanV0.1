#include "esphome.h"
#include "nvs_flash.h"
#include "esp_log.h"

class MyNVSInitComponent : public Component {
 public:
  void setup() override {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      // Erase NVS and try again
      ESP_LOGW("nvs", "NVS init failed, erasing...");
      nvs_flash_erase();
      ret = nvs_flash_init();
    }

    if (ret == ESP_OK) {
      ESP_LOGI("nvs", "NVS initialized successfully.");
    } else {
      ESP_LOGE("nvs", "Failed to initialize NVS: %s", esp_err_to_name(ret));
    }
  }
};
