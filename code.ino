/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/*

TABLE OF CONTENTS:
0) DEPENDENCIES
1) DECLARATIONS, HELPER PROTOTYPES, AND GENERAL SET UP 
2) MAJOR FUNCTION PROTOTYPES
3) SET UP
4) LOOP (MAIN)
5) ACTUAL HELPER FUNCTIONS, IN ORDER OF PROTOTYPE APPEARANCE
6) ACTUAL MAJOR FUNCTIONS, IN ORDER OF PROTOTYPE APPEARANCE

*/
/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/* 0 DEPENDENCIES*/

#include <math.h> //for DSP
//#include <Adafruit_TinyUSB.h>
#include <Adafruit_CircuitPlayground.h> //for convinent functions 

/*Libraries related to the display.*/
#include <SPI.h> //the graphics driver library is built on SPI
#include <Adafruit_SSD1331.h> //used for low level Display Functions and GFX is built on this
#include <Adafruit_GFX.h> //high level display library 
#include <Fonts/TomThumb.h> //Fonts for display
//#include <Fonts/FreeMono9pt7b.h> //Fonts for display
/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/* 1 DECLARATIONS, HELPER PROTOTYPES, AND GENERAL SET UP */

        
/*-----------------------TUNER CONFIG AND GUITAR STRING STRUCT---------------------------------------------------------*/
const int MIC_PIN    = A4;     // external mic on A4
const int N_SAMPLES  = 250;   // larger window = more accurate low notes
int16_t samples[N_SAMPLES];

struct Note {
  const char* name;
  float freq;
};
 
Note strings[] = { //These are the pitches and frequencies of the six different guitar strings. 
  {"E2", 82.41},
  {"A2", 110.00},
  {"D3", 146.83},
  {"G3", 196.00},
  {"B3", 246.94},
  {"E4", 329.63}
};

const int NUM_STRINGS = sizeof(strings) / sizeof(strings[0]);
int currentStringIndex = 0;   // which string we’re tuning (0 = E2, ...)
int lastStringDrawn = -1;     // track OLED state
bool rightPrevFORK = false;        // for edge-detect

/*-----------------------OLED PINS AND THROTTLING OLED & SERIAL---------------------------------------------------------*/
#define OLED_CS   10
#define OLED_DC   9
#define OLED_RST  0
#define OLED_MOSI 2    
#define OLED_CLK  3  

Adafruit_SSD1331 oled(OLED_CS, OLED_DC, OLED_MOSI, OLED_CLK, OLED_RST);

char lastNoteName[4]     = "---";
char lastStatus[10]      = "";
int  lastCentsRounded    = 9999;
int  lastHzRounded       = 0;
bool lastHadSignal       = false;
unsigned long lastPrintMs = 0;



 /*-----------------------PITCH/FREQUENCY MATH FUNCTIONS---------------------------------------------------------*/
// how wide the “good enough” band is
const float IN_TUNE_CENTS       = 20.0;   // same as motor mode (you can drop to 10.0 if you want tighter)
// how far we’re willing to accept a reading from the target note at all
const float MAX_DEVIATION_CENTS = 80.0;   // reject anything further away

bool wasInTune = false;            // for in-tune beep edge
float measureFrequency();
float centsOff(float measured, float target);
const char* tuningStatus(float cents);



 /*-----------------------OLED DRAWING HELPERS---------------------------------------------------------*/
void drawNoSignal();
void drawTuningScreen(const char* noteName, int hzRounded, int centsRounded, const char* status);

void drawCurrentNoteScreen(); //used in fork

 /*-----------------------SPEAKER SET UP & AUDIO HELPERS---------------------------------------------------------*/
#define SPEAKER_PIN 1
void playInTuneBeepIfNeeded(bool inTune);
void playCurrentNotePulse();


 /*-----------------------BUTTON SETUP/TOGGLING HELPERS---------------------------------------------------------*/
bool rightPrev = false;            // to be used in handle right button for toggling
void handleRightButton();

bool firstTime = true; //first time going through normal to get screen to display
bool firstTimeMotor = true; //first time going through motor to get screen to display
bool firstTimeFork = true; //first time going through fork to get screen to display



// ---------- MOTOR CONTROL PINS/MOTOR SET UP----------
const int MOTOR_DIR_PIN = 12;   // HIGH = tighten, LOW = loosen //PINS LINKED UP TO BLUE FRUIT
const int MOTOR_EN_PIN  = 6;    // HIGH = motor on, LOW = motor off

void sendMotorCommand(float cents);

/*------------------------------------------------SELECTION SET UP------------------------------------------------*/ 
int selected_Mode = 0; //for selected mode

