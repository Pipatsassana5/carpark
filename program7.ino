// ==================== DC Motor ====================
const int motorPinUp[]    = {5, 50, 9, 11, 13};
const int motorPinDown[]  = {4, 52, 8, 10, 12};
const int limitSwitchPinDC[] = {39, 37, 35, 33, 31};
const int numDCMotors = 5;
const unsigned long runTimeDC = 20000;

// ==================== Stepper Motor ====================
const int stepEN[]    = {22, 26, 34, 30};
const int stepDIR[]   = {24, 28, 36, 32};
const int stepPWM[]   = {2, 3, 46, 44};
const int stepLimit[] = {29, 25, 23, 27};
const int numStepMotors = 4;
const unsigned long runTimeStep = 15000;
const int stepDirection[] = {LOW, HIGH, LOW, LOW};

// ==================== Button ====================
const int buttonMain = 45;
const int buttonDC2  = 47;
const int buttonSeq2 = 49;
const int buttonSeq3 = 51;
const int buttonSeq4 = 53;

bool systemBusy = false;   // ป้องกันกดซ้อน

// ==================== Function Startup ====================
void startupSequence() {
  for (int i = 0; i < numDCMotors; i++) {
    digitalWrite(motorPinUp[i], HIGH);
    digitalWrite(motorPinDown[i], LOW);
    unsigned long startTime = millis();
    while (millis() - startTime < runTimeDC && digitalRead(limitSwitchPinDC[i]) == HIGH);
    digitalWrite(motorPinUp[i], LOW);
    digitalWrite(motorPinDown[i], LOW);
    delay(500);
  }

  digitalWrite(stepDIR[0], LOW);
  digitalWrite(stepDIR[1], HIGH);
  digitalWrite(stepDIR[2], LOW);
  digitalWrite(stepDIR[3], LOW);

  for (int i = 0; i < numStepMotors; i++) {
    digitalWrite(stepEN[i], LOW);
    unsigned long startTime = millis();
    while (millis() - startTime < runTimeStep && digitalRead(stepLimit[i]) == HIGH) {
      digitalWrite(stepPWM[i], HIGH); delayMicroseconds(1000);
      digitalWrite(stepPWM[i], LOW);  delayMicroseconds(1000);
    }
    digitalWrite(stepEN[i], HIGH);
    delay(500);
  }
  Serial.println("Startup sequence done.");
}

// ==================== Function Stepper ====================
void runStepper(int motorIndex, unsigned long runTime, int direction) {
  digitalWrite(stepDIR[motorIndex], direction);
  digitalWrite(stepEN[motorIndex], LOW);
  unsigned long startTime = millis();
  while (millis() - startTime < runTime) {
    digitalWrite(stepPWM[motorIndex], HIGH); delayMicroseconds(1000);
    digitalWrite(stepPWM[motorIndex], LOW);  delayMicroseconds(1000);
  }
  digitalWrite(stepEN[motorIndex], HIGH);
}

void runDCMotorDown(int motorIndex, unsigned long runTime) {
  digitalWrite(motorPinUp[motorIndex], LOW);
  digitalWrite(motorPinDown[motorIndex], HIGH);
  unsigned long startTime = millis();
  while (millis() - startTime < runTime);
  digitalWrite(motorPinUp[motorIndex], LOW);
  digitalWrite(motorPinDown[motorIndex], LOW);
}

// ==================== Setup ====================
void setup() {
  Serial.begin(9600);

  for (int i = 0; i < numDCMotors; i++) {
    pinMode(motorPinUp[i], OUTPUT);
    pinMode(motorPinDown[i], OUTPUT);
    pinMode(limitSwitchPinDC[i], INPUT_PULLUP);
  }
  for (int i = 0; i < numStepMotors; i++) {
    pinMode(stepEN[i], OUTPUT);
    pinMode(stepDIR[i], OUTPUT);
    pinMode(stepPWM[i], OUTPUT);
    pinMode(stepLimit[i], INPUT_PULLUP);
    digitalWrite(stepEN[i], HIGH);
    digitalWrite(stepDIR[i], stepDirection[i]);
  }

  pinMode(buttonMain, INPUT_PULLUP);
  pinMode(buttonDC2, INPUT_PULLUP);
  pinMode(buttonSeq2, INPUT_PULLUP);
  pinMode(buttonSeq3, INPUT_PULLUP);
  pinMode(buttonSeq4, INPUT_PULLUP);

  startupSequence();
}

