/* 最後更新 11.11.01.46
 * 1007加入"網路時間"by Jyunwei
 * 1008加入firebase事件監聽器
 * 1009重要結論：serial物件"請勿"至於函式傳入值中,否則buffer會溢出
 * firebase語法https://firebase-arduino.readthedocs.io/en/latest/
 * 1021更新-傳送空氣資訊時的格式
  上傳到firebase的值不可有"/" "," " " ":"
*/
//#include <WiFiClientSecure.h>
//#include <UniversalTelegramBot.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>  //https://github.com/FirebaseExtended/firebase-arduino
#include <TimeLib.h>          //https://github.com/PaulStoffregen/Time
#include <WiFiUdp.h>
#include <string.h>

#define FIREBASE_HOST "xxxxxx-xxxxx.firebaseio.com"
#define FIREBASE_AUTH "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define WIFI_SSID "xxxx"
#define WIFI_PASSWORD "xxxxxxxx"
#define NTP_PACKET_SIZE 48  //緩存區大小 訊息的前48個字節為NTP時間戳記
#define update_delay 70     //每計數一次之等待秒數
#define ntp_correct 2       //NTP延遲校正(單位s)

SoftwareSerial worknet_A(D1, D2);   // RX, TX RFID
SoftwareSerial worknet_B(D5, D6);   // RX, TX SECURITY
SoftwareSerial worknet_C(D7, D8);   // RX, TX WEATHER
IPAddress ntpServerIP;      //宣告一個IPAddress物件來存放NTP伺服器的IP
WiFiUDP udp;                //宣告一個UDP物件來傳送和接收封包

String logtext="";                  //Serial接收到之訊息
String disaster="";
unsigned long GMT = 2208988800;     //接收到之時間
byte packetBuffer[NTP_PACKET_SIZE]; //宣告緩存區
const char* ntpServerName="time.nist.gov";//NTP 伺服器名稱

void setup() {
  pinMode(D4, OUTPUT);//D0
  digitalWrite(D4, 0);
  Serial.begin(115200);//9600容易lose
  worknet_A.begin(9600);
  worknet_B.begin(9600);
  worknet_C.begin(9600);
  connecttonet();   //初始化連線
  getTimeAndSet();  //獲取網路時間(可能需要十多秒)
  digitalWrite(D4, 1);
}

void loop() {

  Serial_check();   //接收訊息
  Connect_check();  //檢查連線狀態(Wifi & Firebase)
  check_statue();   //檢查是否接收到"啟動保全"的訊息
  check_alarm();    //檢查是否接收到"啟動警報"的訊息
  check_card();     //檢查是否接收到讀卡資訊
  check_air();      //檢查是否接收到空氣監測資訊
  check_getNTP();   //檢查是否接收到"重新獲取網路時間"的訊息
  logtext="";       //清空
  disaster_check(); //災害檢查
  Firebase_check(); //檢查並傳送Firebase訊息(當Firebase事件發生時)
  delay(10);
}

void disaster_check(){//讀取digital災害偵測資訊
  int list[2][4]={{D3,D3,D3,D3},{991,992,993,994}};
  String new_disaster="abnormal"; //預設輸出為"有異常"
  for(int i=0;i<=3;i++){
    pinMode(list[0][i],INPUT);
    if(digitalRead(list[0][i])==1){
      new_disaster+=list[1][i];
    }
  }if (new_disaster.indexOf("99") <0) { //如果沒有任何異常則輸出"正常"
    new_disaster.replace("abnormal","normal");
  }
  if(!(new_disaster.equals(disaster))){//若相同則不上傳以節省流量
    disaster.remove(0);  //清空disaster
    disaster+=new_disaster;
    Firebase.setString("disaster", nowTime() + "@" + disaster);
    Serial.println("Disaster Put:"+disaster);
  }
}

void connecttonet(){     //初始化連線
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nconnected: ");
  Serial.println(WiFi.localIP());
  // connect to firebase.
  Serial.println("Starting Firebase...");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.stream("");  //事件監聽器
  //start UDP
  Serial.println("Starting UDP...");
  udp.begin(2390);    //本地使用埠2390監聽UDP數據包
}

void Connect_check(){    //檢查連線狀態(Wifi & Firebase)
  while(WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi error");
    connecttonet();
  }if (Firebase.failed()) {
    Serial.println("firebase error");
    Serial.println(Firebase.error());
    connecttonet();
  }
}

void getAndSentLight(){  //抓取firebase的light值並傳給UNO
  String r=Firebase.getString("light");
  worknet_B.println("light:" + r);
  worknet_B.flush();
  Serial.println("sent to worknet_B->light:" + r);
}

void getAndSentStatue(){ //抓取firebase的statue值並傳給UNO
  String r=Firebase.getString("preservation_statue");
  worknet_B.println("statue:" + r);
  worknet_B.flush();
  Serial.println("sent to worknet_B->statue:" + r);
}

