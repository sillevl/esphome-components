import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_SWITCH,
    ENTITY_CATEGORY_CONFIG,
)
from .. import CONF_KALFIRE_ID, Kalfire, kalfire_ns

TestSwitch = kalfire_ns.class_(
    "UnderlyOpenFunctionSwitch", switch.Switch
)

CONF_TEST = "test"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_KALFIRE_ID): cv.use_id(Kalfire),
    cv.Optional(CONF_TEST): switch.switch_schema(
        TestSwitch,
        device_class=DEVICE_CLASS_SWITCH,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:electric-switch",
    ),
}


async def to_code(config):
    kalfire_component = await cg.get_variable(config[CONF_KALFIRE_ID])
    if test_switch_config := config.get(CONF_TEST):
        s = await switch.new_switch(test_switch_config)
        await cg.register_parented(s, config[CONF_KALFIRE_ID])
        # cg.add(kalfire_component.set_underlying_open_function_switch(s))
