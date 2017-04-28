#include "simpletools.h"
#include "abdrive.h"  
#include "ping.h"      
#include "servo.h" 
#include "adcDCpropab.h" 

void QTI();
void IRSensor();
void Turret();
void Encoder();

volatile int stop = 0;
volatile int foodCount = 0;
volatile int encode = 1;
volatile int distanceIterationsL = 0;
volatile int distanceIterationsR = 0;
volatile int leftDistance;
volatile int rightDistance;
volatile int forwardDistance;
volatile long qtiTime;
volatile float IRVoltage;
volatile int distanceIterationsL;
volatile int distanceIterationsR;

int question = 0;

int LED0 = 26;
int LED1 = 27;

int QTIPin = 16; //?
int greyThreshhold = 0; //?
int blackThreshhold = 0; //?
int adcQTI =  1;

int adcIR = 0; 

int ultrasonicPin = 8;
int servoPin = 16;

int leftEncoderPin = 14;
int  rightEncoderPin = 15;

int main()
{
	adc_init(21, 20, 19, 18); 
	
 cog_run(QTI,128);
	//cog_run(IRSensor,32);
	//cog_run(Turret,256);
	//cog_run(Encoder,128);

 pause(2000);
 
  while(1)
  {
    putChar(HOME);                         
    print("Left = %d %c\n", leftDistance, CLREOL);
    print("Right = %d %c\n", rightDistance, CLREOL);
   
    print("Forward = %d %c\n\n", forwardDistance, CLREOL);
     print("EncoderL = %d %c\n", distanceIterationsL, CLREOL);
    print("EncoderR = %d %c\n\n", distanceIterationsR, CLREOL);
    print("qtiTime = %d %c\n", qtiTime, CLREOL);
    print("IRVoltage = %f %c\n", IRVoltage, CLREOL);
    
    pause(100);
  }  
}

void QTI()
{
	int colorChange = 1;
	while(1)
	{
		high(QTIPin);
		pause(5);
   low(QTIPin);
   qtiTime = rc_time(QTIPin,0);
   
		low(QTIPin);
		if(qtiTime>blackThreshhold&&colorChange)
		{
			stop = 1;
			colorChange = 0;
		}
		else
		if(qtiTime>greyThreshhold&&colorChange)
		{
			foodCount+=1;
			colorChange = 0;
			high(LED1);
		}
	
		if(qtiTime<greyThreshhold)
		{
			colorChange = 1;
			low(LED1);
		}
   pause(100);
	}
}

void IRSensor()
{
	float voltage;
	float IRDistance = 3;
	
	while(1)
	{
		voltage = adc_volts(adcIR);
		if(voltage>IRDistance)
		{
			stop = 1;
		}
   IRVoltage = voltage;
	}
 
}
void Turret()
{
	int distanceL;
	int distanceR;
  
	int prevL = 0;
	int prevR=0;
	int turretDelay = 600;
	int sweepDistance = 2.4*0.325*10;
	int width = question;
	int sameDistance = 0;
	while(1)
	{
	    servo_angle(servoPin, 0);                      
      pause(turretDelay);    
      distanceR=ping_cm(ultrasonicPin);   
      rightDistance = distanceR; 
          
                                
      servo_angle(servoPin, 900);                  
      pause(turretDelay);     
      forwardDistance=ping_cm(ultrasonicPin);   
   
                 
      servo_angle(servoPin, 1800);                  
      pause(turretDelay*2);     
      distanceL=ping_cm(ultrasonicPin); 
      leftDistance = distanceL;
      
         
      servo_stop(); 
	  
	  if(prevL==distanceL&&prevR==distanceR)
	  {
		sameDistance+=1;
	  }
	  else
	  {
		  encode = 0;
		  float theta = 1; //From Compass
     int adjustDist = 5;
     int adjustAngle =5;
     int shimmeyDelay = 500;
		  if(distanceL-distanceR>0)
		  {
			drive_goto(adjustAngle,-adjustAngle);pause(shimmeyDelay);
			drive_goto(-adjustDist,-adjustDist);pause(shimmeyDelay);
			drive_goto(-adjustAngle,adjustAngle);pause(shimmeyDelay);
			drive_goto(adjustDist,adjustDist);pause(shimmeyDelay);
		  }            
		  if(distanceR-distanceL>0)
		  {
			drive_goto(-adjustAngle,adjustAngle);pause(shimmeyDelay);
			drive_goto(-adjustDist,-adjustDist);pause(shimmeyDelay);
			drive_goto(adjustAngle,-adjustAngle);pause(shimmeyDelay);
			drive_goto(adjustDist,adjustDist);pause(shimmeyDelay);
		  }       
		  /*int offset = 2000*theta/(width*0.325);
		  if(distanceL>distanceR)
		  {
  			drive_speed(0,2);
  			pause(offset);
		  }
     if(distanceL>distanceR)
		  {
  			drive_speed(2,0);
  			pause(offset);
		  }
     			drive_speed(10,10);*/
		  encode = 1;
	  }
	}
}

void Encoder() 
{
	 int leftEncoder = 0;
	 int rightEncoder = 0;
	 int prevLeftState = 0;
	 int prevRightState = 0;
	 while(1)
	 {
		leftEncoder=input(leftEncoderPin);
		rightEncoder=input(rightEncoderPin);
		if(leftEncoder != prevLeftState)
		{
		   distanceIterationsL+=1;
		} 
		if(rightEncoder != prevRightState)
		{
		   distanceIterationsR+=1;
		} 
   prevLeftState = leftEncoder;
   prevRightState = rightEncoder;
		pause(1);
	 }
	 
}