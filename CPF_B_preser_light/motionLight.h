//motionLight.h  自控燈函式 ver:0923.a
/*
  setup時使用需呼叫gardenLightSetup(int PIN_LED)
	PIN_LED燈的腳位"需要PWM"
  loop時需呼叫 void gardenLight(PIN_LED, lastMoveTime, nowTime, LED_on_time, LED_on_value, LED_off_value)
    PIN_LED			LED腳位
	lastMoveTime	移動感應器的最後移動時間
	nowTime			現在時間
	LED_on_time		停止移動後持續亮燈的時間
	LED_on_value	LED開燈時亮度
	LED_off_value	LED待機時亮度

  checkMotion

  setup時使用需無須呼叫
  loop時需呼叫 checkMotion(byte MotionSingle, int motionPin, unsigned long lastMoveTime, unsigned long nowTime){
  MotionSingle 有移動時的訊號, motionPin 偵測腳位
  lastMoveTime 最後移動時間,   nowTime   現在時間
*/

void motionLightSetup(int PIN_LED){
  pinMode(PIN_LED, OUTPUT);
}

void motionLight(int PIN_LED, unsigned long lastMoveTime, unsigned long nowTime, unsigned long LED_on_time, byte LED_on_value, byte LED_off_value){
  unsigned long noMoveTime=nowTime-lastMoveTime;
  if(noMoveTime<=LED_on_time){
      analogWrite(PIN_LED,LED_on_value);
  }else if(noMoveTime<=(LED_on_time*1.1)){
      analogWrite(PIN_LED,map(noMoveTime, LED_on_value, LED_on_value*1.1, LED_off_value, LED_on_value));
  }else{
	  analogWrite(PIN_LED, LED_off_value);
  }
}
unsigned long checkMotion(byte MotionSingle, int motionPin, unsigned long lastMoveTime, unsigned long nowTime){
  //MotionSingle 有移動時的訊號, motionPin 偵測腳位,lastMoveTime 最後移動時間, nowTime現在時間
  byte Motion=digitalRead(motionPin);
  if(Motion==MotionSingle){//周遭有動靜，最後移動時間為現在時間
    return nowTime;
  }else{  //周遭無動靜，最後移動時間為現傳入的最後移動時間
    return lastMoveTime;
  }
}
