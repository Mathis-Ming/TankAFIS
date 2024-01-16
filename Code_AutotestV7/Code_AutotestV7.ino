#include "MeMegaPi.h"
#include "Wire.h"

//*********** INITIALISATION POUR LE DEPLACEMENT SELON X (TANKAFIS) ************

MeLineFollower lineFinder(PORT_7); // S1 : capteur côté ultrason / S2 : capteur de l'autre côté 

MeMegaPiDCMotor motor1(PORT2B); // côté batterie (gauche)
MeMegaPiDCMotor motor2(PORT3B); // côté pince (droite)
int8_t motorSpeed2 = -78; // -98
int8_t motorSpeed1 = 39; // 50
bool motor1Started = false;
int8_t previous_speed_motor_1 = 0;
bool motor2Started = false;
int8_t previous_speed_motor_2 = 0;

int sensMove = -1;
int compteurCase=0;

int deltaM1 = 3; // correction à apporter au moteur 1 en cas de déviation
int deltaM2 = 2; // correction à apporter au moteur 2 en cas de déviation


//********* INITIALISATION POUR LE DEPLACEMENT SELON Y (RAIL) ************

MeMegaPiDCMotor motorRail(PORT1B);
uint8_t railSpeed = 90; //130
bool motorRailStarted =  false;
int8_t previous_speed_motor_Rail = 0;

#define NUM 1
MeColorSensor colorsensor(PORT_6);
uint8_t colorresult = 0;
long systime = 0,colorcode=0;

//*********** INITIALISATION LECTURE **************

// Définition des bornes des intervalles EN LECTURE
// Borne de la case Vide : 
float borneInf1L = 28.0;  
float borneSup1L = 35.55;  

// Borne de la case 0 : 
float borneInf2L = 17.0; 
float borneSup2L = 28.0;  

// Borne de la case 1 : 
//float borneInf3L = 0.0;  
float borneSup3L = 17.00;

//********* INITIALISATION ECRITURE **********

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

MeMegaPiDCMotor pince(PORT4B);
uint8_t pinceSpeed = 110; //130
bool pinceStarted = false;
int8_t previous_speed_pince = 0;

MeUltrasonicSensor ultraSensor(PORT_8);

float distance = 0.0;
float distanceTemp = 0.0; // cette variable n'est plus utilisée dans la dernière version

int sensWrite = -1; // le sens dans lequel la pince se déplace : -1 => de "vide" vers "1" / 1 => de "1" vers "vide" 
int numberCase = 1; // cette variable n'est plus utilisée dans la dernière version

int currentCase = -2; // l'état de la case courante au moment de la lecture
int newCase = -2; // l'état que la case courante doit avoir après écriture

int start = -1; // D'où on part avec la pince ? => -1 on est du côté de "vide" / 1 on est du côté de "1" 

unsigned long tempsDebut;
unsigned long tempsActuel;
unsigned long tempsEcoule;

//*********** INITIALISATION POUR LA TABLE D INSTRUCTIONS *************

// Définition d'une instruction.
struct Instruction {
  const char* etatCourant;
  int valeurLue;
  int valeurAEcrire;
  int deplacement; // -1 pour droite, 1 pour gauche
  const char* etatSuivant;
};

// Initialisation du tableau d'instructions
const char* etatActuel = "init";
int valeurLueActuelle = 0;

// Taille maximale de tableauInstructions.
const int tailleTableau = 2;

Instruction tableauInstructions[tailleTableau] = {
  { "init", -1, -1, -1, "init" },
  { "init", 0, 0, -1, "e1" }
};

int numeroInstruction=1;
int numeroCase=1;

//*****************Déclaration fonctions********************//
void arretRobot(void);
void jetonNonVu(void);
void VerificationInstruction(Instruction);
void VerificationDeplacement(Instruction);
void VerificationTableauValeurLue(void);
void VerificationExistenceEtatActuel(const char*);


void setup() {
  Serial.begin(115200);
  Serial3.begin(115200);    //The factory default baud rate is 115200
  Serial.println("Bluetooth Start!");
  int sensorState = lineFinder.readSensors();
  colorsensor.SensorInit();

  //ouvrir pince
  messageOn();
  pince_run(pinceSpeed);
  delay(3000);
  pince_stop();
  messagePause();
}