void Serial_check(){     //檢查並接收serial訊息
  //避免一次多個訊息傳入時重複接收 因此用if else ,並使用readStringUntil('\n')
  if(Serial.available() > 0){//debug use
    logtext = Serial.readStringUntil('\n');
    delay(100);
    Serial.println("HardwareSerial got message:"+logtext);
  }else if(worknet_A.available() > 0){
    logtext = worknet_A.readStringUntil('\n');
    worknet_A.flush();
    delay(100);
    Serial.println("SoftwareSerial_A got message:"+logtext);
  }else if(worknet_B.available() > 0){
    logtext = worknet_B.readStringUntil('\n');
    worknet_B.flush();
    delay(100);
    Serial.println("SoftwareSerial_B got message:"+logtext);
  }else if(worknet_C.available() > 0){
    logtext = worknet_C.readStringUntil('\n');
    worknet_C.flush();
    delay(100);
    Serial.println("SoftwareSerial_C got message:"+logtext);
  }
}

void Firebase_check(){   //檢查並傳送Firebase訊息(當Firebase事件發生時)
  while(Firebase.available()>0){
    FirebaseObject event = Firebase.readEvent();
    String eventType = event.getString("type");
    eventType.toLowerCase();
    getAndSentStatue();
    delay(1000);
    getAndSentLight();
    delay(10);
  }
}

void check_statue(){     //檢查是否接收到"啟動保全"的訊息
  if(logtext.indexOf("statue:1")!=-1){
    Firebase.setString("preservation_statue", "1");
    Serial.println("set firebase_preservation_statue:1");
  }
}

void check_alarm(){      //檢查是否接收到"啟動警報"的訊息
  if(logtext.indexOf("alarm:1")!=-1){
    Firebase.setString("preservation_alarm", "1");
    Serial.println("set firebase_preservation_alarm:1");
  }
}

void check_getNTP(){     //檢查是否接收到"重新獲取網路時間"的訊息
  if(logtext.indexOf("getNTP")!=-1){
    getTimeAndSet();
  }
}

void check_card(){       //檢查是否接收到讀卡資訊
  if(logtext.indexOf("card")!=-1){
    logtext.replace("card:","");
    Firebase.setString("log", nowTime() + "@"+ logtext);
    Serial.println("set firebase_log:"+ nowTime() + "@" + logtext);
  }
  if(logtext.indexOf("TRUE")>0){
    Firebase.setString("preservation_statue", "0");
    Firebase.setString("preservation_alarm", "0");
    Serial.println("set firebase_preservation_statue:0");
    worknet_B.println("statue:0");
    worknet_B.flush();
  }
}

void check_air(){        //檢查是否接收到空氣監測資訊
  if(logtext.indexOf("air")!=-1){
    logtext.replace("air:","");
    Firebase.setString("air", "time-"+ nowTime() + "@" + logtext);
    Serial.println("set firebase_air:time:"+ nowTime() + "@" + logtext);
  }
}

String nowTime(){        //回傳時間 格式:yyyy.mm.dd_hh.mm.ss
  String s = (String)year() + "." + (String)month()  + "." + (String)day();
  s = s + "_"+ (String)hour() + "." + (String)minute() + "." + (String)second();
  return s;
}

void getTimeAndSet(){    //從網路獲取時間並更新內部時鐘
  Serial.print("getting NTP");
  do{
    Serial.print(".");
    GMT=getUnixTime();
    Firebase_check();//檢查firebase是否有事件發生 避免因取得時間而造成用戶命令延遲執行的情況
  }while(GMT==0);
  GMT=GMT + 28800 + ntp_correct;    //GMT+8及延遲校正
  Serial.println("get NTP done");
  setTime(GMT);   //以NTP時間更新內部時鐘
}

unsigned long getUnixTime() {//取得NTP時間
  WiFi.hostByName(ntpServerName, ntpServerIP);  //獲取一個隨機的伺服器
  memset(packetBuffer, 0, NTP_PACKET_SIZE);//將緩存區每個byte設為0(清理緩衝區)
  //初始化請求所需的值
  packetBuffer[0]=0b11100011;   // LI, Version, Mode
  packetBuffer[1]=0;            // Stratum, or type of clock
  packetBuffer[2]=6;            // Polling Interval
  packetBuffer[3]=0xEC;         // Peer Clock Precision
  //8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]=49;
  packetBuffer[13]=0x4E;
  packetBuffer[14]=49;
  packetBuffer[15]=52;
  udp.beginPacket(ntpServerIP, 123);          //埠123為網路時間協定通訊埠
  udp.write(packetBuffer, NTP_PACKET_SIZE);   //向NTP伺服器發送請求
  udp.endPacket();
  //NTP packet END
  delay(1000);                  //等待
  int cb=udp.parsePacket();     //回傳接收到之位元數
  unsigned long unix_time=0;    //預設傳回 0, 表示未收到 NTP 回應
  if (cb>0) {
    udp.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long highWord=word(packetBuffer[40], packetBuffer[41]);//將封包讀入緩存區 時間戳從接收到的封包的字節40開始
    unsigned long lowWord=word(packetBuffer[42], packetBuffer[43]); //是四個字節或兩個字長
    unsigned long secsSince1900=highWord << 16 | lowWord;           //將四個字節組合成一個長整數 NTP時間（1900年1月1日以來的秒數）
    unix_time=secsSince1900 - 2208988800UL;                         //更新 unix_time(減掉70年的秒數 即1970年1月1日以來的秒數)
  }
  return unix_time;   //回傳1970年1月1日以來的秒數
}
