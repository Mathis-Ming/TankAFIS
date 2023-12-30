#include "MeMegaPi.h"
#include "Wire.h"

// Définition des bornes des intervalles EN LECTURE
// Borne de la case Vide : 
float borneInf1L = 28.75;  
float borneSup1L = 35.35;  

// Borne de la case 0 : 
float borneInf2L = 18.40; 
float borneSup2L = 26.0;  

// Borne de la case 1 : 
float borneInf3L = 10.40;  
float borneSup3L = 13.60; 

// Définition des bornes des intervalles EN ECRITURE
// Borne de la case Vide : 
float borneInf1 = 30.00;  
float borneSup1 = 32.50;  

// Borne de la case 0 : 
float borneInf2 = 22.50; 
float borneSup2 = 25.0;  

// Borne de la case 1 : 
float borneInf3 = 11.00;  
float borneSup3 = 12.50; 

MeMegaPiDCMotor motorRail(PORT1B);
uint8_t railSpeed = 90; //130

MeMegaPiDCMotor pince(PORT4B);
uint8_t pinceSpeed = 110; //130

#define NUM 1
MeColorSensor colorsensor(PORT_6);
uint8_t colorresult = 0;
long systime = 0,colorcode=0;

MeUltrasonicSensor ultraSensor(PORT_8);

float distance = 0.0;
float distanceTemp = 0.0; // cette variable n'est plus utilisée dans la dernière version

int sensWrite = -1; // le sens dans lequel la pince se déplace : -1 => de "vide" vers "1" / 1 => de "1" vers "vide" 
int numberCase = 1; // cette variable n'est plus utilisée dans la dernière version

int currentCase = -2; // l'état de la case courante au moment de la lecture
int newCase = -2; // l'état que la case courante doit avoir après écriture

int start = -1; // D'où on part avec la pince ? => -1 on est du côté de "vide" / 1 on est du côté de "1" 

int DetectionCouleur()  //Blanc = 0, Rouge = 2, Jaune = 4 ou 5, Vert = 5, Noir = 7 et 9
{
  colorresult = colorsensor.ColorIdentify();
  //Serial.println(colorresult);
  return colorresult;
}

// permet de faire la lecture en fonction de la distance entre le capteur ultrason et le jeton de la case courante
int lecture() {
  distance = ultraSensor.distanceCm();
  if (distance >= borneInf1L && distance <= borneSup1L) {
    return -1;
  } else if (distance >= borneInf2L && distance <= borneSup2L) {
    return 0;
  } else if (distance >= borneInf3L && distance <= borneSup3L) {
    return 1;
  } else {
    return -2; // Retourne -2 si la distance ne correspond à aucun intervalle
  }
}

// la condition d'arrêt pour l'écriture => tant que cette fonction ne renvoie pas "true" on continue de déplacer le jeton
// pour que la fonction renvoie "true" => il faut que le jeton soit détecté dans "newCase"
bool stopWrite(int newCase, float distance) {
  switch (newCase) {
    case -1:
      return (distance >= borneInf1 && distance <= borneSup1);
    case 0:
      return (distance >= borneInf2 && distance <= borneSup2);
    case 1:
      return (distance >= borneInf3 && distance <= borneSup3);
    default:
      return false; // Retourne false si newCase ne correspond pas à -1, 0 ou 1
  }
}

// On déplace la pince jusqu'au jeton
// Au début la pince peut être située à droite ou à gauche (variable "start") => il faut ajuster la sens de rotation du moteur et les delay en conséquence
void MoveToToken()
{
  // The grabber moove to the token
  secure();
  secure();
  colorsensor.TurnOnmodule();
  secure();
  if(start==-1)
  {
    motorRail.run(-railSpeed);
  }
  else 
  {
    motorRail.run(railSpeed);
  }
  while(DetectionCouleur()!=2)
  {  
    delay(100);
    continue;
  }
  if(start==-1)
  {
    delay(200); 
  }

  motorRail.stop();
  colorsensor.TurnOffLight();
  colorsensor.TurnOffmodule();

  // We grab the token 
  pince.run(-pinceSpeed);
  delay(3000);
  pince.stop();
  secure();
}

// Ancienne méthode => exemple : pour déplacer le jeton d'une sous-case, on le déplace de 8.5cm 
void WriteToken(int sensWrite, int numberCase)
{
  distance = ultraSensor.distanceCm();
  distanceTemp=distance+numberCase*sensWrite*8.5;

  secure();

  if(sensWrite<0)
  {
    motorRail.run(sensWrite*railSpeed);
    while (distance > distanceTemp) {
    // Update distance
    distance = ultraSensor.distanceCm();
    delay(100);
    }
    while (distance > distanceTemp) {
    // Update distance
    distance = ultraSensor.distanceCm();
    delay(100);
    }
  }
  else{
    motorRail.run(sensWrite*railSpeed);
    while (distance < distanceTemp) {
    // Update distance
    distance = ultraSensor.distanceCm();  
    delay(100);  
    }
    while (distance < distanceTemp) {
    // Update distance
    distance = ultraSensor.distanceCm();    
    delay(100);
    }
  }
  motorRail.stop();
  delay(300); // petite pause avant de lacher le jeton pour pas qu'il le "jette" => 500
}


// Tant qu'on a pas emmené le jeton dans l'intervalle que l'on veut, on continue de le déplacer
void WriteTokenV2()
{
  currentCase=lecture();
  newCase=1; // cette valeur est la sous-case dans laquelle on veut emmener le jeton
  secure();
  
  if(currentCase<newCase)
  {
    sensWrite=-1;    
  }
  else if(currentCase>newCase)
  {
    sensWrite=1;
  }
  else
  {
    sensWrite=0;
  }

  if(sensWrite!=0 && currentCase!=-2)
  {
    motorRail.run(sensWrite*railSpeed);
    while(stopWrite(newCase,ultraSensor.distanceCm())==false)
    {
      delay(100);      
    }
  }

  motorRail.stop();
  delay(300); // petite pause avant de lacher le jeton pour pas qu'il le "jette" => 500

  // release the token
  pince.run(pinceSpeed);
  delay(3000);
  pince.stop();
}

// On "range" la pince du côté le plus proche 
void reset()
{
  //secure();
  switch (newCase) {
    case -1:
      motorRail.run(railSpeed);
      delay(2000);
      start=-1;
      break;
    case 0:
      motorRail.run(railSpeed);
      delay(4000);
      start=-1;
      break;
    case 1:
      motorRail.run(-railSpeed);
      delay(2000);
      start=1;
      break;
    default:
      break;
  }
  secure();
}

// marque une petite pause afin d'éviter les changements de sens du moteur trop violents => car cela peut entrainer un reset de la carte MegaPi
void secure()
{
  motorRail.stop();
  delay(100);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  colorsensor.SensorInit();
  //distance = ultraSensor.distanceCm();

  //ouvrir pince
  pince.run(pinceSpeed);
  delay(3000);
  pince.stop();

}

void loop() {
  // find & grab the token
   MoveToToken();
  // Lecture & Move the Token to the destination
  //WriteToken(-sensWrite,numberCase);
   WriteTokenV2();
  // The grabber come back home
   reset();
}
