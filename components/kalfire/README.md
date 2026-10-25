# Kalfire component

ESPHome component for a Kalfire fireplace using its documented 0–10 V home-automation input. Validation and build target: **ESPHome 2026.10.0**.

## Features

* Enable flame: Request the fireplace on or off
* ECO mode: Toggle ECO mode on or off
* Flame Height: Set a desired flame height using one of the 8 settings

The component will calculate a voltage for an 0-10V output depending on the settings.

![M5Stack Atom S3 with DAC2](./img/m5stack_atom_s3.png)

The implementation is tested with an [M5Stack Atom S3](https://docs.m5stack.com/en/core/AtomS3) and [M5Stack Unit DAC2](https://docs.m5stack.com/en/unit/Unit_DAC2) module.

## Example

The component requires an ESPHome `output.FloatOutput` whose full-scale output represents **10 V**. Do not invert it or apply `min_power`/`max_power` scaling: doing so changes the protocol voltages, potentially including the off command. For M5Stack Unit DAC2, explicitly configure `model: GP8413` in the `gp8403` section.

You need to provide an output. In the example below `my_gp8403_output_1` is an ESPHome output component id.

```yaml
external_components:
  # Replace <tested-commit> with a published commit containing these fixes.
  source: github://sillevl/esphome-components@<tested-commit>
  components: [ kalfire ]

switch:
  - platform: kalfire
    flame:
      name: Enable flame
    eco_mode:
      name: Enable ECO mode

number:
  - platform: kalfire
    flame_height:
      name: Flame height

kalfire:
  id: my_kalfire
  output: my_gp8403_output_1
```

An example configuration can be found in the [kalfire_example.yaml](../../kalfire_example.yaml) file.

## Requested states and voltage mapping

| Requested state | Nominal voltage |
| --- | ---: |
| Flame disabled (any ECO/height selection) | 0 V |
| Flame enabled, ECO selected | 9.5 V |
| Normal height 1 | 3.000 V |
| Normal height 2 | 3.857 V |
| Normal height 3 | 4.714 V |
| Normal height 4 | 5.571 V |
| Normal height 5 | 6.429 V |
| Normal height 6 | 7.286 V |
| Normal height 7 | 8.143 V |
| Normal height 8 | 9.000 V |

The normal-mode formula is `3 + 6 * (height - 1) / 7`. Heights must be finite
integers from 1 to 8; invalid inputs leave the existing request unchanged.
Changing height during ECO stores the normal-mode selection without leaving ECO.
Turning the flame request off does not clear the current ECO/height selection;
a subsequent enable uses those selections. A reboot resets all three requests.

The switches are assumed-state controls. Their state and the logged voltage describe
**software requests**, not measured voltage, ignition success or flame presence.
The generic float-output interface cannot acknowledge whether an I2C write succeeded.

## Startup and supported switch options

The controller initializes at ESPHome's `PROCESSOR` priority, after the GP8403/GP8413
DAC's default `DATA` initialization and before networking. It requests 0 V and
publishes flame off, ECO off and height 1. Other output implementations must also be
ready before this priority. A per-device `on_boot` workaround is not required.

Both switches accept only `restore_mode: ALWAYS_OFF` (the default); previous on
states are never restored. `inverted` is unsupported. Existing configurations using
other restore modes or inversion now fail validation rather than silently accepting
unsupported behaviour. Startup state publication can trigger configured `on_state`
automations: do not configure those automations to ignite unintentionally.

This is **not an electrical fail-off guarantee**. The DAC may retain a voltage before
initialization, on an MCU crash, on power loss or on I2C failure. No special network-loss
policy or orderly-shutdown handler is added. Existing ESPHome API/Wi-Fi reboot
settings still apply; on a subsequent normal startup the controller requests off.

## Installation and safety boundaries

Kalfire documents 0 V for off, 3–9 V for normal flame modulation and 9.5 V for ECO
in the [G/GP installation manual, section 5.12.1, page 23](https://kalfire.com/downloads/Handleidingen/Gashaarden/G%20and%20GP/Installation%20manual%20Kalfire%20G-GP_EN_V03_2026.pdf).
That is the currently published 2026 manual; confirm the applicable controller revision
and installation instructions for older fireplaces, including units installed in 2021.
The equal voltage steps are not a claim of equal heat output or exact equivalence to
remote-control levels.

Use the manufacturer's home-automation connection, retain its burner controller and
safety interlocks, and have wiring verified by a qualified installer. The manual also
specifies a minimum 2 mA at the KA-02 input: verify output capability and voltage under
load. Never use this component to reset ignition faults, bypass lockouts or directly
control a gas valve. The remote control remains necessary for the documented fault
reset procedure. Hybrid lighting and actual flame/error feedback are not implemented.

## Development

See [tests/README.md](../../tests/README.md) for native regression tests, ESPHome schema
checks and the ESP32-S3 build fixture. Tests and a successful build are not hardware
verification. Published deployments should reference a reviewed commit/tag; fetching a
new component version does not update an already running device until firmware is
built and explicitly uploaded.
