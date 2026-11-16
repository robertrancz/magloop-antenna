/*
 * Magnetic Loop Antenna Motorized Tuner
 * Hardware: Arduino Nano + TMC2208 + LDO-42STH48-1684MAC (0.9 deg) + Polaron KP1-8 (22.5 turns)
 * Features:
 *  - Fail-safe NC end-stop homing (both switches checked, wiring faults detected)
 *  - Live end-stop protection: any switch trip while homed = hard stop + FAULT (re-home required)
 *  - All motion locked out until homing succeeds
 *  - Interrupt-driven quadrature decoding on D2 + D3 (Paul Stoffregen's Encoder library)
 *  - Coarse / Fine tuning toggle via encoder push-button
 *  - 7 Band Presets saved in EEPROM (40m, 30m, 20m, 17m, 15m, 12m, 10m)
 *  - Full control over USB Serial AND HC-05 Bluetooth (9600 baud), non-blocking parser
 *  - Auto coil de-energize (EN = HIGH) when idle to eliminate heat & RF noise
 *
 * WIRING NOTE: STEP moved D3 -> D7 and ENC_B moved D5 -> D3 so both encoder
 * channels sit on hardware interrupt pins. Add a 10k pull-up from TMC2208 EN to
 * VIO so the driver stays disabled while the Nano is in reset / bootloader.
 */

#include <AccelStepper.h>
#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

// ── Pin Definitions ────────────────────────────────────────────────────────
#define STEP_PIN     7
#define DIR_PIN      4
#define ENABLE_PIN   8

#define ENC_A        2    // INT0
#define ENC_B        3    // INT1
#define ENC_SW       6

#define SW_MIN       9    // MIN end-stop (NC to GND -> LOW normal, HIGH triggered / wire broken)
#define SW_MAX       10   // MAX end-stop (NC to GND -> LOW normal, HIGH triggered / wire broken)

#define BT_RX        11   // Connect to HC-05 TX
#define BT_TX        12   // Connect to HC-05 RX via 1k / 2k voltage divider

// ── Stepper & Capacitor Geometry ───────────────────────────────────────────
// LDO 0.9 deg = 400 full steps/rev.
// TMC2208 standalone: MS1/MS2 have internal pull-downs, so floating == GND == 1/8 step
// (the driver interpolates to 1/256 internally). 1/8 -> 3,200 usteps per motor rev.
// KP1-8 = 22.5 turns -> 22.5 * 3,200 = 72,000 total usteps (direct drive).
const int  MICROSTEPS      = 8;
const long GEAR_RATIO      = 1;                          // 1 = direct drive, 5 = optional 5:1 planetary gearbox
const long STEPS_PER_REV   = 400L * MICROSTEPS * GEAR_RATIO;   // usteps per capacitor shaft turn (3,200 direct)
const bool INVERT_DIR      = false;                      // true = reverse motor direction without rewiring

// MAX_POSITION must sit below the point where the MAX switch trips. Commissioning:
// home, jog up (uuu / coarse encoder) until MAX trips -> FAULT shows the position,
// set MAX_POSITION ~1/2 turn (STEPS_PER_REV / 2) below it, re-flash, re-home.
const long MAX_POSITION    = 67500L * GEAR_RATIO;        // ~21.1 turns (direct drive default)

// Encoder steps per detent (KY-040 outputs 4 quadrature edges per click)
const int  ENC_DIVIDER     = 4;
const int  STEPS_FINE      = 4;                          // 4 usteps/click (~0.45 deg of motor shaft)
const long STEPS_COARSE    = STEPS_PER_REV / 10;         // 1/10 turn of capacitor shaft

// ── Speed & Acceleration ───────────────────────────────────────────────────
const float MAX_SPEED      = 3500.0;                     // usteps/sec (~1.1 rev/sec -> ~20s full sweep; x GEAR_RATIO with gearbox)
const float ACCELERATION   = 2500.0;                     // usteps/sec^2
const float HOMING_SPEED   = 800.0;                      // Slow & safe during homing

