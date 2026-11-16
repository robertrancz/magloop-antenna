/*
 * KP1-8 capacitance tester - Arduino Nano, NO extra parts
 *
 * Wiring: capacitor between A0 and A2 (short leads, <10 cm, kept apart).
 * Method: A2 is driven HIGH; the test capacitor shares its charge with the
 * stray capacitance of A0 to GND (~24.5 pF) and the ADC reads the divided
 * voltage:  C = Cstray * val / (1023 - val).
 * Range ~0.5 ... 1000 pF; resolution ~0.05 pF at 5 pF, ~3 pF at 250 pF.
 *
 * Serial Monitor @ 9600, line ending "Newline":
 *   z        zero  - remove leads/clips capacitance (clips attached, capacitor NOT connected)
 *   c100     calibrate with a known capacitor in pF (e.g. 100 pF NP0/C0G or one measured on the NanoVNA)
 *   r        reset zero + calibration to defaults
 * Zero and calibration are stored in EEPROM.
 */

#include <EEPROM.h>

const uint8_t OUT_PIN = A2;
const uint8_t IN_PIN  = A0;
const int     ADC_MAX = 1023;
const int     SAMPLES = 64;
const float   DEFAULT_STRAY_PF = 24.48;
const uint32_t MAGIC = 0x4B503143;           // "KP1C"

float strayPf  = DEFAULT_STRAY_PF;
float zeroPf   = 0.0;

void saveCal() {
  EEPROM.put(0, MAGIC);
  EEPROM.put(4, strayPf);
  EEPROM.put(8, zeroPf);
}

void loadCal() {
  uint32_t m = 0;
  EEPROM.get(0, m);
  if (m == MAGIC) {
    EEPROM.get(4, strayPf);
    EEPROM.get(8, zeroPf);
  }
}

// Average ADC reading of one charge-share measurement (raw, before zero)
float readRaw() {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    pinMode(IN_PIN, INPUT);
    digitalWrite(OUT_PIN, HIGH);
    int v = analogRead(IN_PIN);
    digitalWrite(OUT_PIN, LOW);
    pinMode(IN_PIN, OUTPUT);                 // A0 LOW -> discharge both sides
    digitalWrite(IN_PIN, LOW);
    sum += v;
    delay(2);
  }
  return (float)sum / SAMPLES;
}

float rawToPf(float val) {
  if (val >= ADC_MAX - 1) return -1;         // out of range (> ~1000 pF or shorted)
  return strayPf * val / (ADC_MAX - val);
}

void setup() {
  pinMode(OUT_PIN, OUTPUT);
  digitalWrite(OUT_PIN, LOW);
  pinMode(IN_PIN, OUTPUT);
  digitalWrite(IN_PIN, LOW);
  Serial.begin(9600);
  loadCal();
  Serial.println(F("KP1-8 capacitance tester. Cmds: z (zero), c<pF> (calibrate), r (reset)"));
  Serial.print(F("Cstray=")); Serial.print(strayPf, 2);
  Serial.print(F(" pF  zero=")); Serial.print(zeroPf, 2); Serial.println(F(" pF"));
}

void handleSerial() {
  if (!Serial.available()) return;
  char buf[12];
  int n = Serial.readBytesUntil('\n', buf, sizeof(buf) - 1);
  buf[n] = '\0';
  if (n > 0 && buf[n - 1] == '\r') buf[--n] = '\0';
  if (n == 0) return;

  if (buf[0] == 'z') {
    zeroPf = rawToPf(readRaw());
    saveCal();
    Serial.print(F("Zero set: ")); Serial.print(zeroPf, 2); Serial.println(F(" pF"));
  } else if (buf[0] == 'c') {
    float known = atof(buf + 1);
    float val = readRaw();
    float ratio = val / (ADC_MAX - val);
    float zeroRatio = zeroPf / strayPf;      // leads, as a fraction of Cstray
    if (known <= 0 || val < 1 || val >= ADC_MAX - 1 || ratio <= zeroRatio) {
      Serial.println(F("Calibration failed (connect the known cap, give pF, e.g. c100)"));
      return;
    }
    strayPf = known / (ratio - zeroRatio);
    zeroPf = zeroRatio * strayPf;
    saveCal();
    Serial.print(F("Calibrated: Cstray=")); Serial.print(strayPf, 2); Serial.println(F(" pF"));
  } else if (buf[0] == 'r') {
    strayPf = DEFAULT_STRAY_PF;
    zeroPf = 0;
    saveCal();
    Serial.println(F("Reset to defaults"));
  }
}

void loop() {
  handleSerial();
  float c = rawToPf(readRaw());
  if (c < 0) {
    Serial.println(F("OVER RANGE / SHORT - check plates are not touching"));
  } else {
    c -= zeroPf;
    Serial.print(F("C = "));
    Serial.print(c, c < 20 ? 2 : 1);
    Serial.println(F(" pF"));
  }
  delay(400);
}
