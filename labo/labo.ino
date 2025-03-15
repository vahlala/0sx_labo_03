#define BTN_PIN 2
#include <LCD_I2C.h>
#include <Wire.h>

LCD_I2C lcd(0x27, 16, 2);
const int ThermistorPin = A0;
const int LED = 8;
const int Joystick_X = A1;
const int Joystick_Y = A2;
const int Joystick_Button = 2;
unsigned long currentTime = 0;
unsigned long startTime = 0;
int mintempChange = 24;
int maxtempChange = 25;
bool page0Affiche = true;
int x_raw, y_raw;

byte caractere_personnaliser[8] = {
  0b11111,
  0b00110,
  0b01100,
  0b11011,
  0b01100,
  0b10110,
  0b11011,
  0b01100
};

float tempManager() {
  int Vo;
  const float R1 = 5100;
  const float c1 = 1.129148e-03, c2 = 2.34125e-04, c3 = 8.76741e-08;
  const int coef= 273.15;
  Vo = analogRead(ThermistorPin);
  float R2 = R1 * (1023.0 / (float)Vo - 1.0);
  float logR2 = log(R2);
  float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  float Tc = T - coef;

  if (Tc > maxtempChange){
  digitalWrite(LED,HIGH);
  }else if (Tc < mintempChange){
    digitalWrite(LED,LOW);
  }
  return Tc;
}

void lireJoystick() {
  static int etatPrecedent = HIGH;
  static int state = HIGH;
  const int interval = 80;
  int etatPresent = digitalRead(Joystick_Button);
  static unsigned long previousMillis = 0;

  x_raw = analogRead(Joystick_X);
  y_raw = analogRead(Joystick_Y);

  page0();

  if (currentTime - previousMillis >= interval) {
    previousMillis = currentTime;
    if (etatPresent == LOW && etatPrecedent == HIGH) {
      state = !state;
      lcd.clear();
    }
    etatPrecedent = etatPresent;
  }

  if (state == HIGH && !page0Affiche) {
    page1();
  }
  if (state == LOW && !page0Affiche) {
    page2();
  }
}

void page0() {
  int finDA = 76;
  const int interval = 3000;
  if (page0Affiche) {
    lcd.setCursor(0, 0);
    lcd.print(F("NUEKUMO"));
    lcd.createChar(0, caractere_personnaliser);
    lcd.setCursor(0, 1);
    lcd.write(byte(0));
    lcd.setCursor(13, 1);
    lcd.print((finDA));
    if (currentTime - startTime >= interval) {
      page0Affiche = false;
      lcd.clear();
    }
  }
}

void page1() {
  const int interval = 100;
  static unsigned long previousMillis = 0;
  if (currentTime - previousMillis >= interval) {
    previousMillis = currentTime;
    float Tc = tempManager();
    lcd.setCursor(0, 0);
    lcd.print(F("Temperature:"));
    lcd.print(Tc);
    lcd.print(F(" C"));
    lcd.setCursor(0, 1);
    lcd.print(F("Climatisation:"));
    lcd.print(Tc > maxtempChange ? "ON" : "OFF");
  }
}

void page2() {
 const int valdebut = 0;
 const int valXmilieu = 507;
 const  int valfin = 1023;
 const int Xvoiture0 = -25;
 const int Xvoiture1 = 120;
 const int valYmilieu = 504;
 const int yVoiture0 = -90;
 const int yVoiture1 = 90;
  const int interval = 100;
  static unsigned long previousMillis = 0;
  if (currentTime - previousMillis >= interval) {
    previousMillis = currentTime;
    int vitesse;
    
    if (x_raw < valXmilieu) {
      vitesse = map(x_raw, valdebut, valXmilieu, Xvoiture0, valdebut);
      lcd.setCursor(0, 0);
      lcd.print("Recule:");
      lcd.print(vitesse);
      lcd.print("km/h  ");
    } else if (x_raw > valXmilieu) {
      vitesse = map(x_raw, valXmilieu + 1, valfin, valdebut, Xvoiture1);
      lcd.setCursor(0, 0);
      lcd.print("Avance:");
      lcd.print(vitesse);
      lcd.print("km/h  ");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Neutre:0 km/h  ");
    }

    if (y_raw < 504) {
      int angle = map(y_raw,valdebut, valYmilieu, yVoiture0 , valdebut);
      lcd.setCursor(0, 1);
      lcd.print("G: ");
      lcd.print(angle);
      lcd.print("Deg  ");
    } else if (y_raw > 504) {
      int angle = map(y_raw, valYmilieu + 1, valfin, valdebut, yVoiture1);
      lcd.setCursor(0, 1);
      lcd.print("D:");
      lcd.print(angle);
      lcd.print("Deg  ");
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Droit    ");
    }
  }
}

void afficher() {
  float Tc = tempManager();
  const int interval = 100;
  const long DA = 2412876;
  static unsigned long previousMillis = 0;
  if (currentTime - previousMillis >= interval) {
    previousMillis = currentTime;

    Serial.print("etd:");
    Serial.print(DA);
    Serial.print(",x:");
    Serial.print(x_raw);
    Serial.print(",y:");
    Serial.print(y_raw);
    Serial.print(",Valsys:");
    Serial.print(Tc > maxtempChange ? "1" : "0");
    Serial.println("");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(Joystick_Button, INPUT_PULLUP);
  lcd.begin();
  lcd.backlight();
}

void loop() {
  currentTime = millis();
  lireJoystick();
  afficher();
}