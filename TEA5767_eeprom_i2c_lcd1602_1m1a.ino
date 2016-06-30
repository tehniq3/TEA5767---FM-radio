#include <TEA5767.h>
// from https://github.com/andykarpov/TEA5767
#include <Wire.h>
// from http://arduino-info.wikispaces.com/HAL-LibrariesUpdates
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // adress is 0x20 for LCD16x2

// adapted sketch by Nicu FLORICA (niq_ro from http://www.tehnic.go.ro , http://nicuflorica.blogspot.ro & http://www.arduinotehnic.com )
// version 1m1a, 30.06.2016 - Craiova, Romania

#include <EEPROM.h>
//http://tronixstuff.com/2011/03/16/tutorial-your-arduinos-inbuilt-eeprom/


TEA5767 Radio;
double old_frequency;
double frequency;
int search_mode = 0;
int search_direction;
unsigned long last_pressed;

#define inainte 11
#define inapoi 12

byte f1, f2, f3, f4;  // number for each zone of frequency
double frecventa;
int frecventa10;      // frequency x 10 

void setup() {  
  Serial.begin(9600);
  Wire.begin();
 
/* 
// write data in eeprom memory
EEPROM.write(101,0);
EEPROM.write(102,9);
EEPROM.write(103,8);
EEPROM.write(104,0);
*/

// read value of last frequency
f1 = EEPROM.read(101);
f2 = EEPROM.read(102);
f3 = EEPROM.read(103);
f4 = EEPROM.read(104);

// recover number
frecventa = 100*f1 + 10*f2 + f3 + 0.1*f4;

  Radio.init();
//  Radio.set_frequency(104.5); 
  Radio.set_frequency(frecventa); 
//  lcd.init();
  lcd.begin();
  lcd.backlight(); //backlight is now ON
//  lcd.begin(16,2);
  lcd.clear();

pinMode(inainte, INPUT);
pinMode(inapoi, INPUT);
digitalWrite(inainte, HIGH);
digitalWrite(inapoi, HIGH);



}

void loop() {

  unsigned char buf[5];
  int stereo;
  int signal_level;
  double current_freq;
  unsigned long current_millis = millis();
  
  
  if (Radio.read_status(buf) == 1) {
    current_freq =  floor (Radio.frequency_available (buf) / 100000 + .5) / 10;
    stereo = Radio.stereo(buf);
    signal_level = Radio.signal_level(buf);
    lcd.setCursor(0,0);
    lcd.print("FM: ");
    if (current_freq < 100) lcd.print(" ");
    lcd.print(current_freq);
    lcd.print("MHz ");
    lcd.setCursor(0,1);
    if (stereo) lcd.print("STEREO "); else lcd.print("MONO   ");
    lcd.print(signal_level);
    lcd.print("/15 ");
  }
  
  if (search_mode == 1) {
      if (Radio.process_search (buf, search_direction) == 1) {
          search_mode = 0;  
      }    
      if (Radio.read_status(buf) == 1) {  
      frecventa =  floor (Radio.frequency_available (buf) / 100000 + .5) / 10;
      frecventa10 = 10*frecventa;
 f1 = frecventa10/1000;
 frecventa10 = frecventa10 - 1000*f1;
 f2 = frecventa10/100;
 frecventa10 = frecventa10 - 100*f2;
 f3 = frecventa10/10;
 f4 = frecventa10 - 10*f3;
EEPROM.write(101,f1);
EEPROM.write(102,f2);
EEPROM.write(103,f3);
EEPROM.write(104,f4);
frecventa = 100*f1 + 10*f2 + f3 + 0.1*f4;
Radio.set_frequency(frecventa);
      }

  }
  
 if (digitalRead(inainte) == LOW) { 
    last_pressed = current_millis;
    search_mode = 1;
    search_direction = TEA5767_SEARCH_DIR_UP;
    Radio.search_up(buf);
    delay(750);
  }
  
if (digitalRead(inapoi) == LOW) {    last_pressed = current_millis;
    search_mode = 1;
    search_direction = TEA5767_SEARCH_DIR_DOWN;
    Radio.search_down(buf);
    delay(750);
  } 
   delay(50);
}

