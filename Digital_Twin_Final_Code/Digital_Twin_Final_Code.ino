//beginning stuff (defining and including library stuff)

// moving the motor stuff
#include <AFMotor.h>
AF_DCMotor motor(1, MOTOR12_64KHZ); // create motor #1, 64KHz pwm (make more motors by reinputing this code)
AF_DCMotor motor3(3, MOTOR12_64KHZ);

// temperature sensor stuff
#include <OneWire.h>
int DS18S20_Pin = 5; //DS18S20 Signal pin on digital 2
//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2

// encoder stuff
#define ENCODEROUTPUT 700
const int HALLSEN_A = 3; // Hall sensor A connected to pin 3 (external interrupt)
volatile long encoderValue = 0;
int interval = 1000;
long previousMillis = 0;
long currentMillis = 0;
int rpm = 0;
boolean measureRpm = false;
int motorPwm = 0;

// accelerometer stuff
const int xInput = A0;
const int yInput = A1;
const int zInput = A2;
// initialize minimum and maximum Raw Ranges for each axis
int RawMin = 0;
int RawMax = 1023;
// Take multiple samples to reduce noise
const int sampleSize = 10;
// voltage/current reading stuff
const int vInput = A3;
const int cInput = A4;
//--------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(250000);// set up Serial library at 9600 bps
  
// encoder setup stuff
  EncoderInit();//Initialize the module
   encoderValue = 0;
   previousMillis = millis();
  
// motor move setup stuff
  // set the speed to 200/255 (highest is 255)
  motor.setSpeed(255);
  motor3.setSpeed(255);

// accelerometer setup stuff
  analogReference(EXTERNAL);
}

//---------------------------------------------------------------------------------------------------------

//temp sense function
float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius
  byte data[12];
  byte addr[8];
  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }
  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end
  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  ds.reset_search();
  byte MSB = data[1];
  byte LSB = data[0];
  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  return TemperatureSum;
}

// -----------------------------------------------------------------------------------------------------

void loop() {

// motor move loop stuff
  // change BACKWARD or FORWARD for direction change
  motor3.run(FORWARD);
  
  float temperature = getTemp();
  //Serial.print("Celsius:");
  Serial.print(temperature);
  Serial.print(" ");
    delay(500);

// encoder loop stuff
  currentMillis = millis(); // Update RPM value on every second
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    // Revolutions per minute (RPM) =
    // (total encoder pulse in 1s / motor encoder output) x 60s <--THIS IS WHAT THE RPM EQUATION IS
    rpm = (float)(encoderValue * 60 / ENCODEROUTPUT);
    // Only update display when there is readings
    if ( rpm > 0) {
      //Serial.print("RPM:");
      Serial.print(rpm);
      Serial.print(" ");
      delay(500);
      }
    encoderValue = 0;
  }
 
// accelerometer loop stuff
  //Read raw values
  int xRaw = ReadAxis(xInput);
  int yRaw = ReadAxis(yInput);
  int zRaw = ReadAxis(zInput);
  // Convert raw values to 'milli-Gs"
  long xScaled = map(xRaw, RawMin, RawMax, -3000, 3000);
  long yScaled = map(yRaw, RawMin, RawMax, -3000, 3000);
  long zScaled = map(zRaw, RawMin, RawMax, -3000, 3000);
  // re-scale to fractional Gs
  float xAccel = xScaled / 1000.0;
  float yAccel = yScaled / 1000.0;
  float zAccel = zScaled / 1000.0;
  // X:863,Y:731,Z:718
  //Serial.print("X:");
  Serial.print(xRaw);
  Serial.print(" ");
  //Serial.print("Y:");
  Serial.print(yRaw);
  Serial.print(" ");
  //Serial.print("Z:");
  Serial.print(zRaw);
  Serial.print(" ");
  delay(500);

// voltage/current read loop stuff
  long voltage = analogRead(vInput);
  Serial.print(voltage);
  long current = analogRead(cInput);
  Serial.print(" ");
  Serial.print(current);
  Serial.println(" ");
}

// ----------------------------------------------------------------------------------------------------------

//encoder functions stuff
void EncoderInit()
{
// Attach interrupt at hall sensor A on each rising signal
  attachInterrupt(digitalPinToInterrupt(HALLSEN_A), updateEncoder, RISING);
}
void updateEncoder()
{
  // Add encoderValue by 1, each time it detects rising signal
  // from hall sensor A
  encoderValue++;
}

// ----------------------------------------------------------------------------------------------------------------

// accelerometer functions stuff
// Take samples and return the average
int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
  reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}

// ----------------------------------------------------------------------------------------------------------------