// ── Homing Geometry ────────────────────────────────────────────────────────
const long HOMING_MAX_TRAVEL    = STEPS_PER_REV * 25L;   // Give up after 25 turns
// Travelling-nut end-stops: the nut moves one thread pitch per shaft turn (1.5 mm, magloop_endstop.scad).
// A tripped switch must release within this distance (covers the D2F-L movement differential).
const long SWITCH_RELEASE_LIMIT = STEPS_PER_REV;         // 1 turn
const long HOME_BACKOFF         = STEPS_PER_REV / 8;     // Zero = MIN release point + 1/8 turn margin

// ── Band Presets (EEPROM) ──────────────────────────────────────────────────
const int NUM_BANDS = 7;
const char BAND_CODES[NUM_BANDS][3] PROGMEM = {"40", "30", "20", "17", "15", "12", "10"};   // Name = code + "m"
long bandPresets[NUM_BANDS];
int  activeBand = -1;                                    // -1 = Manual / None

const uint32_t EEPROM_MAGIC = 0x4D4C5031;                // "MLP1" signature

// ── Objects ────────────────────────────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

AccelStepper   stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
Encoder        knob(ENC_A, ENC_B);
SoftwareSerial btSerial(BT_RX, BT_TX);

// ── Non-blocking line reader (one per port) ────────────────────────────────
struct LineReader {
  char          buf[12];
  uint8_t       len;
  bool          overflow;
  unsigned long lastCharTime;
};
const unsigned long LINE_IDLE_TIMEOUT = 300;             // Execute un-terminated input after 300 ms silence

LineReader usbLine = {{0}, 0, false, 0};
LineReader btLine  = {{0}, 0, false, 0};

// ── State Variables ────────────────────────────────────────────────────────
bool          coarseMode      = false;
bool          isHomed         = false;
bool          motorsEnabled   = false;
bool          displayOk       = false;
unsigned long lastMoveTime    = 0;
long          lastDetent      = 0;
const __FlashStringHelper* faultMsg = nullptr;

// ── Forward Declarations ───────────────────────────────────────────────────
void homeCapacitor();
void processCommand(char *cmd, Stream &port);
void pollLine(Stream &port, LineReader &lr);
void printHelp(Stream &port);
void loadPresets();
void savePresets();
void updateDisplay();
void enableDriver(bool enable);
void hardStop();
void setFault(const __FlashStringHelper *msg);
bool requestMove(long target);
long detentFromCount(long count);
void printBandName(Print &out, int idx);
bool switchTripped(uint8_t pin);
int  freeRam();

