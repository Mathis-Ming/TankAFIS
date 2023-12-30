#include "MeMegaPi.h"

MeUltrasonicSensor ultraSensor(PORT_8); 

// Définition des bornes des intervalles
// Borne de la case Vide : 
int borneInf1 = 30.40;  
int borneSup1 = 32.55;  

// Borne de la case 0 : 
int borneInf2 = 22.50; 
int borneSup2 = 25.0;  

// Borne de la case 1 : 
int borneInf3 = 11.15;  
int borneSup3 = 12.35; 

void setup() {
  Serial.begin(9600);
}

void loop() {
  int currentCase = lecture();
  Serial.print("Distance : ");
  Serial.print(ultraSensor.distanceCm());
  Serial.print(" cm, Current Case: ");
  Serial.println(currentCase);
  delay(100);
}

int lecture() {
  int distance = ultraSensor.distanceCm();
  if (distance >= borneInf1 && distance <= borneSup1) {
    return -1;
  } else if (distance >= borneInf2 && distance <= borneSup2) {
    return 0;
  } else if (distance >= borneInf3 && distance <= borneSup3) {
    return 1;
  } else {
    return -2; // Retourne -2 si la distance ne correspond à aucun intervalle
  }
}
