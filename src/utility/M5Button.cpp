#include "M5Button.h"


// Button class

/* static */ std::vector<Button*> Button::instances;

Button::Button(
  int16_t x_, int16_t y_, int16_t w_, int16_t h_,
  bool rot1_ /* = false */,
  const char* name_ /* = "" */,
  ButtonColors off_ /*= {NODRAW, NODRAW, NODRAW} */,
  ButtonColors on_ /* = {NODRAW, NODRAW, NODRAW} */,
  uint8_t datum_ /* = BUTTON_DATUM */,
  int16_t dx_ /* = 0 */,
  int16_t dy_ /* = 0 */,
  uint8_t r_ /* = 0xFF */
) : Zone(x_, y_, w_, h_) {
	instances.push_back(this);
	strncpy(name, name_, 15);
	_pin = -1;
	_state = false;
	_time = millis();
	changed = false;
	_hold_time = -1;
	_lastChange = _time;
	_pressTime = _time;
	// visual stuff
	off = off_;
	on = on_;
	datum = datum_;
	dx = dx_;
	dy = dy_;
	r =  r_;
	drawZone = this;
	strncpy(label, name_, 16);
}

Button::Button(uint8_t pin, uint8_t invert, uint32_t dbTime) : Zone(0, 0, 0, 0) {
	instances.push_back(this);
	hardware(pin, invert, dbTime);
	strncpy(name, "", 15);
	// visual stuff
	off = on = {NODRAW, NODRAW, NODRAW};
	datum = BUTTON_DATUM;
	r =  0xFF;
	drawZone = this;
	strncpy(label, "", 16);
}

Button::~Button() {
	for(int i = 0; i < instances.size(); ++i) {
		if (instances[i] == this) {
			EVENTS->delHandlers(nullptr, this, nullptr);
			instances.erase(instances.begin() + i);
			return;
		}
	}
}

int16_t Button::instanceIndex() {
	for(int16_t i = 0; i < instances.size(); ++i) {
		if (instances[i] == this) return i;
	}
	return -1;
}	

void Button::hardware(uint8_t pin, uint8_t invert, uint32_t dbTime) {
	_pin = pin;
	_invert = invert;
	_dbTime = dbTime;
	pinMode(_pin, INPUT_PULLUP);
	_state = digitalRead(_pin);
	_state = _invert ? !_state : _state;
	_time = millis();
	changed = false;
	_hold_time = -1;
	_lastChange = _time;
	_pressTime = _time;
}

bool Button::read() {
	if (_pin == 0xFF) return _state;
	changed = false;
	_time = millis();
	uint8_t pinVal = digitalRead(_pin);
	pinVal = _invert ? !pinVal: pinVal;
	if (_time - _lastChange >= _dbTime) setState(pinVal);
	return _state;
}

bool Button::setState(bool newState) {
	if (_state != newState) {
		_state = newState;
		_lastChange = _time;
		changed = true;
		if (_state) draw(on); else draw(off);
		if (_state) _pressTime = _time;
	}
	return _state;
}

bool Button::isPressed() { return _state; }

bool Button::isReleased() { return !_state; }

bool Button::wasPressed() { return _state && changed; }

bool Button::wasReleased() {
	return (!_state && changed && millis() - _pressTime < _hold_time);
}

bool Button::wasReleasefor(uint32_t ms) {
	_hold_time = ms;
	return (!_state && changed && millis() - _pressTime >= ms);
}

bool Button::pressedFor(uint32_t ms) {
	Serial.println(_time);
	return (_state && _time - _lastChange >= ms);
}

bool Button::pressedFor(uint32_t ms, uint32_t continuous_time) {
	if (_state && _time - _lastChange >= ms && _time - _lastLongPress >= continuous_time) {
		_lastLongPress = _time;
		return true;
	} 
	return false;
}

bool Button::releasedFor(uint32_t ms) {
	return (!_state && _time - _lastChange >= ms);
}

