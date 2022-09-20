#include <DefaultConfig.h>
#define SERIALLOG 1
#include <DLog.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <Adafruit_PixelDust.h>
#include <Colors.h>
#include <WiFi.h>

// In 80MHz mode, we can do 100 grains at about 120FPS
// In 160MHz mode, we can do 100 grains at about 185FPS which feels nice

#define N_GRAINS    180 // Number of grains of sand
#define MAX_FPS     40 // Maximum redraw rate, frames/second
#define ELASTICITY  128 // Bounciness (0-255)

// Sand object, last 2 args are accelerometer scaling and grain elasticity
Adafruit_PixelDust sand(SCREEN_WIDTH, SCREEN_HEIGHT, N_GRAINS, 1, ELASTICITY);

Adafruit_SSD1351 *tft;

void displayReset() {
  tft->fillScreen(BLACK);
  tft->setCursor(0, 0);
}

// Colors for each one of the grains
uint16_t colors[N_GRAINS];

typedef struct {
  dimension_t x;
  dimension_t y;
} point;
point grains[N_GRAINS];


// Twitter logo in blue
int logoWidth = 16;
int logoHeight = 38;

point linkLogo[] = {
  {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {1, 14}, {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9}, {3, 10}, {3, 11}, {3, 12}, {3, 13}, {3, 14}, {6, 0}, {6, 1}, {6, 2}, {6, 4}, {6, 5}, {6, 6}, {6, 7}, {6, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 2}, {7, 4}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7}, {8, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {11, 4}, {11, 5}, {11, 6}, {11, 7}, {11, 8}, {11, 9}, {11, 10}, {11, 11}, {11, 12}, {11, 13}, {11, 14}, {12, 4}, {12, 14}, {13, 4}, {13, 5}, {13, 6}, {13, 7}, {13, 8}, {13, 14}, {14, 12}, {14, 13}, {14, 14}, {15, 6}, {15, 7}, {15, 10}, {15, 11}, {15, 12}, {16, 5}, {16, 6}, {16, 9}, {16, 10}, {17, 5}, {17, 8}, {17, 9}, {18, 4}, {18, 7}, {18, 8}, {19, 4}, {19, 7}, {20, 4}, {20, 7}, {20, 8}, {20, 9}, {20, 10}, {20, 11}, {20, 12}, {21, 4}, {21, 5}, {21, 13}, {21, 14}, {22, 5}, {22, 6}, {22, 14}, {23, 7}, {23, 8}, {23, 9}, {23, 10}, {23, 11}, {23, 12}, {24, 13}, {25, 0}, {25, 1}, {25, 2}, {25, 3}, {25, 4}, {25, 5}, {25, 6}, {25, 7}, {25, 8}, {25, 9}, {25, 10}, {25, 11}, {25, 12}, {25, 13}, {26, 14}, {27, 14}, {28, 0}, {28, 1}, {28, 2}, {28, 3}, {28, 4}, {28, 5}, {28, 6}, {28, 7}, {28, 8}, {28, 9}, {28, 13}, {28, 14}, {29, 9}, {29, 10}, {29, 13}, {30, 7}, {30, 8}, {30, 9}, {30, 13}, {31, 6}, {31, 7}, {31, 12}, {31, 13}, {31, 14}, {32, 4}, {32, 5}, {32, 14}, {33, 4}, {33, 7}, {33, 8}, {33, 9}, {33, 10}, {33, 11}, {33, 12}, {34, 4}, {34, 5}, {34, 6}, {34, 7}, {34, 12}, {34, 13}, {35, 6}, {35, 13}, {35, 14}
};

// Accelerometer
#if defined(ACCEL_LIS2DW12)
#include <LIS2DW12Sensor.h>
#define ACCEL_ADDR 0x31U
LIS2DW12Sensor* accel;
#elif defined(ACCEL_MMA8452Q)
#include <SparkFun_MMA8452Q.h>
#define ACCEL_ADDR 0x1C
MMA8452Q accel(ACCEL_ADDR);
#endif

// Used for frames-per-second throttle
    uint32_t prevTime = 0;

uint32_t frameCounter = 0;
uint32_t lastFrameMessage = 0;

int begin_retval;


void setup(void) {
  setCpuFrequencyMhz(240);
  log_d("CPU Running at %dMHz", getCpuFrequencyMhz());
  Serial.begin(SERIAL_DATA_RATE);
  Wire.begin(SDA_PIN, SCL_PIN);

  if(!sand.begin()) {
        log_e("sand.begin error");
  }

#if defined(ACCEL_LIS2DW12)
  accel = new LIS2DW12Sensor(&Wire, ACCEL_ADDR);
  // if (!accel->begin()) err(250);
  accel->Enable_X();
#elif defined(ACCEL_MMA8452Q)
  if (!accel.begin(Wire, ACCEL_ADDR)) {
    log_e("Accelerometer error");
  };

#endif

  SPIClass *spi = new SPIClass();
  spi->begin(SCLK_PIN, -1, MOSI_PIN, CS_PIN);
  tft = new Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, spi, CS_PIN, DC_PIN, RST_PIN);
  tft->begin(SPI_SPEED);

  log_e("Setup complete");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_START, INPUT_PULLUP);

  // Max is 700000 in 160MHz mode and 400000 in 80MHz mode
  Wire.setClock(700000); // Run I2C at 700 KHz for faster screen updates

  // Clear the display
  displayReset();

  // Create some colors
  for (int i = 0; i < N_GRAINS; i ++) {
    colors[i] = randomColor();
  }

  sand.randomize(); // Initialize random sand positions

  lastFrameMessage = micros();
}

