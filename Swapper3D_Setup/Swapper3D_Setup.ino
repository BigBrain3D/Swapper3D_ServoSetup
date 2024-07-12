//Swapper3D setup firmware, Author: BigBrain3D, License: AGPLv3
#include <LiquidCrystal.h>
#include <Adafruit_PWMServoDriver.h> 
#include <EEPROM.h>
#include <String.h>
#include <Wire.h>

enum Button {
  NONE,
  UP,
  DOWN,
  LEFT,
  RIGHT,
  SELECT,
  UNKNOWN
};

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

const int numOfServos = 8;

// Servos
const int s_Tool_Rotate = 0; //360d //TR
const int s_Tool_Height = 1; //TH
const uint8_t s_Tool_Lock = 2; //TL
const uint8_t s_QuickSwapHotend_Lock = 3; //QL
const uint8_t s_Holder_Rotate = 4; //360d //HR
const uint8_t s_Cutter_Rotate = 5; //CR
const uint8_t s_Cutter_Action = 6; //CA
const uint8_t s_WasteCup_Action = 7; //WA

const int eePinNum = 0;
const int eeMaxAngle = 1;
const int eeCurrentAngle = 2;
int servos[numOfServos][3]; // pin #, max angle, current angle
int pulselength = 0;
const int servo_pwm_min_360 = 600; //484;
const int servo_pwm_max_360 = 2900; //2376;

const String servos_names[numOfServos] = {
  "Tool_Rotate",
  "Tool_Height",
  "Tool_Lock",
  "QuickSwap_Lock",
  "Holder_Rotate",
  "Cutter_Rotate",
  "Cutter_Action",
  "WasteCup_Action"
};

// Servo Starting positions
const int start_Postion_Tool_Rotate = 146; //148; //104;
const int start_Postion_Tool_Height = 130;//129;
const int start_Postion_Tool_Lock = 193;
const int start_Postion_QuickSwapHotend_Lock = 70;
const int start_Postion_HolderRotate = 15;
const int start_Postion_Cutter_Rotate = 27;
const int start_Postion_Cutter_Action = 176; //175;
const int start_Postion_WastCup_Action = 170; //110

const int end_Postion_Tool_Rotate = 293;
const int end_Postion_Tool_Height = 31;
const int end_Postion_Tool_Lock = 102; //105, 115;
const int end_Postion_QuickSwapHotend_Lock = 104;
const int end_Postion_HolderRotate = 360;
const int end_Postion_Cutter_Rotate = 123;
const int end_Postion_Cutter_Action = 33; //15; was binding at 15
const int end_Postion_WastCup_Action = 164; //125; //99

// start = 0, end = 1
const int servo_EndPositions[numOfServos] {
  end_Postion_Tool_Rotate,
  end_Postion_Tool_Height,
  end_Postion_Tool_Lock,
  end_Postion_QuickSwapHotend_Lock,
  end_Postion_HolderRotate,
  end_Postion_Cutter_Rotate,
  end_Postion_Cutter_Action,
  end_Postion_WastCup_Action,
};

int ServoAngleAdjustments[numOfServos];
int currentServo = 0;
int msAtLastLcdRefresh = 0;
bool screenRefreshRequired = true;
bool servoRefreshRequired = false;
bool currentServoRefreshRequired = false;
int buttonPress;
bool buttonPressed;

const unsigned long debounceDelay = 75;  // Debounce delay in milliseconds
Button lastButton = NONE;
Button stableButton = NONE;
unsigned long lastDebounceTime = 0;

