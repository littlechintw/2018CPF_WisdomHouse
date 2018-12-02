//PM2.5
#include <SoftwareSerial.h>
//LCD
#include "SPI.h"
#include "function.h"
#include "TFT_22_ILI9225.h"

//PM2.5
#define SOFT_TX 2 //out
#define SOFT_RX 3 //要插的 in
// [ PMSx003xx TX ] to [ Arduino PIN 11 ]

//LCD
#define TFT_RST 8
#define TFT_RS  9
#define TFT_CS  10  // SS
#define TFT_SDI 11  // MOSI
#define TFT_CLK 13  // SCK
#define TFT_LED 4   // 0 if wired to +5V directly

#include "time.h"

SoftwareSerial mySerial(SOFT_RX, SOFT_TX); // RX, TX
unsigned char gucRxBuf[100];

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED);
// Use software SPI (slower)
//TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED);

// Variables and constants
uint16_t x, y;
boolean flag = false;

unsigned int pmcf10, pmcf25, pmcf100; /* CF=1根據美國TSI公司的儀器校準 */
unsigned int pmat10, pmat25, pmat100; /* 大氣環境下根據中國氣象局的數據校準 */
unsigned int pmcount03, pmcount05, pmcount10, pmcount25;
float temp, humi;
float thi_n;

int last_aqistate = 0;
int aqicolor = 0;

unsigned long now_time = 0;
unsigned long last_time = 0;

void setup() {
  tft.begin();
  tft.setOrientation(2);//畫面倒轉
  Serial.begin(9600);
  mySerial.begin(9600);
  delay(1000);
  lcdsetup();
  packandsend();
  last_time=millis();
}

void loop() {
  TeHuAndStart();
  aqicolor = aqitool(pmcf25, pmcf100);
  float Td = (humi / 5) - 20 + temp;
  thi_n = THI(temp, Td);
  lcdprint();
  if(last_aqistate != aqicolor){
    todrows(aqicolor);
  }
  last_aqistate = aqicolor;
  now_time=millis();
  Serial.print("last:" + String(last_time));
  Serial.println(" now:" + String(now_time));
  if(now_time-last_time >= (60000)){ //1000*秒數
    packandsend();
    last_time=millis();
  }
}
/*
空氣質量指數（AQI） 健康令人擔憂的程度  顏色
  ---------------------------------------------
  0   to 50        好                綠色
  51  to 100       中等              黃色
  101 to 150       不適於敏感人群     橘色
  151 to 200       不健康            紅色
  201 to 300       非常不健康        紫色
  301 to 500       危險              棗紅色
*/

