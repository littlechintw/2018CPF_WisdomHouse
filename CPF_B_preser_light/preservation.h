//preservation.h

void preser_Setup(int PIN_btn, int PIN_alarm, int PIN_sensor, int PIN_led){
	pinMode(PIN_btn,INPUT);
	pinMode(PIN_sensor,INPUT);
	pinMode(PIN_alarm,OUTPUT);
  pinMode(PIN_led,OUTPUT);

}

int preser_alarm_check(int statue, int PIN_sensor, int setting_value){//回傳是否警報
	if(statue){
		int a=digitalRead(PIN_sensor);
		if(a==setting_value){
			return 0;
		}else{
			return 1;
		}
	}else{
	  return 0;
	}

}

int preser_btn_setting(int PIN_btn){//回傳鎖定鍵是否被按digitalRead(PIN_btn);
	if(digitalRead(PIN_btn)==1){
		return 1;
	}else{
		return 0;
	}
}
