#pragma once
#include "esphome.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "esp_system.h"

#include "esp_system.h"
#include "esp_partition.h"
#include "esp_log.h"


#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_flash.h"
#include <ctype.h>  // for isspace

nvs_iterator_t it = NULL;

static const char *TAG = "NVS_UID_STORE";

void init_nvs() {
  esp_err_t err = nvs_flash_init();

  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      // NVS partition was truncated or incompatible version found
      ESP_LOGW("NVS_INIT", "Erasing NVS and retrying...");
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
  }

  ESP_ERROR_CHECK(err);  // If still error, halt with detailed message
  ESP_LOGI("NVS_INIT", "NVS initialized successfully");
}


bool check_card_exists(nvs_handle_t my_handle, const char *card_data) {
  uint32_t index = 0;
  esp_err_t err = nvs_get_u32(my_handle, "index", &index);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
      index = 0;  // No cards stored yet
  }

  char key[16];
  char stored_card_data[128];  // Assuming card data is less than 128 characters
  size_t length;

  // Loop through all stored keys and compare each one with card_data
  for (uint32_t i = 0; i < index; i++) {
      snprintf(key, sizeof(key), "uid_%lu", (unsigned long)i);
      length = sizeof(stored_card_data);

      // Try to get the stored card data
      err = nvs_get_str(my_handle, key, stored_card_data, &length);
      if (err == ESP_OK) {
          // Compare the stored card data with the input card data
          if (strcmp(stored_card_data, card_data) == 0) {
              return true;  // Card already exists
          }
      }
  }
  return false;  // Card not found
}


// Helper function to trim leading and trailing whitespace
char *trim_whitespace(char *str) {
  char *end;

  // Trim leading space
  while (isspace((unsigned char)*str)) str++;

  if (*str == 0)  // All spaces?
      return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end + 1) = '\0';

  return str;
}




void store_card_data(const char *card_data) {
  nvs_handle_t my_handle;
  esp_err_t err;

  // Copy and normalize card_data by trimming whitespace
  char trimmed_data[64];  // Adjust size as needed
  strncpy(trimmed_data, card_data, sizeof(trimmed_data) - 1);
  trimmed_data[sizeof(trimmed_data) - 1] = '\0';
  
  // Trim leading/trailing whitespace
  char *normalized = trim_whitespace(trimmed_data);

  // Find the first whitespace (if any), and terminate the string before it
  char *space_pos = strchr(normalized, ' ');
  if (space_pos != NULL) {
      *space_pos = '\0';  // Null-terminate the string before the first space
  }

  // Open NVS in read-write mode
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
      ESP_LOGE("NVS", "Error opening NVS: %s", esp_err_to_name(err));
      return;
  }

  // Check if the card data already exists
  if (check_card_exists(my_handle, normalized)) {
      ESP_LOGI("NVS", "Card data %s already exists, not storing.", normalized);
      nvs_close(my_handle);
      return;  // Exit without storing if the card data already exists
  }

  // Get current index
  uint32_t index = 0;
  err = nvs_get_u32(my_handle, "index", &index);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
      index = 0;  // No entries found, starting from 0
  }
  

  // Prepare key name
  char key[16];
  snprintf(key, sizeof(key), "uid_%lu", (unsigned long)index);

  // Store the card data
  err = nvs_set_str(my_handle, key, normalized);
  if (err != ESP_OK) {
      ESP_LOGE("NVS", "Failed to set card data: %s", esp_err_to_name(err));
      nvs_close(my_handle);
      return;
  }

  // Increment and save index
  index++;
  nvs_set_u32(my_handle, "index", index);

  // Commit the changes
  err = nvs_commit(my_handle);
  if (err != ESP_OK) {
      ESP_LOGE("NVS", "Commit failed: %s", esp_err_to_name(err));
  } else {
      ESP_LOGI("NVS", "Stored %s as %s", normalized, key);
  }

  nvs_close(my_handle);
}





