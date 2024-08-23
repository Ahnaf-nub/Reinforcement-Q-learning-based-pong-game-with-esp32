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

// Q-learning variables
#define ACTION_UP 0
#define ACTION_DOWN 1

float qTable[8][2] = { 0 };  // Adjusted state space and action space
float learningRate = 0.2;
float discountFactor = 0.95;

// Function to draw the paddles
void drawPaddle(int x, int y) {
  tft.fillRect(x, y, 2, paddleHeight, ST77XX_WHITE);
}

// Function to draw the ball
void drawBall(int x, int y) {
  tft.fillRect(x, y, 2, 2, ST77XX_WHITE);
}

// Function to move the ball
void moveBall() {
  tft.fillRect(ballX, ballY, 2, 2, ST77XX_BLACK);  // Erase previous ball position
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

  // Ball out of bounds
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

// Function to choose action for Q-learning
int chooseAction(int state) {
  if (random(0, 100) < 10) {
    return random(0, 2);  // Explore
  } else {
    return qTable[state][0] > qTable[state][1] ? ACTION_UP : ACTION_DOWN;  // Exploit
  }
}

// Function to update Q-table
void updateQTable(int state, int action, float reward, int nextState) {
  float oldQ = qTable[state][action];
  float bestNextQ = max(qTable[nextState][0], qTable[nextState][1]);
  qTable[state][action] = oldQ + learningRate * (reward + discountFactor * bestNextQ - oldQ);
}

// AI paddle control with Q-learning
void aiPaddleControl() {
  int state = aiPaddleY / 10;  // Discretize the state
  int action = chooseAction(state);

  // Move AI paddle
  if (action == ACTION_UP && aiPaddleY > 0) aiPaddleY -= 2;
  if (action == ACTION_DOWN && aiPaddleY < tft.height() - paddleHeight) aiPaddleY += 2;

  // Reward: 1 for intercepting the ball, -1 for missing
  int reward = (ballX > tft.width() / 2 && abs(ballY - aiPaddleY) < paddleHeight) ? 1 : -1;
  int nextState = aiPaddleY / 10;

  updateQTable(state, action, reward, nextState);
}

void setup() {
  Serial.begin(115200);
  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);  // Ensure proper orientation for your setup

  // Initialize the MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
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

// Q-learning variables
#define ACTION_UP 0
#define ACTION_DOWN 1
#define ACTION_STAY 1

float qTable[8][8][3] = { 0 };  // State space [AI position][Ball position][Action]
float learningRate = 0.1;
float discountFactor = 0.99;
float explorationRate = 0.5;  // Probability of exploration

// Function to draw the paddles
void drawPaddle(int x, int y) {
  tft.fillRect(x, y, 2, paddleHeight, ST77XX_RED);
}

// Function to draw the ball
void drawBall(int x, int y) {
  tft.fillRect(x, y, 2, 2, ST77XX_WHITE);
}

// Function to move the ball
void moveBall() {
  tft.fillRect(ballX, ballY, 2, 2, ST77XX_BLACK);  // Erase previous ball position
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

  // Ball out of bounds
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

// Function to choose action for Q-learning
int chooseAction(int stateX, int stateY) {
  if (random(0, 100) < explorationRate * 100) {
    return random(0, 3);  // Explore
  } else {
    float upValue = qTable[stateX][stateY][ACTION_UP];
    float downValue = qTable[stateX][stateY][ACTION_DOWN];
    float stayValue = qTable[stateX][stateY][ACTION_STAY];
    if (upValue > downValue && upValue > stayValue) return ACTION_UP;
    if (downValue > upValue && downValue > stayValue) return ACTION_DOWN;
    return ACTION_STAY;
  }
}

// Function to update Q-table
void updateQTable(int stateX, int stateY, int action, float reward, int nextStateX, int nextStateY) {
  float oldQ = qTable[stateX][stateY][action];
  float bestNextQ = max(max(qTable[nextStateX][nextStateY][ACTION_UP], qTable[nextStateX][nextStateY][ACTION_DOWN]), qTable[nextStateX][nextStateY][ACTION_STAY]);
  qTable[stateX][stateY][action] = oldQ + learningRate * (reward + discountFactor * bestNextQ - oldQ);
}

// AI paddle control with Q-learning
void aiPaddleControl() {
  int stateX = aiPaddleY / 10;  // Discretize AI paddle position
  int stateY = ballY / 10;      // Discretize ball position

  int action = chooseAction(stateX, stateY);

  // Move AI paddle
  if (action == ACTION_UP && aiPaddleY > 0) aiPaddleY -= 2;
  if (action == ACTION_DOWN && aiPaddleY < tft.height() - paddleHeight) aiPaddleY += 2;

  // Reward: 1 for intercepting the ball, -1 for missing
  int reward = (ballX > tft.width() / 2 && abs(ballY - aiPaddleY) < paddleHeight) ? 1 : -1;
  int nextStateX = aiPaddleY / 10;
  int nextStateY = ballY / 10;

  updateQTable(stateX, stateY, action, reward, nextStateX, nextStateY);
}

void setup() {
  Serial.begin(115200);
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

  // Setup game
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
  delay(50);  // Adjust speed of the game
}
