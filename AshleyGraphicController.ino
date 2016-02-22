/*------------------------------------------------------------------------------------------------------
                              Author : Evan Richinson aka MrRetupmoc42
               
Arduino Driven Drink Slider System to Automatically Pour Drinks / Shots
   
January 20th 2016   : Stepper Code Imported and Updating for Timing / Postitional Data with Button Input Control
                      Stepper Homing, Drink Dispense Delay based on Position

January 27th 2016   : Created Init Graphics and Arrays for Drink Dispensing.                  

Febuary 2th 2016    : Updated / Merged Graphics into Controller, Updating more of the Move Logic...

Febuary 21st 2016   : Got the Slider Tuned and Moving Posisiton Wise...
                      Integrating Screen and Recipy Configuration into ONE
                   
-------------------------------------------------------------------------------------------------------*/

#include <SPI.h>       // this is needed for display
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <NewPing.h>

// For the Adafruit shield, these are the default.
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();

#define TRIGGER_PIN  12  // 12 Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // 11 Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 //    Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

//Screen Data----------------------------------------------------------------------------------------- 

//Behaviour Data
bool Debug = true;
int Rotation = 1;

//Movement Data-----------------------------------------------------------------------------------------

//Stepper Delay and Timings
const int Stepper_stepdelay = 2;
int Slide_Move_Delay = 100;

//Slider Stepper Pin Setup
const int Slide_directionPin = 8;
const int Slide_stepPin = 9;
const int Slide_HomeLimitPin = 10;

//Dispense Stepper Pin Setup
//const int Dispense_directionPin = 26;
//const int Dispense_stepPin = 27;
//const int Dispense_HomeLimitPin = 28;

//Input Buttons or Screen?
//const int Forward_ButtonPin = 7;
//const int Reverse_ButtonPin = 6;
//const int Mode_ButtonPin = 6;

//Stepper Max Bounds Setup
const int Slide_maxSteps = 1200;
int Slide_StepsCurrent = 0;

const int Slide_Positions = 6;
const int Slide_Positions_steps = Slide_maxSteps / Slide_Positions;
int Slide_Positions_Current = 0;

//Dispensor Stepper Max Bounds Setup
//const int Dispense_maxSteps = 20;

//Future Ultrasonic Data
//double DistanceCm = 0.0;
//double DistanceCm_Offset = 6.0;

//Array Data-----------------------------------------------------------------------------------------

//Drink Recipies Data
const int MaxDrinkRecipies = 4;    //Max Drink Recipies in Array
const int DrinkRecipyData = 2;     //Drink Positions then Drink Sleep Times after Move
const int MaxDrinkRecipyMoves = 4; //Fill Extra Moves with Same or Home Data
int DrinkChoice_Current = 0;

//Drink Screen Names
String DrinkName[MaxDrinkRecipies] = {"  Tequila Sunrise", "     Screw Driver", "  Carribean Bliss", "    Whiskey Sour"}; //, "    Blissletoe"};

//Drink Recipie 1. # of Drinks in Recipie, 2.Drink Recipie Position Data, 3. After Move Sleep Data.
int DrinkRecipies[MaxDrinkRecipies][DrinkRecipyData][MaxDrinkRecipyMoves] = {  
  { {1, 4, 6, 0}, {2000, 2000, 2000, 0} } ,
  { {2, 5, 4, 4}, {2000, 2000, 2000, 2000} } ,
  { {5, 6, 6, 1}, {2000, 2000, 2000, 2000} } ,
  { {3, 2, 3, 2}, {2000, 3000, 2000, 2000} }                                };                                                                        

//----------------------------------------------------------------------------------------

