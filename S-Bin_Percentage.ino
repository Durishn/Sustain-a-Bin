// include the lcd library
#include <LiquidCrystal.h>

//CONSTANTS: used to define pins
const int fsrPin = 0; // the FSR and 10K pulldown are connected to a0
const int buttonPin = 6;     // the number of the pushbutton pin
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//GLOBALS: variables used to track FSR parameters
int fsrAnalog,fsrAnalog1,fsrAnalog2,fsrAnalog3,fsrAnalog4, fsrAnalog5;
int fsrAnalogAVG;
int fsrNoiselessAnalog; 
int fsrVoltage; 
int fsrWeight;
int fsrWeightAVG;
int fsrNoiselessWeight;
unsigned long fsrResistance;
int calibrationWeight, totalWeight;
int LCDflag, flag, debugFlag, counter;
int currWeight, newWeight, weightCounter, userWeightAVG, userCount;
int userWeights[20];
int buttonState = 0;

//---------------------------------------------------------------------------//

//SETUP: Function run once on Arduino start-up
void setup() {
  // set up the LCD's number of columns and rows, & print:
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("-SUSTAIN-A-BIN!-");
  lcd.setCursor(0, 1);
  lcd.print("----------------");
  pinMode(buttonPin, INPUT);
  
  // Set bauds to send debugging information via the Serial monitor
  Serial.begin(9600); 
}

