/*
 * utilise un arduino nano + ds3231 + 2 capteurs de Hall pour possition de la porte + Pont H + un moteur DC
 * Fablabke 2022
 */


/******************* Librairies **************************/


// INCLUDE CHRONO LIBRARY : http://github.com/SofaPirate/Chrono
#include <Chrono.h>


//objet chrono pour DHT11 && NTP (horloge)
Chrono capteur_Chrono;
Chrono horloge_Chrono;



// real time clock, module DS3231
#include <DS3231.h>
#include <Wire.h> // pour i2c

DS3231 myRTC;




// nom des pins esp8266 : https://mechatronicsblog.com/esp8266-nodemcu-pinout-for-arduino-ide/


/******************* définitions des pins ************************************/

// capteur effet hall :
const byte capteur_bas = 8;
const byte capteur_haut =  7;

// controlleur moteur
// https://passionelectronique.fr/tutoriel-l298n/

const byte moteur_a = 3; //  D3 nano
const byte moteur_b = 4; //  D4 nano



/*************************** configuration ********************************************/
const int heure_ouverture = 6;
const int minute_ouverture = 00;

const int heure_fermeture = 22;
const int minute_fermeture = 00;

const int temps_porte_max = 6000; // temps ouverture/fermeture maximum en millisecondes, si dépassé, on arrête le moteur

const int bouton_UPandDOWN = 9;
//const int bouton_DOWN = 10



/*************************** variables ***************************************/
// horloge
int heure;
int minute;


bool century = false;
bool h12Flag;
bool pmFlag;

bool  etat_porte = 0; // 1 = levée / 0 = baissée
bool  moment_de_la_journee = 0; // 1 = jour / 0 = nuit en fonction de l'horloge


void setup() {


  Serial.begin(115200);


  Serial.println("-------------------- Chicken controller -------------");
  Serial.println("startup");


  delay(10);

  //*** Start the I2C interface
  Wire.begin();

  pinMode(capteur_bas, INPUT);
  pinMode(capteur_haut, INPUT);

  pinMode(moteur_a, OUTPUT);
  pinMode(moteur_b, OUTPUT);


  pinMode(bouton_UPandDOWN, INPUT_PULLUP);
  // pinMode(bouton_b, INPUT_PULLUP);



  delay(1000);


  //*** utiliser ce bloc pour ajuster l'heure :
  /*
    myRTC.setClockMode(false);
    myRTC.setYear(22);
    myRTC.setMonth(5);
    myRTC.setDate(19);
    myRTC.setDoW(4);

    myRTC.setHour(16);
    myRTC.setMinute(48);

    myRTC.setSecond(0);

  */


  horloge();

}




void loop() {

  // code de test de la porte:
  /*
    while (1)
    {
    porte_monte();
    delay(8000);
    porte_descend();
    delay(8000);
    }
  */




  //*** toutes les 30 secondes, on check l'horloge
  if (horloge_Chrono.hasPassed(30000) ) {
    horloge_Chrono.restart();  // restart the crono so that it triggers again later
    horloge();
  }


  if (digitalRead(bouton_UPandDOWN) == LOW) {
    // Serial.print("bouton a pressé ");

    if (etat_porte == 0) {
      porte_monte();
    }
    else if (etat_porte == 1) {
      porte_descend();
    }
  }



  delay(100);


}





void porte_monte() {



// Serial.println("Demande ouverture porte");

//*** active le moteur

  int temps = millis();


  // tant que le capter haut n'est pas proche d'un aimant, faire tourner le moteur
  while (digitalRead(capteur_haut) == HIGH && digitalRead(capteur_haut) == HIGH && digitalRead(capteur_haut) == HIGH)
  {
    //Serial.println("Capteur haut inactif");
    digitalWrite(moteur_a, HIGH);
    digitalWrite(moteur_b, LOW);


    // on vérifie si ça ne fait pas trop longtemps que cette porte essaie de bouger :-)
    if (millis() > temps + temps_porte_max) {
      //Serial.println("Capteur haut inactif depuis trop longtemps, moteur coupé!");
      break;

    }

    delay(5);
  }

  // frein moteur
  digitalWrite(moteur_a, LOW);
  digitalWrite(moteur_b, LOW);


  envoiCapteurs();




  delay(500);

  // couper le moteur


  etat_porte = 1;

}

void porte_descend() {

  // Serial.println("Demande fermeture porte");

  // active le moteur


  int temps = millis();

  // tant que le capter bas n'est pas proche d'un aimant, faire tourner le moteur
  while (digitalRead(capteur_bas) == HIGH && digitalRead(capteur_bas) == HIGH && digitalRead(capteur_bas) == HIGH)
  {
    // Serial.println("Capteur bas inactif");
    digitalWrite(moteur_a, LOW);
    digitalWrite(moteur_b, HIGH);

    // on vérifie si ça ne fait pas trop longtemps que cette porte essaie de bouger :-)
    if (millis() > temps + temps_porte_max) {
      //Serial.println("Capteur bas inactif depuis trop longtemps, moteur coupé!");
      break;
    }

    delay(5);
  }


  // frein moteur
  digitalWrite(moteur_a, LOW);
  digitalWrite(moteur_b, LOW);

  delay(500);

  etat_porte = 0;

}


// pour debug

void envoiCapteurs() {

  Serial.print("Capteur haut : ");
  Serial.println(!digitalRead(capteur_haut));


  Serial.print("Capteur bas : ");
  Serial.println(!digitalRead(capteur_bas));

}

void horloge() {

  /*

    Serial.println("Heure RTC DS3132 : ");
    Serial.print("Date: ");
    Serial.println(myRTC.getDate());

    Serial.print("Month: ");
    Serial.println(myRTC.getMonth(century));

      Serial.print("Year :");
    Serial.println(myRTC.getYear());

    Serial.print("Hour : ");
    Serial.println(myRTC.getHour(h12Flag, pmFlag));

    Serial.print("Minute :");
    Serial.println(myRTC.getMinute());

    Serial.print("Second :");
    Serial.println(myRTC.getSecond());

  */


  // maintenant on utilise un circuit horloge en temps réel :

  heure = myRTC.getHour(h12Flag, pmFlag);
  minute = myRTC.getMinute();
  delay(10);
// wake up !!!! verification que la porte est bien ouverte 
  if (heure >= heure_ouverture && heure<heure_fermeture && minute == minute_ouverture) {
    porte_monte();
  }
//time to sleep chickens <3 // verification que la porte est fermée
  else if (heure < heure_ouverture || heure>=heure_fermeture && minute == minute_fermeture) {
  porte_descend();
  }

}
