# Status

## 2026-09-24

**Changed.** Direction reset: standalone frog reading its own pot (soil + light), not
the homestead MQTT concept. Old README/spec replaced; the brief is `docs/brief.md`.
Built the PlatformIO firmware scaffold (display, button, LED pulse, sensors), a
calibration logger (serial + flash, button = "watered" marker), the sprite exporter,
and the mood engine in `firmware/lib/mood` with 19 passing desktop tests. The frog
build is driven by the mood engine. Everything compiles; **nothing has run on the real
board yet** (parts ordered 2026-09-23, not arrived).

**Decided.** Two-layer mood (resting mood + short reactions), so the button always
answers. Light-based night mode ("the frog follows the sun"). One celebration per
watering, not per pour. Earn-never-lose rewards.

**In flight / unverified.** GPIO picks 15/16/17/18/21 and VSYS as a 5 V source need
checking on the physical board. Moisture and light thresholds are placeholders until
the calibration week. Sleepy-love reaction has no art. Speaker code not started.

**Next.** Animation frames (blink, breathe, happy wiggle), then growth stages +
the collection that compounds (one keepsake per plant that reaches flower).

**[non-production]** (in `~/me/queue.md`): board checks with a multimeter when the
boards arrive; solder button + LED and first flash; start the calibration week.
