# Gardagotchi

A garden tamagotchi. A little creature in a chunky, colorful case whose mood is driven by
*real homestead data* — cozy when the greenhouse is warm, thirsty when the rain barrels run
low, panicking when the wind kicks the greenhouse door open. One big button. Press = love.

Built by a grandpa, with an AI, for a very smart 4-year-old. The tech stays invisible;
the creature is the interface.

Full build spec: [`docs/spec.md`](docs/spec.md)

## Structure

```
gardagotchi/
├── esphome/   # device firmware (ESPHome YAML: display, button, buzzer, MQTT)
├── server/    # mood model + MQTT glue (runs on the homelab, not the device)
├── faces/     # creature pixel-art for the round display
├── case/      # 3D print files + print settings (Bambu A1)
└── docs/      # build guide, spec, notes
```

## The idea in one paragraph

Cheap ESP32s at the edge do one dumb job well; the brains live on the homelab server.
The Gardagotchi subscribes to sensor topics over MQTT (greenhouse temp + door, barrel
levels, soil moisture later) and translates them into a creature's emotions a 4-year-old
understands. She presses the big button to say hi. The creature asks for things ("the
flowers are thirsty"), she does them in the real world, the creature celebrates. The
garden has moods, and she can take care of it.

## Build phases

Each phase is a complete gift. Each upgrade is an event — "it learned about the garden!"

- **Phase 1 — now.** Creature + button + chirps, standalone (no WiFi needed). She names
  it, learns press = love. A complete toy on day one.
- **Phase 2 — with the greenhouse monitor.** WiFi + MQTT. Moods from greenhouse temp +
  door. The wind-blows-the-door-open incident becomes *her* early-warning system:
  scared face → she runs to tell Grandpa.
- **Phase 3 — the full network.** Barrels, soil moisture, the quest loop, real-data magic
  (moisture rises after watering → the creature perks up on its own).

## Parts (~$30)

- ESP32 dev board ×2–3 (spares are cheap insurance)
- 1.28" round GC9A01 TFT, 240×240 — the creature's face
- 30 mm arcade button with LED, colorful — the one button
- Piezo buzzer (5 V) — happy chirps, sad boops
- Jumper wires, USB-C cable + 5 V wall adapter
- Case: 3D printed, multicolor — *she* picks the colors

## Design rules

- Faces, not text. She's 4; emotion is carried by face, color, sound, motion.
- One button. No menus, no modes.
- Cause and effect, fast (<1 s). Delayed feedback is no feedback.
- USB wall power only. No batteries in the toy. All electronics enclosed.
- It sleeps when she sleeps (~8 PM–7 AM).
- Mood logic lives on the server, not the device — reflashing a kid's toy to tweak a
  threshold is nobody's idea of fun.

## Status

🚧 Phase 1 — parts on the shopping list, faces being drawn.
