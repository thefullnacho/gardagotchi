# Calibration: finding the moisture thresholds

Goal: pick three numbers from real data — when the frog gets **thirsty**, the **happy**
range, and when it's **soggy**. The WH51 reports soil moisture as 0–100% directly, and
the beds have been logging WH51 history into HA for a while, so thresholds start from
that history and get tuned by watching her pot.

**The plant decides where the lines go.** Her seeds are French marigold, Sparky Mix
(*Tagetes patula*): sun to partial sun, well drained to dry. Marigolds like to dry out
between waterings and rot when kept wet, so **thirsty** belongs at the genuinely dry
end of the data, not at "drier than yesterday", and **soggy** matters more than it
would for a thirstier plant. That's also kind to a 4-year-old: a forgotten day costs
the marigold nothing.

## Setup
1. Put the WH51 in her pot, probe fully in the soil. It runs about a year on its AA
   battery; nothing to wire on this side.
2. Wire the CC1101 to the frog (`wiring.md`, group 4) and flash the logger:
   ```
   cd firmware
   pio run -e calibrate -t upload
   pio device monitor
   ```
   You should see `radio: WH51 found` and a row every time her sensor transmits
   (~every 70 s), plus the BH1750 light line every minute. The screen shows the frog
   with the live numbers underneath.
3. Note her pot's WH51 sensor ID from the first `radio` line and set it in the
   firmware so the frog ignores the bed sensors.
4. Put the BH1750 where her real one will go, facing the same way, so the light numbers
   mean the same thing later.

## During the week
- **Press the button every time you water.** It logs a `watered` row and the frog
  celebrates. Those markers are what make the data readable.
- Water when you'd normally water, including once when it's properly dry, so we see
  "thirsty". If you can spare it, overwater once so we see "soggy" too.
- A power cut is fine: the log survives and the next rows start a new `boot` number.
- Cross-check against HA: the beds' WH51 history already shows what "dry" and "soaked"
  read as percentages, which gives the first threshold guesses before her week's data
  lands.

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
| moisture_pct | WH51 soil moisture, 0–100% |
| soil_temp_c | WH51 soil temperature |
| lux | light level. Stops at about 54,600 (direct sun hits that ceiling; fine for now) |
| rssi | 915 MHz signal strength, for antenna/range sanity |
| event | `boot`, `watered`, or blank |

Then hand me the CSV and I'll plot it and propose the thresholds.