/*------------------------------------------------MAIN FSM SET UP------------------------------------------------*/ 
enum MainState {OFF, 
WELCOME, 
SELECT, 
NORMAL, 
MOTOR, 
FORK};
MainState state = OFF;
MainState next_state = OFF;


/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/* 2 MAJOR FUNCTION PROTOTYPES*/

void normal();
void motor();
void fork();
void Title_Screen();
void Selection_Mode();


/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/*=======================================================================================================================================================*/
/* 3 SET UP*/

void setup() {
  //for circut playground library
  CircuitPlayground.begin();

  pinMode(LED_BUILTIN, OUTPUT); //for debugging
  pinMode(SPEAKER_PIN, OUTPUT);

  pinMode(MOTOR_DIR_PIN, OUTPUT);
  pinMode(MOTOR_EN_PIN, OUTPUT);
  digitalWrite(MOTOR_DIR_PIN, LOW);
  digitalWrite(MOTOR_EN_PIN, LOW);
 
  //OLED SET UP
  oled.begin();
  oled.fillScreen(0x0000);
  oled.setRotation(0);
  oled.setFont(&TomThumb);
  oled.setTextSize(1);
  oled.setTextColor(0xFFFF);
  oled.setCursor(2, 10);
  //oled.print("Tuner ready");
  oled.setCursor(2, 25);
  //oled.print("Target: ");
  //oled.print(strings[currentStringIndex].name);
  //oled.print("Guitar tuner ready"); ??? 
}

/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/*=======================================================================================================================================================*/
/*4 LOOP (MAIN)*/

void loop() {
  switch(state){
    case OFF:// OFF STATE
        if (onOff()){ //Left button is held for 5 sec, it turns on. 
          next_state = WELCOME;
          
        }
      break;

    case WELCOME://WELCOME STATE
        Title_Screen();
        if (CircuitPlayground.leftButton()){ //Right button advances to the select state. 
          next_state = SELECT;
        }
      break; 

    case SELECT: //SELECT STATE
        if (CircuitPlayground.rightButton()) { 
          selected_Mode++;
          if (selected_Mode > 2){
            selected_Mode = 0;
          }
        }
        Selection_Mode();
        if(CircuitPlayground.leftButton()){ //next state is made base on user choice in this state. 
            if (selected_Mode==0){// Normal
              next_state = NORMAL;
            }else if (selected_Mode==1){// Fork 
              next_state = FORK;
            }else if (selected_Mode==2){//Motor
              next_state = MOTOR;
            }
        }
      break;

    case NORMAL: //MODE ONE: NORMAL STATE
        //digitalWrite(LED_BUILTIN, HIGH);

        normal();
        
        if (CircuitPlayground.leftButton()){
          next_state = SELECT;
        }
      break;

    case MOTOR: //MODE TWO: MOTOR STATE
        motor();
        if (CircuitPlayground.leftButton()){
          next_state = SELECT;
        }
      break;

    case FORK: //MODE THREE: FORK STATE
        fork();
        if (CircuitPlayground.leftButton()){
          next_state = SELECT;
          oled.setTextSize(1); 
        }
      break;
  }
  if (onOff()){ //Left button is held for 5 sec, it turns off. 
          next_state = OFF;
  }if (state != next_state) {
    oled.fillScreen(0x0000);
  }

  state = next_state;
}
/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/*=======================================================================================================================================================*/
/*5 ACTUAL HELPER FUNCTIONS, IN ORDER OF PROTOTYPE APPEARANCE*/

bool onOff(){
  if(CircuitPlayground.leftButton() && CircuitPlayground.rightButton()){
    return true;
  }else{
    return false;
  }
}


 /*-----------------------PITCH/FREQUENCY MATH FUNCTIONS---------------------------------------------------------*/


