//garage.h  車庫函式 ver:0918.a
/*
  setup時使用需呼叫 garageSetup(PIN_TRIG, PIN_ECHO, PIN_LEDR, PIN_LEDG)
  loop時需呼叫 garage(PIN_TRIG, PIN_ECHO, PIN_LEDR, PIN_LEDG, garageMin, garageMax)
  PIN_TRIG, PIN_ECHO 為HC-SR04的trig和echo腳位
  PIN_LEDR, PIN_LEDG 為車庫紅燈和綠燈的腳位(腳位需pwm)
  garageMin, garageMax 為車庫警戒值與安全值(單位公分)

*/
int garagePing(int PIN_TRIG, int PIN_ECHO) {//車庫測距 回傳公分
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  return pulseIn(PIN_ECHO, HIGH)/58;
}

void garageSetup(int PIN_TRIG, int PIN_ECHO, int PIN_LEDR, int PIN_LEDG) {
//初始化腳位"主程式setup"需要呼叫這個
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LEDR, OUTPUT);
  pinMode(PIN_LEDG, OUTPUT);
  digitalWrite(PIN_LEDG, LOW);
  digitalWrite(PIN_LEDR, LOW);
}

void setLED(int PIN_LEDR, int PIN_LEDG, int distance, int garageMin, int garageMax){//判斷距離並閃燈
  if (distance > garageMax) {//大於安全值
    digitalWrite(PIN_LEDG, HIGH);
    delay(1);
  }else if(distance < garageMin){//小於警戒值
    digitalWrite(PIN_LEDG, LOW);
    digitalWrite(PIN_LEDR, HIGH);
  }else{
    analogWrite(PIN_LEDG, map(distance, garageMax, garageMin, 255, 0));
	  digitalWrite(PIN_LEDR, HIGH);
    delay(map(distance, garageMax, garageMin, 20, 0)*10 + 15);
    digitalWrite(PIN_LEDR, LOW);
    delay(map(distance, garageMax, garageMin, 20, 0)*10 + 15);
  }
}

void setLED(int PIN_LEDR,int value){//長亮宏燈
  digitalWrite(PIN_LEDR, value);
}

int garage(int PIN_TRIG, int PIN_ECHO, int PIN_LEDR, int PIN_LEDG, int garageMin, int garageMax) {//"主程式loop"呼叫用 閃燈加回傳數值
  //garageMin 距離警戒值
  //garageMax 距離安全值
  int distance = garagePing(PIN_TRIG, PIN_ECHO);
  setLED(PIN_LEDR, PIN_LEDG, distance, garageMin, garageMax);
  delay(10);
  return distance;
}