void drawLogo() {
  uint8_t i = 0;
  dimension_t logoStartY = (SCREEN_HEIGHT - logoHeight) / 2;
  dimension_t logoStartX = (SCREEN_WIDTH - logoWidth) / 2;

  for (uint8_t i = 0; i < N_GRAINS; i++) {
    point coord = linkLogo[i];
    uint8_t newX = logoStartY + coord.x;
    uint8_t newY = logoStartX + coord.y;
    
    sand.setPosition(i, newX, newY);
  }
}

void loop() {
  // Limit the animation frame rate to MAX_FPS. Because the subsequent sand
  // calculations are non-deterministic (don't always take the same amount
  // of time, depending on their current states), this helps ensure that
  // things like gravity appear constant in the simulation.
  uint32_t t;
  while(((t = micros()) - prevTime) < (1000000L / MAX_FPS));
  prevTime = t;

  uint8_t i;
  dimension_t x, y;

  // Display frame calculated on the prior pass.  It's done immediately after the
  // FPS sync (rather than after calculating) for consistent animation timing.
  tft->startWrite();
  for(i = 0; i < N_GRAINS; i++) {
    // Clear the old position of the grain
    tft->setAddrWindow(grains[i].x, grains[i].y, 1, 1);
    tft->SPI_WRITE16(BLACK);

    sand.getPosition(i, &x, &y);

    // Write the new position of the pixel
    tft->setAddrWindow(x, y, 1, 1);
    tft->SPI_WRITE16(colors[i]);

    // Save the grain position so we can erase it after the next iteration
    grains[i] = {x, y};
  }
  tft->endWrite();

  // Read accelerometer and run the physics

#if defined(ACCEL_LIS2DW12)
  int16_t axes[3];
  accel->Get_X_AxesRaw(axes);
  sand.iterate((int)-axes[0], (int)-axes[1], (int)-axes[2]);
#elif defined(ACCEL_MMA8452Q)
  // Wait for the accelerometer to become ready
  while (!accel.available()) {}
  // Read accelerometer and run the physics
  accel.read();
  // log_d("accel.read() %d %d %d", accel.x, accel.y, accel.z);
  sand.iterate((int)(accel.x), (int)(-accel.y), (int)(accel.z));
#endif

  // Log the FPS for debugging once per second
  frameCounter++;
  if ((t - lastFrameMessage) > 1000000L) {
    DLOG("FPS: %f", (float)frameCounter/((t - lastFrameMessage)/1000000.0));
    frameCounter = 0;
    lastFrameMessage = t;
  }

  if (digitalRead(BUTTON_A) == LOW) {
    log_d("Button A pressed");
    sand.iterate(-232, 10310, 910);
  }
  if (digitalRead(BUTTON_B) == LOW) {
    log_d("Button B pressed");
    sand.iterate(-34, -10300, 910);
  }
  if (digitalRead(BUTTON_UP) == LOW) {
    log_d("Button UP pressed");
    sand.iterate(-34, -10300, 910);
  }
  if (digitalRead(BUTTON_DOWN) == LOW) {
    log_d("Button DOWN pressed");
    sand.iterate(-232, 10310, 910);
  }
  if (digitalRead(BUTTON_LEFT) == LOW) {
    log_d("Button LEFT pressed");
    sand.iterate(-10232, 310, 910);
  }
  if (digitalRead(BUTTON_RIGHT) == LOW) {
    log_d("Button RIGHT pressed");
    sand.iterate(10232, 310, 910);
  }
  if (digitalRead(BUTTON_SELECT) == LOW) {
    log_d("Button SELECT pressed");
  }
  if (digitalRead(BUTTON_START) == LOW) {
    drawLogo();
  }
}