void loop() {

  int i, percentage;
  
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
 
  // Analog reading from 0 to 1023
  fsrAnalog = analogRead(fsrPin); 

  // Save last five analog files
  switch (counter % 5) {
    case 0: 
      fsrAnalog1 = fsrAnalog;
    break;
    case 1:
      fsrAnalog2 = fsrAnalog;
    break;
    case 2:
      fsrAnalog3 = fsrAnalog;
    break;
    case 3:
      fsrAnalog4 = fsrAnalog;
    break;
    case 4:
      fsrAnalog5 = fsrAnalog;
    break;
  }

  //Calculate FSR output average from last 5 readings
  fsrAnalogAVG = (fsrAnalog1+fsrAnalog2+fsrAnalog3+fsrAnalog4+fsrAnalog5)/5;
  
  // Voltage reading form 0V to 5V (5000mV) mapped from the Analog reading
  fsrVoltage = map(fsrAnalog, 0, 1023, 0, 5000);

  // If analog is not zero calculate weight and average based on most 
  //accurate exponential function.
  fsrWeight = 0;
  if (fsrAnalog != 0){
    fsrWeight = -100.69386576753435 + (100.5340374291948)
    * exp((0.0015885354347221188) * fsrAnalog); 
    fsrWeightAVG = -100.69386576753435 + (100.5340374291948)
    * exp((0.0015885354347221188) * fsrAnalogAVG); 
  }

  if (buttonState == HIGH){
    calibrationWeight = fsrNoiselessWeight;
    Serial.println("\n-- NEW CALIBRATION --");
    Serial.print(  "         ");
    Serial.println(calibrationWeight);
    lcd.clear();
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("-SUSTAIN-A-BIN!-");
    lcd.setCursor(0, 1);
    lcd.print("   Door Ajar.   ");
    while (buttonState == HIGH) buttonState = digitalRead(buttonPin);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("-SUSTAIN-A-BIN!-");
    lcd.setCursor(0, 1);
    lcd.print("----------------");
    totalWeight = fsrNoiselessWeight - calibrationWeight;
    lcd.print(totalWeight);
  }

  // Allow for transition of drastic change
  if ((fsrAnalogAVG < (fsrAnalog * .95))
    || (fsrAnalogAVG > (fsrAnalog * 1.05)))
    flag = counter;
    
  //NOISELESS: Remove noise from parameter average (9%)
  if (((fsrWeightAVG < (fsrNoiselessWeight * .91))
    || (fsrWeightAVG > (fsrNoiselessWeight * 1.09)))
    && (counter > flag+15))
    fsrNoiselessAnalog = fsrAnalogAVG;
  else if (fsrAnalogAVG <= 20)
    fsrNoiselessAnalog = 0;
  fsrNoiselessWeight = -100.69386576753435 + (100.5340374291948)
    * exp((0.0015885354347221188) * fsrNoiselessAnalog);

  if (counter > 20){
    //every 40 ticks slightly correct total weight
    if ((counter % 40 == 0) && (LCDflag < counter) && (counter > flag+15)){
        fsrNoiselessAnalog += (fsrAnalogAVG-fsrNoiselessAnalog)/5;
        fsrNoiselessWeight = -100.69386576753435 + (100.5340374291948)
        * exp((0.0015885354347221188) * fsrNoiselessAnalog);
        int buff = fsrNoiselessWeight - currWeight;
        if (buff){
          Serial.println("\n-- WEIGHT CORRECTION --");
          Serial.print(  "        of ");
          Serial.println(buff);
          currWeight = fsrNoiselessWeight;
          debugFlag = 1;
          totalWeight = fsrNoiselessWeight - calibrationWeight;
        }
      } 
    
    // Update current weight
    else if ((currWeight != fsrNoiselessWeight) || (LCDflag == counter)){
      if ((counter >=30) && (currWeight < fsrNoiselessWeight)){
        newWeight = fsrNoiselessWeight - currWeight;
        totalWeight = fsrNoiselessWeight - calibrationWeight;
        Serial.println("\n-- NEW DEPOSIT --");
        Serial.print(    "   mass (g)= ");
        Serial.println(newWeight);
        lcd.clear();
        lcd.setCursor(0, 0);
        if (userWeightAVG == 0){
          lcd.print("  NO WASTE HAS  ");
          lcd.setCursor(0,1);
          lcd.print(" BEEN COLLECTED ");
        } else if (userWeightAVG >= newWeight){
          percentage = 100 - (newWeight*100)/userWeightAVG;
          lcd.print(" YOU WASTED ");
          lcd.print(percentage);
          lcd.print("%");
          lcd.setCursor(0, 1);
          lcd.print("LESS THAN OTHERS");
        } else {
          percentage = (newWeight*100)/userWeightAVG - 100;
          lcd.print(" YOU WASTED ");
          lcd.print(percentage);
          lcd.print("%");
          lcd.setCursor(0, 1);
          lcd.print("MORE THAN OTHERS");
         }
        
        //save the last 20 weight deposits & calculate average
        userWeights[weightCounter] = newWeight;
        weightCounter++;
        weightCounter % 20;
        userCount = userWeightAVG = 0;
        for (i = 0; i < 20; i++){
          if (userWeights[i] != 0){
            userCount++;
            userWeightAVG += userWeights[i];
          }
        }
        userWeightAVG = userWeightAVG/userCount;
        
        debugFlag=1;
        LCDflag = counter+50;
      }
      else {
          int buff = fsrNoiselessWeight - currWeight;
          debugFlag = 1;
          Serial.println("\n-- WEIGHT REDUCTION --");
          Serial.print(  "        of ");
          Serial.println(buff);
          lcd.clear();
          lcd.begin(16, 2);
          lcd.setCursor(0, 0);
          lcd.print("-SUSTAIN-A-BIN!-");
          lcd.setCursor(0, 1);
          lcd.print("----------------");
          totalWeight = fsrNoiselessWeight - calibrationWeight;
          lcd.print(totalWeight);
      }  
      currWeight = fsrNoiselessWeight;
    }
  
  
    // Print noisey & noiseless parameters to debug every 5 ticks
    if (debugFlag){
      Serial.print("Tick #:");
      Serial.println(counter);
      Serial.print("  Average Weight Input (inclusive) = ");
      Serial.println(userWeightAVG);
      Serial.print("  Calibrated Weight (g) = ");
      Serial.println(calibrationWeight);
      Serial.println("\nNOISEY:");
      Serial.print("  Current Analog = ");
      Serial.println(fsrAnalog);
      Serial.print("  Analog Average (last 5) = ");
      Serial.println(fsrAnalogAVG);
      Serial.print("  Voltage (mV) = ");
      Serial.println(fsrVoltage); 
      Serial.print("  Current Weight (g) = ");
      Serial.println(fsrWeight);
      Serial.print("  Average Weight (g) = ");
      Serial.println(fsrWeightAVG);
      Serial.println("\nNOISELESS:");
      Serial.print("  Analog = ");
      Serial.println(fsrNoiselessAnalog);
      Serial.print("  Total Weight (g) = ");
      Serial.println(fsrNoiselessWeight);
      Serial.print("  Weight minus Calibration (g) = ");
      Serial.println(totalWeight);
      Serial.println("--------------------");
      debugFlag =0;
    }
  }
  counter++;
  if (counter >= 1000){
    counter = 100;
    flag -= 900;
    LCDflag -= 900;
  }
  
  delay(100);
}
