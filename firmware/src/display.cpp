#include "display.h"
#include "pins.h"

namespace {

// LovyanGFX needs a small class describing the panel and its wiring.
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_GC9A01 panel_;
  lgfx::Bus_SPI bus_;
  lgfx::Light_PWM light_;

 public:
  LGFX() {
    {
      auto cfg = bus_.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;  // wiki says 80 MHz works; start safe
      cfg.freq_read = 16000000;
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = PIN_LCD_SCLK;
      cfg.pin_mosi = PIN_LCD_MOSI;
      cfg.pin_miso = -1;
      cfg.pin_dc = PIN_LCD_DC;
      bus_.config(cfg);
      panel_.setBus(&bus_);
    }
    {
      auto cfg = panel_.config();
      cfg.pin_cs = PIN_LCD_CS;
      cfg.pin_rst = PIN_LCD_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = 240;
      cfg.panel_height = 240;
      cfg.readable = false;
      cfg.invert = true;      // if colors look like a photo negative, set false
      cfg.rgb_order = false;  // if red and blue are swapped, set true
      cfg.bus_shared = false;
      panel_.config(cfg);
    }
    {
      auto cfg = light_.config();
      cfg.pin_bl = PIN_LCD_BL;
      cfg.invert = false;
      cfg.freq = 12000;
      cfg.pwm_channel = 7;  // the button LED uses channel 0
      light_.config(cfg);
      panel_.setLight(&light_);
    }
    setPanel(&panel_);
  }
};

LGFX lcd;
LGFX_Sprite frame(&lcd);  // 240x240 x 16-bit = 115 KB off-screen buffer
constexpr int kScale = 240 / sprites::kW;

}  // namespace

namespace display {

void begin() {
  lcd.init();
  lcd.setBrightness(160);  // 0-255; a kid's nightstand does not need full blast
  frame.setColorDepth(16);
  if (!frame.createSprite(240, 240)) {
    Serial.println("display: could not allocate frame buffer");
  }
}

void drawFace(sprites::Palette p, sprites::State s) {
  const uint8_t* px = sprites::kFrames[p][s];
  const uint16_t* colors = sprites::kColors[p];
  for (int y = 0; y < sprites::kH; ++y) {
    for (int x = 0; x < sprites::kW; ++x) {
      frame.fillRect(x * kScale, y * kScale, kScale, kScale, colors[px[y * sprites::kW + x]]);
    }
  }
}

void drawFooter(const char* text) {
  frame.setTextDatum(textdatum_t::bottom_center);
  frame.setTextColor(TFT_WHITE, TFT_BLACK);
  frame.setFont(&fonts::Font2);
  frame.drawString(text, 120, 222);
}

void present() { frame.pushSprite(0, 0); }

}  // namespace display
