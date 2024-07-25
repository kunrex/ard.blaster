#include <math.h>

#define firePin A3
#define triggerPin 13
#define flywheelPin 12

#define fireRate 650

enum FireState 
{
  Full = 0,
  Burst = 1,
  Single = 2
};

class MotorDriver
{
  private:
    //stby connected to 5V supply
    unsigned int in1;
    unsigned int in2;
    unsigned int pwm;
    unsigned int stby;

  public:
    MotorDriver() { }

    MotorDriver(unsigned int _in1, unsigned int _in2, unsigned int _pwm, unsigned int _stby)
    {
      in1 = _in1;
      in2 = _in2;
      pwm = _pwm;
      stby = _stby;

      pinMode(in1, OUTPUT);
      pinMode(in2, OUTPUT);
      pinMode(pwm, OUTPUT);
      pinMode(stby, OUTPUT);
    }

    void motorDrive(bool motorDirection, int motorSpeed = 255)//motorDirection true = clockwise
    {
      bool pinIn1;  
      
      //Clockwise: In1 = HIGH and In2 = LOW
      //Counter-Clockwise: In1 = LOW and In2 = HIGH
      if (motorDirection)
        pinIn1 = HIGH;
      else
        pinIn1 = LOW;

      digitalWrite(in1, pinIn1);//drive motors
      digitalWrite(in2, !pinIn1);  
      analogWrite(pwm, motorSpeed);//speed

      digitalWrite(stby, HIGH);//disable stby
    }

    void motorStop()
    {
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);  
    }

    void motorBrake()
    {
      analogWrite(pwm, 0); 

      digitalWrite(stby, LOW);//disable stby
    }
};

int* triggerIn;
int* flywheelIn;

bool isShooting;

MotorDriver triggerMotor;
MotorDriver flywheelMotor;

FireState fireState;
unsigned int* timeModifier;
unsigned long* previousTime;

void setup() {
  pinMode(firePin, INPUT);
  pinMode(triggerPin, INPUT);
  pinMode(flywheelPin, INPUT);
  
  triggerIn = new int(0);
  flywheelIn = new int(0);

  isShooting = false;

  triggerMotor =  MotorDriver(11, 10, 9, 8); 
  flywheelMotor = MotorDriver(4, 5, 6, 7);

  fireState = Single;

  timeModifier = new int(1);
  previousTime = new long(0);

  Serial.begin(9600);
}

void manageFireState()
{
  int state = round(3 * analogRead(firePin))/ 1023;

  if(state == 0)
  {
    fireState = Single;
    *timeModifier = 1;
  }
  else if(state == 1)
  {
    fireState = Burst;
    *timeModifier = 3;
  }
  else if(state == 2)
  {
    fireState = Full;
    *timeModifier = 1;
  }
}

void loop() {
  int previousState = *triggerIn;
  *triggerIn = digitalRead(triggerPin);

  *flywheelIn = digitalRead(flywheelPin);

  manageFireState();

  unsigned long currentTime = millis();
  if(*flywheelIn == HIGH && previousState == 0 && (previousState != *triggerIn))
  {
    isShooting = true;
    *previousTime = currentTime;
  }

  if(isShooting && (currentTime - *previousTime >= fireRate * (*timeModifier)))
  {
    if(fireState == Full)
    {
      if(*triggerIn == LOW)
        isShooting = false;
      else
        *previousTime = currentTime;
    }
    else
      isShooting = false;
  }

  if(*flywheelIn == HIGH)
    flywheelMotor.motorDrive(true);
  else
    flywheelMotor.motorStop();
  
  if(isShooting)
    triggerMotor.motorDrive(true);
  else
    triggerMotor.motorStop();
}
