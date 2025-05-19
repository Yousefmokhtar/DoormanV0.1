import esphome.codegen as cg
from esphome.components import output
from esphome import automation
import esphome.config_validation as cv
from typing import Optional
from esphome.const import (
    CONF_ID,
    CONF_TYPE,
    CONF_UUID,
    PLATFORM_ESP32,
    CONF_TRIGGER_ID,
)
ESP_PLATFORMS = [PLATFORM_ESP32]

AUTO_LOAD = ["binary_sensor", "output"]
CONFLICTS_WITH = ["esp32_ble_tracker", "esp32_ble_beacon","esp32_ble_server","ble_client"]

DEPENDENCIES = ['esp32']

nimble_server_ns = cg.esphome_ns.namespace('nimble_server')
NIMBLEServer = nimble_server_ns.class_('NIMBLEServer', cg.Component)

# Configuration constants
CODEOWNERS = ["@asm2me"]
CONF_SERVICES = "services"
CONF_UUID = "uuid"
CONF_UUID128 = "uuid128"
CONF_CHARACTERISTICS = "characteristics"
CONF_PERMISSIONS = "permissions"


CONF_BLE_SERVER_ID = "ble_server_id"
CONF_STATUS_INDICATOR = "status_indicator"
#CONF_SECURITY_MODE = "security_mode"
CONF_SECRET_PASSCODE = "secret_passcode"
CONF_ON_PINPAD_ACCEPTED = "on_pinpad_accepted"
CONF_ON_PINPAD_REJECTED = "on_pinpad_rejected"
CONF_ON_USER_COMMAND = "on_user_command_received"
CONF_ON_USER_SELECTED = "on_user_selected"
CONF_START_ADVERTISING = "start_advertising"
CONF_STOP_ADVERTISING = "stop_advertising"
CONF_ON_CLIENT_CONNECTED = "on_client_connected"


#SECURITY_MODE_NONE = "none"
#SECURITY_MODE_HOTP = "hotp"
#SECURITY_MODE_TOTP = "totp"

#SecurityMode = nimble_server_ns.enum("SecurityMode")
#SECURITY_MODES = {
#    SECURITY_MODE_NONE: SecurityMode.SECURITY_MODE_NONE,
#    SECURITY_MODE_HOTP: SecurityMode.SECURITY_MODE_HOTP,
#    SECURITY_MODE_TOTP: SecurityMode.SECURITY_MODE_TOTP,
#}

def validate_secret_pin(value):
    value = cv.string_strict(value)
    if not value:
        return value
    try:
        value.encode('ascii')
    except UnicodeEncodeError:
        raise cv.Invalid("pin must consist of only ascii characters")
    return value

#validate_security_mode = cv.one_of(*SECURITY_MODES.keys(), lower=True)

#Triggers
ClientConnectedTrigger = nimble_server_ns.class_("ClientConnectedTrigger", automation.Trigger.template(cg.std_string))
PinpadAcceptedTrigger = nimble_server_ns.class_("PinpadAcceptedTrigger", automation.Trigger.template())
PinpadRejectedTrigger = nimble_server_ns.class_("PinpadRejectedTrigger", automation.Trigger.template())
PinpadUserSelectedTrigger = nimble_server_ns.class_("PinpadUserSelectedTrigger", automation.Trigger.template())
PinpadUserCommandTrigger = nimble_server_ns.class_("PinpadUserCommandTrigger", automation.Trigger.template(cg.std_string, cg.std_string))

# Define actions for start_advertising and stop_advertising
StartAdvertisingAction = nimble_server_ns.class_("StartAdvertisingAction", automation.Action)
StopAdvertisingAction = nimble_server_ns.class_("StopAdvertisingAction", automation.Action)

# Permission flags
PERMISSION_READ = "read"
PERMISSION_WRITE = "write"
PERMISSION_NOTIFY = "notify"

# NimBLE permission flags
PERMISSIONS = {
    PERMISSION_READ: 0x0001,    # BLE_GATT_CHR_F_READ
    PERMISSION_WRITE: 0x0002,   # BLE_GATT_CHR_F_WRITE
    PERMISSION_NOTIFY: 0x0004,  # BLE_GATT_CHR_F_NOTIFY
}

def validate_uuid128(value):
    if not isinstance(value, list) or len(value) != 16:
        raise cv.Invalid("UUID128 must be a list of 16 bytes")
    return [cv.hex_uint8_t(x) for x in value]

def validate_characteristic(value):
    if isinstance(value, dict):
        if CONF_UUID in value and CONF_UUID128 in value:
            raise cv.Invalid("Cannot specify both uuid and uuid128")
        if CONF_UUID in value:
            return cv.Schema({
                cv.Required(CONF_UUID): cv.hex_uint16_t,
                cv.Required(CONF_PERMISSIONS): cv.ensure_list(cv.one_of(*PERMISSIONS, lower=True)),
            })(value)
        elif CONF_UUID128 in value:
            return cv.Schema({
                cv.Required(CONF_UUID128): validate_uuid128,
                cv.Required(CONF_PERMISSIONS): cv.ensure_list(cv.one_of(*PERMISSIONS, lower=True)),
            })(value)
        else:
            raise cv.Invalid("Must specify either uuid or uuid128")
    raise cv.Invalid("Expected characteristic to be dictionary")

