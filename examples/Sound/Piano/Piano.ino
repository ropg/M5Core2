/*

Polyphonic synthesizer. Lingering notes will continue to sound as new notes
are played. If you place one finger high on the screen and another low, you
can even play two notes at the same time. As you can see you can have many
synths going at the same time.

*/

#include <M5Core2.h>

#define WKW         40
#define WKH        240
#define BKW         30
#define BKH        160
#define WHITE_KEY  {WHITE, NODRAW, BLACK}
#define BLACK_KEY  {BLACK, NODRAW, NODRAW}
#define NUM_KEYS    14

float notes[NUM_KEYS] = { NOTE_F4 , NOTE_G4 , NOTE_A4 , NOTE_B4 , NOTE_C5,
                          NOTE_D5 , NOTE_E5 , NOTE_F5 , NOTE_Gb4, NOTE_Ab4,
                          NOTE_Bb4, NOTE_Db5, NOTE_Eb5, NOTE_Gb5 };

// (waveform, freq, attack, decay, sustain, release)
Synth synth[NUM_KEYS] = Synth(TRIANGLE, 0, 50, 300, 0.7, 1000);

// Black keys have to come last so they end up on top
Button f_(   0, 0, WKW, WKH, false, "f" , WHITE_KEY);
Button g_(  40, 0, WKW, WKH, false, "g" , WHITE_KEY);
Button A_(  80, 0, WKW, WKH, false, "A" , WHITE_KEY);
Button B_( 120, 0, WKW, WKH, false, "B" , WHITE_KEY);
Button C_( 160, 0, WKW, WKH, false, "C" , WHITE_KEY);
Button D_( 200, 0, WKW, WKH, false, "D" , WHITE_KEY);
Button E_( 240, 0, WKW, WKH, false, "E" , WHITE_KEY);
Button F_( 280, 0, WKW, WKH, false, "F" , WHITE_KEY);
Button gb(  25, 0, BKW, BKH, false, "gb", BLACK_KEY);
Button Ab(  65, 0, BKW, BKH, false, "Ab", BLACK_KEY);
Button Bb( 105, 0, BKW, BKH, false, "Bb", BLACK_KEY);
Button Db( 185, 0, BKW, BKH, false, "Db", BLACK_KEY);
Button Eb( 225, 0, BKW, BKH, false, "Eb", BLACK_KEY);
Button Gb( 305, 0,  15, BKH, false, "Gb", BLACK_KEY);

void setup() {
  M5.begin();
  M5.Buttons.draw();

  // Prettier with top of keys as straight line
  M5.Lcd.fillRect(0, 0, 320, 5, BLACK);

  // Trick to make sure buttons do not draw over eachother anymore
  M5.Buttons.drawFn = nullptr;

  // So that you can swipe from one button to another
  M5.Buttons.pianoMode = true;

  // Set up syths with their notes
  for (uint8_t n = 0; n < 14; n++) {
    synth[n].freq = notes[n];
  }

  M5.Buttons.addHandler(pressKey  , E_TOUCH);
  M5.Buttons.addHandler(releaseKey, E_RELEASE);
}

void loop() {
  M5.update();
}

void pressKey(Event& e) {
  // instanceIndex() -4 because of background, BtnA, BtnB and BtnC.
  uint8_t key = e.button->instanceIndex() - 4;
  synth[key].start();
}


void releaseKey(Event& e) {
  uint8_t key = e.button->instanceIndex() - 4;
  synth[key].stop();
}