void setup() {
  Serial.begin(9600);

  lcd.begin(16, 2);

  byte tmp;
  for (int x = 0; x < 8; x++) {
    tmp = EEPROM.read(x);
    Serial.println(tmp);
    if (tmp < 100 || tmp > 140) {
      EEPROM.write(x, 120);
    }
  }

  // read the eeprom servo values into memory
  int MapMe = 0;
  for (int x = 0; x < 8; x++) {
    MapMe = map(EEPROM.read(x), 100, 140, -20, 20);
    ServoAngleAdjustments[x] = MapMe;
  }

  // pin #, max angle, start angle, current angle
  servos[s_Tool_Rotate][eePinNum] = 15;
  servos[s_Tool_Rotate][eeMaxAngle] = 360;
  servos[s_Tool_Rotate][eeCurrentAngle] = start_Postion_Tool_Rotate;
  servos[s_Tool_Height][eePinNum] = 14; // using this pwm servo port on the servo shield causes random bytes on the serial lines
  servos[s_Tool_Height][eeMaxAngle] = 180;
  servos[s_Tool_Height][eeCurrentAngle] = start_Postion_Tool_Height;
  servos[s_Tool_Lock][eePinNum] = 13;
  servos[s_Tool_Lock][eeMaxAngle] = 280; //micro
  servos[s_Tool_Lock][eeCurrentAngle] = start_Postion_Tool_Lock;
  servos[s_QuickSwapHotend_Lock][eePinNum] = 12; //s_QuickSwapHotend_Lock
  servos[s_QuickSwapHotend_Lock][eeMaxAngle] = 180;
  servos[s_QuickSwapHotend_Lock][eeCurrentAngle] = start_Postion_QuickSwapHotend_Lock;
  servos[s_Holder_Rotate][eePinNum] = 11; //s_ToolHolder_Rotate
  servos[s_Holder_Rotate][eeMaxAngle] = 360;
  servos[s_Holder_Rotate][eeCurrentAngle] = start_Postion_HolderRotate;
  servos[s_Cutter_Rotate][eePinNum] = 10; //s_Cutter_Rotate
  servos[s_Cutter_Rotate][eeMaxAngle] = 180;
  servos[s_Cutter_Rotate][eeCurrentAngle] = start_Postion_Cutter_Rotate;
  servos[s_Cutter_Action][eePinNum] = 9; //s_Cutter_Action
  servos[s_Cutter_Action][eeMaxAngle] = 180;
  servos[s_Cutter_Action][eeCurrentAngle] = start_Postion_Cutter_Action;
  servos[s_WasteCup_Action][eePinNum] = 8; //s_WasteCup_Action
  servos[s_WasteCup_Action][eeMaxAngle] = 280; //micro
  servos[s_WasteCup_Action][eeCurrentAngle] = start_Postion_WastCup_Action;

  // initialize the servo shield
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(300);  // Digital servos run at 300Hz updates

  servoRefreshRequired = false; // this should be FALSE out because if a User accidentally leaves the setup firmware on the controller and assembles the Swapper3D and then powers on then arms and parts are going to crash and break.
  screenRefreshRequired = true;
  currentServoRefreshRequired = false;
  lcd.clear();

  // Added May 10th 2024: found that sometimes the servos were not energizing when the system was powered on
  // and they were not moving to their starting position as saved in the eeprom
  // this could cause an issue where the user following the instruction of setting the start positions
  // places the horn on the output shaft in what they think is the first counter-clockwise spline position
  // but in actuality it would not be that position
  // this could result in the wrong position being saved
  // the following code ensures that each servo is energized as soon as the power is on.
  for (int CurServo = 0; CurServo < 8; CurServo++) {
    pulselength = map(servos[CurServo][eeCurrentAngle] + ServoAngleAdjustments[CurServo], 0, servos[CurServo][eeMaxAngle], 600, 2900);
    pwm.setPWM(servos[CurServo][eePinNum], 0, pulselength);
  }
}

void loop() {
  int buttonValue = analogRead(A0);  // Read the analog value from A0

  Button currentButton = getButton(buttonValue);

  if (screenRefreshRequired) {
    refreshScreen();
  }

  if (servoRefreshRequired) {
    refreshServo();
  }

  if (currentServoRefreshRequired) {
    pulselength = map(servos[currentServo][eeCurrentAngle] + ServoAngleAdjustments[currentServo], 0, servos[currentServo][eeMaxAngle], 600, 2900);
    pwm.setPWM(servos[currentServo][eePinNum], 0, pulselength);
  }

  // Guard clause: If the value is not a valid button value, wait and exit
  if (currentButton == NONE) {
    // Serial.print("Value ignored because it's a non-button value: ");
    // Serial.println(buttonValue);
    delay(50);
    return;
  }

  if (currentButton == UNKNOWN) {
    //Serial.print("Invalid button value detected: ");
    //Serial.println(buttonValue);
    lastButton = currentButton;
    delay(50);
    return;
  }

  if (currentButton != lastButton) {
    lastDebounceTime = millis();  // Update the debounce time
    //Serial.print("Button changed to: ");
    //Serial.println(currentButton);
    lastButton = currentButton;
  }

  // Guard clause: Check if the debounce delay has passed
  if ((millis() - lastDebounceTime) <= debounceDelay) {
    //Serial.println("Waiting for debounce delay");
    delay(50);
    return;
  }

  // Guard clause: Check if the button value has stabilized
  if (currentButton != lastButton) {
    //Serial.println("Button value has not stabilized");
    delay(50);
    return;
  }

  // Update the stable button value if the debounce period has passed and the value is stable
  if (currentButton != stableButton) {
    stableButton = currentButton;
    //Serial.print("Stable button value updated to: ");
    //Serial.println(currentButton);
  }

  // Print the button name
  //Serial.print("Button Value: ");
  //Serial.print(buttonValue);
  //Serial.print(" - Button: ");
  
  switch (stableButton) {
    case UP:
      Serial.println("UP");
      handleUpButton();
      break;
    case DOWN:
      Serial.println("DOWN");
      handleDownButton();
      break;
    case LEFT:
      Serial.println("LEFT");
      handleLeftButton();
      break;
    case RIGHT:
      Serial.println("RIGHT");
      handleRightButton();
      break;
    case SELECT:
      Serial.println("SELECT");
      handleSelectButton();
      break;
    default:
      Serial.println("UNKNOWN BUTTON");
      break;
  }

  delay(50);  // Small delay to help with debouncing
}

