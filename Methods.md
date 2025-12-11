# Methods

This section describes how the guitar tuner was implemented, including hardware configuration, software architecture, pitch detection logic, user interaction, and motor automation. The goal is to clearly explain both what the system does and how it does it, so that another person could reproduce the prototype using this document.

---

## Hardware Overview and System Architecture

The system uses two microcontrollers:

- **Adafruit Circuit Playground Classic (CPC)** — main controller
- **Adafruit Circuit Playground Bluefruit (CPB)** — motor controller

The Classic handles microphone sampling, pitch detection, OLED display output, button input, and audio feedback. The Bluefruit is dedicated to controlling the tuning motor using PWM. This split prevents motor noise and timing issues from interfering with audio processing.

---

## Pin Assignments and Wiring

### Circuit Playground Classic

The external electret microphone is connected to analog pin A4:

```cpp
const int MIC_PIN = A4;
```

The OLED display (SSD1331) is connected via SPI:

```cpp
#define OLED_MOSI 2
#define OLED_CLK  3
#define OLED_CS   10
#define OLED_DC   9
#define OLED_RST  0
```

The on-board speaker is driven using:

```cpp
#define SPEAKER_PIN 1
```

Motor control signals sent to the Bluefruit:

```cpp
const int MOTOR_DIR_PIN = 12;   // direction
const int MOTOR_EN_PIN  = 6;    // enable
```

User interaction uses the built-in left and right buttons:
- Right button: cycle strings or modes
- Left button: confirm selection / return
- Both buttons held: power on/off

---

### Circuit Playground Bluefruit

Control inputs from the Classic:

```cpp
#define DIR_IN A5
#define EN_IN  A6
```

Motor driver outputs:

```cpp
#define AIN1 A1
#define AIN2 A2
#define SLP  A3
```

The Bluefruit reads direction and enable signals and converts them into PWM outputs that drive the DC motor through a motor driver.

---

## Software Libraries Used

### Circuit Playground Classic
- `math.h` — RMS calculation and logarithmic cents conversion
- `Adafruit_CircuitPlayground.h` — microphone, buttons, speaker
- `SPI.h` — SPI communication
- `Adafruit_SSD1331.h` — OLED driver
- `Adafruit_GFX.h` — graphics rendering
- `Fonts/TomThumb.h` — compact font to reduce memory usage

### Circuit Playground Bluefruit
- `Adafruit_TinyUSB.h` — USB and serial support

Adafruit examples were referenced for OLED setup and hardware initialization. 

---

## Guitar String Representation

Each guitar string is represented by a name and its target frequency:

```cpp
struct Note {
  const char* name;
  float freq;
};

Note strings[] = {
  {"E2", 82.41},
  {"A2", 110.00},
  {"D3", 146.83},
  {"G3", 196.00},
  {"B3", 246.94},
  {"E4", 329.63}
};
```

The currently selected string is changed using the right button.

---

## Pitch Detection and Autocorrelation Logic

The tuner uses a **time-domain autocorrelation-style algorithm** to estimate pitch. This approach was chosen because it is more robust than simple zero-crossing methods for guitar signals, which contain harmonics and background noise.

### Sampling Strategy

```cpp
const int N_SAMPLES = 250;
int16_t samples[N_SAMPLES];
```

The microphone signal is sampled `N_SAMPLES` times using `analogRead()`. The total sampling time is measured using `micros()` so the effective sample rate can be calculated dynamically rather than assumed.

### How Autocorrelation Is Used

1. **DC Offset Removal**  
   The average of all samples is subtracted so the waveform is centered around zero. This prevents bias from affecting pitch estimation.

2. **Noise Rejection (RMS Gate)**  
   The RMS energy of the signal is computed. If the RMS value is below a threshold, the signal is treated as silence and ignored.

3. **Lag Search (Autocorrelation)**  
   The algorithm searches over a range of delays (lags) corresponding to roughly 60–500 Hz, which covers the guitar’s frequency range.  
   For each lag, it computes the sum of squared differences between the signal and a shifted version of itself. The lag with the smallest error represents the signal’s fundamental period.

4. **Frequency Calculation**  
   Frequency is computed as the sample rate divided by the best lag.

### Autocorrelation Code

```cpp
float measureFrequency() {
  //  Collect samples and measure sample rate
  unsigned long start = micros(); //take the first time measurement
  for (int i = 0; i < N_SAMPLES; i++) {
    samples[i] = analogRead(MIC_PIN);
  }
  unsigned long end = micros(); //take the second time measurement 
 
  float totalTimeSec = (end - start) / 1000000.0; //find the total time ellapsed in seconds 
  if (totalTimeSec <= 0) return 0.0; //mostly a worry if the timer is wrapping around-- just in case
  float sampleRate = N_SAMPLES / totalTimeSec; //gives us samples per second to be used in frequency calc 
 
  //  Remove DC offset and measure signal energy (centering the signal at zero)
  long sum = 0;
  for (int i = 0; i < N_SAMPLES; i++) {
    sum += samples[i]; //tallying up all of the samples so... 
  }
  float mean = sum / (float)N_SAMPLES; //mean is where the zero lies 
  double energy = 0.0; //reminder about what energy 
  for (int i = 0; i < N_SAMPLES; i++) {
    float v = samples[i] - mean;
    samples[i] = (int16_t)v;
    energy += v * v;
  }
  float rms = sqrt(energy / N_SAMPLES);
 
  // Simple noise gate so we don't react to nothing
  if (rms < 5.0) {   // tweak this threshold if needed
    return 0.0;
  }
 
  // Difference function in guitar range
  int minLag = (int)(sampleRate / 500.0); // ~500 Hz upper bound
  int maxLag = (int)(sampleRate / 60.0);  // ~60 Hz lower bound
 
  if (minLag < 2) minLag = 2;
  if (maxLag >= N_SAMPLES - 1) maxLag = N_SAMPLES - 2;
 
  float bestLag = -1;
  unsigned long bestDiff = 0xFFFFFFFF;
 
  for (int lag = minLag; lag <= maxLag; lag++) {
    unsigned long diff = 0;
 
    for (int i = 0; i < N_SAMPLES - lag; i++) {
      int32_t d = (int32_t)samples[i] - (int32_t)samples[i + lag];
      diff += (unsigned long)(d * d);
    }
 
    if (diff < bestDiff) {
      bestDiff = diff;
      bestLag = lag;
    }
  }
 
  if (bestLag <= 0) return 0.0;
 
  float freq = sampleRate / bestLag;
  return freq;
}
 
float centsOff(float measured, float target) {
  return 1200.0 * log(measured / target) / log(2.0);
}
 
const char* tuningStatus(float cents) {
  if (fabs(cents) < 5.0)  return "IN TUNE";
  if (cents < 0)          return "FLAT";
  return "SHARP";
}
```

