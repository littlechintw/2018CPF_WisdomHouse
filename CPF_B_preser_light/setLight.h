//setLight.h
/*
*/
void LightSetup(int inf[]){
	for(int i =1;i<=inf[0];i++){//inf[0]為燈的數量
		pinMode(inf[i],OUTPUT);
	}
}

void LightSet(int inf[], int value){//設置燈value為連續n位數 2開燈 1關燈
	for(int i =1;i<=inf[0];i++){
		digitalWrite(inf[i],value%10-1);
    value/=10;
	}
}
