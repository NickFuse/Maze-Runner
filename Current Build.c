#include "simpletools.h"
#include "abdrive.h"  
#include "ping.h"      
#include "servo.h" 
#include "adcDCpropab.h" 



void QTI();
void Turret();
void Encoder();
void Compass();

volatile float angle;
volatile int currentBearing;
volatile int distanceIterationsL = 0;
volatile int distanceIterationsR = 0;
volatile int drive = 1;
volatile int encode = 1;
volatile int foodCount = 0;
volatile int forwardDistance = 1;
volatile int leftDistance;
volatile long qtiTime;
volatile int qtiAvg = 1000;
volatile int rightDistance;
volatile int shimmey = 0;
volatile int stop = 0;
volatile int turnDecider = 100;
volatile int turret = 1;
volatile int X;
volatile int Y;


int question = 0;
int turnConstant = 10200;

int LED0 = 26;
int LED1 = 27;

int QTIPin = 16; 
int greyThreshhold = 0; //?
int blackThreshhold = 0; //?
int adcQTI =  1;

int adcIR = 0; 

int ultrasonicPin = 8;
int servoPin = 17;

int leftEncoderPin = 14;
int  rightEncoderPin = 15;

int ioPin = 9;
int enablePin = 10;
int clkPin   =  11;

int main()
{
	adc_init(21, 20, 19, 18); 
	sd_mount(22,23,24,25);
  cog_run(QTI,64);
	cog_run(Turret,256);
	cog_run(Encoder,32);
  //state initialization
  pause(500);
  Mapper();

  while(1)
  {
    putChar(HOME);                         
    print("Left = %d %c\n", leftDistance, CLREOL);
    print("Right = %d %c\n", rightDistance, CLREOL);
    print("Forward = %d %c\n\n", forwardDistance, CLREOL);
    print("EncoderL = %d %c\n", distanceIterationsL, CLREOL);
    print("EncoderR = %d %c\n\n", distanceIterationsR, CLREOL);
    print("qtiTime = %d %c\n", qtiTime, CLREOL);
    
    pause(100);
  }  
}


void Mapper()
{
  drive_ramp(20,20);
  int k = 1;
  int exp = 1;
  int leftAvg = leftDistance;
  int rightAvg = rightDistance;
  while(1)
  {
    print("Left = %d %c\n", leftDistance, CLREOL);
    print("Right = %d %c\n", rightDistance, CLREOL);
    print("Forward = %d %c\n\n", forwardDistance, CLREOL);
    print("TURN? = %d %c\n\n", turnDecider, CLREOL);
    print("QTI = %d %c\n\n", qtiAvg, CLREOL);
    rightAvg = rightAvg*.8+rightDistance*0.2;
    leftAvg = leftAvg*.8+leftDistance*0.2;
    if(leftDistance==0||rightDistance==0||qtiAvg>2000||!drive)
    {
      drive_ramp(0,0);
    }    
    else  
    if(forwardDistance<4)
    {
      encode = 0;
      drive_ramp(0,0);
      while(forwardDistance<10)
      {
        drive_speed(-20,-20);
      }      
      drive_ramp(0,0);
      encode = 1;
    }    
      if(forwardDistance<10)
      {  
        pause(400);
        if(rightAvg+leftAvg<17)
        {
          turn(1);
        }
        else          
        if(rightAvg+leftAvg<20)
        {
          print("U Turn",CLREOL);
          turn(1);
          
          turn(1);
          
        }    
        else
        if(leftAvg>15&&rightAvg>15)
        {
           print("Default Turn",CLREOL);
          turn(-1); 
         
        }        
        else
        if(rightAvg<15)
        {
          print("Left Turn",CLREOL);
          turn(1); 
     
        }
        else
        if(leftAvg<15)
        {
          print("Right Turn",CLREOL);
          turn(-1);
          
        }
        
        drive_ramp(20,20);  
      }    
    else  
    if(rightDistance<20)
      {
        int rightPow = (int)(k*pow(rightDistance,exp));
        if(rightDistance>9)
        drive_speed(12+rightPow,20);
        else
        if(rightDistance<8)
        drive_speed(20,29-rightPow);       
      } 
      else
      if(leftDistance<20)
      {
        
        int leftPow = (int)(k*pow(leftDistance,exp));
        if(leftDistance>9)
        drive_speed(20,12+leftPow);
        else
        if(leftDistance<8)
        drive_speed(29-leftPow,20);       

        
      }

      else
      drive_speed(20,20);
      
      if(forwardDistance>30 && turnDecider > 85 && (leftAvg >30 || rightAvg>30))
      {
        turnDecider =0;
        high(LED0);
        high(LED1);
        pause(100);
        low(LED0);
        low(LED1);
        
        if(input(14))
        {
          pause(300);
          drive_ramp(0,0);
   
          if(leftAvg>20)
            turn(1);
          else
            turn(-1);

          drive_speed(20,20);    
           while(turnDecider<100)
           {
           }       
        }        
          
      }    
        

    pause(100); 
  }   
}

void turn(int d)
{
   turnDecider = 0;
   drive_ramp(-10*d,10*d);
   pause(turnConstant/4);
   drive_ramp(0,0);
   drive_speed(20,20); 
    
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
   qtiAvg = qtiAvg*0.9+qtiTime*0.1;
		low(QTIPin);
    if(qtiAvg>2700)
      drive = 0;
		/*if(qtiTime>blackThreshhold&&colorChange)
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
		}*/
   pause(100);
	}
}

void Turret()
{
	while(1)
	{
   if(turret)
  	 survey();
   else
    pause(10);
  }   
}

void realign()
{
  float threshold = 0; //?
  int speed = 3;
  encode = 0;
  while((angle-currentBearing)*(angle-currentBearing)>threshold)
  {
     drive_speed(speed,-speed);
  }      
  encode = 1;  
  
}  

void survey()
{   
   int prevL = leftDistance;
   int prevR = rightDistance;
   leftDistance=ping_cm(ultrasonicPin+2); 
   pause(5);
   forwardDistance=ping_cm(ultrasonicPin);   
   pause(5); 
   rightDistance=ping_cm(ultrasonicPin+1);  
   pause(5);  
   if(leftDistance ==0)
    high(LED0);
    else
    low(LED0);
   if(rightDistance ==0)
    high(LED1);   
   else
   low(LED1);
   
}

void establishBearing()
{
   
}

void Encoder() 
{
	 int leftEncoder = 0;
	 int rightEncoder = 0;
	 int prevLeftState = 0;
	 int prevRightState = 0;
	 while(1)
	 {
     if(encode)
     {
    		leftEncoder=input(leftEncoderPin);
    		rightEncoder=input(rightEncoderPin);
    		if(leftEncoder != prevLeftState)
    		{
    		   distanceIterationsL+=1;
          turnDecider+=1;
    		} 
    		if(rightEncoder != prevRightState)
    		{
    		   distanceIterationsR+=1;
    		} 
       prevLeftState = leftEncoder;
       prevRightState = rightEncoder;
    }      
  		pause(1);
	 }
	 
}

/*struct state
{
  float x;
  float y;
  float bearing;
};*/