void setup() {
  //Sets the Stepper Pins
  pinMode(Slide_directionPin, OUTPUT);
  pinMode(Slide_stepPin, OUTPUT);
  pinMode(Slide_HomeLimitPin, INPUT);
  
  //pinMode(Dispense_directionPin, OUTPUT);    
  //pinMode(Dispense_stepPin, OUTPUT);
  //pinMode(Dispense_HomeLimitPin, INPUT);

  //pinMode(Forward_ButtonPin, INPUT);
  //pinMode(Reverse_ButtonPin, INPUT);
  //pinMode(Mode_ButtonPin, INPUT);
  

  if (Debug) {
    Serial.begin(9600);
    Serial.println("Hello World.");
  }
 
  tft.begin();
  tft.setRotation(Rotation);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(1);
  
  //if (Debug) tft.println("Arduino HMI Screen Started...");

  if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
    if (Debug) {
      Serial.println("Couldn't Start Arduino HMI Controller");
      Serial.println("Google FT6206 Touchscreen Controller");
      tft.println("Couldn't Start Arduino HMI Controller");
      tft.println("Google FT6206 Touchscreen Controller");
    }
    while (1);
  }
  else {
    if (Debug) {
      Serial.println("Arduino HMI Controller Started...");
      tft.println("Arduino HMI Controller Started...");
    }
  }
  
  if (Debug) {
    Serial.println("Currently Finding Home...");
    tft.println("Currently Finding Home...");
  }

  //DispenseMovetoHome();
  if (Debug) {
    Serial.println("Dispenser Homed...");
    tft.println("Dispenser Homed...");
  }
  
  SlideMovetoHome();
  if (Debug) {
    Serial.println("Drink Positioner Homed...");
    tft.println("Drink Positioner Homed...");
  }

  if (Debug) {
    Serial.println("Sucessfully Initicated,");
    Serial.println("Ready to Pour Your Drinks, Master...");
    tft.println("Sucessfully Initicated,");
    tft.println("Ready to Pour Your Drinks, Master...");
  }

  delay(1000);
  DrawChoiceScreen(); 
}


void loop(void) {
  // Wait for a touch
  if (! ctp.touched()) {
    return;
  }

  // Retrieve a point  
  TS_Point p = ctp.getPoint();

  //Printout the Touch Coordinates for Calibration of Screen Displays
  //if (Debug) Serial.print("("); Serial.print(p.x);
  //if (Debug) Serial.print(", "); Serial.print(p.y);
  //if (Debug) Serial.println(")");

  //When Screen is Touched in the Right Area's
  if (p.x < 40 && p.y > 20) { // Top of Screen Refresh
    DrawChoiceScreen();
  }
  else if (p.x > 120 && p.x < 180 && p.y > 100 && p.y < 230) { // Enter Area
    //Add the Dispense Line to the Screen
    tft.println("     Dispensing Drink");
    if (Debug) Serial.println("Dispensing Drink"); 
    
    //Dispense Drink Recipy Choosen
    DispenseDrink(DrinkChoice_Current);

    //Update Screen because Drink is Poured
    DrawChoiceScreen();
  }
  else if (p.x > 120 && p.x < 180 && p.y > 0 && p.y < 80) { // Forward Button
    if (DrinkChoice_Current < (MaxDrinkRecipies - 1)) {
      DrinkChoice_Current += 1;
      DrawChoiceScreen();
    }
  }
  else if (p.x > 120 && p.x < 180 && p.y > 250 && p.y < 320) { // Back Button
    if (DrinkChoice_Current > 0) {
      DrinkChoice_Current -= 1;
      DrawChoiceScreen();
    }
  }

  delay(100);
}

/*
//Moves Dispenser to Home
void DispenseMovetoHome() {
  digitalWrite(Dispense_directionPin, HIGH);

  //Move Slider to Home
  do {
    digitalWrite(Dispense_stepPin, LOW);
    delay(Stepper_stepdelay);
    digitalWrite(Dispense_stepPin, HIGH);
    delay(Stepper_stepdelay);
  }
  while (!digitalRead(Dispense_HomeLimitPin));

  Dispense_StepsCurrent = 0;
}
*/

//Moves Slider to Home
void SlideMovetoHome() {
  digitalWrite(Slide_directionPin, HIGH);

  //Move Slider to Home
  do {
    digitalWrite(Slide_stepPin, LOW);
    delay(Stepper_stepdelay);
    digitalWrite(Slide_stepPin, HIGH);
    delay(Stepper_stepdelay);
  }
  while (!digitalRead(Slide_HomeLimitPin));

  Slide_StepsCurrent = 0;
}

