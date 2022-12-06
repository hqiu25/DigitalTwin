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
boolean print_labels = false;  // set this to true if we want to print out the labels in between the values

//-------------------------------------------------------------------------------------------------------

void write_value( char *label, float value, boolean new_line ) {
    if (print_labels) {
      Serial.print(label);
      Serial.print(" ");
  }
    if  (new_line) {
      Serial.println(value);
  } 
    else {
      Serial.print(value);
      Serial.print(" ");
  }

}

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
  write_value("Celsius:", temperature, false);
    delay(500);

// encoder loop stuff
  float rpm = get_rpm() ;
  write_value("RPM:", rpm, false);
  // Only update display when there is readings
  if ( rpm > 0) {
      delay(500);
      }
// accelerometer loop stuff
  //Read raw values
 float x_accel = get_acceleration(xInput);
 write_value("X:", x_accel, false);

 float y_accel = get_acceleration(yInput);
 write_value("Y:", y_accel, false);

 float z_accel = get_acceleration(zInput);
 write_value("Z:", z_accel, false);
  delay(500);

// voltage read loop stuff
  float voltage = get_voltage();
  write_value("Voltage:", voltage, false);
//current
  float current = get_current();
  write_value("Current:", current, true);
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

//RPM/encoder function
float get_rpm()
{
    currentMillis = millis(); // Update RPM value on every second
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    // Revolutions per minute (RPM) =
    // (total encoder pulse in 1s / motor encoder output) x 60s <--THIS IS WHAT THE RPM EQUATION IS
    rpm = (float)(encoderValue * 60 / ENCODEROUTPUT);

    encoderValue = 0;
  }
  return rpm;
}
//-------------------------------------------------------------------------------------------------------------------
float get_voltage()
{
  long voltage = analogRead(vInput);
  return (float)voltage;
}
//-------------------------------------------------------------------------------------------------------------------
float get_current()
{
  long current = analogRead(cInput);
  return (float)current;
}

float get_acceleration(int Input)
{
  int Raw = ReadAxis(Input);
  return (float)Raw;
}
