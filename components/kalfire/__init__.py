import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID
from esphome.core import coroutine_with_priority

AUTO_LOAD = ['switch', 'output']

kalfire_ns = cg.esphome_ns.namespace("kalfire")
Kalfire = kalfire_ns.class_("Kalfire", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Kalfire),
    }
).extend(cv.COMPONENT_SCHEMA)

@coroutine_with_priority(80.0)
async def to_code(config):
    rhs = Kalfire.new()
    var = cg.Pvariable(config[CONF_ID], rhs)
    await cg.register_component(var, config)
    cg.add_define("USE_KALFIRE")