void lcdsetup() {
  tft.setFont(Terminal12x16);

  tft.drawText(10, 10, "PM1.0", COLOR_CYAN);
  tft.drawText(70, 10, ":", COLOR_CYAN);
  tft.drawText(100, 10, "NaN", COLOR_CYAN);
  tft.drawText(10, 30, "PM2.5", COLOR_CYAN);
  tft.drawText(70, 30, ":", COLOR_CYAN);
  tft.drawText(100, 30, "NaN", COLOR_CYAN);
  tft.drawText(10, 50, "PM10", COLOR_CYAN);
  tft.drawText(70, 50, ":", COLOR_CYAN);
  tft.drawText(100, 50, "NaN", COLOR_CYAN);

  tft.drawText(10, 70, "Temp", COLOR_YELLOWGREEN);
  tft.drawText(70, 70, ":", COLOR_YELLOWGREEN);
  tft.drawText(100, 70, "NaN", COLOR_YELLOWGREEN);
  tft.drawText(12, 90, "Humi", COLOR_YELLOWGREEN);
  tft.drawText(70, 90, ":", COLOR_YELLOWGREEN);
  tft.drawText(100, 90, "NaN", COLOR_YELLOWGREEN);

  tft.drawText(10, 110, "THI", COLOR_GOLD);
  tft.drawText(70, 110, ":", COLOR_GOLD);
  tft.drawText(100, 110, "NaN", COLOR_GOLD);
  tft.drawText(10, 130, "NaN", COLOR_GOLD);
}
void lcdprint() {
  /*int color, int pmcf10, int pmcf25, int pmcf100, int pmat10, int pmat25, int pmat100, int pmcount03, int pmcount05, int pmcount10, int pmcount25, float temp, float humi*/
  tft.setFont(Terminal12x16);

  tft.drawText(80, 10, (String)(pmcf10) + "    ", COLOR_CYAN);
  tft.drawText(80, 30, (String)(pmcf25) + "    ", COLOR_CYAN);
  tft.drawText(80, 50, (String)(pmcf100) + "    ", COLOR_CYAN);

  tft.drawText(80, 70, (String)(temp) + " C", COLOR_YELLOWGREEN);
  tft.drawText(80, 90, (String)(humi) + " %", COLOR_YELLOWGREEN);

  tft.drawText(80, 110, String(thi_n) + "   ", COLOR_GOLD);
  /*
  氣溫(℃)  10以下    11～15   16～19    20～26    27～30    31以上
  舒適度    非常寒冷  寒　冷   稍有寒意  舒　適    悶　熱    易中暑
   */
   if(thi_n < 11){
    tft.drawText(10, 130, "Very Cold", COLOR_GOLD);
   }
   else if(thi_n < 16){
    tft.drawText(10, 130, "Cold     ", COLOR_GOLD);
   }
   else if(thi_n < 20){
    tft.drawText(10, 130, "Cool     ", COLOR_GOLD);
   }
   else if(thi_n < 27){
    tft.drawText(10, 130, "Well     ", COLOR_GOLD);
   }
   else if(thi_n < 31){
    tft.drawText(10, 130, "Hot      ", COLOR_GOLD);
   }
   else if(thi_n >= 31){
    tft.drawText(10, 130, "Very Hot ", COLOR_GOLD);
   }
}

