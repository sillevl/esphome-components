import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_NUMBER,
    ENTITY_CATEGORY_CONFIG,
    DEVICE_CLASS_POWER,
)
from .. import CONF_KALFIRE_ID, Kalfire, kalfire_ns

CONF_FLAME_HEIGHT_NUMBER = 'flame_height'

FlameHeightNumber = kalfire_ns.class_("FlameHeightNumber", number.Number)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_KALFIRE_ID): cv.use_id(Kalfire),
        cv.Required(CONF_FLAME_HEIGHT_NUMBER): number.number_schema(
            FlameHeightNumber,
            device_class=DEVICE_CLASS_POWER,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:unfold-more-horizontal",
        ),
    }
)

async def to_code(config):
    kalfire_component = await cg.get_variable(config[CONF_KALFIRE_ID])
    if number_config := config.get(CONF_FLAME_HEIGHT_NUMBER):
        n = await number.new_number(
            number_config,
            min_value=1,
            max_value=8,
            step=1,
        )
        await cg.register_parented(n, kalfire_component)
        # cg.add(kalfire_component.add_flame_height_number(n))

