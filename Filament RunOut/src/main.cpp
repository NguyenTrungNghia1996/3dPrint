#include <Arduino.h>

#define encoder_pin 14
#define button_pin 13
void ICACHE_RAM_ATTR counter();
void ICACHE_RAM_ATTR button();
/////////////////////
volatile byte pulses = 0;
unsigned long lastmillis = 0;
long Range = 0;
static volatile unsigned long debounce = 0;
static volatile unsigned long debounce_button = 0;
int button_static = 0;

#include <Wire.h>
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include "images.h"
#include "font.h"
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);
OLEDDisplayUi ui(&display);

void counter()
{
  if (digitalRead(encoder_pin) && (micros() - debounce > 500) && digitalRead(encoder_pin))
  {
    debounce = micros();
    pulses++;
  }
}
void button()
{
    button_static++;
}
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 0 + y, "Total used: ");
  display->setFont(DejaVu_Sans_Bold_19);
  display->drawString(5 + x, 12 + y, String(Range) + "mm");
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 0 + y, "This time use: ");
  display->setFont(DejaVu_Sans_Bold_19);
  display->drawString(5 + x, 12 + y, String(Range) + "mm");
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = {drawFrame2, drawFrame3};

// how many frames are there?
int frameCount = 2;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = {msOverlay};
int overlaysCount = 1;

void setup()
{
  Serial.begin(115200);
  pinMode(encoder_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(encoder_pin), counter, CHANGE);
  attachInterrupt(digitalPinToInterrupt(button_pin), button, FALLING);
  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(RIGHT);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_UP);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);
  ui.disableAutoTransition();
  ui.disableAllIndicators();
  ui.init();

  display.flipScreenVertically();
}

void loop()
{
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0)
  {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    if (millis() - lastmillis >= 1000)
    { //Cập nhập trạng thái sau mỗi 1 giây.
      noInterrupts();
      if (button_static > 0)
      {
        ui.nextFrame();
        ui.update();
        button_static = 0;
      }

      long r = pulses * 2.836129032258065;
      Range = Range + r;
      Serial.print("Range =\t"); //xuất ký tự RPM và một TAB
      Serial.println(Range);     // xuất tộc độ quạt
      pulses = 0;                // khởi lộng lại bộ đếm tần số
      lastmillis = millis();     // cập nhập lại thời điểm cuối cùng ta kiểm tra tốc độ
      interrupts();              //Tiếp tục chạy các interrupt
    }
    delay(remainingTimeBudget);
  }
}