from esphome import pins
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID, CONF_PIN
from esphome.core import coroutine_with_priority

kalfire_ns = cg.esphome_ns.namespace("kalfire")
Kalfire = kalfire_ns.class_("Kalfire", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Kalfire),
        # cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
    }
).extend(cv.COMPONENT_SCHEMA)


@coroutine_with_priority(80.0)
async def to_code(config):
    # pin = await cg.gpio_pin_expression(config[CONF_PIN])
    rhs = Kalfire.new()
    var = cg.Pvariable(config[CONF_ID], rhs)
    await cg.register_component(var, config)
    # cg.add(var.pre_setup())
    cg.add_define("USE_KALFIRE")