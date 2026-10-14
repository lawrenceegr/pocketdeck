# PocketDeck

ZMK board module for the PocketDeck keyboard (nRF52840).

## Requirements

- ZMK `main` with Zephyr 4.1
- Zephyr SDK 0.17.0
- `west`

## Build

From the `app` directory of your ZMK checkout:

```
west build -p always -d /tmp/pocketdeck-zmk-build -b pocketdeck -- -DZMK_EXTRA_MODULES="$HOME/dev/firmware/keyboards-firmware/pocketdeck"
```

## Notes

- The `sym` key holds layer 1, which sends the small legends printed on the keycaps.
- The buzzer is a PWM behavior in `src/behaviors/behavior_buzzer.c`. `&buzzer <hz>` plays a 40 ms tone at that frequency, and it assumes a passive piezo on P0.17.
- The matrix column of the space bar is assumed to be 3 on row 4. Check it against the schematic.
- The battery divider on P0.04 is assumed to be 1 MΩ over 1 MΩ. Set `output-ohms` and `full-ohms` in the `vbatt` node to the real values.
- The flash layout assumes an Adafruit-style bootloader at the top of flash.
- Columns 5 and 6 use the 32 kHz crystal pins, so the low-frequency clock runs from the internal RC oscillator.

## Not implemented

- NPM1100 charge status: the CHG, ISET and MODE pins are declared in the `zephyr,user` node, but nothing uses them.
- Trackball: no pins documented yet.

## Known warnings

- `Deprecated symbol KSCAN is enabled` comes from ZMK's matrix driver and cannot be fixed in this module.