void setup() {
  // Drive EN HIGH (disabled) *before* switching the pin to output, so the driver never glitches on
  digitalWrite(ENABLE_PIN, HIGH);
  pinMode(ENABLE_PIN, OUTPUT);
  motorsEnabled = false;
  stepper.setPinsInverted(INVERT_DIR, false, false);

  Serial.begin(9600);
  btSerial.begin(9600);

  pinMode(SW_MIN, INPUT_PULLUP);
  pinMode(SW_MAX, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  displayOk = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!displayOk) {
    Serial.println(F("WARNING: SSD1306 init failed (not found or out of RAM)"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("MagLoop KP1-8 Tuner"));
    display.println(F("Loading presets..."));
    display.display();
  }

  loadPresets();

  if (displayOk) {
    display.println(F("Homing to MIN..."));
    display.display();
  }

  knob.write(0);
  homeCapacitor();

  printHelp(Serial);
  printHelp(btSerial);
}

void loop() {
  // 1. Live End-Stop Protection.
  //    Zero sits HOME_BACKOFF past the MIN release point and targets are clamped to
  //    0..MAX_POSITION, so a tripped switch while homed means lost steps or a broken wire.
  if (isHomed) {
    bool minHit = switchTripped(SW_MIN);
    bool maxHit = switchTripped(SW_MAX);
    if (minHit || maxHit) {
      hardStop();
      setFault(minHit ? F("MIN end-stop hit - steps lost or wire broken. Re-home.")
                      : F("MAX end-stop hit - steps lost or wire broken. Re-home."));
    }
  }

  // 2. Encoder Push-Button: Short press toggles COARSE/FINE; Long press (1.5s) saves preset
  static unsigned long btnDownTime = 0;
  static bool btnHandled = false;
  if (digitalRead(ENC_SW) == LOW) {
    if (btnDownTime == 0) {
      btnDownTime = millis();
      btnHandled = false;
    } else if (!btnHandled && (millis() - btnDownTime > 1500)) {
      // Bluetooth (SoftwareSerial) output stalls the stepper, so stay silent on BT while moving
      bool btOk = !stepper.isRunning();
      if (activeBand < 0 || activeBand >= NUM_BANDS) {
        Serial.println(F("No active band - recall a band (bXX) before saving."));
        if (btOk) btSerial.println(F("No active band - recall a band (bXX) before saving."));
      } else if (!isHomed || stepper.distanceToGo() != 0) {
        Serial.println(F("Save refused: not homed or still moving."));
        if (btOk) btSerial.println(F("Save refused: not homed or still moving."));
      } else {
        bandPresets[activeBand] = stepper.currentPosition();
        savePresets();
        Serial.print(F("Saved preset "));
        printBandName(Serial, activeBand);
        Serial.println();
        btSerial.print(F("Saved preset "));
        printBandName(btSerial, activeBand);
        btSerial.println();
      }
      btnHandled = true;
    }
  } else {
    if (btnDownTime > 0 && !btnHandled && (millis() - btnDownTime > 40)) {
      coarseMode = !coarseMode;
    }
    btnDownTime = 0;
    btnHandled = false;
  }

  // 3. Rotary Encoder Movement (round to nearest detent so lost edges don't cause jitter)
  long detent = detentFromCount(knob.read());
  if (detent != lastDetent) {
    long clicks = detent - lastDetent;
    lastDetent = detent;
    if (isHomed) {
      long stepSize = coarseMode ? STEPS_COARSE : STEPS_FINE;
      requestMove(stepper.targetPosition() + clicks * stepSize);
    }
  }

  // 4. Serial & Bluetooth Commands (non-blocking)
  pollLine(Serial, usbLine);
  pollLine(btSerial, btLine);

  // 5. Stepper Execution & Auto-Idle Power Down
  if (stepper.distanceToGo() != 0) {
    if (!motorsEnabled) enableDriver(true);
    stepper.run();
    lastMoveTime = millis();
  } else if (motorsEnabled && (millis() - lastMoveTime > 150)) {
    enableDriver(false); // De-energize coils when stopped
  }

  // 6. OLED Refresh only while idle: an I2C frame blocks ~25-30 ms and would stall the stepper
  static unsigned long lastDisplay = 0;
  if (stepper.distanceToGo() == 0 && millis() - lastDisplay >= 150) {
    updateDisplay();
    lastDisplay = millis();
  }
}

// ── Helpers ────────────────────────────────────────────────────────────────
// Noise-filtered end-stop read: a trip must persist across 3 samples (~40 us) so RF or
// stepper-cable spikes can't fake a trip. Costs nothing while the switch reads LOW.
bool switchTripped(uint8_t pin) {
  for (uint8_t i = 0; i < 3; i++) {
    if (digitalRead(pin) == LOW) return false;
    if (i < 2) delayMicroseconds(20);
  }
  return true;
}

int freeRam() {
  extern char __heap_start, *__brkval;
  char top;
  return &top - (__brkval ? __brkval : &__heap_start);
}

void printBandName(Print &out, int idx) {
  out.print(reinterpret_cast<const __FlashStringHelper *>(BAND_CODES[idx]));
  out.print('m');
}

long detentFromCount(long count) {
  // floor((count + 2) / 4): boundaries sit half-way between detents
  long x = count + ENC_DIVIDER / 2;
  return (x >= 0) ? x / ENC_DIVIDER : -((-x + ENC_DIVIDER - 1) / ENC_DIVIDER);
}

// Immediate stop: setCurrentPosition() zeroes speed and sets target = position
void hardStop() {
  stepper.setCurrentPosition(stepper.currentPosition());
}

void setFault(const __FlashStringHelper *msg) {
  isHomed = false;
  faultMsg = msg;
  Serial.print(F("FAULT: "));
  Serial.println(msg);
  btSerial.print(F("FAULT: "));
  btSerial.println(msg);
}

bool requestMove(long target) {
  if (!isHomed) return false;
  stepper.moveTo(constrain(target, 0L, MAX_POSITION));
  return true;
}

// ── Enable / Disable TMC2208 Output Stage ──────────────────────────────────
void enableDriver(bool enable) {
  digitalWrite(ENABLE_PIN, enable ? LOW : HIGH);
  motorsEnabled = enable;
  if (enable) delayMicroseconds(50);
}

// ── Homing Sequence (NC Switch: LOW = normal, HIGH = tripped / open) ───────
static bool homingFail(const __FlashStringHelper *msg) {
  hardStop();
  setFault(msg);
  return false;
}

static bool runHoming() {
  if (switchTripped(SW_MIN) && switchTripped(SW_MAX)) {
    return homingFail(F("Both end-stops open - check wiring."));
  }

  stepper.setCurrentPosition(0);

  // Phase 1: seek MIN. If MAX trips (or stays tripped beyond a short release window)
  // the motor is heading the wrong way -> abort before hitting the capacitor's end.
  bool maxTrippedAtStart = switchTripped(SW_MAX);
  stepper.moveTo(-HOMING_MAX_TRAVEL);
  while (!switchTripped(SW_MIN)) {
    if (switchTripped(SW_MAX)) {
      if (!maxTrippedAtStart || stepper.currentPosition() < -SWITCH_RELEASE_LIMIT) {
        return homingFail(F("MAX hit while homing - DIR reversed?"));
      }
    } else {
      maxTrippedAtStart = false;
    }
    if (stepper.distanceToGo() == 0) {
      return homingFail(F("MIN not found within 25 turns - check switch/wiring."));
    }
    stepper.run();
  }
  hardStop();
  delay(100);

  // Phase 2: back off until MIN releases. A switch that never closes = broken wire.
  stepper.setCurrentPosition(0);
  stepper.moveTo(SWITCH_RELEASE_LIMIT);
  while (switchTripped(SW_MIN)) {
    if (switchTripped(SW_MAX)) {
      return homingFail(F("MAX tripped during MIN back-off."));
    }
    if (stepper.distanceToGo() == 0) {
      return homingFail(F("MIN switch did not release - broken wire?"));
    }
    stepper.run();
  }

  // Phase 3: add safety margin past the release point (MIN bounce ignored here, verified after)
  stepper.moveTo(stepper.currentPosition() + HOME_BACKOFF);
  while (stepper.distanceToGo() != 0) {
    if (switchTripped(SW_MAX)) {
      return homingFail(F("MAX tripped during MIN back-off."));
    }
    stepper.run();
  }
  delay(20);
  if (digitalRead(SW_MIN) == HIGH) {
    return homingFail(F("MIN re-tripped after back-off - check switch."));
  }

  stepper.setCurrentPosition(0);
  return true;
}

void homeCapacitor() {
  isHomed = false;
  faultMsg = nullptr;

  enableDriver(true);

  // If a move is in progress, decelerate to a controlled stop before homing reverses direction
  stepper.stop();
  while (stepper.isRunning()) {
    if (switchTripped(SW_MIN) || switchTripped(SW_MAX)) {
      hardStop();
      break;
    }
    stepper.run();
  }

  stepper.setMaxSpeed(HOMING_SPEED);
  stepper.setAcceleration(ACCELERATION);

  isHomed = runHoming();

  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCELERATION);
  enableDriver(false);
  lastMoveTime = millis();

  // Discard any encoder clicks made while homing was blocking
  lastDetent = detentFromCount(knob.read());
}

