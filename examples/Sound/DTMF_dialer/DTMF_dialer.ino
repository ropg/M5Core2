#include <M5Core2.h>

Synth row_gen;
Synth col_gen;

const uint16_t rowTones[4] = { 697,  770,  852,  941};
const uint16_t colTones[4] = {1209, 1336, 1477, 1633};
const char* keyLabels[16]  = {"1", "2", "3", "A",
                              "4", "5", "6", "B",
                              "7", "8", "9", "C",
                              "*", "0", "#", "D"};
ButtonColors offColors = {BLUE, WHITE, WHITE};
ButtonColors onColors  = {WHITE, BLACK, NODRAW};
Button key[16];

uint32_t rotationLastChecked = 0;
uint8_t columns = 4;

void setup() {
	M5.begin();
  M5.Buttons.pianoMode = true;
  M5.IMU.Init();
  row_gen.gain  = col_gen.gain   = 0.3;
  row_gen.decay = col_gen.decay = 50;   // min tone length
  doButtons();
  M5.Buttons.addHandler(btnPressed , E_TOUCH  );
  M5.Buttons.addHandler(btnReleased, E_RELEASE);
}


void loop() {
	M5.update();
  if (checkRotation(1000)) doButtons();
}

void doButtons() {
  M5.Buttons.setFont(FSSB18);
  uint8_t margin = 6;
  uint16_t scr_w = M5.Lcd.width();
  uint16_t scr_h = M5.Lcd.height();
  uint8_t  btn_w = (scr_w / columns) - margin;
  uint8_t  btn_h = (scr_h / 4) - margin;

  // Show or hide the last column
  for (uint8_t i = 3; i < 16; i += 4) {
     if (columns == 3) key[i].hide(); else key[i].show();
  }

  // Set up all the keys
  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < columns; c++) {
      uint8_t i = (r * 4) + c;
      key[i].userData = i + 1;
      key[i].x = c * (scr_w / columns) + (margin / 2);
      key[i].y = r * (scr_h / 4) + (margin / 2);
      key[i].w = btn_w;
      key[i].h = btn_h;
      key[i].setLabel(keyLabels[i]);
      key[i].off = offColors;
      key[i].on  =  onColors;
    }
  }

  // Cosmetics: "*" char in font too small and too high
  key[12].setFont(FSSB24);
  key[12].dy = 8;

  M5.Buttons.draw();
}

void btnPressed(Event& e) {
  Button& btn = *e.button;
  if (!btn.userData) return;
  M5.Sound.waitForSilence();
  row_gen.freq = rowTones[(btn.userData - 1) / 4];
  col_gen.freq = colTones[(btn.userData - 1) % 4];
  row_gen.start();
  col_gen.start();
}

void btnReleased(Event& e) {
  row_gen.stop();
  col_gen.stop();
}

bool checkRotation(uint16_t msec) {
  if (millis() - rotationLastChecked < msec) return false;
  rotationLastChecked = millis();
  const float threshold = 0.85;
  float ax, ay, az;
  M5.IMU.getAccelData(&ax, &ay, &az);
  uint8_t newRotation;
  if      (ay >  threshold) newRotation = 1;
  else if (ay < -threshold) newRotation = 3;
  else if (ax >  threshold) newRotation = 2;
  else if (ax < -threshold) newRotation = 0;
  else return false;
  if (M5.Lcd.rotation == newRotation) return false;
  columns = newRotation % 2 ? 4 : 3;
  M5.Lcd.clearDisplay();
  M5.Lcd.setRotation(newRotation);
  return true;
}