uint32_t Button::lastChange() { return (_lastChange); }

void Button::addHandler(void (*fn)(Event&), uint16_t eventMask /* = E_ALL */) {
	EVENTS->addHandler(fn, eventMask, this, nullptr);
}

void Button::delHandlers(void (*fn)(Event&) /* = nullptr */) {
	EVENTS->delHandlers(fn, this, nullptr);
}

// visual things for Button

void Button::draw() {
	if (isPressed()) draw(on); else draw(off);
} 

void Button::draw(ButtonColors bc) {
	// use locally set draw function if aplicable, global one otherwise
	if (drawFn) {
		drawFn(this, bc);
	} else if (BUTTONS->drawFn) {
		BUTTONS->drawFn(this, bc);
	}
}

void Button::setLabel(const char* label_) {
	strncpy(label, label_, 50);
}

void Button::setFont(const GFXfont* freeFont_) {
	_freeFont = freeFont_;
	_textFont = 1;
}

void Button::setFont(uint8_t textFont_ /* = 0 */) {
	_freeFont = nullptr;
	_textFont = textFont_;
}

void Button::setTextSize(uint8_t textSize_ /* = 0 */) {
	_textSize = textSize_;
}



// M5Buttons class

/* static */ M5Buttons* M5Buttons::instance;

/* static */ void M5Buttons::drawFunction(Button* button, ButtonColors bc) {
	if (bc.bg == NODRAW && bc.outline == NODRAW && bc.text == NODRAW) return;
	if (!button || !button->drawZone) return;
	Button& b = *button;
	Zone& z = *b.drawZone;
		
	uint8_t r = (b.r == 0xFF) ? min(z.w, z.h) / 4 : b.r;
	
	if (bc.bg != NODRAW) {
		if (r >= 2) {
			TFT->fillRoundRect(z.x, z.y, z.w, z.h, r, bc.bg);
		} else {
			TFT->fillRect(z.x, z.y, z.w, z.h, bc.bg);
		}
	}

	if (bc.outline != NODRAW) {
		if (r >= 2) {
			TFT->drawRoundRect(z.x, z.y, z.w, z.h, r, bc.outline);
		} else {
			TFT->drawRect(z.x, z.y, z.w, z.h, bc.outline);
		}
	}

	if (bc.text != NODRAW && bc.text != bc.bg && b.label != "") {
	
		// figure out where to put the text
		uint16_t tx, ty;
		tx = z.x + (z.w / 2);
		ty = z.y + (z.h / 2);
		
		if (!b.compat) {
			uint8_t margin = max(r / 2, 6);
			switch (b.datum) {
			  case TL_DATUM:
			  case ML_DATUM:
			  case BL_DATUM:
				tx = z.x + margin;
				break;
			  case TR_DATUM:
			  case MR_DATUM:
			  case BR_DATUM:
				tx = z.x + z.w - margin;
				break;
			}
			switch (b.datum) {
			  case TL_DATUM:
			  case TC_DATUM:
			  case TR_DATUM:
				ty = z.y + margin;
				break;
			  case BL_DATUM:
			  case BC_DATUM:
			  case BR_DATUM:
				ty = z.y + z.h - margin;
				break;
			}
		}
		
		// Save state
		uint8_t tempdatum = TFT->getTextDatum();
		uint16_t tempPadding = TFT->padX;
		if (!b.compat) TFT->pushState();
	
		// Actual drawing of text
		TFT->setTextColor(bc.text);
		if (b._textSize) TFT->setTextSize(b._textSize);
		  else TFT->setTextSize(BUTTONS->textSize);
		if (b._textFont) {
			if (b._freeFont) TFT->setFreeFont(b._freeFont); 
			  else TFT->setTextFont(b._textFont);
		} else {
			if (BUTTONS->freeFont) TFT->setFreeFont(BUTTONS->freeFont);
			  else TFT->setTextFont(BUTTONS->textFont);
		}
		TFT->setTextDatum(b.datum);
		TFT->setTextPadding(0);
		TFT->drawString(b.label, tx + b.dx, ty + b.dy);
	
		// Set state back
		if (!b.compat) {
			TFT->popState();
		} else {
			TFT->setTextDatum(tempdatum);
			TFT->setTextPadding(tempPadding);
		}
	}
}

