/*
 * Arduino Nano + ST7735 0.96寸TFT屏 弹球程序
 *
 * 引脚连接：
 *   SCL  -> D9
 *   SDA  -> D10
 *   RES  -> D11
 *   DC   -> D12
 *   CS   -> D13
 *   蜂鸣器 -> A3
 *
 * 功能：
 *   - 屏幕四周红色边框
 *   - 白色球体在边框内弹跳
 *   - 碰到边框反弹并发出"哔"声
 *   - 左上角黄色字体显示碰撞计数
 *   - 计数到100归零重新开始
 */

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// ---- 引脚定义（软件SPI）----
#define TFT_SCLK  9
#define TFT_MOSI  10
#define TFT_RST   11
#define TFT_DC    12
#define TFT_CS    13

#define BUZZER_PIN A3

// ---- BGR屏幕颜色定义（红蓝通道交换）----
#define COLOR_RED     0x001F
#define COLOR_YELLOW  0x07FF
#define COLOR_WHITE   0xFFFF
#define COLOR_BLACK   0x0000
#define COLOR_GREEN   0x07E0

// 软件SPI初始化
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// ---- 屏幕参数（旋转后横屏）----
#define SCREEN_W  160
#define SCREEN_H  80

// ---- 边框与球体参数 ----
#define BORDER_W  2       // 边框宽度（像素）
#define OFFSET_X  1       // 边框右移1像素
#define OFFSET_Y  4       // 边框下移4像素
#define BALL_R    3       // 球半径（像素）
#define BALL_SPEED_X  2   // 水平速度
#define BALL_SPEED_Y  1   // 垂直速度

// ---- 运行区域边界（考虑偏移）----
#define MIN_X  (OFFSET_X + BORDER_W + BALL_R)
#define MAX_X  (SCREEN_W - BORDER_W - BALL_R - 1)
#define MIN_Y  (OFFSET_Y + BORDER_W + BALL_R)
#define MAX_Y  (SCREEN_H - BORDER_W - BALL_R - 1)

// ---- 全局变量 ----
int ballX, ballY;         // 球心坐标
int ballVX, ballVY;       // 球速度
int prevX, prevY;          // 上一帧球心位置
int hitCount = 0;          // 碰撞计数

// ---- 计数显示区域 ----
#define COUNT_X  (OFFSET_X + BORDER_W + 3)
#define COUNT_Y  (OFFSET_Y + BORDER_W + 3)
#define COUNT_W  30
#define COUNT_H  9

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  // 初始化 0.96 寸 160x80 ST7735 屏幕
  tft.initR(INITR_MINI160x80);
  tft.setRotation(3);               // 横屏，根据实际方向可改为 1
  tft.fillScreen(COLOR_BLACK);

  // 画边框
  drawBorder();

  // 球体初始位置：屏幕中央
  ballX = SCREEN_W / 2;
  ballY = SCREEN_H / 2;
  ballVX = BALL_SPEED_X;
  ballVY = BALL_SPEED_Y;
  prevX = ballX;
  prevY = ballY;

  // 显示初始计数
  drawCount();

  // 画初始球体
  tft.fillCircle(ballX, ballY, BALL_R, COLOR_WHITE);
}

void loop() {
  // 1. 擦除上一帧的球（用黑色覆盖）
  tft.fillCircle(prevX, prevY, BALL_R, COLOR_BLACK);

  // 2. 更新位置
  ballX += ballVX;
  ballY += ballVY;

  // 3. 碰撞检测与反弹
  bool hit = false;

  if (ballX <= MIN_X) {
    ballX = MIN_X;
    ballVX = -ballVX;
    hit = true;
  } else if (ballX >= MAX_X) {
    ballX = MAX_X;
    ballVX = -ballVX;
    hit = true;
  }

  if (ballY <= MIN_Y) {
    ballY = MIN_Y;
    ballVY = -ballVY;
    hit = true;
  } else if (ballY >= MAX_Y) {
    ballY = MAX_Y;
    ballVY = -ballVY;
    hit = true;
  }

  // 4. 碰撞处理：计数 + 蜂鸣
  if (hit) {
    hitCount++;
    if (hitCount >= 100) {
      hitCount = 0;
    }
    beep();
    drawCount();
  }

  // 5. 画新位置的球
  tft.fillCircle(ballX, ballY, BALL_R, COLOR_WHITE);

  // 6. 如果旧球位置靠近边框，修补可能被擦除的边框
  if (prevX - BALL_R <= OFFSET_X + BORDER_W + 1 || prevX + BALL_R >= SCREEN_W - BORDER_W - 2 ||
      prevY - BALL_R <= OFFSET_Y + BORDER_W + 1 || prevY + BALL_R >= SCREEN_H - BORDER_W - 2) {
    drawBorder();
  }

  // 7. 记录当前位置，供下一帧擦除
  prevX = ballX;
  prevY = ballY;

  delay(30);   // 约 33 FPS
}

// 画红色边框（带偏移）
void drawBorder() {
  for (int i = 0; i < BORDER_W; i++) {
    tft.drawRect(OFFSET_X + i, OFFSET_Y + i,
                 SCREEN_W - OFFSET_X - 2 * i,
                 SCREEN_H - OFFSET_Y - 2 * i, COLOR_RED);
  }
}

// 蜂鸣器发声：1kHz，持续50ms
void beep() {
  tone(BUZZER_PIN, 1000, 50);
}

// 左上角显示碰撞计数
void drawCount() {
  tft.fillRect(COUNT_X, COUNT_Y, COUNT_W, COUNT_H, COLOR_BLACK);
  tft.setCursor(COUNT_X, COUNT_Y);
  tft.setTextColor(COLOR_YELLOW);
  tft.setTextSize(1);
  tft.print(hitCount);
}
