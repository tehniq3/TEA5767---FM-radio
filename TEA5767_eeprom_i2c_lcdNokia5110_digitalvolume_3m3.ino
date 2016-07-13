/*********************************************************************
This is an example sketch for our Monochrome Nokia 5110 LCD Displays
  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338
These displays use SPI to communicate, 4 or 5 pins are required to
interface
Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!
Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

// Nokia 5110 LCD (PCD8544) from https://code.google.com/p/pcd8544/
/* niq_ro ( http://nicuflorica.blogspot.ro ) case for Nokia 5110 LCD (PCD8544) - LPH 7366:
 For module from China, you must connect like this:
* Pin 1 (RST) -> Arduino digital 6 (D6)
* Pin 2 (CE) -> Arduino digital 7 (D7)
* Pin 3 (DC) -> Arduino digital 5 (D5)
* Pin 4 (DIN) -> Arduino digital 4 (D4)
* Pin 5 (CLK) - Arduino digital 3 (D3)
* Pin 6 (Vcc) -> +5V thru adaptor module (see http://nicuflorica.blogspot.ro/2013/06/afisajul-folosit-la-telefoanele-nokia.html )
* Pin 7 (LIGHT) -> +5V thru 56-100 ohms resistor (for permanent lights) or... other pin control
* Pin 8 (GND) -> GND1 or GND2 
*/

// adapted sketch by niq_ro from http://nicuflorica.blogspot.ro
// version 3.3

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
// Adafruit_PCD8544 display = Adafruit_PCD8544(SCLK, DIN, DC, CS, RST);
Adafruit_PCD8544 display = Adafruit_PCD8544(2, 3, 4, 5, 6);

#include <TEA5767.h>
// from https://github.com/andykarpov/TEA5767
#include <Wire.h>
// from http://arduino-info.wikispaces.com/file/view/Button.zip/405390486/Button.zip

// TEA5767 begin

TEA5767 Radio;
double old_frequency;
double frequency;
int search_mode = 0;
int search_direction;
unsigned long last_pressed;


#define incet 14  // volume - 
#define tare 15  // volume +
#define inainte 9    // search +
#define inapoi 8   // search -

byte f1, f2, f3, f4;  // number for each zone of frequency
double frecventa;
int frecventa10;      // frequency x 10 

#include <EEPROM.h>
//http://tronixstuff.com/2011/03/16/tutorial-your-arduinos-inbuilt-eeprom/


//#define PotWiperVoltage0 0
//#define PotWiperVoltage1 1
int PotWiperVoltage0 = 0;
int RawVoltage0 = 0;
float Voltage0 = 0;
int PotWiperVoltage1 = 1;
int RawVoltage1 = 0;
float Voltage1 = 0;
const int wiper0writeAddr = B00000000;
const int wiper1writeAddr = B00010000;
const int tconwriteAddr = B01000000;
const int tcon_0off_1on = B11110000;
const int tcon_0on_1off = B00001111;
const int tcon_0off_1off = B00000000;
const int tcon_0on_1on = B11111111;
const int slaveSelectPin = 10;    // Uno
//const int slaveSelectPin = 53;    // Mega
const int shutdownPin = 7;
// transform lin to log
float a = 0.2;     // for x = 0 to 63% => y = a*x;
float x1 = 63;
float y1 = 12.6;   // 
float b1 = 2.33;     // for x = 63 to 100% => y = a1 + b1*x;
float a1 = -133.33;

int level, level1, level2, stepi;

int lung = 500;    // pause for pushbutton
int scurt = 25;   // pause for repeat loop

byte sunet = 0;    
byte volmax = 15;
void setup () {

   // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin, OUTPUT);
  // set the shutdownPin as an output:
  pinMode (shutdownPin, OUTPUT);
  // start with all the pots shutdown
  digitalWrite(shutdownPin,LOW);
  // initialize SPI:
  SPI.begin(); 
 
digitalPotWrite(wiper0writeAddr, 0); // Set wiper 0 
digitalPotWrite(wiper1writeAddr, 0); // Set wiper 0 

  
  /* 
// write data in eeprom memory
EEPROM.write(101,0);
EEPROM.write(102,9);
EEPROM.write(103,8);
EEPROM.write(104,0);
EEPROM.write(105,10);
*/

// read value of last frequency
f1 = EEPROM.read(101);
f2 = EEPROM.read(102);
f3 = EEPROM.read(103);
f4 = EEPROM.read(104);
stepi = EEPROM.read(105);

// recover number
frecventa = 100*f1 + 10*f2 + f3 + 0.1*f4;

pinMode(inainte, INPUT);
pinMode(inapoi, INPUT);
pinMode(incet, INPUT);
pinMode(tare, INPUT);
digitalWrite(inainte, HIGH);
digitalWrite(inapoi, HIGH);
digitalWrite(incet, HIGH);
digitalWrite(tare, HIGH);

  Wire.begin();
  Radio.init();
//  Radio.set_frequency(104.5); 
   Radio.set_frequency(frecventa); 
 
//  Serial.begin(9600);  // for local control

  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(55);  // 50..100
  display.clearDisplay();


  // Print a logo message to the LCD.
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("tehnic.go.ro");
  display.setCursor(24, 8);
  display.print("& niq_ro");
  display.setTextSize(2);
  display.setCursor(1, 16);
  display.print("FM");  
  display.setTextSize(1);
  display.setCursor(30, 20);
  display.setTextColor(WHITE, BLACK);
  display.print(" radio ");  
  display.setCursor(0, 32);
  display.setTextColor(BLACK);
  display.print("with TEA5767");
  display.setCursor(0, 40);
  display.print("version ");
  display.setTextColor(WHITE, BLACK);
  display.print("3.3");
  display.display();
  delay (5000);
  display.clearDisplay(); 

int procent1 = stepi * 6.66;
    if (procent1 < 63)  // for x = 0 to 63% => y = a*x;
    {
    level = a * procent1;
    }
    else               // for x = 63 to 100% => y = y1 + b*x;
    {
    level = a1 + b1 * procent1;
    }  
level1 = map(level, 0, 100, 0, 255);      // convert 100% in 256 steps  
digitalPotWrite(wiper0writeAddr, level1); // Set wiper 0 to 255
digitalPotWrite(wiper1writeAddr, level1); // Set wiper 0 to 255
digitalWrite(shutdownPin,HIGH); //Turn off shutdown
}

