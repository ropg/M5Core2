#include <M5Core2.h>

Synth a(SINE, 1000);

Button playButton(50, 80, 220, 80, false, "1000 Hz", {YELLOW, BLACK, NODRAW}, {BLACK, YELLOW, NODRAW});

void setup() {
	M5.begin();
  playButton.setFont(FSSB24);
  playButton.draw();
}

void loop() {
	M5.update();
  if ( playButton.wasPressed()  ) a.start();
  if ( playButton.wasReleased() ) a.stop();
}
