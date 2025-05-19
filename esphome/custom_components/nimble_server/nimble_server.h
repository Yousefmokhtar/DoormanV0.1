#pragma once

#include "esphome/core/component.h"
//#include "esphome/core/log.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/output/binary_output.h"

#include <vector>
#include "host/ble_gap.h"

namespace esphome {
namespace nimble_server {

#define MYNEWT_VAL_BLE_STORE_MAX_CSFCS (8)
static const char *const TAG = "nimble_server";
static void bleprph_advertise(void);
static void bleprph_on_reset(int);
static void ble_app_set_addr(void);
static void bleprph_print_conn_desc(struct ble_gap_conn_desc *desc);
void bleprph_host_task(void *param);
enum State : uint8_t {
  STATE_STOPPED = 0x00,
  STATE_IDLE = 0x01,
  STATE_PIN_ACCEPTED = 0x02,
  STATE_PIN_REJECTED = 0x03
};

class NIMBLEServer : public Component {
public:
    NIMBLEServer() = default;
    virtual ~NIMBLEServer() = default;
    void setup() override;
    void loop() override;
    void ble_write(uint16_t conn_handle, uint16_t attr_handle, const uint8_t *data, size_t len);
    static NIMBLEServer* instance_;
    std::vector<text_sensor::TextSensor*> text_sensors_;
    // Add text sensor registration
    void register_text_sensor(text_sensor::TextSensor *text_sensor) {
        text_sensors_.push_back(text_sensor);
    }
    // Add callback functions
//  void set_on_read(std::function<void(std::string, size_t)> callback) { read_callback_ = callback; }
    void set_on_receive(std::function<void(std::string, size_t)> callback) { receive_callback_ = callback; }

    // Callback members
    std::function<void(std::string, size_t)> receive_callback_;

    void add_on_client_connected_callback(std::function<void(std::string)> callback) {
        client_connected_callback_.add(std::move(callback));
    }
    void add_on_state_callback(std::function<void()> &&f)
    {
        this->state_callback_.add(std::move(f));
    }
    void add_on_user_selected_callback(std::function<void(std::string)> callback) {
        this->user_selected_callback_.add(std::move(callback));
    }


    void add_on_user_command_callback(std::function<void(std::string, std::string)> callback) {
        user_command_callback_.add(std::move(callback));
    }

    void start_advertising();
    void stop_advertising();

  // Setup code.

    bool is_active() const { return this->state_ != STATE_STOPPED; }
    bool is_accepted() const { return this->state_ == STATE_PIN_ACCEPTED; }
    bool is_rejected() const { return this->state_ == STATE_PIN_REJECTED; }
    std::string get_userid() const { return this->user_id_; };
    std::string get_cmd() const { return this->cmd_id_; };

    void set_status_indicator(output::BinaryOutput *status_indicator) { this->status_indicator_ = status_indicator; }
    void set_user_commands(const std::string commands);
    std::string user_id_;
    std::string cmd_id_;

    CallbackManager<void()> state_callback_{};
    CallbackManager<void(std::string)> user_selected_callback_;
    CallbackManager<void(std::string)> client_connected_callback_;
    CallbackManager<void(std::string, std::string)> user_command_callback_;  // Change to two parameters

protected:
    bool should_start_{false};
    bool setup_complete_{false};
    std::string secret_passcode_;
    std::vector<uint8_t> incoming_data_;

    State state_{STATE_STOPPED};
    output::BinaryOutput *status_indicator_{nullptr};

private:
    void start_advertising_();


};

}  // namespace ble_custom
}  // namespace esphome
