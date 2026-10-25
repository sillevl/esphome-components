import math

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output, sensor, text_sensor
from esphome.const import CONF_ID, CONF_OUTPUT, CONF_SENSOR, ENTITY_CATEGORY_DIAGNOSTIC
from esphome.core import coroutine_with_priority

AUTO_LOAD = ["select", "output", "number", "sensor", "text_sensor"]
kalfire_ns = cg.esphome_ns.namespace("kalfire")
Kalfire = kalfire_ns.class_("Kalfire", cg.Component)
CONF_KALFIRE_ID = "kalfire_id"
CONF_SESSION_TIMEOUT = "session_timeout"
CONF_TEMPERATURE_LIMIT = "temperature_limit"
CONF_CEILING = "ceiling"
CONF_RESTART_MARGIN = "restart_margin"
CONF_MAX_AGE = "max_age"
CONF_UNAVAILABLE_GRACE = "unavailable_grace_period"
CONF_TIME_REMAINING = "time_remaining"
CONF_LAST_STOP_REASON = "last_stop_reason"
CONF_TEMPERATURE_STATUS = "temperature_limit_status"


def bounded_duration(value, allow_zero=False):
    value = cv.positive_time_period_milliseconds(value)
    if not (0 if allow_zero else 1) <= value.total_milliseconds <= 86400000:
        raise cv.Invalid("Duration must be between %s and 24h" % ("0ms" if allow_zero else "1ms"))
    return value


def finite_temperature(value):
    value = cv.temperature(value)
    if not math.isfinite(value):
        raise cv.Invalid("Temperature must be finite")
    return value


def restart_margin(value):
    value = cv.temperature_delta(value)
    if not math.isfinite(value) or value < 0:
        raise cv.Invalid("Restart margin must be finite and non-negative")
    return value


CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Kalfire),
    cv.Required(CONF_OUTPUT): cv.use_id(output.FloatOutput),
    cv.Optional(CONF_SESSION_TIMEOUT): bounded_duration,
    cv.Optional(CONF_TEMPERATURE_LIMIT): cv.Schema({
        cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
        cv.Required(CONF_CEILING): finite_temperature,
        cv.Optional(CONF_RESTART_MARGIN, default="0.5°C"): restart_margin,
        cv.Optional(CONF_MAX_AGE, default="5min"): bounded_duration,
        cv.Optional(CONF_UNAVAILABLE_GRACE, default="60s"): lambda v: bounded_duration(v, allow_zero=True),
    }),
    cv.Optional(CONF_TIME_REMAINING): sensor.sensor_schema(
        unit_of_measurement="s", accuracy_decimals=0,
        device_class="duration", entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_LAST_STOP_REASON): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_TEMPERATURE_STATUS): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
}).extend(cv.COMPONENT_SCHEMA)


@coroutine_with_priority(80.0)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.add_output(await cg.get_variable(config[CONF_OUTPUT])))
    if CONF_SESSION_TIMEOUT in config:
        cg.add(var.set_session_timeout(config[CONF_SESSION_TIMEOUT].total_milliseconds))
    if limit := config.get(CONF_TEMPERATURE_LIMIT):
        cg.add(var.set_temperature_limit(
            await cg.get_variable(limit[CONF_SENSOR]), limit[CONF_CEILING], limit[CONF_RESTART_MARGIN],
            limit[CONF_MAX_AGE].total_milliseconds, limit[CONF_UNAVAILABLE_GRACE].total_milliseconds,
        ))
    if conf := config.get(CONF_TIME_REMAINING):
        cg.add(var.set_time_remaining_sensor(await sensor.new_sensor(conf)))
    if conf := config.get(CONF_LAST_STOP_REASON):
        cg.add(var.set_last_stop_reason_sensor(await text_sensor.new_text_sensor(conf)))
    if conf := config.get(CONF_TEMPERATURE_STATUS):
        cg.add(var.set_temperature_limit_status_sensor(await text_sensor.new_text_sensor(conf)))
