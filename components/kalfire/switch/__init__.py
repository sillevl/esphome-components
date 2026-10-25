"""Migration error for the retired independent boolean controls."""
import esphome.config_validation as cv

CONFIG_SCHEMA = cv.invalid(
    "The kalfire switches were replaced by select: - platform: kalfire "
    "with Off / Manual / ECO options. Migrate your dashboard and automations."
)
