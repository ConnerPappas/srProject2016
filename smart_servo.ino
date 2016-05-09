#include <Servo.h>

#define NUMREADINGS 10                                                              // number of readings to take for smoothing
#define NUMSERVOS 2                                                                 // number of servos
#define NUMCALS 2                                                                   // number of calibration endpoints

Servo flexor[NUMSERVOS];
int flexorPins[] = {11, 9};                                                          // these are the pins the servos are plugged into
int feedbackPins[] = {4, 5};                                                        // these are the analog pins for reading
int calVal[][NUMCALS] = {{0, 0}, {0, 0}};                                           // initialize cal values
int previousAngle[] = {0, 0};
int previousPosition[] = {0, 0};
int currentAngle[] = {0, 0};
int currentPosition[] = {0, 0};
bool keepGoing = true;
bool start = false;
bool firstTime = true;
char readVari;

int servoPosition[NUMSERVOS] = {0, 0};                                                       // initialize the servo positions
int trash = 0;                                                                     // ??????????

int readings[NUMSERVOS][NUMREADINGS];                                               // 2D array of readings for smoothing each servo
int total = 0;                                                               // running total of position while smoothing for each servo          

void setup() 
{
  Serial.begin(9600);
  analogReference(EXTERNAL);                                                        // use the external analog reference
  for (int i = 0; i < NUMSERVOS; i++)                                               // attach both servos and gather the endpoints for calibration
  {
    flexor[i].attach(flexorPins[i]);
    flexor[i].write(0);
    //delay(2000);
    //calibrate (flexor[i], i);
  }
}

void loop() 
{  
  if (Serial.available() > 0)
  {
    readVari = Serial.read();
    if (readVari == 't')
    {
      start = true;
    }
    else
    {
      start = false;
    }
  }
  
  if (keepGoing && start)
  {
    if (firstTime)
    {
      for (int i = 0; i < NUMSERVOS; i++)
      {
        servoPosition[i] = smooth(i);                                                  // this if statement is here to cut down the smoothing time on each iteration
        Serial.print("Current position for ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(servoPosition[i]);
      }
      firstTime = false;  
    }
    for (int i = 0; i < NUMSERVOS; i++)                                              // at the beginning of each loop we will smooth out their position
    {
      previousPosition[i] = servoPosition[i];
      //previousAngle[i] = map(servoPosition[i], calVal[i][0], calVal[i][1], 0, 180);  // get the angle of the servo at the beginning of the iteration
      Serial.print("Prevoius position for ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(previousPosition[i]);
    }
    for (int i = 0; i < NUMSERVOS; i++)
    {
      flexor[i].write(previousAngle[i] + 9);
      delay(300);
      previousAngle[i] += 9;
    }
    for (int i = 0; i < NUMSERVOS; i++)
    {
      servoPosition[i] = smooth(i);
      Serial.print("Current position for ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(servoPosition[i]);
      currentPosition[i] = servoPosition[i];
    }
    if (currentPosition[0] <= previousPosition[0] || currentPosition[1] <= previousPosition[1])
    {
      keepGoing = false;
      for (int i = 0; i < NUMSERVOS; i++)
      {
        flexor[i].detach();
      }
    }
  }
}

void calibrate (Servo servoTo, int which)
{ 
  servoTo.write(0);
  //delay(1000);             
  calVal[which][0] = smooth(which);
  servoTo.write(180);
  delay(1000);
  calVal[which][1] = smooth(which);
  servoTo.write(0);
  for (int i = 0; i < NUMSERVOS; i++)
  {
    Serial.print("Calbration value ");
    Serial.print(i);
    Serial.print(" for ");
    Serial.print(which);
    Serial.print(": ");
    Serial.println(calVal[which][i]);
  }
}

int smooth(int which) 
{
  total = 0;
  for (int j = 0; j < NUMREADINGS; j++)
  {
    total += analogRead(feedbackPins[which]);                                            // add the reading to the total
  }       
  return total / NUMREADINGS;                                                // calculate the average and return it
}