void loop() {

  // Exemple d'utilisation de la méthode trouverInstruction.
  // Rechercher l'instruction correspondante.
  Serial.println(" ");   
  Serial.println(" "); 
  Serial.print("Instruction numéro : ");   
  Serial.println(numeroInstruction);
  Serial3.write("Notification : Lancement d'une instruction");

  Serial3.write("Instruction numéro : ");

  Serial3.write("Reading : Début de la lecture");
  valeurLueActuelle=lecture(); //Bug du jeton non lu
  if(valeurLueActuelle==-2){
    jetonNonVu();
    valeurLueActuelle=lecture();
  }

  VerificationExistenceEtatActuel(etatActuel);
  Instruction instructionTrouvee = trouverInstruction(etatActuel, valeurLueActuelle);
  VerificationInstruction(instructionTrouvee);
  affichageInstruction(instructionTrouvee);
  
  int sensorState = lineFinder.readSensors();
  if(sensorState==S1_IN_S2_IN){
    LineFollowerBloque(instructionTrouvee.deplacement);
  }

  if(instructionTrouvee.valeurLue!=instructionTrouvee.valeurAEcrire){
    Serial.println("EXECUTION INSTRUCTION");
    executionInstruction(instructionTrouvee,400);
  }

  VerificationDeplacement(instructionTrouvee);
  deplacementEtatSuivant(instructionTrouvee);

  if(instructionTrouvee.etatSuivant!="")
  {
    nouvelleInstruction(instructionTrouvee);
  }
  numeroInstruction=numeroInstruction+1;
}

// Fonction pour trouver et retourner l'instruction correspondante.
struct Instruction trouverInstruction(const char* etatCourant, int valeurLue) {
  /*
  while(valeurLue==-2){
    jetonNonVu();
    valeurLue=lecture();
  }
  */
  for (int i = 0; i < tailleTableau; ++i) {
    if (strcmp(tableauInstructions[i].etatCourant, etatCourant) == 0 &&
        tableauInstructions[i].valeurLue == valeurLue) {
      return tableauInstructions[i];
    }
  }
  // Retourner une instruction par défaut si aucune correspondance n'est trouvée.
  Serial.println("Aucune instruction ne semble correspondre");
  Serial.print("La valeur lue est :");
  Serial.println(valeurLue);
  VerificationTableValeurLue();
  Instruction instructionParDefaut = {etatCourant, -1, -1, 0, ""};
  Serial.println("Fin programme pour bug");
  return instructionParDefaut;
}

void affichageInstruction(Instruction instructionTrouvee)
{
  // Afficher l'instruction trouvée.
  Serial.print("Etat courant: ");
  Serial.println(instructionTrouvee.etatCourant);
  Serial.print("Valeur lue: ");
  Serial.println(instructionTrouvee.valeurLue);
  Serial.print("Valeur a ecrire: ");
  Serial.println(instructionTrouvee.valeurAEcrire);
  Serial.print("Deplacement: ");
  Serial.println(instructionTrouvee.deplacement == -1 ? "droite" : "gauche");
  //Serial.print("Etat suivant: ");
  //Serial.println(instructionTrouvee.etatSuivant);
}

void executionInstruction(Instruction instructionTrouvee, int delais)
{
  //Trouve et attrape le jeton
  MoveToToken(instructionTrouvee,delais,1);//instructionTrouvee.valeurLue);
  Serial.println("*** MOVE TOKEN PRINCIPAL ***");

  //Le jeton est déplacé vers la case définie par l'instruction
  WriteTokenV2(instructionTrouvee,delais);//instructionTrouvee.valeurLue,instructionTrouvee.valeurAEcrire);
  Serial.println("*** WRITE PRINCIPAL ***");

  //La pince retourne a sa position initiale
  reset(instructionTrouvee,1);//instructionTrouvee.valeurAEcrire);
  Serial.println("*** RESET TOKEN PRINCIPAL ***");
}

void nouvelleInstruction(Instruction instructionTrouvee)
{
  etatActuel = instructionTrouvee.etatSuivant;
}

//*********** FONCTIONS AUXILIAIRES LANCER/ARRETER MOTEUR
void motor_1_run(int8_t vit){
  motor1.run(vit);
  motor1Started = true;
  previous_speed_motor_1 = vit;
}

