#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include <SPI.h>
#include <WiFi101.h>
#include "wiring_private.h"
#define SECRET_SSID "ECUMASTERS"
#define SECRET_PASS "ecumastersemu"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750
#define EMU_FRAME_MAGIC 0xa3
#define interruptPin 16
#if defined (__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined (__AVR_ATmega328P__)
#define STMPE_CS 6
#define TFT_CS   9
#define TFT_DC   10
#define SD_CS    5
#endif
#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
#endif

#define be16toh(s) \
  ((uint16_t)(((s & 0xff00) >> 8) | ((s & 0x00ff) << 8)))

#pragma pack(push)
#pragma pack(1)


typedef struct _emu_frame {
  union {
    struct {
      uint8_t channel,
              magic;
      uint16_t value;
      uint8_t checksum;
    };
    uint8_t bytes[5];
  };
} emu_frame;

emu_frame frame;

#pragma pack(pop)

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
WiFiServer server(8080);
int status = WL_IDLE_STATUS;



int checksum = 0;
int page = 1;
uint16_t values[35];

char printbuf[10];

void render_rpm() {
  sprintf(printbuf, "%4d", values[1]);
  tft.println(printbuf);
}

void render_map() {
  sprintf(printbuf, "%4d", values[2]);
  tft.println(printbuf);
}

void render_tps() {
  sprintf(printbuf, "%4d", values[3]);
  tft.println(printbuf);
}

void render_iat() {
  sprintf(printbuf, "%4d", values[4]);
  tft.println(printbuf);
}


void render_bat() {
  tft.println((float)values[5] / 37, 1);
}

void render_ign() {
  tft.println(values[6] / 2);
}

void render_secinjpw() {
  tft.println(values[7] / 62);
}

void render_injpw() {
  tft.println(values[8] / 62);
}

void render_egt1() {
  tft.println(values[9]);
}

void render_egt2() {
  tft.println(values[10]);

}

void render_knockv() {
  tft.println((float)values[11] / 51, 1);

}

void render_dwell() {
  tft.println(values[12] / 20);
}

void render_afr() {
  tft.println((float)values[12] / 10, 1);
}

void render_gear() {
  tft.println(values[14]);
}

void render_baro() {
  tft.println(values[15]);

}

void render_analog1() {
  tft.println((float)values[16] / 51, 1);
}

void render_analog2() {
  tft.println((float)values[17] / 51, 1);
}

void render_analog3() {
  tft.println((float)values[18] / 51, 1);
}

void render_analog4() {
  tft.println((float)values[19] / 51, 1);
}

void render_injdc() {
  tft.println(values[20] / 2);
}

void render_ecutmp() {
  tft.println(values[21]);

}

void render_oilps() {
  tft.println(values[22] / 16);
}

void render_oiltmp() {
  tft.println(values[23]);

}

void render_fuelps() {
  tft.println(values[24]);

}

void render_clt() {
  tft.println(values[25] / 32);
}

void render_ffcnt() {
  tft.println(values[26] / 2);
}

void render_fftmp() {
  tft.println(values[27]);
}

void render_lambda() {
  tft.println((float)values[28] / 128, 1);
}

void render_speed() {
  tft.println(values[29] / 4);
}

void render_fpdelta() {
  tft.println(values[30] / 4);
}

void render_fuellvl() {
  tft.println(values[31]);
}

void render_table() {
  tft.println(values[32]);
}

void render_lambdatgt() {
  tft.println((float)values[33] / 100, 1);
}

void render_afrtgt() {
  tft.println((float)values[34] / 10, 1);
}

void render_cel() {
  tft.println(be16toh(values[255]));
}

