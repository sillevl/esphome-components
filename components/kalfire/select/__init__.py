import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from .. import CONF_KALFIRE_ID, Kalfire, kalfire_ns

ModeSelect = kalfire_ns.class_("ModeSelect", select.Select)
CONFIG_SCHEMA = select.select_schema(ModeSelect, icon="mdi:fire").extend({
    cv.GenerateID(CONF_KALFIRE_ID): cv.use_id(Kalfire),
})


async def to_code(config):
    var = await select.new_select(config, options=["Off", "Manual", "ECO"])
    parent = await cg.get_variable(config[CONF_KALFIRE_ID])
    await cg.register_parented(var, parent)
    cg.add(parent.set_mode_select(var))
