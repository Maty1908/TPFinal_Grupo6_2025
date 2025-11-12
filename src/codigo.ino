#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// Display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad
byte rows[] = {11, 10, 9, 8};
byte columns[] = {7, 6, 5, 4};
char keys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// LED
const int LED_ROJO = 12;
const int LED_AMARILLO = 13;
const int LED_AZUL = A1;

// Buzzer
#define BUZZER 3

// Sensor de ultrasonido
const int TRIGGER_PIN = A2;
const int ECHO_PIN = 2;

// Variables
char numero[3] = {'\0', '\0', '\0'};
byte indice = 0;
int numero_generado;
int intentos = 1;

// Funciones del teclado
char getKey() {
  char k = '\0';
  for (int r = 0; r < 4; r++) {
    digitalWrite(rows[r], LOW);
    for (int c = 0; c < 4; c++) {
      if (digitalRead(columns[c]) == LOW) {
        k = keys[r][c];
        break;
      }
    }
    digitalWrite(rows[r], HIGH);
    if (k) break;
  }
  return k;
}

char getKeyOnce() {
  static char lastVal = '\0';
  static unsigned long m = 0;
  char val = getKey();
  if (lastVal != val && millis() > (m + 120)) {
    lastVal = val;
    m = millis();
    return lastVal;
  }
  return '\0';
}

// LED
void apagarTodosLosLeds() {
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_AZUL, LOW);
}

void mostrarProximidad(int opcion) {
  int diferencia = abs(numero_generado - opcion);
  apagarTodosLosLeds();

  if (diferencia == 1) digitalWrite(LED_AZUL, HIGH);
  else if (diferencia >= 2 && diferencia <= 5) digitalWrite(LED_AMARILLO, HIGH);
  else if (diferencia > 5) digitalWrite(LED_ROJO, HIGH);
}

// Canción cuando ganas
void reproducirCancion() {
  struct Nota { int freq; int dur; };
  Nota melodia[] = {
    {294,125},{392,125},{440,125},{466,250},{466,125},{466,250},{466,125},{466,125},{466,125},
    {466,125},{466,250},{466,125},{466,250},{466,125},{440,125},{392,125},{466,375},{392,500},
    {294,125},{392,125},{440,125},{466,250},{466,125},{466,250},{466,125},{466,125},{466,125},
    {466,125},{466,250},{466,125},{466,250},{466,125},{440,125},{392,125},{523,750},{0,875},
    {370,125},{0,125},{440,125},{523,250},{0,375},{523,125},{0,125},{523,125},{523,125},{0,125},
    {523,125},{523,250},{523,125},{523,125},{523,125},{523,125},{0,375},{466,500},{0,625},{294,125},
    {392,125},{440,125},{466,250},{466,125},{466,250},{466,125},{466,125},{466,125},{440,250},
    {440,125},{440,250},{440,125},{466,125},{440,125},{392,250},{392,375},{587,250},{587,125},{587,250},{0,0}
  };

  for (int i = 0; i < sizeof(melodia)/sizeof(melodia[0]); i++) {
    int freq = melodia[i].freq;
    int dur = melodia[i].dur;

    if (freq == 0) {
      noTone(BUZZER);
      apagarTodosLosLeds();
      delay(dur);
    } else {
      tone(BUZZER, freq);
      if (i % 2 == 0) {
        digitalWrite(LED_AZUL, HIGH);
        digitalWrite(LED_AMARILLO, LOW);
      } else {
        digitalWrite(LED_AMARILLO, HIGH);
        digitalWrite(LED_AZUL, LOW);
      }
      delay(dur);
      noTone(BUZZER);
    }
  }
  apagarTodosLosLeds();
}

// Canción cuando perdes
void reproducirPerder() {
  struct Nota { int freq; int dur; };
  Nota melodia[] = {
    {880,300},{831,300},{784,300},{740,300},{698,300},
    {659,400},{622,400},{587,500},{0,400},
    {587,300},{523,300},{494,300},{440,500},{0,600}
  };

  for (int i = 0; i < sizeof(melodia)/sizeof(melodia[0]); i++) {
    int freq = melodia[i].freq;
    int dur = melodia[i].dur;

    if (freq == 0) {
      noTone(BUZZER);
      digitalWrite(LED_ROJO, LOW);
      delay(dur);
    } else {
      tone(BUZZER, freq);
      digitalWrite(LED_ROJO, HIGH);
      delay(dur * 0.8);
      digitalWrite(LED_ROJO, LOW);
      delay(dur * 0.2);
      noTone(BUZZER);
    }
  }

  // GAME OVER
  lcd.clear();
  lcd.print("GAME OVER");
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_ROJO, HIGH);
    delay(150);  
    digitalWrite(LED_ROJO, LOW);
    delay(150);
  }
  tone(BUZZER, 200, 300);
  delay(300);
  noTone(BUZZER);

  apagarTodosLosLeds();
}