M5Buttons::M5Buttons() {
	if (!instance) instance = this;
	drawFn = drawFunction;
	freeFont = BUTTON_FREEFONT;
	textFont = BUTTON_TEXTFONT;
	textSize = BUTTON_TEXTSIZE;
}

void M5Buttons::setUnchanged() {
	for ( auto button : Button::instances) {
		if (button->_pin == 0xFF) {
			button->changed = false;
			button->_time = millis();
		}
	}
}

Button* M5Buttons::which(Point& p) {
	for ( auto b : Button::instances ) {
		if (b->_pin == 0xFF && b->contains(p)) return b;
	}
	return nullptr;
}

void M5Buttons::draw() {
	for ( auto button : Button::instances ) button->draw();
}

void M5Buttons::setFont(const GFXfont* freeFont_) {
	freeFont = freeFont_;
	textFont = 1;
}

void M5Buttons::setFont(uint8_t textFont_) {
	freeFont = nullptr;
	textFont = textFont_;
}

void M5Buttons::setTextSize(uint8_t textSize_) {
	textSize = textSize_;
}



// Gesture class

std::vector<Gesture*> Gesture::instances;

Gesture::Gesture(
  Zone& fromZone_,
  Zone& toZone_,
  const char* name_ /* = "" */,
  uint16_t maxTime_ /* = 500 */,
  uint16_t minDistance_ /* = 50 */
) {
	fromZone = &fromZone_;
	toZone = &toZone_;
	strncpy(name, name_, 15);
	maxTime = maxTime_;
	minDistance = minDistance_;
	detected = false;
	instances.push_back(this);
}

Gesture::~Gesture() {
	for(int i = 0; i < instances.size(); ++i) {
		if (instances[i] == this) {
			instances.erase(instances.begin() + i);
			EVENTS->delHandlers(nullptr, nullptr, this);
			return;
		}
	}
}

int16_t Gesture::instanceIndex() {
	for(int16_t i = 0; i < instances.size(); ++i) {
		if (instances[i] == this) return i;
	}
	return -1;
}	

bool Gesture::test(Event& e) {
	if (e.duration > maxTime) return false;
	if (e.from.distanceTo(e.to) < minDistance) return false;
	if (!fromZone->contains(e.from) || !toZone->contains(e.to)) return false;
	detected = true;
	return true;
}

bool Gesture::wasDetected() { return detected; }

void Gesture::addHandler(void (*fn)(Event&), uint16_t eventMask /* = E_ALL */) {
	EVENTS->addHandler(fn, eventMask, nullptr, this);
}

void Gesture::delHandlers(void (*fn)(Event&) /* = nullptr */) {
	EVENTS->delHandlers(fn, nullptr, this);
}



// Event class

const char* Event::typeName() {
	const char *unknown = "E_UNKNOWN";
	const char *eventNames[NUM_EVENTS] = {
		"E_TOUCH", 
		"E_RELEASE",
		"E_MOVE",
		"E_GESTURE",
		"E_TAP",
		"E_DBLTAP",
		"E_DRAGGED",
		"E_PRESSED"
	};
	for (uint8_t i = 0; i < NUM_EVENTS; i++) {
		if ((type >> i) & 1) return eventNames[i];
	}
	return unknown;
}

const char* Event::objName() {
	const char *empty = "";
	if (button) return button->name;
	if (gesture) return gesture->name;
	return empty;
};



// M5Events class

