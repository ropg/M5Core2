#include <M5Core2.h>

#define MAX_INPUT   80
#define MAX_KEYS    30
#define CHAR_WIDTH  14
#define CARET_WIDTH  4
#define INPUT_WIDTH  (CHAR_WIDTH * MAX_INPUT) + CARET_WIDTH
#define INPUT_HEIGHT 25

TFT_eSprite sprite(&M5.Lcd);
TFT_eSprite input(&M5.Lcd);

Button k[MAX_KEYS];
uint8_t k_idx = 0;
String theString = "";
int16_t offset = 0;
int16_t caret = 0;
void (*currentKB)();
int16_t moveOffset, moveStart;
uint32_t topBarLastUpdate = millis();
bool helpBarShown = false;
bool weAreDoneHere = false;

ButtonColors normalOn         = {BLACK, WHITE, NODRAW};
ButtonColors normalOff        = {WHITE, BLACK, BLACK};
ButtonColors specialKeyColors = {LIGHTGREY, BLACK, BLACK};
Zone    topBar   (0,  0, 320,  30);
Zone    keyboard (0, 60, 320, 180);
Gesture backspace(keyboard, keyboard, "bksp" ,  40, DIR_LEFT);
Gesture space    (keyboard, keyboard, "space",  40, DIR_RIGHT);
Gesture help     (topBar  , ANYWHERE, "help" , 120, DIR_DOWN);
Button  sym  (  0, 180,  40, 60, false, "sym" , specialKeyColors);
Button  done (264, 180,  56, 60, false, "done", specialKeyColors);
Button  entry(  0,   0, 320, 60, false, "entry");

void clearKeys() {
  for (int8_t n = 0; n < MAX_KEYS; n++) {
    k[n].off = normalOff;
    k[n].on  = normalOn;
    k[n].dbltapTime = 0;
    k[n].userData = 1;
    k[n].hide();
  }
  k_idx = 0;
}

void placeRow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t num_keys, const char** labels) {
  const uint16_t kw = w / num_keys;
  for (uint8_t n = 0; n < num_keys; n++) {
    k[k_idx].x = x + (n * kw);
    k[k_idx].y = y;
    k[k_idx].w = kw;
    k[k_idx].h = h;
    k[k_idx].setLabel(labels[n]);
    k[k_idx].show();
    k_idx++;
  }
}

// The sprite code draws all the buttons in memory and only then pushes that to
// the display in one go, which prevents flicker while keys are drawn.
void drawButtons() {
  sprite.createSprite(320, 180);
  sprite.fillSprite(BLACK);
  TFT_eSPI* previousDisplay = M5.Buttons.display;
  M5.Buttons.display = &sprite;
  M5.Buttons.dy = -60;
  M5.Buttons.draw();
  sprite.pushSprite(0, 60);
  sprite.deleteSprite();
  M5.Buttons.display = previousDisplay;
  M5.Buttons.dy = 0;
/*
  // This function is the flicker-free equivalent of:
  M5.Lcd.fillRect(0, 60, 320, 180, BLACK);
  M5.Buttons.draw();
*/
}

void defaultKB() {
  currentKB = defaultKB;
  const char* row1[] = { "q", "w", "e", "r", "t", "y", "u", "i", "o", "p" };
  const char* row2[] =   { "a", "s", "d", "f", "g", "h", "j", "k", "l" };
  const char* row3[] =     { "z", "x", "c", "v", "b", "n", "m" };
  clearKeys();
  placeRow(0 ,  60, 320, 60, 10, row1);
  placeRow(16, 120, 288, 60,  9, row2);
  placeRow(40, 180, 224, 60,  7, row3);
  drawButtons();
}

void symbolKB() {
  currentKB = symbolKB;
  const char* row1[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };
  const char* row2[] = { "-", "/", ":", ";", "(", ")", "$", "&", "@", "\"" };
  const char* row3[] =             { ".", ",", "?", "!", "'" };
  clearKeys();
  placeRow(0,   60, 320, 60, 10, row1);
  placeRow(0,  120, 320, 60, 10, row2);
  placeRow(42, 180, 224, 60,  5, row3);
  drawButtons();
}

void shiftSymbolKB() {
  currentKB = shiftSymbolKB;
  const char* row1[] = { "[", "]", "{", "}", "#", "%", "^", "*", "+" };
  const char* row2[] = { "_", "\\", "|", "~", "<", ">", "`", " ", "=" };
  const char* row3[] =             { ".", ",", "?", "!", "'" };
  clearKeys();
  placeRow(0,   60, 320, 60,  9, row1);
  placeRow(0,  120, 320, 60,  9, row2);
  placeRow(42, 180, 224, 60,  5, row3);
  drawButtons();
}

void updateInput() {
  if (caret > theString.length()) caret = theString.length();
  input.fillSprite(BLACK);
  input.drawString(theString, 0, 0);
  input.fillRect((CHAR_WIDTH * caret) - (CARET_WIDTH / 2), 0, CARET_WIDTH, INPUT_HEIGHT, RED);
  input.pushSprite(-offset, 30);
}

