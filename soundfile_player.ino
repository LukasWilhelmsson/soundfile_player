/*
auther: Lukas.W
date: 2025-05-08
description: This is a sound file player that goes in a loop where you can chose what sound files are playing in a loop.
*/
//all the librarys its using
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>


// VS1053 pin setup
#define SHIELD_RESET -1 
#define SHIELD_CS     7
#define SHIELD_DCS    6
#define CARDCS        4
#define SHIELD_DREQ   3

//declares what instance of the VS1053 it is
Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, SHIELD_DREQ, CARDCS);

//4 buttons and defines which pins they are connected to
const int numButtons = 4;
const int buttonPins[numButtons] = {2, 5, 8, 9};

//lists the sound files to play for each button
const char* soundFiles[numButtons] = {"boop.mp3", "bap.mp3", "doop.mp3", "splat.mp3"};

//remembers whether each sound is ON or OFF
bool isActive[numButtons] = {false, false, false, false};

//sets time delay for each sound
unsigned long playOffsets[numButtons] = {0, 1000, 2000, 3000};

//tracks time and defines the full loop length
unsigned long loopStartTime = 0;
const unsigned long loopInterval = 4000;

//knows if a sound has already been played in the current cycle
bool hasPlayed[numButtons] = {false, false, false, false};

// serial monitor
void setup() {
  Serial.begin(9600);

   //sets each button pin as input with pull-up resistors
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

   // starts the music player and If it fail it stops the program
  if (!musicPlayer.begin()) {
    Serial.println("Couldnt find VS1053");
    while (1);
  }

  // sets volume
  musicPlayer.setVolume(50, 50);

  //starts micro sd and if it fails it stops the program
  if (!SD.begin(CARDCS)) {
    Serial.println("SD init failed");
    while (1);
  }
  //interrupts so music plays smoothly
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

  //prints ready and starts the loop timer
  Serial.println("Setup ready");

  loopStartTime = millis();
}

/*sees if a button is pressed and toggles its state then prints which sound was toggled ether on or off*/
void loop() {

  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      delay(200); // Debounce
      isActive[i] = !isActive[i];
      Serial.print("Toggled ");
      Serial.print(soundFiles[i]);
      Serial.print(" -> ");
      Serial.println(isActive[i] ? "ON" : "OFF");
    }
  }

      //calculates how much time has passed in the current loop
  unsigned long now = millis();
  unsigned long elapsed = (now - loopStartTime) % loopInterval;

       /*if no sound is playing it checks if its time to play any active sound that hasnt played yet then plays the first matching one*/
  if (!musicPlayer.playingMusic) {
    for (int i = 0; i < numButtons; i++) {
      if (isActive[i] && !hasPlayed[i] && elapsed >= playOffsets[i]) {
        Serial.print("Playing: ");
        Serial.println(soundFiles[i]);
        musicPlayer.startPlayingFile(soundFiles[i]);
        hasPlayed[i] = true;
        break;
      }
    }
  }

    /*if the loop just restarted then reset hasPlayed and start timing*/
  if (elapsed < 10) {
    for (int i = 0; i < numButtons; i++) {
      hasPlayed[i] = false;
    }
    loopStartTime = now;
  }
}