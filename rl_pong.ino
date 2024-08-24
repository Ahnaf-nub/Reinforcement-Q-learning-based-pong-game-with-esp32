#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define TFT_CS D3
#define TFT_RST D8
#define TFT_DC D7
#define TFT_MOSI D9   // Data out
#define TFT_SCLK D10  // Clock out

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

float qTable[8][8][3] = { 0 };  // Simplified state space and action space
float learningRate = 0.2;
float discountFactor = 0.99;

float epsilon = 1.0;         // Initial epsilon value for exploration
float epsilonDecay = 0.995;  // Decay rate for epsilon
float epsilonMin = 0.01;     // Minimum epsilon value

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
  if (ballY <= 0 || ballY >= tft.height()) ballDirY = -ballDirY;

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

// Function to choose action for Q-learning with epsilon decay
int chooseAction(int stateX, int stateY) {
  if (random(0, 100) / 100.0 < epsilon) {
    return random(0, 3); // Explore: Choose a random action
  } else {
    // Exploit: Choose the action with the highest Q-value
    if (qTable[stateX][stateY][ACTION_UP] > qTable[stateX][stateY][ACTION_DOWN]) {
      return qTable[stateX][stateY][ACTION_UP] > qTable[stateX][stateY][ACTION_STAY] ? ACTION_UP : ACTION_STAY;
    } else {
      return qTable[stateX][stateY][ACTION_DOWN] > qTable[stateX][stateY][ACTION_STAY] ? ACTION_DOWN : ACTION_STAY;
    }
  }
}

// Function to update Q-table
void updateQTable(int stateX, int stateY, int action, float reward, int nextStateX, int nextStateY) {
  float oldQ = qTable[stateX][stateY][action];
  float bestNextQ = max(qTable[nextStateX][nextStateY][ACTION_UP], max(qTable[nextStateX][nextStateY][ACTION_DOWN], qTable[nextStateX][nextStateY][ACTION_STAY]));
  qTable[stateX][stateY][action] = oldQ + learningRate * (reward + discountFactor * bestNextQ - oldQ);
}

// AI paddle control with Q-learning
void aiPaddleControl() {
  int stateX = aiPaddleY / 10;  // Discretize the AI paddle position
  int stateY = ballY / 10;      // Discretize the ball position

  int action = chooseAction(stateX, stateY);

  // Move AI paddle based on the chosen action
  if (action == ACTION_UP && aiPaddleY > 0) aiPaddleY -= 2;
  if (action == ACTION_DOWN && aiPaddleY < tft.height() - paddleHeight) aiPaddleY += 2;
  if (action == ACTION_STAY) ;  // Stay in the same position

  // Calculate reward
  int reward = (ballX > tft.width() / 2 && abs(ballY - aiPaddleY) < paddleHeight) ? 1 : -1;

  // Get the next state
  int nextStateX = aiPaddleY / 10;
  int nextStateY = ballY / 10;

  // Update Q-table
  updateQTable(stateX, stateY, action, reward, nextStateX, nextStateY);
}

// Setup function
void setup() {
  Serial.begin(115200);
  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);

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

  // Setup game
  setupGame();
}

// Main loop
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

  // Decay epsilon
  epsilon = max(epsilon * epsilonDecay, epsilonMin);

  delay(50);  // Adjust speed of the game
}
