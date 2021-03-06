#include <ButtonDebounce.h>

#define be16toh(s) \
  ((uint16_t)(((s & 0xff00) >> 8) | ((s & 0x00ff) << 8)))
            
#define EMU_FRAME_MAGIC 0xa3
#pragma pack(push)
#pragma pack(1)

typedef struct _emu_frame {
  uint8_t channel,
          magic;
  uint16_t value;
  uint8_t checksum;
} emu_frame;

emu_frame frame;

#pragma pack(pop)

#include "LiquidCrystal.h"
LiquidCrystal lcd(8,9,4,5,6,7);

int lcd_key     = 0;
int adc_key_in  = 0;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5


long lastDebounceTime = 0;
long debounceDelay = 50;

int read_LCD_buttons()
{
 adc_key_in = analogRead(0);
 if (adc_key_in > 1000) {
  return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 }
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250) {
  delay(200);
  return btnUP; 
 }
 if (adc_key_in < 450) {
  delay(200);
  return btnDOWN; 
 }
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  
 return btnNONE;  // when all others fail, return this...
}

struct {
  const char *name, *unit;
  float divisor;
  int gaugeMin, gaugeMax;
} channels[256] = {
        { NULL, NULL, 0, 0, 0},
        { "RPM", "RPM", 1, 0, 9000 },
        { "MAP", "kPa", 1, 0, 400 },
        { "TPS", "%", 1, 0, 100 },
        { "IAT", "C", 1, -40, 120 },
        { "Battery voltage", "V", 37, 8, 20 },
        { "Igntion Angle", "deg", 2, -20, 60 },
        { "Secondary inj. PW", "ms", 62, 0, 25 },
        { "Injectors PW", "ms", 62, 0, 25 },
        { "EGT #1", "C", 1, 0, 1100 },
        { "EGT #2", "C", 1, 0, 1100 },
        { "Knock Level", "V", 51, 0, 5 },
        { "Dwell Time", "ms", 20, 0, 10 },
        { "AFR", "AFR", 10, 10, 20 },
        { "Gear", "", 1, 0, 6 },
        { "BARO", "kPa", 1, 50, 120 },
        { "Analog #1", "V", 51, 0, 5 },
        { "Analog #2", "V", 51, 0, 5 },
        { "Analog #3", "V", 51, 0, 5 },
        { "Analog #4", "V", 51, 0, 5 },
        { "Injectors DC", "%", 2, 0, 100 },
        { "ECU Temperature", "C", 1, -40, 120 },
        { "Oil pressure", "Bar", 16, 0, 12 },
        { "Oil temperature", "C", 1, 0, 160 },
        { "Fuel pressure", "Bar", 32, 0, 7 },
        { "CLT", "C", 1, -40, 220 },
        { "FF Ethanol content", "%", 2, 0, 100 },
        { "FF Fuel Temp", "C", 1, -30, 120 },
        { "Lambda", "λ", 128, 0.7, 1.3 },
        { "Vehicle Speed", "km/h", 4, 0, 300 },
        { "Fuel pressure delta", "kPa", 1, 100, 500 },
        { "Fuel level", "%", 1, 0, 100 },
        { "Tables set", "", 1, 0, 1 },
        { "Lambda target", "λ", 100, 0.7, 1.3 },
        { "AFR Target", "AFR", 10, 10, 20 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        { NULL, NULL, 0, 0, 0 },
        {"cel", "", 1, 0, 0}
};

void setup() {
      lcd.begin(16,2);
      lcd.setCursor(0,0);
      lcd.print("EcuMasters ");
      lcd.setCursor(0,1);
      lcd.print("Plug me in!");
      Serial.begin(19200);
      Serial1.begin(19200);
}

int checksum = 0;
int lcdChannel = 1;
int reading = 0;

void DEBUG() {
  
  Serial.print("Channel: ");
  Serial.print(channels[frame.channel].name);
  Serial.print(": ");
  Serial.print((float)be16toh(frame.value) / channels[frame.channel].divisor, 0);
  Serial.print(" ");
  Serial.print(channels[frame.channel].unit);
  Serial.print("\n");
  lcd_key = read_LCD_buttons();  // read the buttons
  switch (lcd_key) {
    case btnUP:
      {
        lcdChannel++;
        break;
      }
    case btnDOWN:
      {
        lcdChannel--;
        break;
      }
    case btnNONE:
      {
        break;
      }
   }
   if (lcdChannel == 36) {
      lcdChannel = 1;
   }
   if (lcdChannel == 0) {
    lcdChannel = 35;
   }
   lcd.clear();
   lcd.setCursor(14,0);
   lcd.print(lcdChannel);
}

void loop() {
      //DEBUG();
      size_t readlen;
      emu_frame raw;
      if (Serial1.available() >= 5) {
        while ((readlen = Serial1.readBytes((char *)&frame, sizeof(frame))) != 0) {
          uint8_t checksum;
          for (;;) {
            checksum = frame.channel + frame.magic + ((frame.value & 0xff00) >> 8) + (frame.value & 0x00ff) % 255;
            if (checksum == frame.checksum) {
              break;
            }
            if (frame.magic != 0xa3) {
              memmove(&frame, ((uint8_t *)&frame) + 1, sizeof(emu_frame) - 1);
              frame.checksum = (uint8_t)Serial1.read();
            } else {
              break;
            }
          };
          if (lcdChannel == frame.channel) {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(channels[lcdChannel].name);
            lcd.setCursor(14,0);
            lcd.print(lcdChannel);
            lcd.setCursor(0,1);
            lcd.print(be16toh(frame.value) / channels[frame.channel].divisor);
            lcd.print(" ");
            lcd.setCursor(10,1);
            lcd.print(channels[lcdChannel].unit);
          }
          lcd_key = read_LCD_buttons();  // read the buttons
          switch (lcd_key) {
            case btnUP:
              {
                lcdChannel++;
                break;
              }
            case btnDOWN:
            {
              lcdChannel--;
              break;
            }
            case btnNONE:
            {
              break;
            }
          }
          if (lcdChannel == 36) {
            lcdChannel = 1;
          }
          if (lcdChannel == 0) {
            lcdChannel = 35;
          }
        }
      }
}
