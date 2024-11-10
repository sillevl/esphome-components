import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID, CONF_OUTPUT
from esphome.core import coroutine_with_priority

AUTO_LOAD = ['switch', 'output']

CONF_KALFIRE_OUTPUT = 'output'

kalfire_ns = cg.esphome_ns.namespace("kalfire")
Kalfire = kalfire_ns.class_("Kalfire", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Kalfire),
        cv.Required(CONF_KALFIRE_OUTPUT): cv.use_id(CONF_OUTPUT),
    }
).extend(cv.COMPONENT_SCHEMA)

@coroutine_with_priority(80.0)
async def to_code(config):
    rhs = Kalfire.new()
    var = cg.Pvariable(config[CONF_ID], rhs)
    await cg.register_component(var, config)

    output = await cg.get_variable(config[CONF_KALFIRE_OUTPUT])
    cg.add(var.set_output(output))
    cg.add_define("USE_KALFIRE")