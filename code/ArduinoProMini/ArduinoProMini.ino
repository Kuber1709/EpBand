#define LED_PIN 7
#define ZUM_PIN 6
#define BUTTON_PIN 5


#include "Wire.h"
#include <EEPROM.h>
#include <MPU6050.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <GyverButton.h>
#include <SoftwareSerial.h>


MPU6050 mpu;
MAX30105 particleSensor;
SoftwareSerial mySerial(3, 2);
GButton button(BUTTON_PIN, LOW_PULL);


struct Settings {
  bool sound_flag;
  bool light_flag;
  char clicks[5];
};
Settings settings;

struct Information {
  String history[50]  = {"19.03.2025-21:40", "15.03.2025-12:42", "16.03.2025-19:20"};
  byte pulse = 70;
};
Information information;

const long shakeThreshold = 9000;
const unsigned long interval = 4000;
const int threshold = 5;
unsigned long timestamps[threshold];
int index = 0;
unsigned long mpuTimer = 0;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

char inputSequence[5];
int inputIndex = 0;
unsigned long lastPressTime = 0;
const unsigned long resetTimeout = 4000;

unsigned long previousMillis = 0;
bool ledState = LOW;  

unsigned long wTimer = 0;
bool button_flag = 0;
bool pulse_flag = 1; // датчик пульса временно не работает, по умолчанию - пульс повышен
bool mpu_flag = 0;
bool w_flag = 0;
bool send_flag = 0;


bool checkPattern() {
  for (int i = 0; i < 5; i++) if (inputSequence[i] != settings.clicks[i]) return false;
  return true;
}


void check_button() {
  button.tick();

  if (button.isClick()) {
    if (inputIndex < 5) inputSequence[inputIndex++] = '.';
    lastPressTime = millis();
  }
  if (button.isHolded()) {
    if (inputIndex < 5) inputSequence[inputIndex++] = '_';
    lastPressTime = millis();
  }

  if (millis() - lastPressTime > resetTimeout && inputIndex > 0) {
    inputIndex = 0;
    memset(inputSequence, 0, sizeof(inputSequence));
  }

  if (inputIndex == 5) {
    if (checkPattern()) button_flag = 1;

    inputIndex = 0;
    memset(inputSequence, 0, sizeof(inputSequence));
  }
}


void check_mpu() {
  if (millis() - mpuTimer >= 100) {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    long totalAccel = abs(ax) + abs(ay) + abs(az);
    Serial.println(totalAccel);
    if (totalAccel - shakeThreshold > 0) {
      timestamps[index] = millis();
      byte cnt = 0;
      for (byte i = 0; i < threshold; i++) if (millis() - timestamps[i] <= interval) cnt++;
      if (cnt >= threshold) mpu_flag = 1;
    }
    mpuTimer = millis();
  }
}


void check_pulse() {
  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);
    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++) {
        beatAvg += rates[x];
      }
      beatAvg /= RATE_SIZE;                           
    }
  }
  information.pulse = beatsPerMinute;
  if (information.pulse > 90) pulse_flag = 1;
  else pulse_flag = 0;
}


void setup() {
  EEPROM.get(0, settings);
  mySerial.begin(115200);
  delay(100);
  mySerial.write('s');
  mySerial.write((byte*)&settings, sizeof(settings));

  Wire.begin();
  mpu.initialize();
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);

  particleSensor.begin(Wire, I2C_SPEED_FAST);
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0); 

  button.setDebounce(50);
  button.setTimeout(300);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);
  pinMode(ZUM_PIN, OUTPUT);
  digitalWrite(ZUM_PIN, 0);

  Serial.begin(9600);
}


void loop() {
  if (w_flag) {
    if (millis() - wTimer > 10000) {
      if (send_flag) {
      mySerial.write('w');
      send_flag = 0;
    }
    if (millis() - previousMillis >= 500) {
      previousMillis = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }}
    check_button();
    if (button_flag) {
      mpu_flag = 0;
      w_flag = 0;
      ledState = 0;
      digitalWrite(LED_PIN, ledState);
    }
  }
  else if (!mpu_flag) check_mpu();
  else if (pulse_flag) {
    send_flag = 1;
    w_flag = 1;
    wTimer = millis();
    ledState = 1;
    digitalWrite(LED_PIN, ledState);
  }
  // check_pulse();
  read_esp();
}


void read_esp() {
  char c = mySerial.read();
  if (c == 's') {
    mySerial.readBytes((byte*)&settings, sizeof(settings));
    mySerial.readString();
    EEPROM.put(0, settings);
  }
  else if (c == 'i') {
    mySerial.write((byte*)&information, sizeof(information));
    Serial.println(c);
  }
}

