import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_SWITCH,
    ENTITY_CATEGORY_CONFIG,
)
from .. import CONF_KALFIRE_ID, Kalfire, kalfire_ns

FlameSwitch = kalfire_ns.class_("FlameSwitch", switch.Switch)
EcoModeSwitch = kalfire_ns.class_("EcoModeSwitch", switch.Switch)

CONF_FLAME_SWITCH = "flame"
CONF_ECO_MODE_SWITCH = "eco_mode"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_KALFIRE_ID): cv.use_id(Kalfire),
    cv.Required(CONF_FLAME_SWITCH): switch.switch_schema(
        FlameSwitch,
        device_class=DEVICE_CLASS_SWITCH,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:fire",
    ),
    cv.Required(CONF_ECO_MODE_SWITCH): switch.switch_schema(
        EcoModeSwitch,
        device_class=DEVICE_CLASS_SWITCH,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:sprout",
    ),
}


async def to_code(config):
    kalfire_component = await cg.get_variable(config[CONF_KALFIRE_ID])
    if flame_switch_config := config.get(CONF_FLAME_SWITCH):
        s = await switch.new_switch(flame_switch_config)
        await cg.register_parented(s, config[CONF_KALFIRE_ID])
        # cg.add(kalfire_component.add_eco_mode_switch(s))
    if eco_mode_switch_config := config.get(CONF_ECO_MODE_SWITCH):
        s = await switch.new_switch(eco_mode_switch_config)
        await cg.register_parented(s, config[CONF_KALFIRE_ID])
        # cg.add(kalfire_component.add_flame_switch(s))
