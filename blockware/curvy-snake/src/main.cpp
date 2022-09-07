#define SERIALLOG 1
#include <DLog.h>

#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <Colors.h>

#include <CurvySnake.h>
#include <DefaultConfig.h>

Adafruit_SSD1351 *tft;

void localDisplayReset() {
  tft->fillScreen(BLACK);
  tft->setCursor(0, 0);
}

void setup(void)
{
  SPIClass *spi = new SPIClass();
  spi->begin(SCLK_PIN, -1, MOSI_PIN, CS_PIN);
  tft = new Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, spi, CS_PIN, DC_PIN, RST_PIN);
  tft->begin(SPI_SPEED);

  // initialize srand
  randomSeed(ESP.getCycleCount());

  localDisplayReset();

  CurvySnake_setup(tft);
}

void loop()
{
  CurvySnake_loop(tft);
}