void load_all_uids(void) {
  // Check if the input value is "1234"
  nvs_handle_t my_handle;
  esp_err_t err;

  err = nvs_open("storage", NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
      ESP_LOGE("NVS", "Failed to open NVS: %s", esp_err_to_name(err));
      return;
  }


  // Read index to determine how many items to try loading
  uint32_t index = 0;
  err = nvs_get_u32(my_handle, "index", &index);
  if (err != ESP_OK) {
      ESP_LOGW("NVS", "No UID entries found.");
      nvs_close(my_handle);
      return;
  }

  char key[16];
  char value[128];  // assuming each UID string won't exceed 128 bytes
  size_t length;
  ESP_LOGI("NVS", "Number of stored cards: %lu", (unsigned long)index);

  for (uint32_t i = 0; i < index; i++) {
      snprintf(key, sizeof(key), "uid_%lu", (unsigned long)i);
      length = sizeof(value);
      err = nvs_get_str(my_handle, key, value, &length);
      if (err == ESP_OK) {
          ESP_LOGI("UIDS", "Key: %s, Value: %s", key, value);
      } else {
          ESP_LOGW("UIDS", "Key: %s not found or error: %s", key, esp_err_to_name(err));
      }
  }

  nvs_close(my_handle);
}




void eraser() {
    // Erase NVS storage
    esp_err_t err = nvs_flash_erase();
    if (err == ESP_OK) {
        ESP_LOGI("Factory Reset", "NVS erased successfully.");
    } else {
        ESP_LOGE("Factory Reset", "Failed to erase NVS.");
    }

    // Restart the device to apply changes
}



bool compare_uuid_in_nvs(const char *x) {
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open NVS in read-only mode
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error opening NVS: %s", esp_err_to_name(err));
        return false;  // Return false if unable to open NVS
    }

    // Read the current index from NVS
    uint32_t index = 0;
    err = nvs_get_u32(my_handle, "index", &index);
    if (err != ESP_OK) {
        ESP_LOGW("NVS", "No UID entries found.");
        nvs_close(my_handle);
        return false;  // Return false if no entries found
    }

    char key[16];
    char stored_uuid[128];  // Assuming each UUID string won't exceed 128 bytes
    size_t length;

    // Loop through all stored keys (UUIDs)
    for (uint32_t i = 0; i < index; i++) {
        snprintf(key, sizeof(key), "uid_%lu", (unsigned long)i);
        length = sizeof(stored_uuid);
        
        // Retrieve the stored UUID from NVS
        err = nvs_get_str(my_handle, key, stored_uuid, &length);
        if (err == ESP_OK) {
            // Compare the stored UUID with the input UUID (x)
            if (strcmp(stored_uuid, x) == 0) {
                ESP_LOGI("NVS", "Match found Y Y Y Y Y Card Accepted: %s with key: %s", stored_uuid, key);
                nvs_close(my_handle);
                return true;  // UUID found in NVS
            }
        } else {
            ESP_LOGW("NVS", "Error reading key %s: %s", key, esp_err_to_name(err));
        }
    }

    nvs_close(my_handle);
    ESP_LOGI("NVS", "Unmatch X X X X X Card Not Accepted ");

    return false;  // UUID not found after comparing all entries
}








void delete_uid(const char *card_data) {
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
      return;
  }

  uint32_t index = 0;
  err = nvs_get_u32(my_handle, "index", &index);
  if (err != ESP_OK || index == 0) {
      ESP_LOGW(TAG, "No UID entries found.");
      nvs_close(my_handle);
      return;
  }

  char key[16], next_key[16];
  char stored_card[128];
  size_t length;
  bool found = false;
  uint32_t match_index = 0;

  for (uint32_t i = 0; i < index; i++) {
      snprintf(key, sizeof(key), "uid_%lu", (unsigned long)i);
      length = sizeof(stored_card);
      err = nvs_get_str(my_handle, key, stored_card, &length);
      if (err == ESP_OK && strcmp(stored_card, card_data) == 0) {
          found = true;
          match_index = i;
          break;
      }
  }

  if (!found) {
      ESP_LOGI(TAG, "UID not found: %s", card_data);
      nvs_close(my_handle);
      return;
  }

  // Shift all entries after the matched index down
  for (uint32_t i = match_index; i < index - 1; i++) {
      snprintf(key, sizeof(key), "uid_%lu", (unsigned long)i);
      snprintf(next_key, sizeof(next_key), "uid_%lu", (unsigned long)(i + 1));
      length = sizeof(stored_card);
      err = nvs_get_str(my_handle, next_key, stored_card, &length);
      if (err == ESP_OK) {
          nvs_set_str(my_handle, key, stored_card);
      }
  }

  // Erase the last item (now duplicated)
  snprintf(key, sizeof(key), "uid_%lu", (unsigned long)(index - 1));
  nvs_erase_key(my_handle, key);

  // Update index
  index--;
  nvs_set_u32(my_handle, "index", index);

  // Commit changes
  err = nvs_commit(my_handle);
  if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to commit delete: %s", esp_err_to_name(err));
  } else {
      ESP_LOGI(TAG, "Deleted UID: %s", card_data);
  }
  
  const esp_partition_t *boot_partition = esp_ota_get_boot_partition();
  ESP_LOGI("partition partition", "Boot partition label: %s", boot_partition->label);


  nvs_close(my_handle);
}





