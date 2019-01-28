/*
  Author: Nic Durish (n.durish@gmail.com)

  This script is a test script to test 16x2 LCD, using the
  LiquidCrystal libary and Analog inputs. An analog input 
  is read from pin 0, and the results are printed onto an
  LCD conntected in the following manner:
    LCD PINS - ARDUINO PNS 
          RS - digital pin 12
      Enable - digital pin 11
          D4 - digital pin 5
          D5 - digital pin 4
          D6 - digital pin 3
          D7 - digital pin 2
         R/W - ground
         VSS - ground
         VCC - 5V
 */

/* include the LCD library & initialize the library 
with the numbers of the interface pins*/
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("FSR ANALOG VALUE");
  Serial.begin(9600);
}

void loop() {

  // read the input on analog pin 0 & print to lcd
  int sensorValue = analogRead(A0);
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(6, 1);
  lcd.print(sensorValue);
  Serial.println(sensorValue);
  delay(100);
}

