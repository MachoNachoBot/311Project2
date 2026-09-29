const int ledPins[2] = {2, 3};
unsigned long blinkDuration[2] = {0, 0};
unsigned long lastUpdate[2] = {0, 0};
int currentOut[2] = {LOW, LOW};

char rxBuffer[24];
int rxIndex = 0;
int targetLed = 0;
int promptStage = 0;


void updateSingleLed(int i, unsigned long now) {
  if (blinkDuration[i] > 0 && (now - lastUpdate[i] >= blinkDuration[i])) {
    lastUpdate[i] = now;
    currentOut[i] = (currentOut[i] == LOW) ? HIGH : LOW;
    digitalWrite(ledPins[i], currentOut[i]);
  }
}

void processCommand(long parsedVal, unsigned long now) {
  if (promptStage == 0) {
    if (parsedVal == 1 || parsedVal == 2) {
      targetLed = parsedVal - 1;
      promptStage = 1;
      Serial.println("What interval (in msec)?");
    } else {
      Serial.println("Invalid selection. What LED? (1 or 2)");
    }
  } else {
    if (parsedVal > 0) {
      unsigned long halfTime = parsedVal / 2;
      if (halfTime == 0) {
        halfTime = 1;
      }
      
      blinkDuration[targetLed] = halfTime;
      lastUpdate[targetLed] = now;
      currentOut[targetLed] = LOW;
      digitalWrite(ledPins[targetLed], LOW);
      
      promptStage = 0;
      Serial.println("What LED? (1 or 2)");
    } else {
      Serial.println("Invalid time. What interval (in msec)?");
    }
  }
}


void taskLed1(unsigned long now) {
  updateSingleLed(0, now);
}

void taskLed2(unsigned long now) {
  updateSingleLed(1, now);
}

void taskSerial(unsigned long now) {
  if (Serial.available() > 0) {
    char incoming = Serial.read();

    if (incoming == '\n' || incoming == '\r') {
      if (rxIndex > 0) {
        rxBuffer[rxIndex] = '\0';
        long parsedVal = atol(rxBuffer);
        rxIndex = 0;
        processCommand(parsedVal, now);
      }
    } else if (rxIndex < (int)sizeof(rxBuffer) - 1) {
      rxBuffer[rxIndex++] = incoming;
    }
  }
}


typedef void (*TaskFunc)(unsigned long);
const TaskFunc taskTable[] = {
  taskLed1,
  taskLed2,
  taskSerial
};
const int NUM_TASKS = sizeof(taskTable) / sizeof(taskTable[0]);

void setup() {
  for (int i = 0; i < 2; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  
  Serial.begin(9600);
  Serial.println("What LED? (1 or 2)");
}


void loop() {
  unsigned long now = millis();
  for (int i = 0; i < NUM_TASKS; i++) {
    taskTable[i](now);
  }
}