"""Exercise real ESPHome validation/code generation without credentials/hardware."""
from pathlib import Path
import subprocess
import sys

import pytest
import yaml

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / "tests/configs/kalfire.yaml"


def run_esphome(tmp_path, change=None, generate=False):
    config = yaml.safe_load(FIXTURE.read_text())
    config["external_components"]["source"]["path"] = str(ROOT / "components")
    config["esphome"]["build_path"] = str(tmp_path / "build")
    if change:
        change(config)
    path = tmp_path / "test.yaml"
    path.write_text(yaml.safe_dump(config, sort_keys=False))
    command = [sys.executable, "-m", "esphome"]
    command += ["compile", "--only-generate"] if generate else ["config"]
    result = subprocess.run(command + [str(path)], capture_output=True, text=True, timeout=90)
    return result, tmp_path / "build"


def add_limits(config):
    config["sensor"] = [{"platform": "template", "id": "room_temperature", "unit_of_measurement": "°C"}]
    config["kalfire"].update({
        "session_timeout": "90min",
        "temperature_limit": {
            "sensor": "room_temperature", "ceiling": "23°C", "restart_margin": "0.5°C",
            "max_age": "5min", "unavailable_grace_period": "60s",
        },
        "time_remaining": {"name": "Time remaining"},
        "last_stop_reason": {"name": "Last stop reason"},
        "temperature_limit_status": {"name": "Temperature limit status"},
    })


@pytest.mark.parametrize("limits", [False, True])
def test_validation_and_codegen(tmp_path, limits):
    result, build = run_esphome(tmp_path, add_limits if limits else None, generate=True)
    assert result.returncode == 0, result.stdout + result.stderr
    generated = (build / "src/main.cpp").read_text()
    for call in ["set_mode_select(", "set_flame_height_number("]:
        assert call in generated
    assert "set_model(gp8403::GP8413)" in generated
    assert '"Off"' in generated and '"Manual"' in generated and '"ECO"' in generated
    if limits:
        for call in ["set_session_timeout(", "set_temperature_limit(", "set_time_remaining_sensor("]:
            assert call in generated
    else:
        assert "set_session_timeout(" not in generated
        assert "set_temperature_limit(" not in generated


@pytest.mark.parametrize("duration", ["0s", "-1s", "25h"])
def test_invalid_session_duration_rejected(tmp_path, duration):
    result, _ = run_esphome(tmp_path, lambda c: c["kalfire"].update(session_timeout=duration))
    assert result.returncode != 0
    assert "session_timeout" in result.stdout + result.stderr


@pytest.mark.parametrize("key,value", [
    ("ceiling", "nan"), ("restart_margin", "-1°C"),
    ("max_age", "0s"), ("max_age", "25h"),
    ("unavailable_grace_period", "-1s"), ("unavailable_grace_period", "25h"),
])
def test_invalid_temperature_limits_rejected(tmp_path, key, value):
    def change(config):
        add_limits(config)
        config["kalfire"]["temperature_limit"][key] = value
    result, _ = run_esphome(tmp_path, change)
    assert result.returncode != 0
    assert key in result.stdout + result.stderr


def test_binary_output_rejected(tmp_path):
    def change(config):
        config["output"].append({"platform": "gpio", "id": "binary_output", "pin": "GPIO3"})
        config["kalfire"]["output"] = "binary_output"
    result, _ = run_esphome(tmp_path, change)
    assert result.returncode != 0
    assert "FloatOutput" in result.stdout + result.stderr


def test_legacy_switches_have_migration_error(tmp_path):
    def change(config):
        config.pop("select")
        config["switch"] = [{"platform": "kalfire", "flame": {"name": "Flame"}, "eco_mode": {"name": "ECO"}}]
    result, _ = run_esphome(tmp_path, change)
    assert result.returncode != 0
    assert "select" in result.stdout + result.stderr


def test_temperature_limit_requires_sensor(tmp_path):
    def change(config):
        add_limits(config)
        del config["kalfire"]["temperature_limit"]["sensor"]
    result, _ = run_esphome(tmp_path, change)
    assert result.returncode != 0
    assert "sensor" in result.stdout + result.stderr


def test_timer_and_temperature_are_independent(tmp_path):
    def change(config):
        add_limits(config)
        config["kalfire"].pop("session_timeout")
    result, _ = run_esphome(tmp_path, change)
    assert result.returncode == 0, result.stdout + result.stderr


def test_timer_without_temperature(tmp_path):
    result, _ = run_esphome(tmp_path, lambda c: c["kalfire"].update(session_timeout="30min"))
    assert result.returncode == 0, result.stdout + result.stderr