// ── Command Input (USB Serial & Bluetooth) ─────────────────────────────────
void pollLine(Stream &port, LineReader &lr) {
  while (port.available()) {
    char c = port.read();
    lr.lastCharTime = millis();
    if (c == '\n' || c == '\r') {
      if (lr.len > 0 && !lr.overflow) {
        lr.buf[lr.len] = '\0';
        processCommand(lr.buf, port);
      }
      lr.len = 0;
      lr.overflow = false;
    } else if (lr.len < sizeof(lr.buf) - 1) {
      lr.buf[lr.len++] = c;
    } else {
      lr.overflow = true;
    }
  }

  // Some BT terminal apps send no line ending: execute after a short silence
  if (lr.len > 0 && millis() - lr.lastCharTime > LINE_IDLE_TIMEOUT) {
    if (!lr.overflow) {
      lr.buf[lr.len] = '\0';
      processCommand(lr.buf, port);
    }
    lr.len = 0;
    lr.overflow = false;
  }
}

static int bandIndex(const char *code) {
  for (int i = 0; i < NUM_BANDS; i++) {
    if (strcmp_P(code, BAND_CODES[i]) == 0) return i;
  }
  return -1;
}

static void replyNotHomed(Stream &port) {
  port.println(F("Not homed - send 'home' first."));
}