void fixOffset() {
  int16_t stringWidth = theString.length() * CHAR_WIDTH;
  int16_t maxOffset = stringWidth - 320 + CARET_WIDTH;
  if (offset > maxOffset) offset = maxOffset;
  if (offset < 0) offset = 0;
}

void insertStr(String str) {
  if (theString.length() >= MAX_INPUT) return;
  theString = theString.substring(0, caret) + str + theString.substring(caret);
  caret++;
  fixOffset();
  if ((caret * CHAR_WIDTH) + (CARET_WIDTH / 2) - offset > 320) offset = (caret * CHAR_WIDTH) - 320 + (CARET_WIDTH / 2);
  updateInput();
}

void keyHandler(Event& e) {
  Button& b = *e.button;
  if (!b.userData) return;
  String t = b.getLabel();
  if (e == E_DRAGGED) {
    if (e.isDirection(DIR_UP   )) t.toUpperCase();
    else return;
  }
  insertStr(t);
}

void spaceHandler(Event& e) {
  insertStr(" ");
  if (currentKB != defaultKB) defaultKB();
}

void backspaceHandler(Event& e) {
  if (caret) {
    theString = theString.substring(0, caret - 1) + theString.substring(caret);
    caret--;
    offset -= CHAR_WIDTH;
    fixOffset();
    updateInput();
  }
}

void symHandler(Event& e) {
  if (e == E_TAP || e == E_PRESSED) {
    if      (currentKB == defaultKB) symbolKB();
    else if (currentKB == symbolKB)  shiftSymbolKB();
    else    defaultKB();
  }
}
void doneHandler(Event& e) {
  if (e == E_TAP || e == E_PRESSED) weAreDoneHere = true;
}

void entryHandler(Event& e) {
  if (e == E_TOUCH) {
    moveOffset = offset;
    moveStart = e.from.x;
    caret = ((offset + e.to.x) / CHAR_WIDTH);
    updateInput();
  }
  if (e == E_MOVE) {
    int16_t stringWidth = theString.length() * CHAR_WIDTH;
    if (e.to.y < 160 && stringWidth > 320) {
      int16_t oldOffset = offset;
      int16_t moveBy = moveStart - e.to.x;
      offset = moveOffset + moveBy;
      fixOffset();
      if (offset != oldOffset) input.pushSprite(-offset, 30);
    } else {
      caret = ((offset + e.to.x) / CHAR_WIDTH);
      updateInput();
    }
  }
}


void showHelpBar() {
  helpBarShown = true;
  M5.Lcd.fillRect(0, 0, 320, 19, TFT_LIGHTGREY);
  M5.Lcd.setTextDatum(TC_DATUM);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setFreeFont(FSS9);
  M5.Lcd.drawString("swipe down from top for help", 160, 1);
}

void showPrompt() {
  helpBarShown = false;
  M5.Lcd.fillRect(0, 0, 320, 19, TFT_LIGHTGREY);
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setFreeFont(FSS9);
  M5.Lcd.drawString("Enter wifi key:", 10, 1);
}

void helpScreen(Event& e) {
  clearKeys();
  sym.hide();
  done.hide();
  M5.Lcd.fillRect(0, 0, 320, 240, BLACK);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.setFreeFont(FSSB24);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.drawString("HELP", 160, 120);
  while (M5.Buttons.event != E_TAP && M5.Buttons.event != E_PRESSED) M5.update();
  sym.show();
  done.show();
  showHelpBar();
  updateInput();
  defaultKB();
}


String keyboardInput() {
  theString = "";
  M5.Lcd.fillScreen(BLACK);
  M5.Buttons.pushState();
  M5.Buttons.setFont(FSSB12);
  M5.Buttons.addHandler(keyHandler, E_TAP + E_PRESSED + E_DRAGGED);
  done.setFont(FSS9);
  done.dbltapTime = 0;
  done.addHandler(doneHandler);
  sym.setFont(FSS9);
  sym.addHandler(symHandler);
  sym.dbltapTime = 0;
  space.addHandler(spaceHandler);
  backspace.addHandler(backspaceHandler);
  help.addHandler(helpScreen);
  entry.addHandler(entryHandler);
  input.setColorDepth(8);
  input.createSprite(INPUT_WIDTH,INPUT_HEIGHT);
  input.setFreeFont(FMB12);
  input.setTextDatum(TL_DATUM);
  input.setTextColor(GREEN, BLACK);
  updateInput();
  showPrompt();
  defaultKB();
  topBarLastUpdate = millis();
  weAreDoneHere = false;

  while (!weAreDoneHere) {
    if (millis() - topBarLastUpdate > 5000) {
      topBarLastUpdate = millis();
      if (helpBarShown) showPrompt(); else showHelpBar();
    }
    M5.update();
  }

  input.deleteSprite();
  M5.Lcd.fillScreen(BLACK);
  M5.Buttons.popState();
  return theString;
}

void setup() {
  M5.begin();

  while (true) {
    Serial.println(keyboardInput());
  }
}

void loop() {
  M5.update();
}