//Moves Slider from Previous to Next Postition 
void SlideMovetoPosition(bool Direction, int MoveCount) {
  
  //Movement Code ( False Moves Outward, True Moves Inward )
  if (Direction) digitalWrite(Slide_directionPin, HIGH);
  else digitalWrite(Slide_directionPin, LOW);

  //If Forward and Not Over Max OR Reverse and Not Under Min
  if (!Direction && (Slide_StepsCurrent + MoveCount) <= Slide_maxSteps || Direction && (Slide_StepsCurrent - MoveCount) >= 0) {

    //Ouput to User
    if (Debug) Serial.println("Performing Requested Move."); 
    
    //Move Slider
    for(int i = 0; i < MoveCount; i++) {
      digitalWrite(Slide_stepPin, LOW);
      delay(Stepper_stepdelay);
      digitalWrite(Slide_stepPin, HIGH);
      delay(Stepper_stepdelay);
    }
    delay(Slide_Move_Delay);
    
    if (Direction) Slide_StepsCurrent -= MoveCount;
    else Slide_StepsCurrent += MoveCount;

    Slide_Positions_Current = Slide_StepsCurrent / Slide_Positions_steps;

    //if (Debug) Serial.println(Slide_StepsCurrent); 
    //if (Debug) Serial.println(Slide_Positions_Current); 
  }
  else if (Debug) Serial.println("Didn't Perform Move, Out of Bounds?"); 
}

//Dispenses Drink Combo
void DispenseDrink(int RecipyNumber) {
  if (Debug) Serial.println("Going to make the Drink");

    for(int i = 0; i < MaxDrinkRecipyMoves; i++) {
      //Movement Calculations off Position Data                 Drink Posistion Data                                           Drink Posistion Data
      if (Slide_Positions_Current > DrinkRecipies[RecipyNumber][0][i]) SlideMovetoPosition(true, ((DrinkRecipies[RecipyNumber][0][i] - Slide_Positions_Current) * -1) * Slide_Positions_steps);
      if (Slide_Positions_Current < DrinkRecipies[RecipyNumber][0][i]) SlideMovetoPosition(false, (DrinkRecipies[RecipyNumber][0][i] - Slide_Positions_Current) * Slide_Positions_steps);

      //Output Debug Data
      //if (Debug) Serial.println(i);
      //if (Debug) Serial.println(DrinkRecipy1[i]);
      //if (Debug) Serial.println(Slide_Positions_Current);

      //Delay after Move                Drink Sleep Data
      delay(DrinkRecipies[RecipyNumber][1][i]);

      //Dispense();
      //Delay(2000);
    }
}

/*
//Dispenses Liquid ( Might need to Vary Timings per Drink Choice )
void Dispense() {
  digitalWrite(Dispense_directionPin, LOW);
  
  for(int i = 0; i < 50; i++) {
    digitalWrite(Dispense_directionPin, LOW);
    delay(Dispense_stepdelay);
    digitalWrite(Dispense_directionPin, HIGH);
    delay(Dispense_stepdelay);
  }

  //Dispense Proper Amount Drink Delay
  delay(1000);

  digitalWrite(Dispense_directionPin, HIGH);
  
  for(int i = 0; i < 50; i++) {
    digitalWrite(Dispense_directionPin, LOW);
    delay(Dispense_stepdelay);
    digitalWrite(Dispense_directionPin, HIGH);
    delay(Dispense_stepdelay);
  }
}
*/

//Updates the Screen
void DrawChoiceScreen() {
  tft.setRotation(Rotation);
  
  tft.fillScreen(ILI9341_BLACK);
  
  tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_PINK); 
    tft.setTextSize(2);
  tft.println("       Welcome to :");
  tft.println("Ashley's Drink Dispensory");
  tft.println();
  tft.println();
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(3);
  tft.println(DrinkName[DrinkChoice_Current]); // drink name
  tft.println();
  tft.println();
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(4);
  tft.println(" <  Enter  > ");
  tft.println();
    tft.setTextColor(ILI9341_YELLOW); 
    tft.setTextSize(2);
  //if(Dispense) tft.println("     Dispensing Drink"); //Puts the Dispensing Line Test
}

/*
void GetDistance() {
  unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  DistanceCm = uS / US_ROUNDTRIP_CM; // Convert ping time to distance in cm and print result (0 = outside set distance range)
  Serial.print("Ping: ");
  Serial.print(DistanceCm);
  Serial.println("cm");
}
*/
