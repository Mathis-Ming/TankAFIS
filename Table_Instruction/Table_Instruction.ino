// Définition d'une instruction.
struct Instruction {
  const char* etatCourant;
  int valeurLue;
  int valeurAEcrire;
  int deplacement; // -1 pour droite, 1 pour gauche
  const char* etatSuivant;
};

// Taille maximale de tableauInstructions.
const int tailleTableau = 12;

// Initialisation d'un tableau d'instructions avec les données du tableau.
Instruction tableauInstructions[tailleTableau] = {
  {"init", -1, -1, -1, "init"},
  {"init", 0, 0, -1, "e1"},
  {"init", 1, 1, -1, "e1"},
  {"e1", -1, -1, 1, "e2"},
  {"e1", 0, 0, -1, "e1"},
  {"e1", 1, 1, -1, "e1"},
  {"e2", 0, 1, -1, "fin"},
  {"e2", 1, 0, 1, "e3"},
  {"e3", -1, 1, -1, "fin"},
  {"e3", 0, 1, -1, "fin"},
  {"e3", 1, 0, 1, "e3"},
  {"fin", -1, -1, -1, "fin"}
};

// Fonction pour trouver et retourner l'instruction correspondante.
Instruction trouverInstruction(const char* etatCourant, int valeurLue) {
  for (int i = 0; i < tailleTableau; ++i) {
    if (strcmp(tableauInstructions[i].etatCourant, etatCourant) == 0 &&
        tableauInstructions[i].valeurLue == valeurLue) {
      return tableauInstructions[i];
    }
  }
  
  // Retourner une instruction par défaut si aucune correspondance n'est trouvée.
  Instruction instructionParDefaut = {"", -1, -1, 0, ""};
  return instructionParDefaut;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Exemple d'utilisation de la méthode trouverInstruction.
  const char* etatActuel = "e2";
  int valeurLueActuelle = 0;

  // Rechercher l'instruction correspondante.
  Instruction instructionTrouvee = trouverInstruction(etatActuel, valeurLueActuelle);
  
  // Afficher l'instruction trouvée.
  Serial.print("Etat courant: ");
  Serial.println(instructionTrouvee.etatCourant);
  Serial.print("Valeur lue: ");
  Serial.println(instructionTrouvee.valeurLue);
  Serial.print("Valeur a ecrire: ");
  Serial.println(instructionTrouvee.valeurAEcrire);
  Serial.print("Deplacement: ");
  Serial.println(instructionTrouvee.deplacement == -1 ? "droite" : "gauche");
  Serial.print("Etat suivant: ");
  Serial.println(instructionTrouvee.etatSuivant);
}
