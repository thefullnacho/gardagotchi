// Keeps the garden (growth days, plant number, keepsakes, waiting surprise) in flash,
// so a power cut or unplugging the toy loses nothing.
#pragma once
#include "garden.h"

namespace garden_store {
// Returns what was saved, or a brand-new first plant if nothing was (first power-on).
garden::Saved load();
void save(const garden::Saved& s);
}  // namespace garden_store
