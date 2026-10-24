const int TOUCH_PIN = 2;      
const int LED_MAIN_PIN = 3;   
const int BUTTON_A_PIN = 6;   
const int BUTTON_B_PIN = 7;   
const int LED_B_PIN = 5;      

int qteCount = 0;
unsigned long qteStartTime = 0;
const int targetPresses = 3;         
const unsigned long qteWindow = 3000; 

bool isSafe = false;                  
unsigned long safeStartTime = 0;
const unsigned long safeDuration = 4000; 

bool lastButtonAState = HIGH;
bool lastTouchState = HIGH; 
bool lastButtonBState = HIGH; 

void setup() {
  Serial.begin(9600);
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  
  pinMode(LED_MAIN_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);

  Serial.println("ARDUINO_READY");
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. CEK TOUCH SENSOR
  int touchState = digitalRead(TOUCH_PIN);
  if (touchState == LOW && lastTouchState == HIGH) {
    digitalWrite(LED_MAIN_PIN, HIGH);
    Serial.println("TOUCH_PRESSED"); 
  } else if (touchState == HIGH && lastTouchState == LOW) {
    digitalWrite(LED_MAIN_PIN, LOW);
    Serial.println("TOUCH_RELEASED");
  }
  lastTouchState = touchState;

  // 2. LOGIKA QTE (Button A)
  bool currentButtonA = digitalRead(BUTTON_A_PIN);
  if (currentButtonA == LOW && lastButtonAState == HIGH) { 
    if (qteCount == 0) {
      qteStartTime = currentMillis; 
    }
    qteCount++;
  }
  lastButtonAState = currentButtonA;

  if (qteCount >= targetPresses) {
    isSafe = true;
    safeStartTime = currentMillis;
    qteCount = 0;
    Serial.println("QTE_SUCCESS");
  }

  if (qteCount > 0 && (currentMillis - qteStartTime > qteWindow)) {
    Serial.println("QTE_FAILED");
    qteCount = 0; 
  }

  // 3. LOGIKA STATUS AMAN & TIMER RE-ARMED
  if (isSafe) {
    digitalWrite(LED_B_PIN, LOW); // LED B mati (Aman / Sistem Mati Sesaat)
    if (currentMillis - safeStartTime > safeDuration) {
      isSafe = false; // Sistem RE-ARMED (Hidup Lagi)
      Serial.println("SYSTEM_RE-ARMED");
      
      // Tepat saat re-armed, cek fisik Button B: apakah pemain sedang melepasnya?
      int checkB = digitalRead(BUTTON_B_PIN);
      if (checkB == HIGH) { // HIGH artinya dilepas
        Serial.println("PLAY_AUDIO_BUTTON_B_RELEASED");
      }
    }
  } else {
    digitalWrite(LED_B_PIN, HIGH); // LED B menyala (Sistem Aktif)
  }

  // 4. CEK BUTTON B (Hold Button - Menggunakan logika aslimu yang sudah sempurna)
  int buttonBState = digitalRead(BUTTON_B_PIN);
  
  // Jika tombol dilepas DAN sistem aktif (!isSafe)
  if (buttonBState == HIGH && lastButtonBState == LOW && !isSafe) {
    Serial.println("PLAY_AUDIO_BUTTON_B_RELEASED");
  }
  
  // Jika tombol ditekan kembali (menghentikan alarm)
  if (buttonBState == LOW && lastButtonBState == HIGH) {
    Serial.println("STOP_ALARM");
  }
  
  lastButtonBState = buttonBState;
  
  delay(10); 
}