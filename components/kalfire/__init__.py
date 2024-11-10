import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID, CONF_OUTPUT, CONF_NUMBER, CONF_SWITCHES, ENTITY_CATEGORY_CONFIG
from esphome.core import coroutine_with_priority
from esphome.components import number

AUTO_LOAD = ['switch', 'output', 'number']

kalfire_ns = cg.esphome_ns.namespace("kalfire")
Kalfire = kalfire_ns.class_("Kalfire", cg.Component)

CONF_KALFIRE_ID = 'kalfire_id'

CONF_KALFIRE_OUTPUT = 'output'

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
    cg.add(var.add_output(output))

    cg.add_define("USE_KALFIRE")