void todrows(int color) {
  if(color == 1){
    tft.drawText(12, 150, "AQI           ", COLOR_GREEN);
    tft.drawText(70, 150, ":", COLOR_GREEN);
    tft.drawText(80, 150, "Green   ", COLOR_GREEN);
    tft.drawText(10, 170, "Low                  ", COLOR_GREEN);
    tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_GREEN);
    tft.drawRectangle(1, 1, tft.maxX() - 2, tft.maxY() - 2, COLOR_GREEN);
    tft.drawRectangle(2, 2, tft.maxX() - 3, tft.maxY() - 3, COLOR_GREEN);
    tft.drawRectangle(3, 3, tft.maxX() - 4, tft.maxY() - 4, COLOR_GREEN);
    tft.drawRectangle(4, 4, tft.maxX() - 5, tft.maxY() - 5, COLOR_GREEN);
  }
  else if(color == 2){
    tft.drawText(12, 150, "AQI           ", COLOR_YELLOW);
    tft.drawText(70, 150, ":", COLOR_YELLOW);
    tft.drawText(80, 150, "Yellow   ", COLOR_YELLOW);
    tft.drawText(10, 170, "Medium               ", COLOR_YELLOW);
    tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_YELLOW);
    tft.drawRectangle(1, 1, tft.maxX() - 2, tft.maxY() - 2, COLOR_YELLOW);
    tft.drawRectangle(2, 2, tft.maxX() - 3, tft.maxY() - 3, COLOR_YELLOW);
    tft.drawRectangle(3, 3, tft.maxX() - 4, tft.maxY() - 4, COLOR_YELLOW);
    tft.drawRectangle(4, 4, tft.maxX() - 5, tft.maxY() - 5, COLOR_YELLOW);
  }
  else if(color == 3){
    tft.drawText(12, 150, "AQI           ", COLOR_ORANGE);
    tft.drawText(70, 150, ":", COLOR_ORANGE);
    tft.drawText(80, 150, "Orange   ", COLOR_ORANGE);
    tft.drawText(10, 170, "More than Medium     ", COLOR_ORANGE);
    tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_ORANGE);
    tft.drawRectangle(1, 1, tft.maxX() - 2, tft.maxY() - 2, COLOR_ORANGE);
    tft.drawRectangle(2, 2, tft.maxX() - 3, tft.maxY() - 3, COLOR_ORANGE);
    tft.drawRectangle(3, 3, tft.maxX() - 4, tft.maxY() - 4, COLOR_ORANGE);
    tft.drawRectangle(4, 4, tft.maxX() - 5, tft.maxY() - 5, COLOR_ORANGE);
  }
  else if(color == 4){
    tft.drawText(12, 150, "AQI           ", COLOR_RED);
    tft.drawText(70, 150, ":", COLOR_RED);
    tft.drawText(80, 150, "Red      ", COLOR_RED);
    tft.drawText(10, 170, "High                 ", COLOR_RED);
    tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_RED);
    tft.drawRectangle(1, 1, tft.maxX() - 2, tft.maxY() - 2, COLOR_RED);
    tft.drawRectangle(2, 2, tft.maxX() - 3, tft.maxY() - 3, COLOR_RED);
    tft.drawRectangle(3, 3, tft.maxX() - 4, tft.maxY() - 4, COLOR_RED);
    tft.drawRectangle(4, 4, tft.maxX() - 5, tft.maxY() - 5, COLOR_RED);
  }
  else if(color == 5){
    tft.drawText(12, 150, "AQI           ", COLOR_VIOLET);
    tft.drawText(70, 150, ":", COLOR_VIOLET);
    tft.drawText(80, 150, "Purple   ", COLOR_VIOLET);
    tft.drawText(10, 170, "Very High            ", COLOR_VIOLET);
    tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_VIOLET);
    tft.drawRectangle(1, 1, tft.maxX() - 2, tft.maxY() - 2, COLOR_VIOLET);
    tft.drawRectangle(2, 2, tft.maxX() - 3, tft.maxY() - 3, COLOR_VIOLET);
    tft.drawRectangle(3, 3, tft.maxX() - 4, tft.maxY() - 4, COLOR_VIOLET);
    tft.drawRectangle(4, 4, tft.maxX() - 5, tft.maxY() - 5, COLOR_VIOLET);
  }
  else if(color == 6){
    tft.drawText(12, 150, "AQI           ", COLOR_BROWN);
    tft.drawText(70, 150, ":", COLOR_BROWN);
    tft.drawText(80, 150, "Brown    ", COLOR_BROWN);
    tft.drawText(10, 170, "Serious              ", COLOR_BROWN);
    tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_BROWN);
    tft.drawRectangle(1, 1, tft.maxX() - 2, tft.maxY() - 2, COLOR_BROWN);
    tft.drawRectangle(2, 2, tft.maxX() - 3, tft.maxY() - 3, COLOR_BROWN);
    tft.drawRectangle(3, 3, tft.maxX() - 4, tft.maxY() - 4, COLOR_BROWN);
    tft.drawRectangle(4, 4, tft.maxX() - 5, tft.maxY() - 5, COLOR_BROWN);
  }
  else{
    tft.drawText(10, 150, "PRINT ERROR!!          ", COLOR_WHITE);
    tft.drawText(10, 170, "color:" + String(color) + "                 ", COLOR_WHITE);
    tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_WHITE);
    tft.drawRectangle(1, 1, tft.maxX() - 2, tft.maxY() - 2, COLOR_WHITE);
    tft.drawRectangle(2, 2, tft.maxX() - 3, tft.maxY() - 3, COLOR_WHITE);
    tft.drawRectangle(3, 3, tft.maxX() - 4, tft.maxY() - 4, COLOR_WHITE);
    tft.drawRectangle(4, 4, tft.maxX() - 5, tft.maxY() - 5, COLOR_WHITE);
  }
}