static void moveCmd(long target, Stream &port) {
  if (!requestMove(target)) replyNotHomed(port);
}

void processCommand(char *cmd, Stream &port) {
  // Trim whitespace and lowercase in place
  while (*cmd == ' ' || *cmd == '\t') cmd++;
  int len = strlen(cmd);
  while (len > 0 && (cmd[len - 1] == ' ' || cmd[len - 1] == '\t')) cmd[--len] = '\0';
  if (len == 0) return;
  for (int i = 0; i < len; i++) cmd[i] = tolower(cmd[i]);

  // SoftwareSerial TX blocks all interrupts ~1 ms per char, which stalls the stepper.
  // While the motor is running, Bluetooth replies are cut down to a few characters.
  bool brief = (&port == &btSerial) && stepper.isRunning();

  if (strcmp_P(cmd, PSTR("h")) == 0 || strcmp_P(cmd, PSTR("?")) == 0) {
    if (brief) port.println(F("Busy"));
    else printHelp(port);
  } else if (strcmp_P(cmd, PSTR("home")) == 0) {
    port.println(F("Homing..."));
    if (displayOk) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println(F("Homing to MIN..."));
      display.display();
    }
    homeCapacitor();
    if (isHomed) port.println(F("Homed OK."));
    else port.println(F("Homing FAILED - see FAULT message."));
  } else if (strcmp_P(cmd, PSTR("stop")) == 0) {
    stepper.stop();                                      // Controlled deceleration
    port.println(brief ? F("OK") : F("Stopping."));
  } else if (strcmp_P(cmd, PSTR("p")) == 0) {
    if (requestMove(0)) port.println(brief ? F("OK") : F("Parking to 0..."));
    else replyNotHomed(port);
  } else if (strcmp_P(cmd, PSTR("u")) == 0) {
    moveCmd(stepper.targetPosition() + STEPS_FINE, port);
  } else if (strcmp_P(cmd, PSTR("d")) == 0) {
    moveCmd(stepper.targetPosition() - STEPS_FINE, port);
  } else if (strcmp_P(cmd, PSTR("uu")) == 0) {
    moveCmd(stepper.targetPosition() + STEPS_COARSE, port);
  } else if (strcmp_P(cmd, PSTR("dd")) == 0) {
    moveCmd(stepper.targetPosition() - STEPS_COARSE, port);
  } else if (strcmp_P(cmd, PSTR("uuu")) == 0) {
    moveCmd(stepper.targetPosition() + STEPS_PER_REV, port);
  } else if (strcmp_P(cmd, PSTR("ddd")) == 0) {
    moveCmd(stepper.targetPosition() - STEPS_PER_REV, port);
  } else if (cmd[0] == 'b' && bandIndex(cmd + 1) >= 0) {
    // Recall band preset: b40, b30, b20, b17, b15, b12, b10
    int idx = bandIndex(cmd + 1);
    if (requestMove(bandPresets[idx])) {
      activeBand = idx;
      if (brief) { port.println(F("OK")); return; }
      port.print(F("Recalling "));
      printBandName(port, idx);
      port.print(F(" -> "));
      port.println(constrain(bandPresets[idx], 0L, MAX_POSITION));
    } else {
      replyNotHomed(port);
    }
  } else if (cmd[0] == 's' && bandIndex(cmd + 1) >= 0) {
    // Save current position to band preset: s40, s30, s20, s17, s15, s12, s10
    int idx = bandIndex(cmd + 1);
    if (!isHomed) {
      replyNotHomed(port);
    } else if (stepper.distanceToGo() != 0) {
      port.println(brief ? F("Busy") : F("Save refused: still moving."));
    } else {
      activeBand = idx;
      bandPresets[idx] = stepper.currentPosition();
      savePresets();
      port.print(F("Saved "));
      printBandName(port, idx);
      port.print(F(" = "));
      port.println(bandPresets[idx]);
    }
  } else {
    // Direct numeric position jump (max 7 digits -> no long overflow, covers gearbox range)
    bool isNumeric = (len <= 7);
    for (int i = 0; isNumeric && i < len; i++) {
      if (!isDigit(cmd[i])) isNumeric = false;
    }
    if (!isNumeric) {
      port.println(brief ? F("?") : F("Unknown command - send 'h' for help."));
      return;
    }
    long pos = atol(cmd);
    if (pos > MAX_POSITION) {
      if (brief) { port.println(F("?")); return; }
      port.print(F("Out of range (0.."));
      port.print(MAX_POSITION);
      port.println(F(")"));
    } else if (requestMove(pos)) {
      activeBand = -1;
      if (brief) { port.println(F("OK")); return; }
      port.print(F("Moving to "));
      port.println(pos);
    } else {
      replyNotHomed(port);
    }
  }
}

