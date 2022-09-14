#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include "adafruit_benchmark.h"
#include <DefaultConfig.h>

Adafruit_SSD1351 *tft;

void setup(void) {
  Serial.begin(SERIAL_DATA_RATE);
  Serial.printf("Starting adafruit-example with SPI speed %d", SPI_SPEED);
  SPIClass *spi = new SPIClass();
  spi->begin(SCLK_PIN, -1, MOSI_PIN, CS_PIN);
  tft = new Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, spi, CS_PIN, DC_PIN, RST_PIN);
  tft->begin(SPI_SPEED);}

void loop() {
  uint16_t start = millis();

  runAdafruitBenchmark(tft);
  
  uint16_t time = millis() - start;
  Serial.printf("done in %dms\n", time);
  delay(1000);
}
