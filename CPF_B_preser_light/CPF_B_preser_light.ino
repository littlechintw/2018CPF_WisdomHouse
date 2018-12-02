/*
 * 最後更新10.10
 */


/*函示庫宣告
*/
#include "motionLight.h"
#include "setLight.h"
#include "preservation.h"
#include <string.h>
#include <time.h>
#include <SoftwareSerial.h>

/*組態設定
 */
int Light_Value=111;//2開燈1關燈 第一燈為最後一碼
int Lingh_inf[]={3,A0,A1,A2};//共n+1碼 第1碼為燈數量
unsigned long  Garden_light_inf[]={3,2,0};//花園燈資訊pin,sensor,lastMoveTime
unsigned long  Entrance_light_inf[]={5,4,0};//玄關燈資訊pin,sensor,lastMoveTime
int Preser_inf[]={10,11,12,13};//btn,alarm_beep,sensor,led

SoftwareSerial worknet(8, 9); // RX, TX

/*
  變數設定
 */
String str;
int Preser_statue=0;
int Preser_alarm=0;
int Preser_sensor_log;

void debugPrint(String d){
  Serial.println(d);
}

void setup() {
  Serial.begin(9600);
  worknet.begin(9600);
  debugPrint("Debug MODE");
  LightSetup(Lingh_inf);
  motionLightSetup(Garden_light_inf[0]);
  motionLightSetup(Entrance_light_inf[0]);
  preser_Setup(Preser_inf[0], Preser_inf[1], Preser_inf[2], Preser_inf[3]);
}

void loop() {
  check_preser();

  Garden_light_inf[2]=checkMotion(0, Garden_light_inf[1], Garden_light_inf[2], millis());   //設定lastMoveTime
  Entrance_light_inf[2]=checkMotion(0, Entrance_light_inf[1], Entrance_light_inf[2], millis());   //設定lastMoveTime
  motionLight(Garden_light_inf[0], Garden_light_inf[2], millis(), 10000, 255, 0);
  motionLight(Entrance_light_inf[0], Entrance_light_inf[2], millis(), 10000, 255, 0);

  if (Serial.available()) {
    str=Serial.readStringUntil('\n');
  }if (worknet.available()) {
    str=worknet.readStringUntil('\n');
  }
  if(str==""){return;}
  debugPrint("Get string "+str);
  check_light();
  check_alarm();
  check_statue();

  str="";
}

void check_preser(){//保全檢查
  if(preser_btn_setting(Preser_inf[0])==1 & Preser_statue==0){//檢查按鈕 若按下則設定保全
    preser_on_set();
    worknet.println("statue:1");
    debugPrint("btn Push ststue on");
  }if(preser_alarm_check(Preser_statue, Preser_inf[2], Preser_sensor_log)==1){//若異常則警報
    Preser_alarm=1;
    Preser_statue=0;
    worknet.println("alarm:1");
    debugPrint("alarm");
  }digitalWrite(Preser_inf[1],Preser_alarm);//設定beeper
  digitalWrite(Preser_inf[3],Preser_statue);//設定statue LED
}

void preser_on_set(){//啟用保全並紀錄sensor值_done
  Preser_statue = 1;
  Preser_sensor_log = digitalRead(Preser_inf[2]);
  debugPrint("start perser and log");
}

void check_light(){//檢查是否接收到燈光資訊_done
  if(str.indexOf("light")!=-1){//
    str.replace("light:","");
    LightSet(Lingh_inf,str.toInt());
    debugPrint("set light");
  }else{
    return;//沒接收到則跳出
  }
}

void motion_light(unsigned long inf[]){//自控燈設定_done
  inf[2]=checkMotion(1,inf[1],inf[2], millis());
  motionLight(inf[0], inf[2], millis(), 10000, 255, 30);
}

void check_alarm(){//檢查是否接收到警報解除資訊_done
  if(str.indexOf("alarm")!=-1){//解除警報
    Preser_statue = 0;
    Preser_alarm  = 0;
    worknet.println("statue:0");
    worknet.println("alarm:0");
    debugPrint("statue and alarm off");
  }else{
    return;//沒接收到則跳出
  }
}

void check_statue(){//檢查是否接收到保全資訊(開或關)_done
  if(str.indexOf("statue")!=-1){
    str.replace("statue:","");
    if(str.toInt()==1 & Preser_statue==0){
      preser_on_set();
      debugPrint("statue and alarm on by online");
    }else if(str.toInt()==0){//解除
      Preser_statue = 0;
      Preser_alarm  = 0;
      debugPrint("statue and alarm off");
    }
  }else{
    return;//沒接收到則跳出
  }
}