def validate_service(value):
    if isinstance(value, dict):
        if CONF_UUID in value and CONF_UUID128 in value:
            raise cv.Invalid("Cannot specify both uuid and uuid128")
        if CONF_UUID in value:
            return cv.Schema({
                cv.Required(CONF_UUID): cv.hex_uint16_t,
                cv.Required(CONF_CHARACTERISTICS): cv.ensure_list(validate_characteristic),
            })(value)
        elif CONF_UUID128 in value:
            return cv.Schema({
                cv.Required(CONF_UUID128): validate_uuid128,
                cv.Required(CONF_CHARACTERISTICS): cv.ensure_list(validate_characteristic),
            })(value)
        else:
            raise cv.Invalid("Must specify either uuid or uuid128")
    raise cv.Invalid("Expected service to be dictionary")

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(NIMBLEServer),
    cv.Required(CONF_SERVICES): cv.ensure_list(validate_service),


#    cv.Required(CONF_SECURITY_MODE): validate_security_mode,
    cv.Required(CONF_SECRET_PASSCODE): cv.string_strict,
    cv.Optional(CONF_ON_PINPAD_ACCEPTED): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PinpadAcceptedTrigger),
        }
    ),
    cv.Optional(CONF_ON_PINPAD_REJECTED): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PinpadRejectedTrigger),
        }
    ),
    cv.Optional(CONF_ON_USER_SELECTED): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PinpadUserSelectedTrigger),
        }
    ),
    cv.Optional(CONF_ON_USER_COMMAND): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PinpadUserCommandTrigger),
        }
    ),
    cv.Optional(CONF_ON_CLIENT_CONNECTED): automation.validate_automation(
	{
	    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ClientConnectedTrigger),
	}),

    cv.Optional(CONF_STATUS_INDICATOR): cv.use_id(output.BinaryOutput),
    cv.Optional(CONF_START_ADVERTISING, default=True): cv.boolean,  # Add this line
    cv.Optional(CONF_STOP_ADVERTISING, default=False): cv.boolean,  # Add this line



}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)


#    cg.add(var.set_security_mode(
#        SECURITY_MODES[config[CONF_SECURITY_MODE]],
#        config[CONF_SECRET_PASSCODE]
#    ))
    for conf in config.get(CONF_ON_CLIENT_CONNECTED, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.std_string, "mac")], conf)

    for conf in config.get(CONF_ON_PINPAD_ACCEPTED, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.std_string, "user"), (cg.std_string, "cmd")], conf)

    for conf in config.get(CONF_ON_PINPAD_REJECTED, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.std_string, "user"), (cg.std_string, "cmd")], conf)
    
    for conf in config.get(CONF_ON_USER_SELECTED, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.std_string, "user")], conf)


    for conf in config.get(CONF_ON_USER_COMMAND, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.std_string, "mac"), (cg.std_string, "cmd")], conf)


    if CONF_STATUS_INDICATOR in config:
        status_indicator = await cg.get_variable(config[CONF_STATUS_INDICATOR])
        cg.add(var.set_status_indicator(status_indicator))




    for service in config[CONF_SERVICES]:
        if CONF_UUID in service:
            # Handle 16-bit UUID service
            characteristics = []
            for char in service[CONF_CHARACTERISTICS]:
                flags = 0
                for permission in char[CONF_PERMISSIONS]:
                    flags |= PERMISSIONS[permission]
                characteristics.append((char[CONF_UUID], flags))

            # Create vector of pairs using ArrayInitializer
            chars_array = cg.ArrayInitializer(
                *(cg.ArrayInitializer(uuid, flags) for uuid, flags in characteristics)
            )

#            cg.add(var.add_service(cg.uint16(service[CONF_UUID]), chars_array))
        else:
            # Handle 128-bit UUID service
            service_uuid_arr = cg.ArrayInitializer(
                *(cg.uint8(x) for x in service[CONF_UUID128])
            )

            chars_array = []
            for char in service[CONF_CHARACTERISTICS]:
                flags = 0
                for permission in char[CONF_PERMISSIONS]:
                    flags |= PERMISSIONS[permission]

                char_uuid_arr = cg.ArrayInitializer(
                    *(cg.uint8(x) for x in char[CONF_UUID128])
                )
                chars_array.append(cg.ArrayInitializer(char_uuid_arr, flags))

            chars_initializer = cg.ArrayInitializer(*chars_array)
#            cg.add(var.add_service_128(service_uuid_arr, chars_initializer))