Button getButton(int buttonValue) {
  if (buttonValue >= 0 && buttonValue <= 36) {
    return RIGHT;
  } else if (buttonValue >= 53 && buttonValue <= 101) {
    return UP;
  } else if (buttonValue >= 255 && buttonValue <= 261) {
    return DOWN;
  } else if (buttonValue >= 407 && buttonValue <= 411) {
    return LEFT;
  } else if (buttonValue >= 638 && buttonValue <= 648) {
    return SELECT;
  } else if (buttonValue > 650) {
    return NONE;
  } else {
    return UNKNOWN;
  }
}

bool isValidButtonValue(int buttonValue) {
  Button button = getButton(buttonValue);
  if (button == NONE || button == UNKNOWN) {
    Serial.print("Invalid button value detected in isValidButtonValue: ");
    Serial.println(buttonValue);
    return false;
  }
  return true;
}

void handleUpButton() {
  if (ServoAngleAdjustments[currentServo] < 0) {
    ServoAngleAdjustments[currentServo]++;
    screenRefreshRequired = true;
    currentServoRefreshRequired = true;
  }
  delay(250); //100 is good for full angle
}

void handleDownButton() {
  if (ServoAngleAdjustments[currentServo] > -20) {
    ServoAngleAdjustments[currentServo]--;
    screenRefreshRequired = true;
    currentServoRefreshRequired = true;
  }
  delay(250); //100 is good for full angle
}

void handleLeftButton() {
  if (currentServo > 0) {
    currentServo--;
    screenRefreshRequired = true;
  }
  delay(250);
}

void handleRightButton() {
  if (currentServo < numOfServos - 1) {
    currentServo++;
    screenRefreshRequired = true;
  }
  delay(250);
}

void handleSelectButton() {
  // save the servo values to eeprom
  // the range is -20 to 20 which maps to 100 to 140 so it can be stored as a byte
  byte tmp2 = 0;
  for (int x = 0; x < 8; x++) {
    if (x < 100 || x > 140) {
      tmp2 = map(ServoAngleAdjustments[x], -20, 20, 100, 140);
      EEPROM.write(x, tmp2);
    }
  }

  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Settings Saved");
  delay(1500);

  screenRefreshRequired = true;
  servoRefreshRequired = true;
}

void refreshScreen() {
  screenRefreshRequired = false;

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(servos_names[currentServo]);
  lcd.setCursor(0, 1);
  lcd.print(ServoAngleAdjustments[currentServo]);
}

void refreshServo() {
  servoRefreshRequired = false;

  // set servos to end positions
  // refresh ONLY the ONE servo that is currently being worked on.
  pulselength = map(servo_EndPositions[currentServo] + ServoAngleAdjustments[currentServo], 0, servos[currentServo][eeMaxAngle], servo_pwm_min_360, servo_pwm_max_360);
  pwm.setPWM(servos[currentServo][eePinNum], 0, pulselength);

  delay(3000);

  pulselength = map(servos[currentServo][eeCurrentAngle] + ServoAngleAdjustments[currentServo], 0, servos[currentServo][eeMaxAngle], 600, 2900);
  pwm.setPWM(servos[currentServo][eePinNum], 0, pulselength);

  delay(500);
}
