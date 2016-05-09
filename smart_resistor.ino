#include <Servo.h>
#include <math.h>

const int MAXPOSITION = 180;
const int NUMSERVOS = 2;
const int NUMREADINGS = 10;
Servo flexor[NUMSERVOS];
int flexorPins[] = {9, 11}; //check before running
int feedbackPin = 3;
int currentPos = 1;
double total = 0.0;
double forcePoints[] = {0.0, 0.0};
double strainPoints[] = {1.0, 0.0};
double forceErrors[] = {0.0, 0.0};
double newForce = 0.0;
double newForceError = 0.0;
double predictedForce = 0.0;
double predictedForceError = 0.0;
double slope = 0.0;
double slopeError = 0.0;
double intercept = 0.0;
double interceptError = 0.0;
double newStrain = 0.0;
double startingVoltage;
double readVoltage;
double multiplier = 5.0 / 1023.0;
double A = 469.1;
double B = -.5704;
double C = -1.469;
double deltaA = 9.826;
double deltaB = .1524;
double deltaC = .0143;
double deltaV = .0001;
bool noDuh = true;
bool keepGoing = false;
bool start = false;
bool firstGo = true;
char readSerial;

void setup() 
{
  Serial.begin(9600);
  for (int i = 0; i < NUMSERVOS; i++)
  {
    flexor[i].attach(flexorPins[i]);
    flexor[i].write(0);
  }
  startingVoltage = analogRead(feedbackPin) * multiplier;
  Serial.print("Starting voltage: ");
  Serial.println(startingVoltage);
  delay(1000);
  //analogReference(EXTERNAL);
}

void loop() 
{
  delay(100);
  if (noDuh)
  {
    if (Serial.available() > 0)
    {
      readSerial = Serial.read();
      if (readSerial == 't')
      {
        keepGoing = true;
        noDuh = false;
        Serial.println("Now starting");
        //delay(1000);
      }
    }
  }
  
  /*if (start)
  {
    for (int i = 0; (i < NUMSERVOS && currentPos < MAXPOSITION); i++)
    {
      flexor[i].write(currentPos + 1);
      Serial.print("Exit ");
      Serial.println(i);
    }
    currentPos++;
    newStrain += 1.0;
    strainPoints[0] = strainPoints[1];
    strainPoints[1] = newStrain;
    readVoltage = smooth() * multiplier;
    Serial.println(readVoltage);
    if ((readVoltage) < (startingVoltage))
    {
      start = false;
      keepGoing = true;
      Serial.println("Now going");
      //delay(1000);
    }
  }*/

  if (keepGoing)
  {
    for (int i = 0; (i < NUMSERVOS && currentPos <= MAXPOSITION); i++)
    {
      flexor[i].write(currentPos);
      Serial.println(currentPos);
    }
    currentPos++;
    newStrain += 1.0;
    if (!firstGo)
    {
      calcSlope();
      calcSlopeError();
      calcIntercept();
      calcInterceptError();
      calcPredictedForce();
      calcPredictedForceError();
    }
    /*
    {
      firstGo = false;
    }*/
    strainPoints[0] = strainPoints[1];
    strainPoints[1] = newStrain;
    readVoltage = smooth() * multiplier;
    V2Fmapping(readVoltage);
    forceError (readVoltage);
    forcePoints[0] = forcePoints[1];
    forcePoints[1] = newForce;
    forceErrors[0] = forceErrors[1];
    forceErrors[1] = newForceError;
    if (currentPos <= MAXPOSITION)
    {
      Serial.print("Predicted Force: ");
      Serial.println(predictedForce);
      Serial.print("Measured Force: ");
      Serial.println(newForce);
    }
    if (!firstGo)
    {
      if (ceilf(predictedForce * 100)/100 > ceilf(newForce*100)/100)
      {
        keepGoing = false;
        Serial.println("Done");
      }
    }
    else
    {
      firstGo = false;
    }
  }
}

double smooth() 
{
  total = 0;
  for (int j = 0; j < NUMREADINGS; j++)
  {
    total += analogRead(feedbackPin);
  }
  return total / NUMREADINGS;
}

void V2Fmapping(double inputVoltage)
{
  double F = (A*exp(C*inputVoltage)) + B;

  newForce = F;
}

void forceError (double inputVoltage)
{
  double dA = deltaA * exp(C*inputVoltage);
  double dB = deltaB;
  double dC = -A * inputVoltage * deltaC * exp(C*inputVoltage);
  double dV = -A * C * deltaV * exp(C*inputVoltage);

  double dF = sqrt((dA*dA) + (dB*dB) + (dC*dC) + (dV*dV));

  newForceError = dF;
}

void calcSlope ()
{
  slope = (forcePoints[1] - forcePoints[0])/(strainPoints[1] - strainPoints [0]);
}

void calcSlopeError ()
{
  double dmSqrd = ((forceErrors[1]*forceErrors[1]) + (forceErrors[0]*forceErrors[0]))/(strainPoints[1] - strainPoints[0]);
  
  slopeError = sqrt(dmSqrd);//xsdcdvcdc
}

void calcIntercept ()
{
  intercept = forcePoints[1] - (slope * strainPoints[1]);
}

void calcInterceptError ()
{
  double bSqrd = (forceErrors[1]*forceErrors[1]) + (strainPoints[1]*strainPoints[1]*slopeError*slopeError);

  interceptError = sqrt(bSqrd);
}

void calcPredictedForce ()
{
  predictedForce = (slope*newStrain) + intercept;
}

void calcPredictedForceError ()
{
  double fSqurd = (newStrain*newStrain*slopeError*slopeError) + (interceptError*interceptError);
  
  predictedForceError = sqrt(fSqurd);
}

