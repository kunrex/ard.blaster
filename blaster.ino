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

struct Motor
{
  public:
    unsigned int in1;
    unsigned int in2;
    unsigned int pwm;
    unsigned int stby;
};

int triggerIn;
int flywheelIn;

bool isShooting;

Motor triggerMotor;
Motor flywheelMotor;

FireState fireState;
unsigned int timeModifier;
unsigned long previousTime;

void initMotor(Motor* motor, unsigned int in1, unsigned int in2, unsigned int pwm, unsigned int stby)
{
  motor->in1 = in1;
  motor->in2 = in2;
  motor->pwm = pwm;
  motor->stby = stby;

  pinMode(motor->in1, INPUT);
  pinMode(motor->in2, INPUT);
  pinMode(motor->pwm, INPUT);
  pinMode(motor->stby, INPUT);
}

void setup() {
  pinMode(firePin, INPUT);
  pinMode(triggerPin, INPUT);
  pinMode(flywheelPin, INPUT);
  
  triggerIn = 0;
  flywheelIn = 0;

  isShooting = false;

  initMotor(&triggerMotor, 11, 10, 9, 8); 
  initMotor(&flywheelMotor, 4, 5, 6, 7);

  fireState = Single;

  timeModifier = 1;
  previousTime = 0;

  //Serial.begin(9600);
}

void manageFireState()
{
  int state = round(3 * analogRead(firePin))/ 1023;

  if(state == 0)
  {
    fireState = Single;
    timeModifier = 1;
  }
  else if(state == 1)
  {
    fireState = Burst;
    timeModifier = 3;
  }
  else if(state == 2)
  {
    fireState = Full;
    timeModifier = 1;
  }
}

void motorDrive(Motor* motor, bool motorDirection, int motorSpeed = 255)
{
  bool pinIn1;  
      
  //Clockwise: In1 = HIGH and In2 = LOW
  //Counter-Clockwise: In1 = LOW and In2 = HIGH
  if (motorDirection)
    pinIn1 = HIGH;
  else
    pinIn1 = LOW;

  digitalWrite(motor->in1, pinIn1);//drive motors
  digitalWrite(motor->in2, !pinIn1);  
  analogWrite(motor->pwm, motorSpeed);//speed

  digitalWrite(motor->stby, HIGH);
}

void motorStop(Motor* motor)
{
  digitalWrite(motor->in1, LOW);
  digitalWrite(motor->in2, LOW);  
}

void motorBrake(Motor* motor)
{
  analogWrite(motor->pwm, 0); 
  digitalWrite(motor->stby, LOW);
}

void loop() {
  int previousState = triggerIn;
  triggerIn = digitalRead(triggerPin);

  flywheelIn = digitalRead(flywheelPin);

  manageFireState();

  unsigned long currentTime = millis();
  if(flywheelIn == HIGH && previousState == LOW && triggerIn == HIGH)
  {
    isShooting = true;
    previousTime = currentTime;
  }

  if(isShooting && (currentTime - previousTime) >= fireRate * timeModifier)
  {
    if(fireState == Full)
    {
      if(triggerIn == LOW)
        isShooting = false;
      else
        previousTime = currentTime;
    }
    else
      isShooting = false;
  }

  if(flywheelIn == HIGH)
    motorDrive(&flywheelMotor, true);
  else
    motorBrake(&flywheelMotor);
  
  if(isShooting)
    motorDrive(&triggerMotor, true);
  else
    motorStop(&triggerMotor);
}
