# Kalfire tests

Use Python 3.12 or newer and install `requirements-dev.txt` in a virtual environment.
The dependency file pins ESPHome to 2026.10.0.

```sh
python -m pip install -r requirements-dev.txt
python tests/run_cpp_tests.py
python -m pytest tests/test_config.py -q
python -m esphome compile tests/configs/kalfire.yaml
python -m esphome compile tests/configs/kalfire-limits.yaml
```

The native tests need `g++`. If it is unavailable, a Linux Docker image containing
`g++` can be used, for example:

```sh
python tests/run_cpp_tests.py --docker-image gcc:14
```

The Docker runner mounts this repository read-only and disables container networking.
Image download, if needed, is handled by Docker before the container runs.
The native harness compiles the actual component C++ against small ESPHome test
doubles and a deterministic monotonic clock. It covers all eight levels, invalid
inputs, Off/Manual/ECO, local expiry, non-resetting deadlines, temperature ceilings,
restart margin, invalid/stale readings, grace recovery, clock wrap, startup-off and
synchronous callback re-entrancy. It does not simulate physical I2C.

The Python tests invoke the real ESPHome validator/code generator, including
negative tests for invalid limits, missing sensor references, legacy switch YAML
and binary outputs. The firmware fixtures build the real module and GP8413 driver
for ESP32-S3, both with and without optional limits. They contain no Wi-Fi/OTA
credentials or upload step. The limits fixture's template sensor is simulated, not
a real temperature measurement. Both are **build-only test images**, not replacements
for a deployed fireplace configuration.

## Windows / Git Bash

ESP-IDF rejects an inherited `MSYSTEM` variable, and its default cache path can
exceed Windows path limits. Set `ESPHOME_ESP_IDF_PREFIX` to a short, writable local
path and invoke the build from PowerShell (without `MSYSTEM`), or remove `MSYSTEM`
from the build subprocess environment. Do not change machine-wide registry settings
merely to run these tests.

## Limits

Passing tests and a firmware build do not establish electrical fail-off, output
current capability, actual voltage accuracy, flame presence or compatibility with
every Kalfire controller revision. No tests should operate a live fireplace.