// ==================== Loop ====================
void loop() {
  static unsigned long pressStartTime = 0;
  static bool buttonPressed = false;

  if (!systemBusy) {
    // -------------------- ปุ่ม 45 --------------------
    if (digitalRead(buttonMain) == LOW && !buttonPressed) {
      pressStartTime = millis();
      buttonPressed = true;
    }

    if (digitalRead(buttonMain) == HIGH && buttonPressed) {
      unsigned long pressDuration = millis() - pressStartTime;
      buttonPressed = false;

      if (pressDuration < 5000) {
        // ----- กดแล้วปล่อยทันที -----
        systemBusy = true;
        Serial.println("Main Sequence Start (short press)");
        runStepper(0, 14500, HIGH);
        delay(500);
        runStepper(1, 13000, LOW);
        delay(500);
        runDCMotorDown(0, 6000);
        delay(2000);
        startupSequence();
        Serial.println("Main Sequence Done");
        systemBusy = false;
      }
      else {
        // ----- กดค้าง ≥ 5 วิ → Run ทุกปุ่ม -----
        systemBusy = true;
        Serial.println("Run all sequences (long press)");

        // Seq ปุ่ม 45
        runStepper(0, 14500, HIGH);
        delay(500);
        runStepper(1, 13000, LOW);
        delay(500);
        runDCMotorDown(0, 6000);
        delay(2000);
        startupSequence();

        // Seq ปุ่ม 47
        Serial.println("DC Motor 2 down 12s");
        runDCMotorDown(1, 7000);
        delay(2000);
        startupSequence();

        // Seq ปุ่ม 49
        Serial.println("Stepper3 right 13s, Stepper4 right 12s, DC3 down 12s");
        runStepper(2, 15000, HIGH);
        delay(500);
        runStepper(3, 13000, HIGH);
        delay(500);
        runDCMotorDown(2, 6000);
        delay(2000);
        startupSequence();

        // Seq ปุ่ม 51
        Serial.println("Stepper2 left 12s, DC4 down 5s");
        runStepper(1, 13000, LOW);
        delay(500);
        runDCMotorDown(3, 4000);
        delay(2000);
        startupSequence();

        // Seq ปุ่ม 53
        Serial.println("Stepper4 left 12s, DC5 down 5s");
        runStepper(3, 13000, HIGH);
        delay(500);
        runDCMotorDown(4, 4000);
        delay(2000);
        startupSequence();

        Serial.println("All sequences done.");
        systemBusy = false;
      }
    }

    // -------------------- ปุ่ม 47 --------------------
    else if (digitalRead(buttonDC2) == LOW) {
      systemBusy = true;
      while (digitalRead(buttonDC2) == LOW);
      Serial.println("DC Motor 2 down 12s");
      runDCMotorDown(1, 7000);
      delay(2000);
      startupSequence();
      systemBusy = false;
    }

    // -------------------- ปุ่ม 49 --------------------
    else if (digitalRead(buttonSeq2) == LOW) {
      systemBusy = true;
      while (digitalRead(buttonSeq2) == LOW);
      Serial.println("Stepper3 right 13s, Stepper4 right 12s, DC3 down 12s");
      runStepper(2, 15000, HIGH);
      delay(500);
      runStepper(3, 13000, HIGH);
      delay(500);
      runDCMotorDown(2, 6000);
      delay(2000);
      startupSequence();
      systemBusy = false;
    }

    // -------------------- ปุ่ม 51 --------------------
    else if (digitalRead(buttonSeq3) == LOW) {
      systemBusy = true;
      while (digitalRead(buttonSeq3) == LOW);
      Serial.println("Stepper2 left 12s, DC4 down 5s");
      runStepper(1, 13000, LOW);
      delay(500);
      runDCMotorDown(3, 4000);
      delay(2000);
      startupSequence();
      systemBusy = false;
    }

    // -------------------- ปุ่ม 53 --------------------
    else if (digitalRead(buttonSeq4) == LOW) {
      systemBusy = true;
      while (digitalRead(buttonSeq4) == LOW);
      Serial.println("Stepper4 left 12s, DC5 down 5s");
      runStepper(3, 13000, HIGH);
      delay(500);
      runDCMotorDown(4, 4000);
      delay(2000);
      startupSequence();
      systemBusy = false;
    }
  }
}
