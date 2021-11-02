#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <pitches.h>

#define MODE_WAIT 0
#define MODE_RUN 1

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_TEXT_SIZE 2
#define SCREEN_PADDING 2
#define SCREEN_LINE 20 // OLED text internal size is 8 this should be 8*Text_Size+padding*2
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

const int motorSpeed = 56  ;
const int redButtonPin = 12;
const int whtButtonPin = 11;
const int motorPin = 10;
const int ledFanPin = 9;
const int bepperPin = 8;

// notes in the melody:
int melody[] = {
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6};
int duration = 500;  // 500 miliseconds

int activeMode = 0;
int timeSelect = 1;
int timeChoices[] = {120, 300, 600, 900};

int timeCounter = 0;
int timeMinutes = 0;
int timeSeconds = 0;
char timeDisplay[10];

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
   Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //Serial.print("Intializing display...\n");
  // Flip screen....
  //display.setRotation(2);
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  
  pinMode(motorPin, OUTPUT);
  pinMode(redButtonPin, INPUT_PULLUP); 
  pinMode(whtButtonPin, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(redButtonPin) == LOW  && activeMode == 0) {
    activeMode = 1;
    delay(500);
  }
  
  switch (activeMode) {
    case MODE_WAIT:
      executeWaitMode();
      break;
    case MODE_RUN:
      executeRunMode();
      break;
    default:
      display.clearDisplay();
      displayLineInfo(0, "ERROR", 1);
      displayLineInfo(1, "Unknown Mode");
      break;      
  }
}

void executeWaitMode() {
  if (digitalRead(whtButtonPin) == LOW) {
    timeSelect++;
    delay(250);
  }

  int maxTimeSelect = sizeof(timeChoices)/sizeof(int)-1;
  if (timeSelect > maxTimeSelect) {
    timeSelect = 0;
  }

  int timeMinutes = timeChoices[timeSelect]/60;
  char displayMinutes[4];
  itoa(timeMinutes, displayMinutes, 10);
  
  displayLineInfo(0, "<< Time", 0); 
  strcpy(timeDisplay, "  ");
  strcat(timeDisplay, displayMinutes);
  strcat(timeDisplay, " min");
  displayLineInfo(1, timeDisplay, 1);  
  displayLineInfo(2, "    Run >>", 0);
}

void executeRunMode() {
  displayLineInfo(0, "Exposing", 0);

  displayLineInfo(2, " Cancel >>", 0);

  int timeStart = timeChoices[timeSelect];
  bool toggle;
  int timeCounter = timeStart;
  char displayMinutes[4];
  char displaySeconds[4];
  int timeMinutes = 0;
  int timeSeconds = 0;
  float percentage = 0;
  while(activeMode == 1) {     
    // Kick off the system
    if (timeCounter == timeStart) {
      //playStartNote();
      analogWrite(motorPin, motorSpeed);
      analogWrite(ledFanPin, 255);
    }
    
    // Calculate minutes and seconds and convert to char
    timeMinutes = timeCounter/60;
    timeSeconds = timeCounter%60;
    itoa(timeMinutes, displayMinutes, 10);
    itoa(timeSeconds, displaySeconds, 10);   

    // Handle updating the display time
    strcpy(timeDisplay, "  ");
    strcat(timeDisplay, displayMinutes);
    strcat(timeDisplay, ":");
    if (timeSeconds < 10) {
      strcat(timeDisplay, "0");
    }
    strcat(timeDisplay, displaySeconds);
    displayLineInfo(1, timeDisplay, 1);

    // Update precentage amount
    percentage = (1.0 - (timeCounter*1.0/timeStart)) * 100.0;
    displayProgress(percentage);

    // Check if we are done, when timer is 0 or less    
    if(timeCounter < 1){
      activeMode = 0; // stop sequence
      displayLineInfo(1, "Done!", 1);
      displayClearLine(2);
    }

    // Check if the user canceled
    if (digitalRead(redButtonPin) == LOW) {
      activeMode = 0; // stop sequence
      displayLineInfo(1, "Canceled!");
      displayClearLine(2);
    }
    
    // Handle time increment
    timeCounter--;
    delay(1000);
  }

  // Once we break the while loop, reset everything
  analogWrite(motorPin, 0);
  analogWrite(ledFanPin, 0);
  playEndNote();
  delay(2500);
  displayProgress(0);
}

void playStartNote() {
  playNote(NOTE_AS4, 166);
  playNote(NOTE_AS4, 166);
  playNote(NOTE_AS4, 166);
  
  playNote(NOTE_F5, 666);
  playNote(NOTE_C6, 666);
  
  playNote(NOTE_AS5, 166);
  playNote(NOTE_A5, 166);
  playNote(NOTE_G5, 166);
  playNote(NOTE_F6, 666);
  playNote(NOTE_C6, 333);

  playNote(NOTE_AS5, 166);
  playNote(NOTE_A5, 166);
  playNote(NOTE_G5, 166);
  playNote(NOTE_F6, 666);
  playNote(NOTE_C6, 333);

  playNote(NOTE_AS5, 166);
  playNote(NOTE_A5, 166);
  playNote(NOTE_AS5, 166);
  playNote(NOTE_G5, 666);
}

void playEndNote() {
  playNote(NOTE_E2, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_E3, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_D3, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_E2, 166);

  playNote(NOTE_C3, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_E3, 166);
  playNote(NOTE_AS2, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_B2, 166);
  playNote(NOTE_C3, 166);

  playNote(NOTE_E2, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_E3, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_D3, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_E2, 166);

  playNote(NOTE_C3, 166);
  playNote(NOTE_E2, 166);
  playNote(NOTE_E3, 166);
  playNote(NOTE_AS2, 999);
}

void playNote(int note, int duration) {
  tone(bepperPin, note, duration*0.9);
  delay(duration);
}

void displayClearLine(int line) {
  display.fillRect(0, line*SCREEN_LINE, 128, SCREEN_LINE, SSD1306_BLACK);
  display.display();
}

void displayLineInfo(int line, char* info) {
  displayLineInfo(line, info, 0);
}

void displayProgress(float percentage) {
  int width = percentage;
  display.fillRect(0, SCREEN_LINE*3, width, 4, SSD1306_WHITE);
  display.fillRect(width, SCREEN_LINE*3, 128-width, 4, SSD1306_BLACK);
}

void displayLineInfo(int line, char* info, int drawMode) {
  // Put something here to make sure we add white spaces to the end
  display.setTextSize(SCREEN_TEXT_SIZE);
  if (drawMode > 0) {
    display.fillRect(0, line*SCREEN_LINE, 128, SCREEN_LINE, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  // Draw 'inverse' text
  }
  else {
    display.fillRect(0, line*SCREEN_LINE, 128, SCREEN_LINE, SSD1306_BLACK);
    display.setTextColor(SSD1306_WHITE); // Draw white text
  }
  
  display.setCursor(SCREEN_PADDING, (line*SCREEN_LINE)+SCREEN_PADDING); // Set the top-left corner
  display.println(info);
  display.display();
}
