import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = []  # No need for "nvs_flash" as a dependency here
CODEOWNERS = ["@your_github_username"]
MULTI_CONF = True

my_nvs_ns = cg.esphome_ns.namespace("my_nvs")
MyNVSComponent = my_nvs_ns.class_("MyNVSComponent", cg.Component)

# ✅ Correct schema using cv.Schema (NOT cg.Schema)
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.declare_id(MyNVSComponent),
}).extend(cv.COMPONENT_SCHEMA)  # Removed the nvs_flash extension

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
