

/******************* Hardware **************************/
/*
    uno
    capteur hall
    servo
    dht
    clock DS3231
    1 ou 2 bouton(s)
*/

/******************* Librairies **************************/
/*
    DHT sensor from adafruit
    Chrono
    RTClib (https://microcontrollerslab.com/ds3231-rtc-module-pinout-interfacing-with-arduino-features/)
      SCL A5
      SDA A4


*/


// INCLUDE CHRONO LIBRARY : http://github.com/SofaPirate/Chrono
#include <Chrono.h>

#include <DHT.h>
//objet chrono pour DHT11 && NTP (horloge)

Chrono horloge_Chrono;
Chrono porte_Chrono;

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"
RTC_DS3231 rtc;

//servomoteur continu
#include <Servo.h>
Servo servo_porte;



/******************* définitions des pins ************************************/

// capteur effet hall :
const byte capteur_pin = 13;     // capteur fin de course bas de la porte (D0 sur esp8266)

const byte sda_pin = A4;
const byte scl_pin = A5;

const byte DHTPin = 5;

const byte bouton_UPandDOWN = 2;

const byte servo_pin = 9;
/*************************** configuration ********************************************/
const int heure_ouverture = 6;
const int minute_ouverture = 00;

const int heure_fermeture = 22;
const int minute_fermeture = 00;

const int temps_porte_max = 4000; // temps ouverture maximum en millisecondes, si dépassé, on arrête le moteur
const int temps_porte_descend = 3000; // temps fermeture maximum en millisecondes, si dépassé, on arrête le moteur


/*************************** variables ***************************************/
//dht
float t, h;

// for DHT11,
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2

#define DHTType DHT11

DHT dht(DHTPin, DHTType);

bool etat_porte = 0;
bool etat_capteur = 0;

const byte servo_vitesse_up = 100;
const byte servo_vitesse_down = 80;
const byte servo_vitesse_pause = 90;

byte heure_maintenant;
byte  minute_maintenant;

/*************************** setUp ***************************************/
void setup() {

  Serial.begin(115200);
  dht.begin();

  pinMode(capteur_pin, INPUT_PULLUP);
  pinMode(bouton_UPandDOWN, INPUT_PULLUP);

  servo_porte.attach(servo_pin); // D5 sur esp
  servo_porte.write(90);
  // servo_porte.detach();

  delay(1000);

  // utiliser ce bloc pour ajuster l'heure :
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();

  }
  // January 21, 2014 at 3am you would call:
 // rtc.adjust(DateTime(2022, 5, 27, 18, 12, 50));

}





void loop() {

  // code de test de la porte:
  /*
    while (1)
    {
      porte_monte();
      delay(2000);
      servo_porte.write(servo_vitesse_pause);
      delay(5000);
          porte_descend();
      delay(2000);
          servo_porte.write(servo_vitesse_pause);
      delay(5000);

    }
  */

  btn();



  //*** toutes les 30 secondes, on check l'horloge
  if (horloge_Chrono.hasPassed(5000) ) {
    horloge_Chrono.restart();  // restart the crono so that it triggers again later
    horloge();
  }


  delay(100);

}

void btn () {
  if ( digitalRead(bouton_UPandDOWN) == LOW ) {
    Serial.print("bouton est pressé ");

    if (etat_porte == 0) {
      porte_monte();
    }
    else if (etat_porte == 1) {
      porte_descend();
    }
  }
}
void porte_monte() {

  Serial.println("Demande ouverture porte");

  if (etat_porte == 1) {
    Serial.println("Porte déjà OUVERTE");
  } else {
    porte_Chrono.restart();  // restart the crono so that it triggers again later
    servo_porte.attach(servo_pin); // D5 sur esp
    servo_porte.write(90);

    // tant que le capteur_pin haut n'est pas proche d'un aimant, faire tourner le moteur
    while (1) {

      //Serial.println("capteur_pin haut inactif");
      servo_porte.write(servo_vitesse_up);
      delay(5);
      Serial.print(digitalRead(capteur_pin));

      if (digitalRead(capteur_pin) == LOW)
      {
        Serial.println("Capteur haut OK ");

        break;
      }

      if (porte_Chrono.hasPassed(temps_porte_max) ) {
        Serial.println("Capteur haut inactif depuis trop longtemps, moteur coupé!");
        porte_Chrono.restart();  // restart the crono so that it triggers again later
        break;
      }

    }

    servo_porte.write(servo_vitesse_pause);
    etat_porte = 1;
    delay(500);

    servo_porte.detach();


  }
}
void porte_descend() {

  Serial.println("Demande fermeture porte");
  if (etat_porte == 0) {
    Serial.println("Porte déjà FERMEE");
  } else {
    Serial.println("Fermeture en cours");
    servo_porte.attach(servo_pin); // D5 sur esp
    servo_porte.write(90);


    porte_Chrono.restart();  // restart the crono so that it triggers again later


    // on vérifie si ça ne fait pas trop longtemps que cette porte essaie de bouger :-)
    while (1) {
      servo_porte.write(servo_vitesse_down);
      delay(5);


      if (porte_Chrono.hasPassed(temps_porte_descend) ) {
        Serial.println("Temps descend écoulé");
        porte_Chrono.restart();  // restart the crono so that it triggers again later
        break;
      }
      // frein moteur
    }
    delay(100);
    servo_porte.write(servo_vitesse_pause);
    etat_porte = 0;
    delay(500);

  servo_porte.detach();

  }
}

void horloge() {
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();



  // maintenant on utilise un circuit horloge en temps réel :

  heure_maintenant = now.hour();
  minute_maintenant = now.minute();
  delay(10);
  // wake up !!!! verification que la porte est bien ouverte
  if (heure_maintenant == heure_ouverture && heure_maintenant == minute_ouverture ) {
    porte_monte();
  }
  //time to sleep chickens <3 // verification que la porte est fermée
  else if ( heure_maintenant == heure_fermeture && minute_maintenant == minute_fermeture) {
    porte_descend();
  }

}
