import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_SWITCH,
    CONF_RESTORE_MODE,
)
from .. import CONF_KALFIRE_ID, Kalfire, kalfire_ns

FlameSwitch = kalfire_ns.class_("FlameSwitch", switch.Switch)
EcoModeSwitch = kalfire_ns.class_("EcoModeSwitch", switch.Switch)

CONF_FLAME_SWITCH = "flame"
CONF_ECO_MODE_SWITCH = "eco_mode"

def control_switch_schema(class_, icon):
    # A semantic fireplace command must not invert or restore a prior ON state.
    return switch.switch_schema(
        class_,
        block_inverted=True,
        default_restore_mode="ALWAYS_OFF",
        device_class=DEVICE_CLASS_SWITCH,
        icon=icon,
    ).extend({
        cv.Optional(CONF_RESTORE_MODE, default="ALWAYS_OFF"): cv.enum(
            {"ALWAYS_OFF": switch.SwitchRestoreMode.SWITCH_ALWAYS_OFF},
            upper=True,
            space="_",
        ),
    })


CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_KALFIRE_ID): cv.use_id(Kalfire),
    cv.Required(CONF_FLAME_SWITCH): control_switch_schema(FlameSwitch, "mdi:fire"),
    cv.Required(CONF_ECO_MODE_SWITCH): control_switch_schema(EcoModeSwitch, "mdi:sprout"),
})


async def to_code(config):
    kalfire_component = await cg.get_variable(config[CONF_KALFIRE_ID])
    if flame_switch_config := config.get(CONF_FLAME_SWITCH):
        s = await switch.new_switch(flame_switch_config)
        await cg.register_parented(s, config[CONF_KALFIRE_ID])
        cg.add(kalfire_component.set_flame_switch(s))
    if eco_mode_switch_config := config.get(CONF_ECO_MODE_SWITCH):
        s = await switch.new_switch(eco_mode_switch_config)
        await cg.register_parented(s, config[CONF_KALFIRE_ID])
        cg.add(kalfire_component.set_eco_mode_switch(s))
