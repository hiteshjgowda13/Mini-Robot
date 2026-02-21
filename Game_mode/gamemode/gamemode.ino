//***********************************************************************************************
// ESP32 + SSD1306 + RoboEyes + Touch Menu + Pong Game
//***********************************************************************************************

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

#define TOUCH_PIN 2
#define LONG_PRESS_TIME 4000

#define BTN_UP 4
#define BTN_DOWN 5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RoboEyes<Adafruit_SSD1306> roboEyes(display);

// ----------- STATES -----------
enum State {
  EYES_MODE,
  MENU_MODE,
  MESSAGE_MODE,
  GAME_MODE
};

State currentState = EYES_MODE;

// ----------- TOUCH VARIABLES -----------
unsigned long touchStart = 0;
bool touching = false;
int tapCount = 0;
bool lastTouchState = LOW;

// ----------- MESSAGE TIMER -----------
unsigned long messageStartTime = 0;

// ----------- GAME VARIABLES -----------
int playerY = 22;
int aiY = 22;
const int paddleHeight = 18;
const int paddleWidth = 4;

int ballX = 64;
int ballY = 32;
int ballSpeedX = 3;
int ballSpeedY = 2;

void setup() {

  Serial.begin(115200);

  pinMode(TOUCH_PIN, INPUT);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  delay(250);

  if (!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) {
    while (true);
  }

  display.clearDisplay();
  display.display();

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
}

void loop() {

  switch (currentState) {

    case EYES_MODE:
      roboEyes.update();
      handleLongPress();
      break;

    case MENU_MODE:
      handleMenu();
      break;

    case MESSAGE_MODE:
      showMessage();
      break;

    case GAME_MODE:
      playPong();
      break;
  }
}

// ================= LONG PRESS =================
void handleLongPress() {

  int touchState = digitalRead(TOUCH_PIN);

  if (touchState == HIGH && !touching) {
    touching = true;
    touchStart = millis();
  }

  if (touchState == HIGH && touching) {
    if (millis() - touchStart >= LONG_PRESS_TIME) {
      currentState = MENU_MODE;
      tapCount = 0;
      touching = false;
      lastTouchState = LOW;
      display.clearDisplay();
    }
  }

  if (touchState == LOW) {
    touching = false;
  }
}

// ================= MENU =================
void handleMenu() {

  int touchState = digitalRead(TOUCH_PIN);

  if (lastTouchState == HIGH && touchState == LOW) {
    tapCount++;
    delay(200);
  }

  lastTouchState = touchState;

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.print("1. Pong");

  if (tapCount >= 1) {
    display.setCursor(110, 20);
    display.print(".");
  }

  display.display();

  if (tapCount >= 2) {
    currentState = MESSAGE_MODE;
    messageStartTime = millis();
  }
}

// ================= MESSAGE =================
void showMessage() {

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 28);
  display.print("Use The ears");
  display.display();

  if (millis() - messageStartTime >= 3000) {
    resetGame();
    currentState = GAME_MODE;
  }
}

// ================= RESET GAME =================
void resetGame() {
  playerY = 22;
  aiY = 22;
  ballX = 64;
  ballY = 32;
  ballSpeedX = 3;
  ballSpeedY = 2;
}

// ================= PONG GAME =================
void playPong() {

  display.clearDisplay();

  // Player control
  if (digitalRead(BTN_UP) == LOW) {
    playerY -= 3;
  }

  if (digitalRead(BTN_DOWN) == LOW) {
    playerY += 3;
  }

  if (playerY < 0) playerY = 0;
  if (playerY > SCREEN_HEIGHT - paddleHeight)
    playerY = SCREEN_HEIGHT - paddleHeight;

  // AI control
  int aiCenter = aiY + paddleHeight / 2;

  if (ballY > aiCenter + 5) aiY += 2;
  else if (ballY < aiCenter - 5) aiY -= 2;

  if (aiY < 0) aiY = 0;
  if (aiY > SCREEN_HEIGHT - paddleHeight)
    aiY = SCREEN_HEIGHT - paddleHeight;

  // Ball movement
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  if (ballY <= 0 || ballY >= SCREEN_HEIGHT)
    ballSpeedY = -ballSpeedY;

  // Player collision
  if (ballX <= 8 &&
      ballY >= playerY &&
      ballY <= playerY + paddleHeight) {
    ballSpeedX = -ballSpeedX;
  }

  // AI collision
  if (ballX >= SCREEN_WIDTH - 8 &&
      ballY >= aiY &&
      ballY <= aiY + paddleHeight) {
    ballSpeedX = -ballSpeedX;
  }

  // Reset if missed
  if (ballX < 0 || ballX > SCREEN_WIDTH) {
    resetGame();
  }

  // Draw paddles and ball
  display.fillRect(2, playerY, paddleWidth, paddleHeight, WHITE);
  display.fillRect(SCREEN_WIDTH - 6, aiY, paddleWidth, paddleHeight, WHITE);
  display.fillCircle(ballX, ballY, 3, WHITE);

  display.display();
}