void motor_2_run(int8_t vit){
  motor2.run(vit);
  motor2Started = true;
  previous_speed_motor_2 = vit;
}

void motor_Rail_run(int8_t vit){
  motorRail.run(vit);
  motorRailStarted = true;
  previous_speed_motor_Rail = vit;
}

void pince_run(int8_t vit){
  pince.run(vit);
  pinceStarted = true;
  previous_speed_pince = vit;
}

void motor_1_stop(){
  motor1.stop();
  motor1Started = false;
}

void motor_2_stop(){
  motor2.stop();
  motor2Started = false;
}

void motor_Rail_stop(){
  motorRail.stop();
  motorRailStarted = false;
}

void pince_stop(){
  pince.stop();
  pinceStarted = false;
}

//*********** FONCTIONS AUXILIAIRES LECTURE ************

// permet de faire la lecture en fonction de la distance entre le capteur ultrason et le jeton de la case courante
int lecture() {  
  distance = ultraSensor.distanceCm();
  int currentCase=-2;
  if (distance >= borneInf1L && distance <= borneSup1L) {
    currentCase=-1;
  } else if (distance >= borneInf2L && distance <= borneSup2L) {
    currentCase=0;
  } else if (distance <= borneSup3L) {
    currentCase=1;
  } else {
    currentCase=-2; // Retourne -2 si la distance ne correspond à aucun intervalle
  }
  Serial.print("Le jeton se situe actuellement à la case :");
  Serial.println(currentCase);
  return currentCase;
}

//*********** FONCTIONS AUXILIAIRES DEPLACEMENT SELON Y (RAIL) ************

// marque une petite pause afin d'éviter les changements de sens du moteur trop violents => car cela peut entrainer un reset de la carte MegaPi
void secure_rail()
{
  motor_Rail_stop();
  delay(100);
}

int DetectionCouleur()  //Blanc = 0, Rouge = 2, Jaune = 4 ou 5, Vert = 5, Noir = 7 et 9
{
  colorresult = colorsensor.ColorIdentify();
  //Serial.println(colorresult);
  return colorresult;
}

// On déplace la pince jusqu'au jeton
// Au début la pince est située à droite (variable "start") => il faut ajuster la sens de rotation du moteur et les delay en conséquence
void MoveToToken(Instruction instructionTrouvee, int delais, int sensWrite)
{
  messagePause();
  int currentCase = instructionTrouvee.valeurLue;
  int newCase = instructionTrouvee.valeurAEcrire;
  Serial.println("Je me déplace vers le jeton");
  // The grabber moove to the token
  secure_rail();
  secure_rail();
  colorsensor.TurnOnmodule();
  secure_rail();
  motor_Rail_run(-sensWrite*railSpeed);
  while(DetectionCouleur()!=2)
  { 
    messagePause();
    delay(100);
    Serial.print("x");
    continue;
  }

  delay(delais);
  messagePause();
  Serial.print("J'ai trouvé le jeton");

  motor_Rail_stop();
  colorsensor.TurnOffLight();
  colorsensor.TurnOffmodule();

  // We grab the token 
  pince_run(-pinceSpeed);
  delay(3000);
  pince_stop();
  secure_rail();
  Serial3.write("Reading : Fin de la lecture");
}