void printHelp(Stream &port) {
    port.println(F("=== MagLoop KP1-8 Ready ==="));
    port.print(F("Pos: ")); port.print(stepper.currentPosition());
    port.print(F(" / ")); port.print(MAX_POSITION);
    port.println(isHomed ? F("  [HOMED]") : F("  [NOT HOMED]"));
    if (faultMsg) { port.print(F("FAULT: ")); port.println(faultMsg); }
    port.println(F("Cmds: u/d (fine), uu/dd (coarse), uuu/ddd (1 rev), p (park), stop, home"));
    port.print(F("Goto: 0..")); port.print(MAX_POSITION); port.println(F(" (absolute usteps)"));
    port.println(F("Bands: b40..b10 (recall), s40..s10 (save current pos)"));
    port.print(F("Free RAM: ")); port.println(freeRam());
}

// ── EEPROM Preset Storage ──────────────────────────────────────────────────
void loadPresets() {
    uint32_t magic = 0;
    EEPROM.get(0, magic);
    if (magic == EEPROM_MAGIC) {
        for (int i = 0; i < NUM_BANDS; i++) {
            EEPROM.get(sizeof(uint32_t) + i * sizeof(long), bandPresets[i]);
        }
    } else {
        // Sensible initial spread across 0..67500 (40m is high cap, 10m is low cap)
        long defaults[NUM_BANDS] = {52000L, 34000L, 18000L, 12000L, 8500L, 5500L, 3000L};
        for (int i = 0; i < NUM_BANDS; i++) bandPresets[i] = defaults[i] * GEAR_RATIO;
        savePresets();
    }
}

void savePresets() {
    EEPROM.put(0, EEPROM_MAGIC);
    for (int i = 0; i < NUM_BANDS; i++) {
        EEPROM.put(sizeof(uint32_t) + i * sizeof(long), bandPresets[i]);
    }
}

// ── OLED Status Screen ─────────────────────────────────────────────────────
void updateDisplay() {
    if (!displayOk) return;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("MagLoop "));
    if (!isHomed) {
        display.print(faultMsg ? F("[FAULT]") : F("[!HOME]"));
    } else if (activeBand >= 0) {
        display.print(F("["));
        printBandName(display, activeBand);
        display.print(F("]"));
    } else {
        display.print(F("[OK]"));
    }

    // Percentage across top right
    int pct = map(constrain(stepper.currentPosition(), 0L, MAX_POSITION), 0L, MAX_POSITION, 0, 100);
    display.setCursor(98, 0);
    display.print(pct);
    display.print(F("%"));

    // Large current step readout
    display.setTextSize(2);
    display.setCursor(0, 18);
    display.print(stepper.currentPosition());

    // Bottom status lines
    display.setTextSize(1);
    display.setCursor(0, 44);
    display.print(F("Step: "));
    display.print(coarseMode ? F("COARSE (1/10)") : F("FINE"));
    display.setCursor(0, 55);
    if (isHomed) {
        display.print(F("Tgt: "));
        display.print(stepper.targetPosition());
    } else {
        display.print(F("NO HOME"));
    }
    if (digitalRead(SW_MIN) == HIGH) display.print(F(" [MIN!]"));
    if (digitalRead(SW_MAX) == HIGH) display.print(F(" [MAX!]"));
    display.display();
}
