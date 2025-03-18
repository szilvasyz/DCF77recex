#include <U8g2lib.h>
#include <dcfrec.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define DBG
#define BLINK
//#define INP 5
#define INP 16

DCFrec receiver(INP);
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/13, /* reset=*/ U8X8_PIN_NONE);  


void setup() {

  #ifdef BLINK
    receiver.begin(LED_BUILTIN);
  #else
    receiver.begin(PIN_NONE);
  #endif

  Serial.begin(115200);

  u8g2.begin();
  u8g2.setFont(u8g2_font_rosencrantz_nbp_tf);	// choose a suitable font
  u8g2.setFontPosTop();
}


static char buf[80];
const char* DWS[] = {"-0-", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

void loop() {

  DCFtime mytime;

  while (true) {

    receiver.getTime(&mytime);

    if (mytime.newtime) {
      sprintf(buf, "%s %02u-%02u-%02u %02u:%02u", DWS[mytime.dow], mytime.year, mytime.month, mytime.day, mytime.hour, mytime.minute);
      Serial.println(buf);

      u8g2.clearBuffer();					// clear the internal memory
      u8g2.home();
      u8g2.print(buf);	// write something to the internal memory
      u8g2.sendBuffer();					// transfer internal memory to the display

    }
  }
}
