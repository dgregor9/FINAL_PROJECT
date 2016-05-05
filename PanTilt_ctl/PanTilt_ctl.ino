/* 
   -- ECE590 - Robot Design --
   David Gregory - Final Project
   OpenCM9.04 - Motor Control & USB Interface
   
   Controls 2 Dynamixel motors - Pan/Tilt Configuration
   
   Controlled by "ARC Controller's" touchpad.
   Uses the Touchpad (X,Y) coordinates to adjust view angle
   
*/

 /* Serial device defines for DXL1 bus */ 
#include <stdio.h>
#include <string.h>

#define DXL_BUS_ID 1  //Dynamixel on Serial1(USART1)  <-OpenCM9.04

/* Dynamixel Motor IDs */
#define PAN_MOTOR 1  //(X coordinate)
#define TILT_MOTOR 3   //(Y coordinate)

#define PAN_SPEED  200
#define TILT_SPEED  200
#define PAN_CENTER 512
#define TILT_CENTER 512

#define TORQ 1000

byte *StringBuffer;
byte StringCount;

int rxFlag;    //Interrupt flag 
#define OFF 0x00
#define ON  0x01


Dynamixel Dxl(DXL_BUS_ID);  //initiate TTL bus

void setup(){
  //Attach  serialUSB interrupt
  SerialUSB.attachInterrupt(usbInterrupt);
  
  rxFlag = OFF;  //clear SerialUSB RX flag
    
  pinMode(BOARD_LED_PIN, OUTPUT);  //toggleLED_Pin_Out; For FUN!!
  
  // Initialize the dynamixel bus:
  // Dynamixel 2.0 Baudrate -> 0: 9600, 1: 57600, 2: 115200, 3: 1Mbps 
  Dxl.begin(3);
  Dxl.jointMode(PAN_MOTOR);  //Using Position settings for motors
  Dxl.jointMode(TILT_MOTOR);
  
  Dxl.writeByte(PAN_MOTOR, 24, 0);  //Disable Torque Enable; NEEDED before changing CW/CCW Angles Limits
  Dxl.writeByte(TILT_MOTOR, 24, 0);  //Disable Torque Enable; NEEDED before changing CW/CCW Angles Limits
  
  Dxl.writeByte(PAN_MOTOR, 13, 50); //Set lower Limit Voltage to 5 volts
  Dxl.writeByte(TILT_MOTOR, 13, 50); //Set lower Limit Voltage to 5 volts
  
  
  Dxl.ccwAngleLimit(PAN_MOTOR, 1023);
  Dxl.ccwAngleLimit(TILT_MOTOR, 1023);
  Dxl.cwAngleLimit(PAN_MOTOR, 0);
  Dxl.cwAngleLimit(TILT_MOTOR, 0);
  
  
  Dxl.maxTorque(PAN_MOTOR, TORQ);
  Dxl.maxTorque(TILT_MOTOR, TORQ);
  
  Dxl.goalSpeed(PAN_MOTOR, PAN_SPEED);  
  Dxl.goalSpeed(TILT_MOTOR, TILT_SPEED);
  
  Dxl.goalPosition(PAN_MOTOR, PAN_CENTER);  //Center Servos
  Dxl.goalPosition(TILT_MOTOR, TILT_CENTER);    //Center Servos
   
  toggleLED();  
   
}//End setup()



///////////////////////////////////////////////////////////
//USB max packet data is maximum 64byte, so nCount can not exceeds 64 bytes
void usbInterrupt(byte* buffer, byte nCount){
  
  rxFlag = ON;            //set flag
  StringBuffer = buffer;  //share buffer address
  buffer[nCount] = '\0';  //add null to end of buffered string
  StringCount = nCount + 1;   //copy buffer size
  
} //end usbInterrupt


void loop(){
  int Xcord;
  int Ycord;
  char Xbuf[9];
  char Ybuf[9];
  int i;
  int y_offset;
  
  //Flash the led
  //toggleLED();
  //delay(1000);
  
    /**********************************/
    if(rxFlag == ON){    //wait for flag
       
       rxFlag = OFF;    //clear flag

      
      /******** Troubleshooting *********
      for(i=0; i < StringCount; i++){  //printf_SerialUSB_Buffer[N]_receive_Data
        SerialUSB.print((char)StringBuffer[i]);
      }
      SerialUSB.println("");
      **********************/
      
      //Clear Buffers//
      for(i=0; i < 9; i++){
        Xbuf[i] = 0;
        Ybuf[i] = 0;
      }
      
        // Parse string for (X,Y) coordinates of Pan/Tilt//
          i=0;
          while(StringBuffer[i] != 32){
            Xbuf[i] = (char)StringBuffer[i]; //copy x value
            //SerialUSB.print((char)Xbuf[i]);
            i++;
          } //end Xbuf parse
          
          i++; //inc i
          y_offset = i; //capture y buffer offset
          while(StringBuffer[i] != 0){
           Ybuf[i - y_offset] = StringBuffer[i]; //record value in first buffer
           i++;
          } //end Ybuf parse
        
        Xcord = (int)strtol(Xbuf, NULL, 10);  //convert char to int
        Ycord = (int)strtol(Ybuf, NULL, 10);  
       
        /******Troubleshooting******/
        SerialUSB.print("Xcord = ");
        SerialUSB.println(Xcord);
        SerialUSB.print("Ycord = ");
        SerialUSB.println(Ycord);
        /***************************/
     
        /*********** UPDATE PAN/TILT Goal POSITION *********/
        Dxl.goalPosition(PAN_MOTOR, Xcord);    //Sent from ARC touchpad
        Dxl.goalPosition(TILT_MOTOR, Ycord);
        
        delay(100);  //allow time for motor to move
             
  }//end if(rxFlag)
  /********************************/
  
  //Dxl.writeByte(TILT_MOTOR, 18, 0);
  
}//end loop