void measure(){
  pmcf10 = gucRxBuf[4] * 256 + gucRxBuf[5];
  pmcf25 = gucRxBuf[6] * 256 + gucRxBuf[7];
  pmcf100 = gucRxBuf[8] * 256 + gucRxBuf[9];
  pmat10 = gucRxBuf[10] * 256 + gucRxBuf[11];
  pmat25 = gucRxBuf[12] * 256 + gucRxBuf[13];
  pmat100 = gucRxBuf[14] * 256 + gucRxBuf[15];
  pmcount03 = gucRxBuf[16] * 256 + gucRxBuf[17];
  pmcount05 = gucRxBuf[18] * 256 + gucRxBuf[19];
  pmcount10 = gucRxBuf[20] * 256 + gucRxBuf[21];
  pmcount25 = gucRxBuf[22] * 256 + gucRxBuf[23];
  temp = (float)(gucRxBuf[24] * 256 + gucRxBuf[25]) / 10;
  humi = (float)(gucRxBuf[26] * 256 + gucRxBuf[27]) / 10;
}

void goto_print(void) {
  Serial.println("========= PMS5003T =========");
  Serial.print("PM1.0_CF1:" + String(pmcf10) + "   ");
  Serial.print("PM2.5_CF1:" + String(pmcf25) + "   ");
  Serial.println("PM10_CF1:" + String(pmcf100));
  Serial.print("PM1.0_AT:" + String(pmat10) + "   ");
  Serial.print("PM2.5_AT:" + String(pmat25) + "   ");
  Serial.println("PM10_AT:" + String(pmat100));
  Serial.println("PMcount0.3:" + String(pmcount03));
  Serial.println("PMcount0.5:" + String(pmcount05));
  Serial.println("PMcount1.0:" + String(pmcount10));
  Serial.println("PMcount2.5:" + String(pmcount25));
  Serial.println("Temperature:" + String(temp) + " C");
  Serial.println("Humidity:" + String(humi) + " %");
  Serial.println("Version:" + String(gucRxBuf[28]));
  Serial.println("Error Code:" + String(gucRxBuf[29]));
}

void TeHuAndStart() {
  if (mySerial.available()) {
    delay(50);
    unsigned char i = 0;
    while (mySerial.available()) {
      gucRxBuf[i] = mySerial.read();
      i++;
    }
    if (((gucRxBuf[0] == 0x42) && (gucRxBuf[1] == 0x4d)) && (i >= 12)) {
      int chksum = 0;
      unsigned char DTL = (gucRxBuf[2] << 8) + gucRxBuf[3];
      for (unsigned char i = 0; i < (DTL + 2); i++) {
        chksum += gucRxBuf[i];
      }
      unsigned char csH = (chksum >> 8);
      unsigned char csL = (chksum & 0xFF);
      if ((csH == gucRxBuf[DTL + 2]) && (csL == gucRxBuf[DTL + 3])) {
        measure();
        //goto_print();
      }
      else {
        mySerial.flush();
      }
    }
    else {
      mySerial.flush();
    }
  }
}

void packandsend(){
  //hum tem pm2.5 pm10 aqi
  String sendtext = "air:hum-" + String(humi);
  sendtext += "@tem-" + String(temp);
  sendtext += "@pm2.5-" + String(pmcf25);
  sendtext += "@pm10-" + String(pmcf100);
  if(aqicolor == 1){
    sendtext += "@aqi-Low";
  }
  else if(aqicolor == 2){
    sendtext += "@aqi-Medium";
  }
  else if(aqicolor == 3){
    sendtext += "@aqi-More_than_Medium";
  }
  else if(aqicolor == 4){
    sendtext += "@aqi-High";
  }
  else if(aqicolor == 5){
    sendtext += "@aqi-Very_High";
  }
  else if(aqicolor == 6){
    sendtext += "@aqi-Serious";
  }
  else{
    sendtext += "@aqi-SENSOR_ERROR";
  }
  sendtext += "@body-" + String(thi_n);
  mySerial.print(sendtext);
  Serial.println(sendtext);
}
