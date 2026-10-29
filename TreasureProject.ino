const int NUM_OBJ = 5;
const int NUM_PILLARS = 5;

const int TOUCH_PIN = 2;

const int PILLAR_LED[5]   = {   4,   7,  10,  A0,  A3 };
const int PILLAR_HOLD[5]  = {   5,   8,  11,  A1,  A4 };
const int PILLAR_QTE[5]   = {   6,   9,  12,  A2,  A5 };

const int HOLD_PILLAR[5]  = {   0, 4, 3, 2, 1 };
const int QTE_PILLAR[5]   = {   2, 1, 0, 4, 3 };
const char OBJ_NAME[5]    = { 'A', 'B', 'C', 'D', 'E' };

const unsigned long QTE_WINDOW   = 3000; 
const int           QTE_TARGET   = 3;  
const unsigned long REBOOT_DELAY = 5000; 
const unsigned long DEBOUNCE_MS  = 30;   

enum State { ARMED, HACKED };

struct Objective {
  State state;
  int qteCount;
  unsigned long qteStart;
  unsigned long hackedAt;
  int ledPin;
  int holdPin;
  int qtePin;
};

struct Btn {
  bool stable;          
  bool raw;
  unsigned long changedAt;
};

Objective obj[NUM_OBJ];
Btn holdBtn[NUM_OBJ];
Btn qteBtn[NUM_OBJ];

bool alarmIsPlaying = false;
bool lastTouchState = HIGH;

bool updateBtn(bool &stable, bool &raw, unsigned long &changedAt, int pin, unsigned long now) {
  bool r = digitalRead(pin);
  if (r != raw) {
    raw = r;
    changedAt = now;
  }
  if (stable != raw && (now - changedAt) >= DEBOUNCE_MS) {
    stable = raw;
    return (stable == LOW);
  }
  return false;
}

void initBtn(bool &stable, bool &raw, unsigned long &changedAt, int pin) {
  raw = digitalRead(pin);
  stable = raw;
  changedAt = 0;
}

void setup() {
  Serial.begin(9600);

  pinMode(TOUCH_PIN, INPUT_PULLUP);

  for (int i = 0; i < NUM_OBJ; i++) {
    obj[i].ledPin  = PILLAR_LED[HOLD_PILLAR[i]];
    obj[i].holdPin = PILLAR_HOLD[HOLD_PILLAR[i]];
    obj[i].qtePin  = PILLAR_QTE[QTE_PILLAR[i]];

    pinMode(obj[i].ledPin, OUTPUT);
    pinMode(obj[i].holdPin, INPUT_PULLUP);
    pinMode(obj[i].qtePin, INPUT_PULLUP);

    obj[i].state = ARMED;
    obj[i].qteCount = 0;
    obj[i].qteStart = 0;
    obj[i].hackedAt = 0;
    
    initBtn(holdBtn[i].stable, holdBtn[i].raw, holdBtn[i].changedAt, obj[i].holdPin);
    initBtn(qteBtn[i].stable, qteBtn[i].raw, qteBtn[i].changedAt, obj[i].qtePin);
  }

  Serial.println("ARDUINO_READY");
}

void loop() {
  unsigned long now = millis();
  
  int touchState = digitalRead(TOUCH_PIN);
  if (touchState == LOW && lastTouchState == HIGH) {
    Serial.println("TOUCH_PRESSED");
  } else if (touchState == HIGH && lastTouchState == LOW) {
    Serial.println("TOUCH_RELEASED");
  }
  lastTouchState = touchState;

  bool alarmNeeded = false;

  for (int i = 0; i < NUM_OBJ; i++) {
    bool qtePressed = updateBtn(qteBtn[i].stable, qteBtn[i].raw, qteBtn[i].changedAt, obj[i].qtePin, now);
    updateBtn(holdBtn[i].stable, holdBtn[i].raw, holdBtn[i].changedAt, obj[i].holdPin, now);

    Objective &o = obj[i];

    if (o.state == ARMED) {
      if (qtePressed) {
        if (o.qteCount == 0 || (now - o.qteStart) > QTE_WINDOW) {
          o.qteCount = 0;
          o.qteStart = now;
        }
        o.qteCount++;

        if (o.qteCount >= QTE_TARGET) {
          o.state = HACKED;
          o.hackedAt = now;
          o.qteCount = 0;
          Serial.print("QTE_");
          Serial.print(OBJ_NAME[i]);
          Serial.println("_SUCCESS");
        }
      }
    } else {
      if ((now - o.hackedAt) >= REBOOT_DELAY) {
        o.state = ARMED;
        o.qteCount = 0;
        Serial.print("OBJ_");
        Serial.print(OBJ_NAME[i]);
        Serial.println("_REARMED");
      }
    }

    digitalWrite(o.ledPin, o.state == ARMED ? HIGH : LOW);

    if (o.state == ARMED && holdBtn[i].stable == HIGH) {
      alarmNeeded = true;
    }
  }

  if (alarmNeeded && !alarmIsPlaying) {
    Serial.println("PLAY_AUDIO_BUTTON_B_RELEASED");
    alarmIsPlaying = true;
  } else if (!alarmNeeded && alarmIsPlaying) {
    Serial.println("STOP_ALARM");
    alarmIsPlaying = false;
  }

  delay(5);
}