void MoveToTokenJetonNonVu(int delais)
{
  messagePause();
  Serial.println("Je me déplace vers le jeton");
  // The grabber moove to the token
  secure_rail();
  secure_rail();
  colorsensor.TurnOnmodule();
  secure_rail();
  motor_Rail_run(sensWrite*railSpeed);
  while(DetectionCouleur()!=2)
  {  
    messagePause();
    delay(100);
    Serial.print("x");
    continue;
  }

  delay(delais);
  messagePause();
  Serial.print("J'ai trouvé le jeton");


  motor_Rail_stop();
  colorsensor.TurnOffLight();
  colorsensor.TurnOffmodule();

  // We grab the token 
  pince_run(-pinceSpeed);
  delay(3000);
  pince_stop();
  secure_rail();
}
// On "range" la pince du côté droit
void reset(Instruction instructionTrouvee,int sensWrite)
{
  messagePause();
  railSpeed = 130;
  int newCase = instructionTrouvee.valeurAEcrire;
  //secure_rail();
  Serial.println("Retour à la maison");
  if(sensWrite>0){
    switch (newCase) {
    case -1:
      motor_Rail_run(sensWrite*railSpeed);
      delay(750);
      break;
    case 0:
      motor_Rail_run(sensWrite*railSpeed);
      delay(2000);
      break;
    case 1:
      motor_Rail_run(sensWrite*railSpeed);
      delay(4500);
      break;
    default:
      break;
    }
  }
  if(sensWrite<0){
    switch (newCase) {
    case -1:
      motor_Rail_run(sensWrite*railSpeed);
      delay(4500);
      break;
    case 0:
      motor_Rail_run(sensWrite*railSpeed);
      delay(2000);
      break;
    case 1:
      motor_Rail_run(sensWrite*railSpeed);
      delay(750);
      break;
    default:
      break;
    }
  }
  messagePause();
  secure_rail();
  railSpeed = 90;
}

void resetJetonNonVu(int currentCase)
{
  messagePause();
  railSpeed = 130;
  Serial.println("Retour à la maison");
  switch (currentCase) {
  case -1:
    motor_Rail_run(sensWrite*railSpeed);
    delay(750);
    break;
  case 0:
    motor_Rail_run(sensWrite*railSpeed);
    delay(2000);
    break;
  case 1:
    motor_Rail_run(sensWrite*railSpeed);
    delay(4500);
    break;
  default:
    break;
  }
  secure_rail();
  messagePause();
  railSpeed = 90;
}

//*********** FONCTIONS AUXILIAIRES ECRITURE ************

