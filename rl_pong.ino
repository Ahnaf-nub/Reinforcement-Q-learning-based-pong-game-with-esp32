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

// Score and Missed Balls
int playerScore = 0;
int aiScore = 0;
int playerMissed = 0;
int aiMissed = 0;

// Q-learning variables
#define ACTION_UP 0
#define ACTION_DOWN 1
#define ACTION_STAY 1

float qTable[8][8][3] = { 0 };  // State space [AI position][Ball position][Action]
float learningRate = 0.1;
float discountFactor = 0.99;
float explorationRate = 0.3;  // Probability of exploration

// Function to draw the paddles
void drawPaddle(int x, int y) {
  tft.fillRect(x, y, 2, paddleHeight, ST77XX_RED);
}

// Function to draw the ball
void drawBall(int x, int y) {
  tft.fillRect(x, y, 5, 5, ST77XX_BLUE);
}

// Function to move the ball
void moveBall() {
  tft.fillRect(ballX, ballY, 5, 5, ST77XX_BLACK);  // Erase previous ball position
  ballX += ballDirX;
  ballY += ballDirY;

  // Ball collision with top and bottom
  if (ballY <= 0 || ballY >= tft.height() - 2) ballDirY = -ballDirY;

  // Ball collision with player paddle
  if (ballX <= 2 && ballY >= playerPaddleY && ballY <= playerPaddleY + paddleHeight) {
    ballDirX = -ballDirX;
    playerScore++;
  }

  // Ball collision with AI paddle
  if (ballX >= tft.width() - 4 && ballY >= aiPaddleY && ballY <= aiPaddleY + paddleHeight) {
    ballDirX = -ballDirX;
    aiScore++;
  }

  // Ball out of bounds (player side)
  if (ballX <= 0) {
    ballX = tft.width() / 2;  // Reset ball to center
    ballY = tft.height() / 2;
    ballDirX = -ballDirX;  // Change direction
    playerMissed++;
  }

  // Ball out of bounds (AI side)
  if (ballX >= tft.width()) {
    ballX = tft.width() / 2;  // Reset ball to center
    ballY = tft.height() / 2;
    ballDirX = -ballDirX;  // Change direction
    aiMissed++;
  }

  drawBall(ballX, ballY);
}

// Function to initialize the game
void setupGame() {
  ballX = tft.width() / 2;
  ballY = tft.height() / 2;
  playerPaddleY = tft.height() / 2 - paddleHeight / 2;
  aiPaddleY = tft.height() / 2 - paddleHeight / 2;
  tft.fillScreen(ST77XX_BLACK);
  displayScore();
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

void displayScore() {
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);

  tft.setCursor(0, 0);
  tft.print("P1 Score: ");
  tft.print(playerScore);

  tft.setCursor(0, 10);
  tft.print("P1 Missed: ");
  tft.print(playerMissed);

  tft.setCursor(tft.width() - 60, 0);
  tft.print("AI Score: ");
  tft.print(aiScore);

  tft.setCursor(tft.width() - 60, 10);
  tft.print("AI Missed: ");
  tft.print(aiMissed);
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

  // Clear previous paddles and score area
  tft.fillRect(0, 0, tft.width(), 20, ST77XX_BLACK);
  tft.fillRect(0, 20, 2, tft.height() - 20, ST77XX_BLACK);
  tft.fillRect(tft.width() - 2, 20, 2, tft.height() - 20, ST77XX_BLACK);

  // Draw paddles
  drawPaddle(0, playerPaddleY);
  aiPaddleControl();
  drawPaddle(tft.width() - 2, aiPaddleY);

  moveBall();
  delay(50);  // Adjust speed of the game
}
