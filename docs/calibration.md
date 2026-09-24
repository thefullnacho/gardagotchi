# Calibration week: finding the moisture thresholds

Goal: a week of soil + light readings from a test pot while you water the way you
normally would. From that we set three numbers: when the frog gets **thirsty**, the
**happy** range, and when it's **soggy**.

## Setup
1. Wire the soil sensor and BH1750 (`wiring.md`, group 3).
2. Flash the logger:
   ```
   cd firmware
   pio run -e calibrate -t upload
   pio device monitor
   ```
   You should see `sensors: soil found, light found` and a CSV line every minute.
   The screen shows the frog with the live numbers underneath.
3. Push the soil sensor into the test pot up to the line on the board. The white
   electronics end stays **above** the soil.
4. Put the BH1750 where her real one will go, facing the same way, so the light numbers
   mean the same thing later.
5. Move it to the wall adapter and **write down the time you plug it in**. The board
   has no clock, so rows are counted in seconds since power-on. The daily light curve
   will also show sunrise, which gives a second way to line the times up.

## During the week
- **Press the button every time you water.** It logs a `watered` row and the frog
  celebrates. Those markers are what make the data readable.
- Water when you'd normally water, including once when it's properly dry, so we see
  "thirsty". If you can spare it, overwater once so we see "soggy" too.
- Logging runs once a minute; a week is about 10,000 rows (~400 KB). The flash holds months.
- A power cut is fine: the log survives and the next rows start a new `boot` number.

## Getting the data off
Plug the board into the computer and run:
```
python3 tools/calib_pull.py
```
It saves `data/calib-<date>.csv` and doesn't erase anything on the board. Or, in
`pio device monitor`, type `dump`, `status`, or `help`.

## CSV columns
| column | meaning |
|---|---|
| boot | increments on every power-up |
| uptime_s | seconds since that power-up |
| unix_time | real time if the clock was set this boot, else 0 |
| moisture | raw capacitive reading. Roughly 200 in dry air to 2000 in water; soil sits between |
| soil_temp_c | the sensor chip's temperature (approximate) |
| lux | light level. Stops at about 54,600 (direct sun hits that ceiling; fine for now) |
| event | `boot`, `watered`, or blank |

Then hand me the CSV and I'll plot it and propose the thresholds.
