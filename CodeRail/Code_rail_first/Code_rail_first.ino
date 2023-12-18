#include "MeMegaPi.h"
#include "Wire.h"

MeLineFollower lineFinder(PORT_7);
// test modif mathis 18/12 

MeMegaPiDCMotor motorRail(PORT1B);
uint8_t railSpeed = 90;

MeMegaPiDCMotor pince(PORT4B);
uint8_t pinceSpeed = 150; //50

MeMegaPiDCMotor motor1(PORT2B); // droit
MeMegaPiDCMotor motor2(PORT3B); // gauche
int8_t motorSpeed2 = -98;
int8_t motorSpeed1 = 50;

#define NUM 1
MeColorSensor colorsensor(PORT_6);
uint8_t colorresult = 0;
long systime = 0,colorcode=0;

MeUltrasonicSensor ultraSensor(PORT_8);
float distance = 0.0;
float distanceTemp = 0.0;


bool fin = false;

int sensMove = -1; // -1 => vers la droite et 1 => vers la gauche
int sensWrite = -1; // -1 => de vide vers 1 et 1 => de 1 vers vide 
int numberCase = 1;

int DetectionCouleur()  //Blanc = 0, Rouge = 2, Jaune = 4 ou 5, Vert = 5, Noir = 7 et 9
{
  colorresult = colorsensor.ColorIdentify();
  //Serial.println(colorresult);
  return colorresult;
}

void MoveToNextToken(int sensMove)
{
  int count=0;

  while (distance < 35.0) {
  // Move forward
  motor1.run(sensMove*motorSpeed1);
  motor2.run(sensMove*motorSpeed2);

  // Update distance
  distance = ultraSensor.distanceCm();
  delay(100);
  
  }
  while (distance < 35.0) {
  // Move forward
  motor1.run(sensMove*motorSpeed1);
  motor2.run(sensMove*motorSpeed2);

  // Update distance
  distance = ultraSensor.distanceCm();
  delay(100);
  
  }
  // moove to the next token with ultrasonic sensor 
  // Check if distance is greater than 40.0
  /*while (count != 2) {
    // Move forward
    motor1.run(sensMove*motorSpeed);
    motor2.run(sensMove*motorSpeed);
    if(distance < 40.0){
      count++;
    }
    else{
      count = 0;
    }

    // Update distance
    distance = ultraSensor.distanceCm();
    delay(100);
  }*/
  
    
    while (distance > 35.0) {
    // Move forward
    motor1.run(sensMove*motorSpeed1);
    motor2.run(sensMove*motorSpeed2);

    // Update distance
    distance = ultraSensor.distanceCm();
    delay(100);
  }

   while (distance > 35.0) {
    // Move forward
    motor1.run(sensMove*motorSpeed1);
    motor2.run(sensMove*motorSpeed2);

    // Update distance
    distance = ultraSensor.distanceCm();
    delay(100);
  }

  delay(100);
  

  // Show distance on the monitor (test)
  //Serial.print("Distance : ");
  //Serial.print(distance);
  //Serial.println(" cm");

  // little delay because ultrasonic sensor detects too early
 // delay(100);

  // Stop motors
  motor1.stop();
  motor2.stop();
}

void MoveToNextToken3(int sensMove)
{
    int sensorState = lineFinder.readSensors();
    

    motor1.run(sensMove*motorSpeed1);
    motor2.run(sensMove*motorSpeed2);
    /*
    while (sensorState==S1_IN_S2_IN) {
      delay(100);
      continue;
    } */
    while(sensorState==S1_IN_S2_IN){
      sensorState = lineFinder.readSensors();
      delay(200);
      Serial.println("COUCOU");
    }

    while (sensorState==S1_IN_S2_OUT) {
      sensorState = lineFinder.readSensors();
      delay(200);
      switch(sensorState){
        case S1_IN_S2_OUT: Serial.println("S1 IN S2 OUT"); break;
        case S1_IN_S2_IN: Serial.println("S1 IN S2 IN"); break;

      }
    }
    motor1.stop();
    motor2.stop();


}

void MoveToNextToken2(int sensMove)
{

    motor1.run(sensMove*motorSpeed1);
    motor2.run(sensMove*motorSpeed2);
    delay(1150);
    motor1.stop();
    motor2.stop();
}

void WriteToken(int sensWrite, int numberCase)
{
  // The grabber moove to the token
  motorRail.run(-railSpeed);
  while(DetectionCouleur()!=2)
  {  
    delay(100);
    continue;
  }    
  while(DetectionCouleur()!=2)
  {  
    delay(100);
    continue;
  }          
  delay(100);
  motorRail.stop();

  // We grab the token 
  pince.run(-pinceSpeed);
  delay(3000);
  pince.stop();

  distance = ultraSensor.distanceCm();
  //distanceTemp=numberCase*(distance+sensWrite*9.0);
  distanceTemp=distance+numberCase*sensWrite*8.5;

  if(sensWrite<0)
  {
    while (distance > distanceTemp) {
    motorRail.run(sensWrite*railSpeed);
    // Update distance
    distance = ultraSensor.distanceCm();
    delay(100);
    }
    while (distance > distanceTemp) {
    motorRail.run(sensWrite*railSpeed);
    // Update distance
    distance = ultraSensor.distanceCm();
    delay(100);
    }
  }
  else{
    while (distance < distanceTemp) {
    motorRail.run(sensWrite*railSpeed);
    // Update distance
    distance = ultraSensor.distanceCm();  
    delay(100);  
    }
    while (distance < distanceTemp) {
    motorRail.run(sensWrite*railSpeed);
    // Update distance
    distance = ultraSensor.distanceCm();    
    delay(100);
    }
  }
  motorRail.stop();
}

void reset()
{
  // release the token
  pince.run(pinceSpeed);
  delay(3000);
  pince.stop();

  // come back to the start position
  motorRail.run(railSpeed);
  delay(6000);
  motorRail.stop();
  
}

void setup() {
  Serial.begin(9600);
  colorsensor.SensorInit();
  distance = ultraSensor.distanceCm();

  //ouvrir pince
  pince.run(pinceSpeed);
  delay(5000);
  pince.stop();
}

void loop() {

  if (fin==false)
  {
    // scenario 1 : 
    // première case
    // On avance jusqu'au prochain jeton
    MoveToNextToken3(sensMove);
    // On avance jusqu'à serrer le jeton puis on le déplace
    WriteToken(-sensWrite,numberCase);
    // We come back to a start position
    reset();
    
    // deuxième case :
    // On avance jusqu'au prochain jeton
    MoveToNextToken3(sensMove);
    // On avance jusqu'à serrer le jeton puis on le déplace
    WriteToken(-sensWrite,numberCase);
    // We come back to a start position
    reset();

    // troisième case : 
    // On avance jusqu'au prochain jeton
    MoveToNextToken3(sensMove);
    // On avance jusqu'à serrer le jeton puis on le déplace
    WriteToken(sensWrite,2);
    // We come back to a start position
    reset();

    fin=true;
  }
}