---

## Cents Error and Tuning Classification

Pitch error is measured in **cents**, making tuning precision consistent across strings:

```cpp
float centsOff(float measured, float target) {
  return 1200.0 * log(measured / target) / log(2.0);
}
```

```cpp
const char* tuningStatus(float cents) {
  if (fabs(cents) < 5.0)  return "IN TUNE";
  if (cents < 0)          return "FLAT";
  return "SHARP";
}
```
In addition to the RMS noise gate, the tuner ignores detected pitches that are too far from the selected string’s target. Two thresholds are used.
```cpp
const float IN_TUNE_CENTS = 20.0;
const float MAX_DEVIATION_CENTS = 80.0;
```

If the absolute cents error exceeds MAX_DEVIATION_CENTS, the reading is discarded. This prevents background noise, adjacent strings, or unrelated sounds from affecting the display or driving the motor.

---

## State Machine and Modes

```cpp
enum MainState { OFF, WELCOME, SELECT, NORMAL, MOTOR, FORK };
```

- **Normal Mode:** visual tuner with OLED and audio feedback
- **Motor Mode:** automatically adjusts tuning peg
- **Fork Mode:** plays reference tone through speaker

### Motor Automation Logic

```cpp
void sendMotorCommand(float cents) {
  const float DEAD_ZONE = IN_TUNE_CENTS;

  if (fabs(cents) < DEAD_ZONE) {
    digitalWrite(MOTOR_EN_PIN, LOW);   // stop motor
    return;
  }

  bool tighten = (cents < 0);  // FLAT => tighten, SHARP => loosen
  digitalWrite(MOTOR_DIR_PIN, tighten ? HIGH : LOW);
  digitalWrite(MOTOR_EN_PIN, HIGH);
}

```
[Main MCU code](mainMCUcode.ino) |
[Helper MCU code](helperMCUcode.ino) |
[View General Pinout](assets/images/tuner_pinout.pdf) |
[View FSM](assets/images/tuner_fsm.pdf)

---

<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
</head>

<body>

  <h2>Bill of Materials</h2>

  <p>
    These are all the components used throughout the project. 
    Alligator clips and connecting wires were provided.
  </p>

  <p><strong>Total budget:</strong> $150</p>

  <table border="1" cellspacing="0" cellpadding="6">
    <thead>
      <tr>
        <th>Item</th>
        <th>Supplier</th>
        <th>Quantity</th>
        <th>Total Cost</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td><a href="https://www.adafruit.com/product/1063">Microphone</a></td>
        <td>Adafruit</td>
        <td>1</td>
        <td>$6.95</td>
      </tr>
      <tr>
        <td><a href="https://www.adafruit.com/product/4227">STEMMA Speaker Amp</a></td>
        <td>Adafruit</td>
        <td>1</td>
        <td>$1.95</td>
      </tr>
      <tr>
        <td><a href="https://www.adafruit.com/product/3777">Micro Servo Motor</a></td>
        <td>Adafruit</td>
        <td>1</td>
        <td>$2.95</td>
      </tr>
      <tr>
        <td><a href="https://www.adafruit.com/product/684">OLED Display</a></td>
        <td>Adafruit</td>
        <td>1</td>
        <td>$29.95</td>
      </tr>
      <tr>
        <td><a href="https://www.digikey.com/en/products/detail/hammond-manufacturing/BH4AAW/3869836">
          4xAA Battery Holder</a></td>
        <td>Digi-Key</td>
        <td>1</td>
        <td>$4.65</td>
      </tr>
      <tr>
        <td><a href="https://learn.adafruit.com/adafruit-circuit-playground-bluefruit">
          Circuit Playground Bluefruit</a></td>
        <td>Adafruit</td>
        <td>1</td>
        <td>$24.95</td>
      </tr>
      <tr>
        <td><a href="https://www.adafruit.com/product/3297">DC Motor Driver</a></td>
        <td>Adafruit</td>
        <td>1</td>
        <td>$5.95</td>
      </tr>
      <tr>
        <td><a href="https://www.adafruit.com/product/3000">Circuit Playground Classic</a></td>
        <td>Adafruit</td>
        <td>1</td>
        <td>$19.95</td>
      </tr>
      <tr>
        <td><strong>Total</strong></td>
        <td>—</td>
        <td>—</td>
        <td><strong>$97.30</strong></td>
      </tr>
    </tbody>
  </table>
</body>
</html>


[Introduction](introduction.md) |
[Methods](Methods.md) |
[Results](results.md) |
[Schedule](schedule.md) |
[Issues](issues.md) |
[Ethics](ethics.md) |
[Accessibility](accessibility.md) |
[References](references.md)


