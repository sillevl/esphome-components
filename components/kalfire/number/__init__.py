import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_NUMBER,
    ENTITY_CATEGORY_CONFIG,
)
from .. import CONF_KALFIRE_ID, Kalfire, kalfire_ns

SensitivityNumber = kalfire_ns.class_("FlameHeightNumber", number.Number)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_KALFIRE_ID): cv.use_id(Kalfire),
        cv.Optional(CONF_NUMBER): number.number_schema(
            SensitivityNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:archive-check-outline",
        ),
    }
)

async def to_code(config):
    kalfire_component = await cg.get_variable(config[CONF_KALFIRE_ID])
    if number_config := config.get(CONF_NUMBER):
        n = await number.new_number(
            number_config,
            min_value=1,
            max_value=8,
            step=1,
        )
        await cg.register_parented(n, kalfire_component)
        cg.add(kalfire_component.add_flame_height_number(n))