struct {
  const char *name, *unit;
  void (*render)();
} channels[256] = {
  { NULL, NULL, NULL},
  { "RPM", "RPM", render_rpm },
  { "MAP", "kPa", render_map },
  { "TPS", "%", render_tps },
  { "IAT", "C", render_iat },
  { "Battery", "V", render_bat },
  { "Ign. Angle", "deg", render_ign },
  { "Inj. PW", "ms", render_injpw },
  { "EGT #1", "C", render_egt1 },
  { "EGT #2", "C", render_egt2 },
  { "Knock Level", "V", render_knockv },
  { "Dwell", "ms", render_dwell },
  { "AFR", "AFR", render_afr },
  { "Gear", "", render_gear },
  { "BARO", "kPa", render_baro },
  { "Analog #1", "V", render_analog1 },
  { "Analog #2", "V", render_analog2 },
  { "Analog #3", "V", render_analog3 },
  { "Analog #4", "V", render_analog4 },
  { "Inj. DC", "%", render_injdc },
  { "ECU Temp.", "C", render_ecutmp },
  { "Oil press.", "Bar", render_oilps },
  { "Oil temp.", "C", render_oiltmp },
  { "Fuel press.", "Bar", render_fuelps },
  { "CLT", "C", render_clt },
  { "FF content", "%", render_ffcnt },
  { "FF Temp", "C", render_fftmp },
  { "Lambda", "λ", render_lambda },
  { "Speed", "kmh", render_speed },
  { "FP delta", "kPa", render_fpdelta },
  { "Fuel lvll", "%", render_fuellvl },
  { "Table", "", render_table },
  { "Lambda tgt.", "λ", render_lambdatgt },
  { "AFR Target", "AFR", render_afrtgt },
  {"cel", "", render_cel}
};

#include <Fonts/FreeSerifBoldItalic12pt7b.h>

void render_page() {
  tft.setFont(&FreeSerifBoldItalic12pt7b);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 20);
  tft.setTextSize(1);
  tft.println(channels[page].name);
  tft.setCursor(230, 235);
  tft.setTextSize(1);
  tft.println(channels[page].unit);
  tft.setTextSize(5);
}


void setup() {
  WiFi.setPins(8,7,4,2);

  memset(&values, 0, sizeof(values));

  tft.begin();
  if (!ts.begin()) {
    while (1);
  } 


  tft.setRotation(1);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  
  render_page();

  Serial.begin(19200);
  Serial1.begin(19200);

  status = WiFi.beginAP(ssid, pass);
  if ( status != WL_AP_LISTENING) { 
    Serial.println("Couldn't get a wifi connection");
    while(true);
  }
  server.begin();
  
}

void SERCOM0_Handler() {
  Serial1.IrqHandler();
}

void Uart::IrqHandler() {
  if (sercom->availableDataUART()) {
    rxBuffer.store_char(sercom->readDataUART());
  }

  if (sercom->isUARTError()) {
    sercom->acknowledgeUARTError();
    // TODO: if (sercom->isBufferOverflowErrorUART()) ....
    // TODO: if (sercom->isFrameErrorUART()) ....
    // TODO: if (sercom->isParityErrorUART()) ....
    sercom->clearStatusUART();
  }
  WiFiClient client = server.available();
  if (rxBuffer.isFull()) {
    size_t available;
    for (available = Serial1.available(); available--;) {
      memmove(&frame, ((uint8_t *)&frame) + 1, sizeof(frame) - 1);
      frame.bytes[4] = Serial1.read();
      if (frame.magic == 0xa3) {
        uint8_t checksum = frame.channel + frame.magic + ((frame.value & 0xff00) >> 8) + (frame.value & 0x00ff) & 0xff;
        if (frame.checksum == checksum) {
          values[frame.channel] = be16toh(frame.value);
          
          client.write((byte*)&frame, sizeof(frame));
          memset(&frame, 0, sizeof(frame));
        }
      }
    }
  }
}

void loop() {
  TS_Point p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  tft.setCursor(100, 175);
  channels[page].render();

  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    page++;
    if (page >= 35) {
      page = 1;
    }
    render_page();
  }
}
