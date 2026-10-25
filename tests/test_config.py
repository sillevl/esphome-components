"""Exercise real ESPHome validation and code generation without credentials/hardware."""
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


def test_validation_and_codegen(tmp_path):
    result, build = run_esphome(tmp_path, generate=True)
    assert result.returncode == 0, result.stdout + result.stderr
    generated = (build / "src/main.cpp").read_text()
    for call in ["set_flame_switch(", "set_eco_mode_switch(", "set_flame_height_number("]:
        assert call in generated
    assert "set_model(gp8403::GP8413)" in generated
    assert "set_device_class(\"power\")" not in generated
    assert "ENTITY_CATEGORY_CONFIG" not in generated


@pytest.mark.parametrize("control", ["flame", "eco_mode"])
@pytest.mark.parametrize("mode", ["ALWAYS_ON", "RESTORE_DEFAULT_ON", "RESTORE_DEFAULT_OFF", "DISABLED"])
def test_unsupported_restore_modes_rejected(tmp_path, control, mode):
    def change(config):
        config["switch"][0][control]["restore_mode"] = mode
    result, _ = run_esphome(tmp_path, change)
    assert result.returncode != 0
    assert "restore_mode" in result.stdout + result.stderr


@pytest.mark.parametrize("control", ["flame", "eco_mode"])
def test_inversion_rejected(tmp_path, control):
    def change(config):
        config["switch"][0][control]["inverted"] = True
    result, _ = run_esphome(tmp_path, change)
    assert result.returncode != 0
    assert "Inverted is not supported" in result.stdout + result.stderr


def test_binary_output_rejected(tmp_path):
    def change(config):
        config["output"].append({"platform": "gpio", "id": "binary_output", "pin": "GPIO3"})
        config["kalfire"]["output"] = "binary_output"
    result, _ = run_esphome(tmp_path, change)
    assert result.returncode != 0
    assert "FloatOutput" in result.stdout + result.stderr


def test_explicit_always_off_accepted(tmp_path):
    def change(config):
        for control in ["flame", "eco_mode"]:
            config["switch"][0][control]["restore_mode"] = "ALWAYS_OFF"
    result, _ = run_esphome(tmp_path, change)
    assert result.returncode == 0, result.stdout + result.stderr
