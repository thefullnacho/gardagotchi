# Gardagotchi

A garden tamagotchi. A pixel-art frog on a round screen that watches over a real potted
plant — and travels with her when she picks it up and carries it. When the soil dries out,
the frog gets thirsty. When she waters it, the sensor notices and the frog celebrates on
its own. One big button: press = love.

Built by a grandpa, with an AI, for a 4-year-old who already knows how seeds grow and is
building the habit of looking after one. The tech stays invisible; the frog is the
interface.

Standalone: no WiFi, no server, no app. It hears its own pot over 915 MHz and nothing else.

## What's here

```
gardagotchi/
├── firmware/   # PlatformIO + Arduino + LovyanGFX, Waveshare ESP32-S3-LCD-1.28
│   ├── lib/mood/      # the mood engine (plain C++, tested on the desktop)
│   ├── src/           # display, button + LED, radio, calibration logger
│   └── test/          # mood tests: pio test -e native
├── faces/      # frog.py draws every face; export_sprites.py feeds the firmware
├── tools/      # calib_pull.py: copy the calibration log off the board
└── docs/       # brief, wiring, calibration, design notes
```

## Design rules

- Faces, not text. Game icons (stars, gems) are fine; words are not.
- One button, and it always answers in under a second.
- The frog only asks for what she can fix, never at night, and never gets worse for
  waiting. Negative moods fade on their own. No guilt mechanics.
- Earn, never lose. Rewards only accumulate; nothing shows a missed day.
- One celebration per watering, not per pour, so pouring more never earns more.
- Battery-powered and portable: she carries it. USB-C charging. Everything enclosed,
  rounded, no small parts.

## Hardware

Waveshare ESP32-S3-LCD-1.28 (round 240x240 GC9A01, onboard IMU), CC1101 915 MHz module
(SPI) hearing an Ecowitt WH51 soil sensor in her pot, BH1750 light sensor, MAX98357A amp
+ 4 ohm speaker, 30 mm LED arcade button, 3.7V LiPo on the MX1.25 header (charges over
USB-C), printed case (Bambu A1). Pin map and solder checklist:
[`docs/wiring.md`](docs/wiring.md).

## Build

```
cd firmware
pio test -e native                  # mood tests, no hardware needed
pio run -e frog -t upload           # the frog
pio run -e calibrate -t upload      # soil + light logging
```

## Docs

- [`docs/brief.md`](docs/brief.md): the concept, hardware, and open questions
- [`docs/firmware-notes.md`](docs/firmware-notes.md): mood layers, night mode, rewards
- [`docs/calibration.md`](docs/calibration.md): finding the moisture thresholds
- [`docs/wiring.md`](docs/wiring.md): GPIO map and solder checklist
- [`STATUS.md`](STATUS.md): where things stand
