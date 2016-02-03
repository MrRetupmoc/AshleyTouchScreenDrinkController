#include <SPI.h>       // this is needed for display
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>


// For the Adafruit shield, these are the default.
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();



bool Dispense = false;

int DrinkChoice = 0;
const int DrinkChoice_Max = 5;

String DrinkName[DrinkChoice_Max] = {"  Tequila Sunset", "     Fireball", "     Whiskey", "    Terminator", "    Jager Bomb"};
int DrinkDispenseTime[DrinkChoice_Max] = {3000 , 1500, 2000, 4123, 2392};

//int BottleLocations[6] = { 0, 12, 15, 18, 21 };
//int Recipy[Drink#,DrinkRecipie] = { { 1,2,3 }, { 3,1,2 } };
//int DrinkMix[DrinkChoice_Max, 6] = { {1,2,3,4,5,6}, {1,0,0,0,0,0}, };

void setup() {
  Serial.begin(9600);
  Serial.println("Drink Dispensor Going Up"); 
 
  tft.begin();

  if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }
  Serial.println("Capacitive touchscreen started");
  
  DrawScreen(); 
}


void loop(void) {
  // Wait for a touch
  if (! ctp.touched()) {
    return;
  }

  // Retrieve a point  
  TS_Point p = ctp.getPoint();

  // Print out the touch coordinates
  Serial.print("("); Serial.print(p.x);
  Serial.print(", "); Serial.print(p.y);
  Serial.println(")");

  //When Screen is Touched in the Right Area's
  if (p.x < 40 && p.y > 20) { // Top of Screen Refresh
    DrawScreen(); 
  }
  else if (p.x > 120 && p.x < 180 && p.y > 100 && p.y < 230) { // Enter Area
    //Dispense = true;
    //DrawScreen();
    tft.println("     Dispensing Drink");
    delay(DrinkDispenseTime[DrinkChoice]);
    //Dispense = false;
    DrawScreen();
  }
  else if (p.x > 120 && p.x < 180 && p.y > 0 && p.y < 80) { // Forward Button
    if (DrinkChoice < (DrinkChoice_Max - 1)) {
      DrinkChoice += 1;
      DrawScreen();
    }
  }
  else if (p.x > 120 && p.x < 180 && p.y > 250 && p.y < 320) { // Back Button
    if (DrinkChoice > 0) {
      DrinkChoice -= 1;
      DrawScreen();
    }
  }

  delay(100);
}

void DrawScreen() {
  tft.setRotation(1);
  
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
  tft.println(DrinkName[DrinkChoice]); // drink name
  tft.println();
  tft.println();
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(4);
  tft.println(" <  Enter  > ");
  tft.println();
    tft.setTextColor(ILI9341_YELLOW); 
    tft.setTextSize(2);
  //if(Dispense) tft.println("     Dispensing Drink");
}
