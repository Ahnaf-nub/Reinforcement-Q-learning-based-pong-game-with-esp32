# Pong Game with Q-Learning AI on ESP32
This project is a modern take on the classic Pong game, implemented on an ESP32 microcontroller with an ST7735 TFT display (80x160 pixels) and an MPU6050 gyro sensor for paddle control. The game features a Q-learning-based AI opponent that learns and improves its gameplay over time.

### Features:
**Classic Pong Gameplay: Enjoy the timeless two-player Pong experience with a human player vs. AI.**
**Q-Learning AI: The AI opponent uses Q-learning to adapt and improve its strategy based on the game's progress, providing an evolving challenge.**
**Real-Time Paddle Control: Use the MPU6050 gyro sensor to control your paddle by tilting the device, offering an intuitive and interactive gaming experience.**

### Hardware Requirements:
1. **ESP32 microcontroller**
2. **ST7735 TFT display (80x160 pixels) 0.96 inch**
3. **MPU6050 gyro sensor**
Getting Started:
Clone the Repository:
```
git clone https://github.com/Ahnaf-nub/Reinforcement-Q-learning-based-pong-game-with-esp32.git
```
Set Up the Hardware: Connect the ST7735 display and MPU6050 sensor to your ESP32 as per the provided wiring diagram.
Upload the Code: Open the project in the Arduino IDE, configure the correct board settings for ESP32, and upload the code.
Play the Game: Use the MPU6050 to control your paddle and try to outscore the AI!
