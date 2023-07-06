//Swapper3D setup firmware, Author: BigBrain3D, License: AGPLv3
#include<LiquidCrystal.h>
#include <Adafruit_PWMServoDriver.h> 
#include <EEPROM.h>
#include <String.h>
#include <Wire.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();



const int numOfServos = 8;

//Servos
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
int servos[numOfServos][3]; //pin #, max angle, current angle
int pulselength = 0;
const int servo_pwm_min_360 = 600; //484;
const int servo_pwm_max_360 = 2900;// 2376;

const String servos_names[numOfServos] = {
									  "Tool_Rotate"
									, "Tool_Height"
									, "Tool_Lock"
									, "QuickSwap_Lock"
									, "Holder_Rotate"
									, "Cutter_Rotate"
									, "Cutter_Action"
									, "WasteCup_Action"
								   };



//Servo Starting positions
const int start_Postion_Tool_Rotate = 104;
const int start_Postion_Tool_Height = 129;
const int start_Postion_Tool_Lock = 193; 
const int start_Postion_QuickSwapHotend_Lock = 70;
const int start_Postion_HolderRotate = 15;
const int start_Postion_Cutter_Rotate = 27;
const int start_Postion_Cutter_Action= 175;
const int start_Postion_WastCup_Action = 170; //110

const int end_Postion_Tool_Rotate = 293;
const int end_Postion_Tool_Height = 31;
const int end_Postion_Tool_Lock = 105; //115; 
const int end_Postion_QuickSwapHotend_Lock = 104;
const int end_Postion_HolderRotate = 360;
const int end_Postion_Cutter_Rotate = 123;
const int end_Postion_Cutter_Action= 15;
const int end_Postion_WastCup_Action = 125; //99