// Ultrasónico
long medirDistancia() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long duracion = pulseIn(ECHO_PIN, HIGH);
  long distancia = duracion * 0.034 / 2;
  return distancia;
}

// Juego
void siguienteIntento();
void reiniciarJuego();

void siguienteIntento() {
  intentos++;
  if (intentos > 3) {
    lcd.clear(); lcd.print("Sin intentos!");
    lcd.setCursor(0,1); lcd.print("Era: "); lcd.print(numero_generado);
    reproducirPerder();
    delay(1000);
    reiniciarJuego();
    return;
  }
  indice = 0;
  numero[0] = '\0';
  lcd.clear();
  lcd.print("Intento "); lcd.print(intentos); lcd.print("/3");
  lcd.setCursor(0,1); lcd.print("Num (10-20):");
}

void reiniciarJuego() {
  intentos = 1;
  indice = 0;
  numero[0] = '\0';
  numero_generado = random(10, 21);
  apagarTodosLosLeds();
  lcd.clear();
  lcd.print("Nuevo numero!");
  delay(1500);
  lcd.clear();
  lcd.print("Intento 1/3");
  lcd.setCursor(0,1); lcd.print("Num (10-20):");
}

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(rows[i], OUTPUT);
    pinMode(columns[i], INPUT_PULLUP);
    digitalWrite(rows[i], HIGH);
  }

  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  randomSeed(analogRead(A0));

  numero_generado = random(10, 21);

  lcd.setCursor(0,0); lcd.print("Adivina el num!");
  delay(1500);
  lcd.clear();
  lcd.print("Intento 1/3");
  lcd.setCursor(0,1); lcd.print("Num (10-20):");
}

void loop() {
  char k = getKeyOnce();

  if (k) {
    if (k >= '0' && k <= '9') {
      if (indice < 2) {
        numero[indice] = k;
        indice++;
        numero[indice] = '\0';
        lcd.setCursor(11,1);
        lcd.print("    ");
        lcd.setCursor(11,1);
        lcd.print(numero);
      }
    }

    else if (k == '*') {
      if (indice > 0) {
        indice--;
        numero[indice] = '\0';
        lcd.setCursor(11,1); lcd.print("    ");
        lcd.setCursor(11,1); lcd.print(numero);
      }
    }

    else if (k == '#') {
      if (indice > 0) {
        numero[indice] = '\0';
        int opcion = atoi(numero);
        lcd.clear();

        if (opcion < 10 || opcion > 20) {
          lcd.print("Numero invalido");
          lcd.setCursor(0,1); lcd.print("Elija otro num");
          delay(2000);
          indice = 0; numero[0] = '\0';
          lcd.clear(); lcd.print("Intento "); lcd.print(intentos); lcd.print("/3");
          lcd.setCursor(0,1); lcd.print("Num (10-20):");
        }

        else if (opcion == numero_generado) {
          apagarTodosLosLeds();
          lcd.clear(); lcd.print("Felicidades!");
          lcd.setCursor(0,1); lcd.print("Adivinaste!");
          reproducirCancion();

          long objetivo = numero_generado;
          long distancia = 0;
          lcd.clear();
          lcd.print("Ubica el objeto");

          while (true) {
            distancia = medirDistancia();
            lcd.setCursor(0,1);

            if (distancia >= objetivo - 2 && distancia <= objetivo + 2) {
              lcd.clear();
              lcd.print("Objetivo logrado!");
              delay(800);
              break;
            }
            else if (distancia < objetivo) {
              lcd.print("Alejate! ");
              lcd.print(distancia); lcd.print("cm ");
            }
            else {
              lcd.print("Acercate! ");
              lcd.print(distancia); lcd.print("cm ");
            }
            delay(250);
          }
          reiniciarJuego();
        }

        else {
          mostrarProximidad(opcion);
          if (opcion < numero_generado) lcd.print("Es mayor!");
          else lcd.print("Es menor!");
          lcd.setCursor(0,1);
          lcd.print("Intento "); lcd.print(intentos); lcd.print("/3");
          delay(3000);
          siguienteIntento();
        }
        indice = 0;
        numero[0] = '\0';
      }
    }
  }
}