void loop () {
  
  unsigned char buf[5];
  int stereo;
  int signal_level;
  double current_freq;
  unsigned long current_millis = millis();
  
  if (Radio.read_status(buf) == 1) {
    current_freq =  floor (Radio.frequency_available (buf) / 100000 + .5) / 10;
    stereo = Radio.stereo(buf);
    signal_level = Radio.signal_level(buf);
   display.setTextSize(2);
   display.setTextColor(BLACK);
   display.setCursor(0,0);
   if (current_freq < 100) display.print(" ");
   display.print(current_freq,1);
   display.setTextSize(1);
//   display.setCursor(47,15);
   display.print("MHz");
   // display level of FM signal..
   display.setCursor(50,40);
   display.setTextSize(1);
 //  display.setTextColor(WHITE, BLACK);
   if (signal_level<10) display.print(" ");
   display.print(signal_level);
   display.print("/15");

if (stepi <=0) stepi = 0;
if (stepi >=volmax) stepi = volmax;
   display.setCursor(28,40);
   display.setTextSize(1);
   display.setTextColor(WHITE, BLACK);
   if (stepi<10) display.print(" ");
   display.print(stepi);
//   display.print("/15");

 
   display.display();
   delay (500);
display.clearDisplay(); 

/*
// draw a signal level triangle...
   display.drawLine(80, 30, 80, 45, BLACK);
   display.drawLine(80, 45, 50, 45, BLACK);
  display.drawLine(50, 45, 80, 30, BLACK);
*/

// draw signal value
int sus = 8;
int sl = signal_level;
   for (int x = 0; x < sl; x++)
   { 
   display.drawLine(50+2*x, 45-sus, 50+2*x, 45-x-sus, BLACK);
   }

// draw an antenna
   display.drawLine(55, 32-sus, 55, 40-sus, BLACK);
   display.drawLine(56, 32-sus, 56, 40-sus, BLACK);
   display.drawLine(52, 32-sus, 55, 36-sus, BLACK);
   display.drawLine(51, 32-sus, 55, 37-sus, BLACK);
   display.drawLine(59, 32-sus, 56, 36-sus, BLACK);
   display.drawLine(60, 32-sus, 56, 37-sus, BLACK);


// draw volume steps
   for (int x = 0; x < stepi; x++)
   { 
//   display.drawLine(32-2*x, 47, 32-2*x, 47-x, BLACK);
   display.drawLine(16, 47-2*x, 16-x, 47-2*x, BLACK); 
   }

   display.setTextSize(1);
   display.setTextColor(BLACK);
   display.setCursor(18,24);
   display.print("V");
   display.setCursor(18,32);
   display.print("O");
   display.setCursor(18,40);
   display.print("L");
   
   if (stereo) {
/*
     display.setCursor(16,32);
   display.setTextSize(1);
   display.setTextColor(BLACK);
   display.print("STEREO");
*/   

   for (int x = 0; x < stepi; x++)
   { 
//   display.drawLine(32+2*x, 47, 32+2*x, 47-x, BLACK);
    display.drawLine(24, 47-2*x, 24+x, 47-2*x, BLACK);
   } 
    
   }
    else
   {
// nothing
   }     

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

if (sunet == 1) {
if (stepi <= 0) stepi = 0;
if (stepi >= volmax) stepi = volmax;
  EEPROM.write(105,stepi);
int procent1 = stepi * 6.66;
    if (procent1 < 63)  // for x = 0 to 63% => y = a*x;
    {
    level = a * procent1;
    }
    else               // for x = 63 to 100% => y = y1 + b*x;
    {
    level = a1 + b1 * procent1;
    }  
level1 = map(level, 0, 100, 0, 255);      // convert 100% in 256 steps  

if (level1 >255) level1 = 255;
digitalPotWrite(wiper0writeAddr, level1); // Set wiper 0 to 255
digitalPotWrite(wiper1writeAddr, level1); // Set wiper 0 to 255

sunet = 0;
}

  
 if (digitalRead(inainte) == LOW) { 
//    if (digitalRead(16) == LOW) { 
    last_pressed = current_millis;
    search_mode = 1;
    search_direction = TEA5767_SEARCH_DIR_UP;
    Radio.search_up(buf);
    delay(lung);
  }
    
 if (digitalRead(inapoi) == LOW) { 
    last_pressed = current_millis;
    search_mode = 1;
    search_direction = TEA5767_SEARCH_DIR_DOWN;
    Radio.search_down(buf);
    delay(lung);
  } 
  
 if (digitalRead(incet) == LOW) { 
    stepi = stepi -1;
    if (stepi <= 0) stepi == 0;
   sunet = 1;
    delay(lung/5);
   } 
   
 if (digitalRead(tare) == LOW) { 
    stepi = stepi +1;
    if (stepi > volmax ) stepi == volmax;
    sunet = 1;
    delay(lung/5);
  } 
 
  
   delay(scurt);
}


// This function takes care of sending SPI data to the pot.
void digitalPotWrite(int address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
}
