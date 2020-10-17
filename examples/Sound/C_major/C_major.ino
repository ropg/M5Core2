#include <M5Core2.h>

// Waveform, Frequency, Attack, Decay, Sustain, Release, Gain
Synth c4(SINE, NOTE_C4, 50, 500, 0.7, 1000, 0.8);
Synth c5(SINE, NOTE_C5, 50, 500, 0.7, 1000);
Synth e5(SINE, NOTE_E5, 50, 500, 0.7, 1000);
Synth g5(SINE, NOTE_G5, 50, 500, 0.7, 1000);

Button playButton(50, 80, 220, 80, false, "C major", {YELLOW, BLACK, NODRAW});

void setup() {
  M5.begin();
  playButton.setFont(FSSB24);
  playButton.draw();
}

void loop() {
  M5.update();
  if (playButton.wasPressed()) playChord();
}

void playChord() {
  playButton.hide(BLACK);
  c4.playFor(1000);
  M5.Sound.delay(1000);
  c5.playFor(5000);
  M5.Sound.delay(1000);
  e5.playFor(4000);
  M5.Sound.delay(1000);
  g5.playFor(3000);
  M5.Sound.waitForSilence();
  playButton.show();
}