float measureFrequency() {
  // 1) Collect samples and measure sample rate
  unsigned long start = micros(); //take the first time measurement
  for (int i = 0; i < N_SAMPLES; i++) {
    samples[i] = analogRead(MIC_PIN);
  }
  unsigned long end = micros(); //take the second time measurement 
 
  float totalTimeSec = (end - start) / 1000000.0; //find the total time ellapsed in seconds 
  if (totalTimeSec <= 0) return 0.0; //mostly a worry if the timer is wrapping around-- just in case
  float sampleRate = N_SAMPLES / totalTimeSec; //gives us samples per second to be used in frequency calc 
 
  // 2) Remove DC offset and measure signal energy (centering the signal at zero)
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
 
  // 3) Difference function in guitar range
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


/*-----------------------OLED DRAWING HELPERS---------------------------------------------------------*/
void drawNoSignal() {
  oled.fillScreen(0x0000);

  // Top line: "No signal"
  //(&TomThumb);
  oled.setTextSize(1);
  oled.setTextColor(0xFFFF);
  oled.setCursor(2, 10);
  oled.print("No signal");

  // Big target note in center
  oled.fillRect(0, 16, 96, 32, 0x0000);
  //oled.setFont(&TomThumb);
  oled.setTextSize(2);
  oled.setTextColor(0xF800);   // red
  oled.setCursor(10, 40);
  oled.print(strings[currentStringIndex].name);

  // Bottom hint
  oled.fillRect(0, 48, 96, 16, 0x0000);
  //oled.setFont(&TomThumb);
  oled.setTextSize(1);
  oled.setTextColor(0x07E0);
  oled.setCursor(2, 58);
  oled.print("Toggle string (R btn)");

  strcpy(lastNoteName, "---");
  strcpy(lastStatus, "");
  lastCentsRounded = 9999;
  lastHzRounded = 0;
}

// note screen during tuning
void drawTuningScreen(const char* noteName, int hzRounded, int centsRounded, const char* status) {
  // Top: frequency
  oled.fillRect(0, 0, 96, 16, 0x0000);
  //oled.setFont(&TomThumb);
  oled.setTextSize(1);
  oled.setCursor(2, 10);
  oled.setTextColor(0x07E0); // green-ish
  oled.print("Freq: ");
  oled.print(hzRounded);
  oled.print(" Hz");

  // Middle: big note
  oled.fillRect(0, 16, 96, 32, 0x0000);
  //oled.setFont(&TomThumb);
  oled.setTextSize(2);
  oled.setCursor(10, 40);
  oled.setTextColor(0xF800);
  oled.print(noteName);

  // Bottom: cents + status
  oled.fillRect(0, 48, 96, 16, 0x0000);
  oled.setTextSize(1);
  oled.setCursor(2, 58);
  oled.setTextColor(0xFFFF);
  oled.print(centsRounded);
  oled.print(" cents ");
  oled.print(status);
}
void drawCurrentNoteScreen() { //used in fork 
  oled.fillScreen(0x0000);

  // Top: mode label
  //oled.setFont(&TomThumb);
  oled.setTextSize(1);
  oled.setTextColor(0xFFFF);
  oled.setCursor(2, 10);
  oled.print("Tunning Fork mode");

  // Middle: big note name (E2, A2, etc.)
  oled.fillRect(0, 16, 96, 32, 0x0000);
  oled.setTextSize(2);
  oled.setTextColor(0xF800);   // red
  oled.setCursor(10, 40);
  oled.print(strings[currentStringIndex].name);

 
  lastStringDrawn = currentStringIndex;
}

 /*-----------------------SPEAKER MODE SET UP & AUDIO HELPERS---------------------------------------------------------*/
void playInTuneBeepIfNeeded(bool inTune) {
  if (inTune && !wasInTune) {
    tone(SPEAKER_PIN, 1200, 150);
  }
  wasInTune = inTune;
}

void playCurrentNotePulse() {
  int f = strings[currentStringIndex].freq;

  // A slightly smoother pulse: longer tone, longer pause
  tone(SPEAKER_PIN, f);
  delay(700);     // tone duration
  noTone(SPEAKER_PIN);
  delay(350);     // pause between pulses
}

 /*-----------------------BUTTON SETUP/TOGGLING HELPERS---------------------------------------------------------*/
void handleRightButton() {
  bool rightNow = CircuitPlayground.rightButton();

  if (rightNow && !rightPrev) {
    currentStringIndex++;
    if (currentStringIndex >= NUM_STRINGS) currentStringIndex = 0;

    tone(SPEAKER_PIN, 1500, 50);   // click

    strcpy(lastNoteName, "---");
    lastHadSignal = false;

    // Immediately show new target even if silent
    drawNoSignal();
  }

  rightPrev = rightNow;
}
// ---------- MOTOR CONTROL PINS/MOTOR SET UP----------
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

/*========================================================================================================================================================*/
/*========================================================================================================================================================*/
/*=======================================================================================================================================================*/
/*6 ACTUAL MAJOR FUNCTIONS, IN ORDER OF PROTOTYPE APPEARANCE*/

void normal() {
  //bool firstTime = true;
  handleRightButton();
  
  
  
  float hz = measureFrequency();
  unsigned long now = millis();
  
  if (hz > 30.0 && hz < 1000.0) {
    Note target = strings[currentStringIndex];
    float cents = centsOff(hz, target.freq);

    if (fabs(cents) > MAX_DEVIATION_CENTS) {
      playInTuneBeepIfNeeded(false);
      return;
    }

    const char* status = tuningStatus(cents);
    bool inTune = (fabs(cents) < IN_TUNE_CENTS);

    int hzRounded    = (int)(hz + 0.5);
    int centsRounded = (int)(cents + (cents >= 0 ? 0.5 : -0.5));

    // audio
    playInTuneBeepIfNeeded(inTune);

    bool changed = true; 
    if (changed) {

      drawTuningScreen(target.name, hzRounded, centsRounded, status);

      strcpy(lastNoteName, target.name);
      strcpy(lastStatus, status);
      lastCentsRounded = centsRounded;
      lastHzRounded    = hzRounded;
      lastHadSignal    = true;
    }
    changed =
      (strcmp(target.name, lastNoteName) != 0) ||
      (strcmp(status,      lastStatus)   != 0) ||
      (centsRounded != lastCentsRounded) ||
      (hzRounded    != lastHzRounded)    ||
      !lastHadSignal;
  } else {
    // no valid frequency
    playInTuneBeepIfNeeded(false);
    //
    if (lastHadSignal||firstTime) {
      drawNoSignal();
      lastHadSignal = false;

      if (now - lastPrintMs > 200) {
        lastPrintMs = now;
      }
    }
  }
  firstTime = false; 
}

void motor() {
  handleRightButton();

  float hz = measureFrequency();
  unsigned long now = millis();

  if (hz > 30.0 && hz < 1000.0) {
    Note target = strings[currentStringIndex];
    float cents = centsOff(hz, target.freq);

    // Ignore stuff way off target note
    if (fabs(cents) > MAX_DEVIATION_CENTS) {
      sendMotorCommand(0.0);
      playInTuneBeepIfNeeded(false);
      return;
    }

    const char* status = tuningStatus(cents);
    bool inTune = (fabs(cents) < IN_TUNE_CENTS);

    int hzRounded    = (int)(hz + 0.5);
    int centsRounded = (int)(cents + (cents >= 0 ? 0.5 : -0.5));

    // motor + audio
    sendMotorCommand(cents);
    playInTuneBeepIfNeeded(inTune);

    bool changed = true; //so the first time pitch is detected it can get through 

    if (changed) {
      drawTuningScreen(target.name, hzRounded, centsRounded, status);

      strcpy(lastNoteName, target.name);
      strcpy(lastStatus, status);
      lastCentsRounded = centsRounded;
      lastHzRounded    = hzRounded;
      lastHadSignal    = true;
    }
    changed =
      (strcmp(target.name, lastNoteName) != 0) ||
      (strcmp(status,      lastStatus)   != 0) ||
      (centsRounded != lastCentsRounded) ||
      (hzRounded    != lastHzRounded)    ||
      !lastHadSignal;
  } else {
    // no valid frequency
    sendMotorCommand(0.0);
    playInTuneBeepIfNeeded(false);

    if (lastHadSignal||firstTimeMotor) {
      drawNoSignal();
      lastHadSignal = false;

      if (now - lastPrintMs > 200) {
        lastPrintMs = now;
      }
    }
  }
  firstTimeMotor = false;
}
void fork() {
 

  if (CircuitPlayground.rightButton()||firstTimeFork) { 
    firstTimeFork = false;
    drawCurrentNoteScreen();
    currentStringIndex++;
    if (currentStringIndex >= NUM_STRINGS) {
      currentStringIndex = 0;
    }
    delay(200);
  }


  playCurrentNotePulse();
}
void Title_Screen() {
  oled.setTextColor(0xFFFF, 0x0000);
  oled.setCursor(2, 10);
  oled.print("Welcome");
  oled.setCursor(2, 25);
  oled.print("Press L to Select Mode");
}
void Selection_Mode() {
  
  const char* modes[3] = {"Normal Mode", "Fork Mode", "Motor Mode"}; // FOR SELECTED MODE: normal mode = 0 Fork Mode =1 Motor Mode = 2 (could turn into enum)

  oled.setCursor(2, 10);
  oled.setTextColor(0xFFFF); // white title
  oled.print("Select Mode:");

  for (int i = 0; i < 3; i++) {
    int y = 25 + i * 10;
    oled.setCursor(2, y);

    if (i == selected_Mode) {
      oled.setTextColor(0x001F, 0xFFFF); // blue text 
    } else {
      oled.setTextColor(0xFFFF, 0x0000); 
    }

    oled.print(modes[i]);
  }
}
