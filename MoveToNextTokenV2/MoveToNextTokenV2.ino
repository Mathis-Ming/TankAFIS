#include "MeMegaPi.h"
#include "Wire.h"

MeLineFollower lineFinder(PORT_7); // S1 : capteur côté ultrason / S2 : capteur de l'autre côté 

MeMegaPiDCMotor motor1(PORT2B); // côté batterie (gauche)
MeMegaPiDCMotor motor2(PORT3B); // côté rien du tt (droite)
int8_t motorSpeed2 = -78; // -98
int8_t motorSpeed1 = 39; // 50

int sensMove = -1;
int compteurCase=0;

int deltaM1 = 3; // correction apportée au moteur 1
int deltaM2 = 2; // correction apportée au moteur 2

void securePropulsion()
{
  motor1.stop();
  motor2.stop();
  delay(100);
}

void Test(int sensMove)
{
  motor1.run(sensMove*(motorSpeed1));
  motor2.run(sensMove*motorSpeed2);
  delay(7000);
  sensMove=-sensMove;
  motor1.run(sensMove*(motorSpeed1));
  motor2.run(sensMove*motorSpeed2);
  delay(7000);
  sensMove=-sensMove;
}

void FollowLine(int sensMove)
{
  /*if(sensMove==1)
  {
    motor1.run(sensMove*(motorSpeed1+deltaM1)); //5
    motor2.run(sensMove*motorSpeed2);
    delay(400);
    motor1.stop();
    motor2.stop();
  }*/

  int sensorState = lineFinder.readSensors();
  int end = 0;
  while(end==0)
  {
    sensorState = lineFinder.readSensors();
    switch(sensorState) {
    case S1_OUT_S2_OUT: 
        // cas blanc/blanc => il faut revenir sur la ligne noir
        // accelere moteur droit 
        Serial.println("accelere droit");
        motor1.stop();
        motor2.stop();
        motor1.run(sensMove*motorSpeed1);
        motor2.run(sensMove*(motorSpeed2-deltaM2)); //10
        break;

    case S1_OUT_S2_IN: 
        //Serial.println("Stop");
        motor1.stop();
        motor2.stop();
        compteurCase+=1;
        end=1;
        //delay(1000);
        break;

    case S1_IN_S2_OUT: 
        // cas noir/blanc => il faut stabiliser
        Serial.println("stabilise");
        motor1.stop();
        motor2.stop();
        motor1.run(sensMove*(motorSpeed1+deltaM1)); //5
        motor2.run(sensMove*motorSpeed2);
        break;

    case S1_IN_S2_IN: 
        Serial.println("Stop");
        motor1.stop();
        motor2.stop();
        compteurCase+=1;
        end=1;
        //delay(1000);
        break;
      } 
    delay(100);        
  }

  /*if(sensMove==1)
  {
    motor1.run(-sensMove*(motorSpeed1+deltaM1)); //5
    motor2.run(-sensMove*motorSpeed2);
    delay(200);
    motor1.stop();
    motor2.stop();
  }*/

}

// PAS UTILISE
// faiblesse : parfois il rate un case surement à cause du delay entre deux lectures. 
// solution : réduire la vitesse de déplacement 
// solutuion 2 : réduire le delay (c'est préconisé avec 200 mais j'ai mis 100 et ça a l'air de marcher)
void MoveToNextToken3(int sensMove)
{
    int sensorState = lineFinder.readSensors();
    
    motor1.run(sensMove*motorSpeed1);
    motor2.run(sensMove*motorSpeed2);

    /*
    // sortir de la case précédente 
    while(sensorState==S1_IN_S2_IN){
      sensorState = lineFinder.readSensors();
      delay(200);
      Serial.println("COUCOU");
    }*/

    // le S2 est sur du blanc, dès qu'il est sur du noir on s'arrête
    while (sensorState==S1_IN_S2_OUT) {
      sensorState = lineFinder.readSensors();
      delay(100);
      /*
      switch(sensorState){
        case S1_IN_S2_OUT: Serial.println("S1 IN S2 OUT"); break;
        case S1_IN_S2_IN: Serial.println("S1 IN S2 IN"); break; 
      }*/
    }
    motor1.stop();
    motor2.stop();
}


void setup() {
   Serial.begin(9600);
   int sensorState = lineFinder.readSensors();
  // put your setup code here, to run once:
  //motor1.run(sensMove*motorSpeed1+10);
  //motor2.stop();
  delay(1000);

}

void loop() {

  securePropulsion();
  securePropulsion();

  while(compteurCase<=7)
  { 
    FollowLine(sensMove);
    delay(350); // ici on fait l'écriture
    // Si on est toujours sur la ligne on sort
    int sensorState = lineFinder.readSensors();
    while (sensorState==S1_OUT_S2_IN || sensorState==S1_IN_S2_IN) {
      motor1.run(sensMove*(motorSpeed1+deltaM1));
      motor2.run(sensMove*motorSpeed2);
      sensorState = lineFinder.readSensors();
      delay(100);
    }
  }
  compteurCase=0;
  sensMove=-sensMove;
  if(sensMove==-1)
  {
    deltaM1=2;
    deltaM2=2;
  }
  else 
  {
    deltaM1=1;
    deltaM2=3;
  }

  //Test(sensMove);

}