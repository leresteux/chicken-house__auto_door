
// leresteux.net avec 123piano.com / 2022
/******************* Hardware **************************/
/*
    arduino uno / nano
    capteur hall
    servo continu
    clock DS3231
    1
*/
/******************* Librairies **************************/
/*
    Servo
    Chrono
    RTClib (https://microcontrollerslab.com/ds3231-rtc-module-pinout-interfacing-with-arduino-features/) en IIC
*/


// INCLUDE CHRONO LIBRARY : http://github.com/SofaPirate/Chrono
#include <Chrono.h>

Chrono horloge_Chrono;
Chrono porte_Chrono;

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"
RTC_DS3231 rtc;

//servomoteur continu
#include <Servo.h>
Servo servo_porte;
/******************* définitions des pins ************************************/
const byte capteur_pin = 8;     // capteur fin de course bas de la porte (D0 sur esp8266)
const byte bouton_UPandDOWN = 2;
const byte servo_pin = 10;
/*************************** configuration ********************************************/
const int heure_ouverture = 6;
const int minute_ouverture = 30;

const int heure_fermeture = 22;
const int minute_fermeture = 30;

const int temps_porte_max = 10000; // temps ouverture maximum en millisecondes, si dépassé, on arrête le moteur
const int temps_porte_descend = 4500; // temps fermeture maximum en millisecondes, si dépassé, on arrête le moteur


/*************************** variables ***************************************/

bool etat_porte = 0;
bool etat_capteur = 0;

const byte servo_vitesse_up = 100;//ok
const byte servo_vitesse_down = 80;//ok
const byte servo_vitesse_pause = 90;

byte heure_maintenant;
byte  minute_maintenant;

/*************************** setUp ***************************************/
void setup() {

  Serial.begin(9600);

  pinMode(capteur_pin, INPUT_PULLUP);
  pinMode(bouton_UPandDOWN, INPUT_PULLUP);


  delay(1000);

  // utiliser ce bloc pour ajuster l'heure :
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();

  } else {
    serial_date();
  }

  /*** reglage heure ***/
  // s'écrit ainsi rtc.adjust(DateTime(année, mois, jour, heure, minute, second));
  // rtc.adjust(DateTime(2022, 5, 29, 18, 00, 00));

}

void loop() {

  // code de test de la porte:
  /*
    while (1)
    {
      porte_monte();
      delay(2000);
      servo_porte.write(servo_vitesse_pause);
      delay(2000);
          porte_descend();
      delay(2000);
          servo_porte.write(servo_vitesse_pause);
      delay(2000);

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
    //si la porte est ouverte elle se ferme
    if (etat_porte == 0) {
      porte_monte();
    }
    // si fermée elle s'ouvre
    else if (etat_porte == 1) {
      porte_descend();
    }
  }
}
void porte_monte() {

  Serial.println("Demande ouverture porte");

  if (digitalRead(capteur_pin) == LOW) {
    Serial.println("Porte déjà OUVERTE");
  } else {
    porte_Chrono.restart();  // restart the crono so that it triggers again later
    servo_porte.attach(servo_pin); // D5 sur esp

    // tant que le capteur_pin haut n'est pas proche d'un aimant, faire tourner le moteur
    while (1) {

      //Serial.println("capteur_pin haut inactif");
      servo_porte.write(servo_vitesse_up);
      delay(5);
      Serial.println(digitalRead(capteur_pin));

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
  serial_date();
  DateTime now = rtc.now();
  heure_maintenant = now.hour();
  minute_maintenant = now.minute();
  delay(10);
  // wake up !!!! verification que la porte est bien ouverte
  if (heure_maintenant == heure_ouverture && minute_maintenant == minute_ouverture ) {
    porte_monte();
  }
  //time to sleep chickens <3 // verification que la porte est fermée
  else if ( heure_maintenant == heure_fermeture && minute_maintenant == minute_fermeture) {
    porte_descend();
  }
  //verification tout les heures piles que la porte est bien ouverte
  else if ( heure_maintenant > heure_ouverture && heure_maintenant < heure_fermeture && minute_maintenant == 00) {
    porte_monte();
  }
}
void serial_date() {
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
}
/*                                                                                                    
                                                                                                    
                                                        *&@@@@&(                                    
                                ,@@@@@@@&@/        ,@@@@@@&@@&@@@@@                                 
                              @@@@@(. .(@@@@@@&@@@@@&@%         *@@@@.                              
                             &@@@.         #@&&&@&.               *@@@&                             
                             @@@&                                   @&&%                            
                       /@@@@@@@@@                                   (&@@                            
                     #@@@@@@@@@@%                                   ,&@&/                           
                    #@@@(                                           %@@@                            
                    @@@@                                           &&@&(                            
                    @&@@                                         @&@@@.                             
                    #@@&,                                 ,(&@@@@@@&                                
                     @@@&                 *&@@@@@@@&@@&@@@@@@@@#                                    
                     ,&@@&          ,&@@@@@@@#*,.     .*#&@@@@@&%                                   
                      .&&@@      &@@@@@,                      .&@@@@*                               
                        @@@@( ,@@@@#                              /@@@@                             
                          &@@@@@&,      ,@%.(                        %@@@.                          
                           @@@@#      *@@@&#@@&                        (@@&                         
                          @@@@@@@,    (@@@@@@@@                          &@&,                       
                        #@@@@   @&*     (@@@&.                            /@&*                      
                      .@@@@,     @@                       /%               /@&,                     
                     (&@@@       *@@.                     @                 #@@                     
                    (&@@@@@@@@@@@@@@@@.                 *&,                  @@%                    
                    @@@@@@/.     &@@@@@@@.            /&&                    /@&                    
                                #@@@( &@@@@@@@%##%@@@@(                       @@.                   
                               (&@&%   @@@&,,,,,.                             &@,                   
                              @@&&,    &@@@                                   %&*                   
                             @@@@      @@@&                                   #@(                   
                            #@@@     .&@@@.                                   /@#                   
                             @@@@@&@@@@@&/                                     *                    
                                ,*,.#@@@,                                                           
                                   %&@@.                                                            
                                  @@@@                                                              
                                 @@@&                                                               
                               .&@@%                                                                
                                /%.
*/