void delete_all_uids() {
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
      return;
  }

  uint32_t index = 0;
  err = nvs_get_u32(my_handle, "index", &index);
  if (err != ESP_OK) {
      ESP_LOGW(TAG, "No index found, nothing to delete.");
      index = 0;
  }

  char key[16];
  for (uint32_t i = 0; i < index; i++) {
      snprintf(key, sizeof(key), "uid_%lu", (unsigned long)i);
      err = nvs_erase_key(my_handle, key);
      if (err == ESP_OK) {
          ESP_LOGI(TAG, "Deleted key: %s", key);
      } else {
          ESP_LOGW(TAG, "Failed to delete key: %s - %s", key, esp_err_to_name(err));
      }
  }

  // Reset index to 0
  nvs_set_u32(my_handle, "index", 0);

  // Commit the changes
  err = nvs_commit(my_handle);
  if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to commit after delete: %s", esp_err_to_name(err));
  }

  nvs_close(my_handle);
}






void estimate_nvs_card_capacity() {
    nvs_stats_t nvs_stats;
    esp_err_t err = nvs_get_stats(NULL, &nvs_stats);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to get NVS stats: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI("NVS", "NVS Statistics:");
    ESP_LOGI("NVS", "  Used entries: %d", nvs_stats.used_entries);
    ESP_LOGI("NVS", "  Free entries: %d", nvs_stats.free_entries);
    ESP_LOGI("NVS", "  Total entries: %d", nvs_stats.total_entries);
    ESP_LOGI("NVS", "  Namespace count: %d", nvs_stats.namespace_count);
    // Log full stats
    // Estimate how many more cards can be saved
    // Each card uses 2 entries: 1 for the UID and 1 for the index
    int estimated_cards = nvs_stats.free_entries / 2;

    if (estimated_cards > 0) {
        // Reserve 1 entry for the index if it's the first time
        estimated_cards -= 1;
    }

    ESP_LOGI("NVS", "Estimated additional cards that can be stored: %d", estimated_cards);
}







void print_flash_info() {
    const char *TAG = "FLASH_INFO";

    // Get flash size using esp_flash_get_size
    uint32_t flash_size = 0;
    esp_err_t err = esp_flash_get_size(NULL, &flash_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get flash size: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Flash size: %u bytes (%.2f MB)", flash_size, flash_size / (1024.0 * 1024));

    // Print app partitions
    ESP_LOGI(TAG, "App Partitions:");
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    while (it != NULL) {
        const esp_partition_t *part = esp_partition_get(it);
        ESP_LOGI(TAG, "Label: %s, Type: 0x%x, Subtype: 0x%x, Offset: 0x%x, Size: 0x%x",
                 part->label, part->type, part->subtype, part->address, part->size);
        it = esp_partition_next(it);
    }

    // Print data partitions
    it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
    ESP_LOGI(TAG, "Data Partitions:");
    while (it != NULL) {
        const esp_partition_t *part = esp_partition_get(it);
        ESP_LOGI(TAG, "Label: %s, Type: 0x%x, Subtype: 0x%x, Offset: 0x%x, Size: 0x%x",
                 part->label, part->type, part->subtype, part->address, part->size);
        it = esp_partition_next(it);
    }

    // Always release iterators
    esp_partition_iterator_release(it);
}