M5Events* M5Events::instance;

M5Events::M5Events() {
	if (!instance) instance = this;
}

Event M5Events::fireEvent(uint8_t finger, uint16_t type, Point& from,
  Point& to, uint16_t duration, Button* button, Gesture* gesture) {
	Event e;
	e.finger = finger;
	e.type = type;
	e.from = from;
	e.to = to;
	e.duration = duration;
	e.button = button;
	e.gesture = gesture;
	if (e.button && e.type == E_TOUCH) e.button->setState(true);
	if (e.button && e.type == E_RELEASE) e.button->setState(false);
	for ( auto h : _eventHandlers ) {
		if (!(h.eventMask & e.type)) continue;
		if (h.button && h.button != e.button) continue;
		if (h.gesture && h.gesture != e.gesture) continue;
		if (h.eventMask & E_BTNONLY && !e.button) continue;
		h.fn(e);
	}
	return e;
}

void M5Events::addHandler(
  void (*fn)(Event&),
  uint16_t eventMask /* = E_ALL */,
  Button* button /* = nullptr */,
  Gesture* gesture /* = nullptr */
) {
	EventHandler handler;
	handler.fn = fn;
	handler.eventMask = eventMask;
	handler.button = button;
	handler.gesture = gesture;
	_eventHandlers.push_back(handler);
}

void M5Events::delHandlers(
  void (*fn)(Event&) /* = nullptr */,
  Button* button /* = nullptr */,
  Gesture* gesture /* = nullptr */
) {
	for(int i = _eventHandlers.size() - 1; i >= 0 ; --i) {
		if (fn && fn != _eventHandlers[i].fn) continue;
		if (button && _eventHandlers[i].button != button) continue;
		if (gesture && _eventHandlers[i].gesture != gesture) continue;
		_eventHandlers.erase(_eventHandlers.begin() + i);
	}
}



// TFT_eSPI_Button compatibility mode

TFT_eSPI_Button::TFT_eSPI_Button() : Button(0,0,0,0) {
	compat = true;
}

void TFT_eSPI_Button::initButton(
  TFT_eSPI *gfx, 
  int16_t x, int16_t y, uint16_t w, uint16_t h, 
  uint16_t outline, uint16_t fill, uint16_t textcolor,
  char *label,
  uint8_t textsize
) {
	initButtonUL(gfx, x - (w / 2), y - (h / 2), w, h, outline, fill, 
	  textcolor, label, textsize);
}

void TFT_eSPI_Button::initButtonUL(
  TFT_eSPI *gfx,
  int16_t x_, int16_t y_, uint16_t w_, uint16_t h_, 
  uint16_t outline, uint16_t fill, uint16_t textcolor, 
  char *label_,
  uint8_t textsize_
) {
	x = x_;
	y = y_;
	w = w_;
	h = h_;
	off = {fill, textcolor, outline};
	on = {textcolor, fill, outline};
	setTextSize(textsize_);
	strncpy(label, label_, 9);
}

void TFT_eSPI_Button::setLabelDatum(int16_t dx_, int16_t dy_, uint8_t datum_ /* = MC_DATUM */) {
	dx = dx_;
	dy = dy_;
	datum = datum_;
}

void TFT_eSPI_Button::drawButton(bool inverted /* = false */, String long_name /* = "" */) {
	char oldLabel[51];
	if (long_name != "") {
		strncpy(oldLabel, label, 50);
		strncpy(label, long_name.c_str(), 50);
	}
	draw(inverted ? on : off);
	if (long_name != "") strncpy(label, oldLabel, 50);
}

bool TFT_eSPI_Button::contains(int16_t x, int16_t y) { return Button::contains(x, y); }

void TFT_eSPI_Button::press(bool p) { setState(p); }

bool TFT_eSPI_Button::justPressed() { return wasPressed(); }

bool TFT_eSPI_Button::justReleased() { return wasReleased(); }