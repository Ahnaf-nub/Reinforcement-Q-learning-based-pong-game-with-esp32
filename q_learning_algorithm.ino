// Q-learning variables
#define ACTION_UP 0
#define ACTION_DOWN 1
#define ACTION_STAY 1

float qTable[8][8][3] = { 0 };  // State space [AI position][Ball position][Action]
float learningRate = 0.2;
float discountFactor = 0.95;
float explorationRate = 0.2;  // Probability of exploration

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