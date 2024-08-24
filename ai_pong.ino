#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define TFT_CS D3
#define TFT_RST D8
#define TFT_DC D7
#define TFT_MOSI D9
#define TFT_SCLK D10

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_MPU6050 mpu;

// Pong Game Variables
int ballX, ballY;
int ballDirX = 1, ballDirY = 1;
int playerPaddleY = 0, aiPaddleY = 0;
const int paddleHeight = 20;

// Function to draw the paddles
void drawPaddle(int x, int y) {
  tft.fillRect(x, y, 2, paddleHeight, ST77XX_RED);
}

// Function to draw the ball
void drawBall(int x, int y) {
  tft.fillRect(x, y, 4, 4, ST77XX_BLUE);
}

// Function to move the ball
void moveBall() {
  tft.fillRect(ballX, ballY, 4, 4, ST77XX_BLACK);  // Erase previous ball position
  ballX += ballDirX;
  ballY += ballDirY;

  // Ball collision with top and bottom
  if (ballY <= 0 || ballY >= tft.height() - 2) ballDirY = -ballDirY;

  // Ball collision with player paddle
  if (ballX <= 2 && ballY >= playerPaddleY && ballY <= playerPaddleY + paddleHeight) {
    ballDirX = -ballDirX;
  }

  // Ball collision with AI paddle
  if (ballX >= tft.width() - 4 && ballY >= aiPaddleY && ballY <= aiPaddleY + paddleHeight) {
    ballDirX = -ballDirX;
  }

  if (ballX <= 0 || ballX >= tft.width()) {
    ballX = tft.width() / 2;  // Reset ball to center
    ballY = tft.height() / 2;
    ballDirX = -ballDirX;  // Change direction
  }

  drawBall(ballX, ballY);
}

// Function to initialize the game
void setupGame() {
  ballX = tft.width() / 2;
  ballY = tft.height() / 2;
  playerPaddleY = tft.height() / 2 - paddleHeight / 2;
  aiPaddleY = tft.height() / 2 - paddleHeight / 2;
}

void setup() {
  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);

  // Initialize the MPU6050
  if (!mpu.begin()) {
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.println("Failed to find MPU6050");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  setupGame();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  // Update player paddle position using MPU6050 data
  playerPaddleY = map(a.acceleration.z, -5, -1.5, 0, tft.height() - paddleHeight);

  // Clear previous paddles
  tft.fillRect(0, 0, 2, tft.height(), ST77XX_BLACK);
  tft.fillRect(tft.width() - 2, 0, 2, tft.height(), ST77XX_BLACK);

  // Draw paddles
  drawPaddle(0, playerPaddleY);
  aiPaddleControl();
  drawPaddle(tft.width() - 2, aiPaddleY);
  moveBall();
  delay(40);  // Adjust speed of the game
}