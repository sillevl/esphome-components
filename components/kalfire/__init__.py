import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID, CONF_OUTPUT, CONF_NUMBER, CONF_SWITCHES
from esphome.core import coroutine_with_priority

AUTO_LOAD = ['switch', 'output', 'number']

CONF_KALFIRE_OUTPUT = 'output'
CONF_FLAME_HEIGHT_NUMBER = 'flame_height'
CONF_ENABLE_FLAME_SWITCH = 'enable_flame'
CONF_ENABLE_ECO_SWITCH = 'eco_mode'

kalfire_ns = cg.esphome_ns.namespace("kalfire")
Kalfire = kalfire_ns.class_("Kalfire", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Kalfire),
        cv.Required(CONF_KALFIRE_OUTPUT): cv.use_id(CONF_OUTPUT),
        cv.Required(CONF_FLAME_HEIGHT_NUMBER): cv.use_id(CONF_NUMBER),
        cv.Required(CONF_ENABLE_FLAME_SWITCH): cv.use_id(CONF_SWITCHES),
        cv.Required(CONF_ENABLE_ECO_SWITCH): cv.use_id(CONF_SWITCHES),
    }
).extend(cv.COMPONENT_SCHEMA)

@coroutine_with_priority(80.0)
async def to_code(config):
    rhs = Kalfire.new()
    var = cg.Pvariable(config[CONF_ID], rhs)
    await cg.register_component(var, config)

    output = await cg.get_variable(config[CONF_KALFIRE_OUTPUT])
    cg.add(var.add_output(output))
    flame_number = await cg.get_variable(config[CONF_FLAME_HEIGHT_NUMBER])
    cg.add(var.add_flame_height_number(flame_number))
    enable_flame = await cg.get_variable(config[CONF_ENABLE_FLAME_SWITCH])
    cg.add(var.add_enable_flame_switch(enable_flame))
    enable_eco = await cg.get_variable(config[CONF_ENABLE_ECO_SWITCH])
    cg.add(var.add_eco_mode_switch(enable_eco))
    cg.add_define("USE_KALFIRE")