// la condition d'arrêt pour l'écriture => tant que cette fonction ne renvoie pas "true" on continue de déplacer le jeton
// pour que la fonction renvoie "true" => il faut que le jeton soit détecté dans "newCase"
bool stopWrite(int newCase, float distance) {
  messagePause();
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
// Tant qu'on a pas emmené le jeton dans l'intervalle que l'on veut, on continue de le déplacer
void WriteTokenV2(Instruction instructionTrouvee, int delais)
{
  messagePause();
  Serial3.write("Writing : Début de l'écriture");

  int bug = 0;

  //currentCase est la sous-case dans laquelle se trouve actuellement le jeton
  int currentCase = instructionTrouvee.valeurLue;
  //newCase est la sous-case dans laquelle on veut emmener le jeton
  int newCase = instructionTrouvee.valeurAEcrire;
  Serial.print("Je déplace le jeton vers la case :");
  Serial.println(newCase);

  secure_rail();

  if(currentCase<newCase)
  {
    sensWrite=-1;
    Serial.print("Pour cela je me déplace vers la droite");    
  }
  else if(currentCase>newCase)
  {
    sensWrite=1;
    Serial.print("Pour cela je me déplace vers la gauche");   
  }
  else
  {
    sensWrite=0;
  }

  if(sensWrite!=0 && currentCase!=-2)
  {
    float distance_instant_t = ultraSensor.distanceCm();
    Serial.print("Distance à l'instant t : ");
    Serial.println(distance_instant_t);
    motor_Rail_run(sensWrite*railSpeed);
    int compteur_stagnation=0;
    int compteur_deviation=0;
    tempsDebut = millis();
    while(stopWrite(newCase,ultraSensor.distanceCm())==false)
    {
      messagePause();
      Serial.print("Distance: ");
      Serial.println(ultraSensor.distanceCm());
      if(distance_instant_t == ultraSensor.distanceCm() || lecture()==currentCase)
      {
        compteur_stagnation=compteur_stagnation+1;
        Serial.print("Compteur stagnation : ");
        Serial.println(compteur_stagnation);
      }
      if(lecture()==-2)
      {
        compteur_deviation=compteur_deviation+1;
        Serial.print("Compteur deviation : ");
        Serial.println(compteur_deviation);
      }

      tempsActuel = millis();
      tempsEcoule = tempsActuel - tempsDebut;
      Serial.print("Temps écoulé: ");
      Serial.println(tempsEcoule);

      if(compteur_stagnation>15 && tempsEcoule>3500)
      {
        bug=1;
        Serial.println("LE JETON N A PAS ETE PRIS ");
        pince_run(pinceSpeed);
        delay(3000);
        pince_stop();
        messagePause();
        if(delais<700){
          delais=delais+300;
          Serial.print("LE DELAIS EST DESORMAIS DE : ");
          Serial.println(delais);
          compteur_stagnation=0;
          reset(instructionTrouvee,sensWrite);
          MoveToToken(instructionTrouvee,delais,sensWrite);
          WriteTokenV2(instructionTrouvee,delais);
        }
      }
      if(compteur_deviation>10)
      {
        bug=1;
        Serial.println("LE JETON N A PAS ETE PRIS ET A ETE DEVIE");
        pince_run(pinceSpeed);
        delay(3000);
        pince_stop();
        messagePause();
        if(delais<1000){
          delais=delais+500;
          Serial.print("LE DELAIS EST DESORMAIS DE : ");
          Serial.println(delais);
          compteur_deviation=0;
          reset(instructionTrouvee,sensWrite);
          MoveToToken(instructionTrouvee,delais,sensWrite);
          WriteTokenV2(instructionTrouvee,delais);
        }
      }
      /*
      if(compteur_stagnation<50 && tempsEcoule>20000)
      { 
        bug=1;
        Serial.println("LE JETON A DES DIFFICULTES A SE DEPLACER: MAUVAISE PRISE");
        pince_run(pinceSpeed);
        delay(3000);
        pince_stop();
        delay(100);
        messagePause();
        pince_run(-pinceSpeed);
        delay(3000);
        pince_stop();
        messagePause();
        reset(instructionTrouvee,sensWrite);
        MoveToToken(instructionTrouvee,delais,sensWrite);
        WriteTokenV2(instructionTrouvee,delais);
      }
      
      */
    }

  }
  Serial.println("Le jeton est arrivé à bon port");
  Serial3.write("Notification : Le jeton est arrivé à bon port");   
  motor_Rail_stop();
  delay(300); // petite pause avant de lacher le jeton pour pas qu'il le "jette" => 500
  messagePause();
  // release the token
  if(bug==1){
    Serial.println("OUVERTURE malgré bug");
    pince_run(pinceSpeed);
    delay(3000);
    pince_stop();
    messagePause();
  }
  if(bug==0){
    Serial.println("OUVERTURE normale");
    pince_run(pinceSpeed);
    delay(3000);
    pince_stop();
    messagePause();
  }
  Serial3.write("Writing : Fin de l'écriture");
}

//*********** FONCTIONS AUXILIAIRES POUR LE DEPLACEMENT SELON X (TANKAFIS) ************

void securePropulsion()
{
  motor_1_stop();
  motor_2_stop();
  delay(100);
}

void deplacementEtatSuivant(Instruction instructionTrouvee)
{
  Serial3.write("Moving : Début du déplacement");
  messagePause();
  securePropulsion();
  securePropulsion();
  sensMove=instructionTrouvee.deplacement;
  //if(sensMove>0){
  //  compteurCase=-1;
  //}

  while(compteurCase<1)
  { 
    messagePause();
    FollowLine(sensMove);
    delay(350); // ici on fait l'écriture
    // Si on est toujours sur la ligne on sort
    int sensorState = lineFinder.readSensors();

    /*
    while (sensorState==S1_OUT_S2_IN || sensorState==S1_IN_S2_IN) {
      motor_1_run(sensMove*(motorSpeed1+deltaM1));
      motor_2_run(sensMove*motorSpeed2);
      sensorState = lineFinder.readSensors();
      delay(100);
    }*/
  }
  compteurCase=0;
  Serial3.write("Moving : Fin du déplacement");
}

void FollowLine(int sensMove)
{
  messagePause();
  int sensorState = lineFinder.readSensors();
  int end = 0;
  //Serial.println(sensMove);
  while(end==0)
  {
    messagePause();
    sensorState = lineFinder.readSensors();
    switch(sensorState) {
    case S1_OUT_S2_OUT: 
        // cas blanc/blanc => il faut revenir sur la ligne noir
        // accelere moteur droit 
        //Serial.println("accelere droit");
        motor_1_stop();
        motor_2_stop();
        motor_1_run(sensMove*motorSpeed1);
        motor_2_run(sensMove*(motorSpeed2-deltaM2)); //10
        break;

    case S1_OUT_S2_IN:
        // cas blanc/noir
        //Serial.println("Stop");
        motor_1_stop();
        motor_2_stop();
        compteurCase+=1;
        end=1;
        //delay(1000);
        break;

    case S1_IN_S2_OUT: 
        // cas noir/blanc => il faut stabiliser
        //Serial.println("stabilise");
        motor_1_stop();
        motor_2_stop();
        motor_1_run(sensMove*(motorSpeed1+deltaM1)); //5
        motor_2_run(sensMove*motorSpeed2);
        break;

    case S1_IN_S2_IN: 
        // cas noir/noir
        //Serial.println("Stop");
        if(sensMove<0){
          delay(100);
          numeroCase=numeroCase+1;
        }
        else{
          delay(1250);
          messagePause();
          numeroCase=numeroCase-1;
        }
        motor_1_stop();
        motor_2_stop();
        compteurCase+=1;
        end=1;
        //delay(1000);
        break;
      } 
    delay(100);        
  }
}

//*********** FONCTIONS AUXILIAIRES POUR LE BLUETOOTH ************

void messageOn(){
  while(true){
    if(Serial3.available()){
      if(Serial3.readString() == "on")
      {
        break;
      }
      else
      {
        continue;
      }
    }
  }
}
void messagePause(){
  if(Serial3.available()){
    if(Serial3.readString() == "pause")
      {
        Pause();
      }
  }
}
void Pause(){
    motor1.stop();
    motor2.stop();
    motorRail.stop();
    pince.stop();
    while(true){
      if(Serial3.available()){
        if(Serial3.readString() == "stop")
        {
          arretRobot();
        }
        else
        {
          break;
        }
    }
  }
  if (motor1Started == true){
    motor1.run(previous_speed_motor_1);
  }
  if (motor2Started == true){
    motor2.run(previous_speed_motor_2);
  }
  if (motorRailStarted == true){
    motorRail.run(previous_speed_motor_Rail);
  }

}
//*********** FONCTIONS AUXILIAIRES POUR BUGS ROBOT ************

void arretRobot(){
  Serial.println("Arrêt robot pour bug");
  motor_1_stop();
  motor_2_stop();
  motor_Rail_stop();
  pince_stop();
  //setup();
  while(true){

  }
}

void VerificationExistenceEtatActuel(const char* etatCourant){
  if(etatActuel!=""){
    bool erreur=true;
    for (int i = 0; i < tailleTableau; ++i) {
      if (strcmp(tableauInstructions[i].etatCourant, etatCourant) == 0) {
        erreur=false;
      }
    }
    if(erreur==true){
      //envoi d'un message d'erreur à l'interface via Bluetooth "Etat Actuel Inexistant"
      Serial3.write("Error : L'état actuel n'existe pas dans la table des instructions");   
      Serial.println("Error : L'état actuel n'existe pas dans la table des instructions");   
      arretRobot();
    }
  }
}

void VerificationTableValeurLue(){
  bool erreur = false;
    for (int i = 0; i < tailleTableau; ++i) {
    if ((tableauInstructions[i].valeurLue != 0) && (tableauInstructions[i].valeurLue != 1) && (tableauInstructions[i].valeurLue != -1)) {
      erreur=true;
    }
  }
  if(erreur==true){
    //envoyer un message d'erreur à l'interface via bluetooth
    Serial3.write("Error : Il y a une erreur dans la table d'instruction : une valeur lue a mal été renseignée");
    Serial.println("Il y a une erreur dans la table d'instruction : une valeur lue a mal été renseignée");   
    arretRobot();
  }

}

void VerificationInstruction(Instruction instructionTrouvee){
  if(abs(instructionTrouvee.deplacement)!=1 && instructionTrouvee.deplacement!=0)
  {
    //envoyer un message d'erreur à l'interface via bluetooth
    Serial3.write("Error : Il y a une erreur dans la table d'instruction : la valeur du déplacement a mal été renseignée");
    Serial.print("Il y a une erreur dans la table d'instruction : la valeur du déplacement a mal été renseignée, la valeur est de ");
    Serial.println(instructionTrouvee.deplacement);
    arretRobot();
  }
  if(abs(instructionTrouvee.valeurLue)!=1 && instructionTrouvee.valeurLue!=0)
  {
    //envoyer un message d'erreur à l'interface via bluetooth
    Serial3.write("Error : Il y a une erreur dans la table d'instruction : la valeur lue a mal été renseignée");
    Serial.println("Il y a une erreur dans la table d'instruction : la valeur lue a mal été renseignée, la valeur est de ");
    Serial.println(instructionTrouvee.valeurLue);
    arretRobot();
  }
  if(abs(instructionTrouvee.valeurAEcrire)!=1 && instructionTrouvee.valeurAEcrire!=0)
  {
    //envoyer un message d'erreur à l'interface via bluetooth
    Serial3.write("Error : Il y a une erreur dans la table d'instruction : la valeur à écrire a mal été renseignée");
    Serial.println("Il y a une erreur dans la table d'instruction : la valeur à écrire a mal été renseignée, la valeur est de ");
    Serial.println(instructionTrouvee.valeurAEcrire);
    arretRobot();
  }
}

void VerificationDeplacement(Instruction instructionTrouvee){
  if(numeroCase>10 && numeroCase<1){
    //envoyer un message d'erreur à l'interface via bluetooth
    Serial3.write("Error : Il y a une erreur dans la table d'instruction le déplacement suivant est impossible");
    Serial.println("Il y a une erreur dans la table d'instruction : le déplacement suivant est impossible : ");
    Serial.println(instructionTrouvee.deplacement);
    arretRobot();
  }
  Serial.print("NUMERO DE CASE : ");
  Serial.println(numeroCase);
}

void jetonNonVu(){
  messagePause();
  int sensorState = lineFinder.readSensors();
  int sensMove=1;
  Serial3.write("Warning : Le jeton n'est pas vu");
  
  while(lecture()==-2)
  {
    messagePause();
    Serial.println("Jeton non vu");
    sensorState = lineFinder.readSensors();
    switch(sensorState) 
    {
      case S1_OUT_S2_OUT: 
        // cas blanc/blanc => il faut revenir sur la ligne noir
        // accelere moteur droit 
        //Serial.println("accelere droit");
        motor_1_stop();
        motor_2_stop();
        motor_1_run(sensMove*motorSpeed1);
        motor_2_run(sensMove*(motorSpeed2-deltaM2)); //10
        break;

      case S1_OUT_S2_IN:
        // cas blanc/noir
        //Serial.println("Stop");
        motor_1_stop();
        motor_2_stop();
        //delay(1000);
        break;

      case S1_IN_S2_OUT: 
        // cas noir/blanc => il faut stabiliser
        //Serial.println("stabilise");
        motor_1_stop();
        motor_2_stop();
        motor_1_run(sensMove*(motorSpeed1+deltaM1));
        motor_2_run(sensMove*motorSpeed2);
        break;

      case S1_IN_S2_IN: 
        // cas noir/noir
        //Serial.println("Stop")
        motor_1_stop();
        motor_2_stop();
        sensMove=-sensMove;
        if(lecture()==-2){
          LineFollowerBloque(-1);
          MoveToTokenJetonNonVu(300);
          messagePause();
          if(lecture()!=-2){
            pince_run(pinceSpeed);
            delay(3000);
            pince_stop();
            messagePause();
            delay(100);
            resetJetonNonVu(lecture());
          }
          else{
            //Une maintenance doit être réalisée
            arretRobot();
          }
        }
        break;   
    }
  }
}

void LineFollowerBloque(int sensMove)
{
  messagePause();
  Serial3.write("Warning : Le jeton n'est pas vu");
  int sensorState = lineFinder.readSensors();
  int compteur_case=0;
  while(sensorState==S1_IN_S2_IN)
  {
    messagePause();
    sensorState = lineFinder.readSensors();
    Serial.println("Bloqué sur la ligne noire");
    motor_1_run(sensMove*(motorSpeed1+deltaM1));
    motor_2_run(sensMove*motorSpeed2);
    delay(100);
  }
  if(sensMove<0){
    numeroCase=numeroCase+1;
  }
  else{
    numeroCase=numeroCase-1;
  }
  motor_1_stop();
  motor_2_stop();
}


