#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "nimble_server.h"

namespace esphome {
namespace nimble_server {

class PinpadAcceptedTrigger : public Trigger<std::string, std::string> {
 public:
  PinpadAcceptedTrigger(NIMBLEServer *pinpad) {
    pinpad->add_on_state_callback([this, pinpad]() {
      if (pinpad->is_accepted()) {
        this->trigger(pinpad->get_userid(), pinpad->get_cmd());
      }
    });
  }
};

class PinpadRejectedTrigger : public Trigger<std::string, std::string> {
public:
    PinpadRejectedTrigger(NIMBLEServer *pinpad) {
        pinpad->add_on_state_callback([this, pinpad]() {
        if (pinpad->is_rejected()) {
            this->trigger(pinpad->get_userid(), pinpad->get_cmd());
        }
    });
  }
};


class PinpadUserCommandTrigger : public Trigger<std::string, std::string> {
public:
    PinpadUserCommandTrigger(NIMBLEServer *pinpad) {
        pinpad->add_on_user_command_callback([this](const std::string &mac, const std::string &cmd) {
            this->trigger(mac, cmd);  // Trigger with both parameters
        });
    }
};


class PinpadUserSelectedTrigger : public Trigger<std::string> {
public:
  PinpadUserSelectedTrigger(NIMBLEServer *pinpad) {
    pinpad->add_on_user_selected_callback([this](const std::string &user_id) {
        this->trigger(user_id);
    });
  }
};

class ClientConnectedTrigger : public Trigger<std::string> {
public:
    ClientConnectedTrigger(NIMBLEServer *server) {
        server->add_on_client_connected_callback([this](const std::string &mac) {
            this->trigger(mac);
        });
    }
};


}  // namespace esp32_ble_pinpad
}  // namespace esphome