//start = 0, end = 1
const int servo_EndPositions[numOfServos]{
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


void setup()
{
  Serial.begin(9600);
  
  lcd.begin(16, 2); 
   
  
  //int MapMe = map(119, 100, 140, -20, 20);
    //  Serial.println(MapMe);
  
  
  
  //when saving eeprom values of the servo offsets
  //we use a byte 100 to 140
  //which maps to -20 to 20
  //if the eeprom values are outside the range
  //probably setup hasn't run before
  // so set them to "ZERO" in the range (20)
  byte tmp;
    for(int x=0; x<8; x++)
    {
      tmp = EEPROM.read(x);
      Serial.println(tmp);
      if(tmp<100 || tmp>140)
      {
        EEPROM.write(x,120); 
      }
    }
  

  	//read the eeprom servo values into memory
  	int MapMe = 0;
  	for(int x=0; x<8; x++)
    {	    	
      MapMe = map(EEPROM.read(x), 100, 140, -20, 20);
      
      ServoAngleAdjustments[x]= MapMe;       
    } 
  
  
  	//pin #, max angle, start angle, current angle
	servos[s_Tool_Rotate][eePinNum] = 15;
	servos[s_Tool_Rotate][eeMaxAngle] = 360;
	servos[s_Tool_Rotate][eeCurrentAngle] = start_Postion_Tool_Rotate;
	servos[s_Tool_Height][eePinNum] = 14; //using this pwm servo port on the servo shield causes random bytes on the serial lines
	servos[s_Tool_Height][eeMaxAngle] = 180;
	servos[s_Tool_Height][eeCurrentAngle] = start_Postion_Tool_Height;
	servos[s_Tool_Lock][eePinNum] = 13;
	servos[s_Tool_Lock][eeMaxAngle] = 280;//micro
	servos[s_Tool_Lock][eeCurrentAngle] = start_Postion_Tool_Lock;
	servos[s_QuickSwapHotend_Lock][eePinNum] = 12;//s_QuickSwapHotend_Lock
	servos[s_QuickSwapHotend_Lock][eeMaxAngle] = 180;//
	servos[s_QuickSwapHotend_Lock][eeCurrentAngle] = start_Postion_QuickSwapHotend_Lock;//
	servos[s_Holder_Rotate][eePinNum] = 11;//s_ToolHolder_Rotate
	servos[s_Holder_Rotate][eeMaxAngle] = 360;//
	servos[s_Holder_Rotate][eeCurrentAngle] = start_Postion_HolderRotate;//
	servos[s_Cutter_Rotate][eePinNum] = 10;//s_Cutter_Rotate
	servos[s_Cutter_Rotate][eeMaxAngle] = 180;//
	servos[s_Cutter_Rotate][eeCurrentAngle] = start_Postion_Cutter_Rotate;//
	servos[s_Cutter_Action][eePinNum] = 9;//s_Cutter_Action
	servos[s_Cutter_Action][eeMaxAngle] = 180;//
	servos[s_Cutter_Action][eeCurrentAngle] = start_Postion_Cutter_Action;//
	servos[s_WasteCup_Action][eePinNum] = 8;//s_WasteCup_Action
	servos[s_WasteCup_Action][eeMaxAngle] = 280;//micro
	servos[s_WasteCup_Action][eeCurrentAngle] = start_Postion_WastCup_Action;//
  

  //initialize the servo shield
	pwm.begin();
	pwm.setOscillatorFrequency(27000000);
	pwm.setPWMFreq(300);  // Digtal servos run at 300Hz updates

  
   //screenRefreshRequired = true; //this should be commented out because if a User accidentally leaves the setup firmware on the controller and assembles the Swapper3D and then powers on then arms and parts are going to crash and break.
   lcd.clear();
}

void loop()
{
  buttonPress = analogRead(A0);
		switch (buttonPress)
		{
      // no ups. Making sure that they can only go down. This enforces the Holder rotation.
      // UP
      case 50 ... 150:
        if(ServoAngleAdjustments[currentServo] < 0)
        {
          ServoAngleAdjustments[currentServo]++;
          screenRefreshRequired = true;
          currentServoRefreshRequired = true;
        }
        delay(250); //100 is good for full angle
      break;

			//*DOWN
			case 200 ... 300:
        if (ServoAngleAdjustments[currentServo] > -20)
        {
          ServoAngleAdjustments[currentServo]--;
          screenRefreshRequired = true;
          currentServoRefreshRequired = true;
        }
        delay(250);  //100 is good for full angle
      break;

			//LEFT
			case 350 ... 450:
        if(currentServo > 0)
        {
          currentServo--;
          screenRefreshRequired = true;
        }
        delay(250);
      break;

			//*RIGHT
			case 0 ... 49:
        if(currentServo < numOfServos - 1)
        {
          currentServo++;
          screenRefreshRequired = true;
        }
        delay(250);
      break;
      
			//*SELECT
			case 600 ... 700:
        //save the servo values to eeprom
        //the range is -20 to 20 which maps to 100 to 140 so it can be stored as a byte
        byte tmp2=0;
        for(int x=0; x<8; x++)
        {
          if(x<100 || x>140)
          {
            tmp2= map(ServoAngleAdjustments[x], -20, 20, 100, 140);

            EEPROM.write(x,tmp2); 
          }
        }
    
        lcd.setCursor(0,0);
        lcd.clear();
        lcd.print("Settings Saved");
        delay(1500);

        screenRefreshRequired = true;
        servoRefreshRequired = true;
			//*DEFAULT
			default:
			  buttonPressed = false;
			  break;
    }
  

  
  //millis() - msAtLastLcdRefresh > 100
     
  if (screenRefreshRequired)
  {
    screenRefreshRequired = false;

    
    lcd.setCursor(0,0);
    lcd.print("                ");   
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,0);
    lcd.print(servos_names[currentServo]);
    lcd.setCursor(0,1);
    //lcd.print(servos[currentServo][eeCurrentAngle]+ServoAngleAdjustments[currentServo]); //show full angle
    lcd.print(ServoAngleAdjustments[currentServo]); //show only adjustment



  }

  if (servoRefreshRequired)
  {
    servoRefreshRequired = false;
    
    //set servos to end positions
    //refresh ONLY the ONE servo that is currently being worked on.
    pulselength = map(servo_EndPositions[currentServo]+ServoAngleAdjustments[currentServo], 0, servos[currentServo][eeMaxAngle], servo_pwm_min_360, servo_pwm_max_360);
    pwm.setPWM(servos[currentServo][eePinNum], 0, pulselength);  
    
    delay(3000);

    pulselength = map(servos[currentServo][eeCurrentAngle]+ServoAngleAdjustments[currentServo], 0, servos[currentServo][eeMaxAngle], 600, 2900);
    pwm.setPWM(servos[currentServo][eePinNum], 0, pulselength);	   

    delay(500);
  }

  if(currentServoRefreshRequired)
  {
      pulselength = map(servos[currentServo][eeCurrentAngle]+ServoAngleAdjustments[currentServo], 0, servos[currentServo][eeMaxAngle], 600, 2900);
      pwm.setPWM(servos[currentServo][eePinNum], 0, pulselength);	 
  }
}
