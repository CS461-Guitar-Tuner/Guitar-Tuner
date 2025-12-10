# Methods

This section describes how the guitar tuner was implemented, including the hardware configuration, software architecture, pitch detection logic, user interface design, and motor automation system. The goal of this section is to provide enough detail that another person could reproduce the prototype using the same or similar components.

---

## Hardware Overview and System Architecture

The system is split across two microcontrollers:

- **Adafruit Circuit Playground Classic (CPC)** — main controller  
- **Adafruit Circuit Playground Bluefruit (CPB)** — motor control

This separation was intentional. The Classic handles time-sensitive audio sampling, pitch detection, user interface logic, and display updates. The Bluefruit handles motor control and PWM generation, isolating high-current motor behavior from audio processing.

---

## Pin Assignments and Wiring

### Circuit Playground Classic (Main Controller)

**Audio Input**
- External electret microphone connected to:
  - `MIC_PIN = A4`

**OLED Display (SSD1331, SPI)**
- `OLED_MOSI = 2`
- `OLED_CLK  = 3`
- `OLED_CS   = 10`
- `OLED_DC   = 9`
- `OLED_RST  = 0`

Communication with the display is handled over SPI using the `Adafruit_SSD1331` and `Adafruit_GFX` libraries.

**Speaker Output**
- `SPEAKER_PIN = 1`
Used to generate confirmation beeps and reference tones via `tone()`.

**Motor Control Signals (to Bluefruit)**
- `MOTOR_DIR_PIN = 12` — motor direction  
- `MOTOR_EN_PIN  = 6`  — motor enable/disable  

These pins send simple digital signals to the Bluefruit board.

**User Input**
- Built-in **left** and **right** buttons on the Circuit Playground:
  - Right button: cycle strings or modes
  - Left button: confirm selection, return to menu
  - Both buttons held: power on/off

---

### Circuit Playground Bluefruit (Motor Controller)

**Inputs from Classic**
- `DIR_IN = A5`
- `EN_IN  = A6`

**Motor Driver Outputs**
- `AIN1 = A1` (PWM)
- `AIN2 = A2` (PWM)
- `SLP  = A3` (sleep/enable pin)

The Bluefruit reads direction and enable signals from the Classic and converts them into PWM signals that drive the DC motor through a motor driver board.

---

## Software Libraries Used

### Circuit Playground Classic
- `math.h`  
  Used for signal processing math such as square roots and logarithms for RMS and cents calculations.

- `Adafruit_CircuitPlayground.h`  
  Simplifies access to buttons, analog inputs, and audio output.

- `SPI.h`  
  Required for SPI communication with the OLED display.

- `Adafruit_SSD1331.h`  
  Display driver for the OLED.

- `Adafruit_GFX.h`  
  Higher-level graphics support for text and drawing primitives.

- `Fonts/TomThumb.h`  
  A very small bitmap font chosen to conserve flash memory, which is limited on the Classic.

### Circuit Playground Bluefruit
- `Adafruit_TinyUSB.h`  
  Provides USB and Serial support for debugging. Motor control itself uses only basic GPIO and PWM functionality.

We referenced Adafruit example code for OLED initialization and Circuit Playground basics, but all tuning logic, state machines, motor control signaling, and multi-mode behavior were implemented by our team.

---

## Guitar String Representation

The six standard guitar strings are stored in a structured array:

struct Note {
  const char* name;
  float freq;
};


<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>Bill of Materials</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
</head>
<body>
  <h1>Bill of Materials</h1>
  <p><strong>Total budget: $150</strong></p>

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
        <td><a href="https://www.digikey.com/en/products/detail/hammond-manufacturing/BH4AAW/3869836">4xAA Battery Holder</a></td>
        <td>Digi-Key</td>
        <td>1</td>
        <td>$4.65</td>
      </tr>
      <tr>
        <td><a href="https://learn.adafruit.com/adafruit-circuit-playground-bluefruit">Circuit Playground Bluefruit</a></td>
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
        <td><strong>Total</strong></td>
        <td>—</td>
        <td>—</td>
        <td><strong>$77.35</strong></td>
      </tr>
    </tbody>
  </table>
</body>
</html>
