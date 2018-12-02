#include <SPI.h>
#include <MFRC522.h>     // rfid函式庫
#include <SoftwareSerial.h>
#include <Servo.h>
Servo myservo;
#include "garage.h"  //車庫用韓式庫
#define RST_PIN A0        // 讀卡機的重置腳位
#define SS_PIN 10        // 晶片選擇腳位
MFRC522 mfrc522(SS_PIN, RST_PIN);  // 建立MFRC522物件
#define PIN_TRIG A3
#define PIN_ECHO A4
#define PIN_LEDR A2
#define PIN_LEDG A1
#define garageMin 7
#define garageMax 18
const int beeper = 2;
const int green = 5;
const int red = 6;
const int cardnum = 3; //註冊卡片數量
int card[cardnum][4];
/*
   rfid接腳
   SDA  = 10
   SCK  = 13
   MOSI = 11
   MISO = 12
*/
const int worknet_rx = 8;
const int worknet_tx = 9;
unsigned long lastTime = -4999;

SoftwareSerial worknet(worknet_rx, worknet_tx); // RX, TX

void setup() {
  myservo.attach(A5);
  myservo.write(45);
  // put your setup code here, to run once:
  Serial.begin(9600);
  worknet.begin(9600);
  Serial.println("RFID reader is ready!");
  SPI.begin();
  mfrc522.PCD_Init();   // 初始化MFRC522讀卡機模組
  pinMode(beeper, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  analogWrite(green, 0);
  analogWrite(red, 0);

  cardid(1, 0, 0, 0, 0); //卡片ID-1
  cardid(2, 0, 0, 0, 0); //卡片ID-2
  cardid(3, 0, 0, 0, 0); //卡片ID-3

  garageSetup(PIN_TRIG, PIN_ECHO, PIN_LEDR, PIN_LEDG);
}

void loop() {
  rfiduid();
  garage(PIN_TRIG, PIN_ECHO, PIN_LEDR, PIN_LEDG, garageMin, garageMax);
  if(millis()-lastTime>0&&millis()-lastTime<5000){
    myservo.write(135);
  }
  else{
    myservo.write(45);
  }
}

void rfiduid(){ //測試卡片是否正確
  String cmmd;
  // 確認是否有新卡片
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    setLED(PIN_LEDR,1);
    byte *id = mfrc522.uid.uidByte;   // 取得卡片的UID
    byte idSize = mfrc522.uid.size;   // 取得UID的長度

    Serial.print("PICC type: ");      // 顯示卡片類型
    // 根據卡片回應的SAK值（mfrc522.uid.sak）判斷卡片類型
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    Serial.print("UID Size: ");       // 顯示卡片的UID長度值
    Serial.println(idSize);

    for (byte i = 0; i < idSize; i++) {  // 逐一顯示UID碼
      Serial.print("id[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.println(id[i]);       // 以16進位顯示UID值
    }
    mfrc522.PICC_HaltA();  // 讓卡片進入停止模式
    compare(id[0], id[1], id[2], id[3]);
    Serial.println();
    setLED(PIN_LEDR,0);
  }
}

void cardid(int num , int id_0 , int id_1 , int id_2 , int id_3) { //登入卡號
  card[num - 1][0] = id_0;
  card[num - 1][1] = id_1;
  card[num - 1][2] = id_2;
  card[num - 1][3] = id_3;
}

void compare(int id_0 , int id_1 , int id_2 , int id_3) { //配對卡號
  for (int i = 0; i < cardnum; i++){
    if (id_0 == card[i][0] && id_1 == card[i][1] && id_2 == card[i][2] && id_3 == card[i][3]){
      final(1,id_0,id_1,id_2,id_3);
      return 0;
    }
  }
  final(0,id_0,id_1,id_2,id_3);
}

void final(int r,int id_0 , int id_1 , int id_2 , int id_3) { //查詢後動作
  String text;
  if (r == 1) //卡片正確動作
  {
    text = "card:@TRUE";
    analogWrite(green, 200);
    myservo.write(135);
    for (int i = 0; i < 80; i++)
    {
      digitalWrite(beeper, HIGH);
      delay(1);
      digitalWrite(beeper, LOW);
      delay(1);
    }
    lastTime = millis();
    delay(1000);
    analogWrite(green, 0);
  }
  else if (r == 0) //卡片錯誤動作
  {
    text = "card:@FALSE";
    analogWrite(red, 200);
    myservo.write(45);
    lastTime = millis() - 5001;
    Serial.println(String(millis()) + " * " + String(lastTime));
    for (int i = 0; i < 160; i++)
    {
      digitalWrite(beeper, HIGH);
      delay(2);
      digitalWrite(beeper, LOW);
      delay(2);
    }
    analogWrite(red, 0);
  }
  text += "@ID-" + String(id_0) + "-" + String(id_1) + "-"  + String(id_2) + "-"  + String(id_3);
  worknet.print(text);
}
