# 2018CPF_WisdomHouse

## 此空間為2018生活智慧「乘雲駕物」全國雲教授創意教材設計競賽 參賽之作品"Wisdom House"的作品資料相關檔案內容
[此專案網頁 litttlechin.github.io/2018CPF_WisdomHouse](https://litttlechin.github.io/2018CPF_WisdomHouse/)
## 包含： Arduino完整程式碼

```
其餘資料請參考：https://drive.google.com/drive/folders/1GSY-UShCJWh5c2Uh6Y0BbHS5mOVITrsB
1.作品說明書pdf檔、docx檔
2.作品中設計的app資料(App使用Kodular製作)aia原始檔、apk安裝檔
3.運作範例影片：https://youtu.be/POMS-dYBSxo
```

```
以上資料中的相關金鑰(Firebase網路API金鑰...)皆已模糊化可能無法直接運行，需套用自己的金鑰
如有任何問題請聯繫：littlechintw@gmail.com、jyunwei364@gmail.com
```

## 需要修改的數據
### CPF_C_RFID/CPF_C_RFID.ino
```
const int cardnum = {欲註冊卡片數量};
cardid(1, 0, 0, 0, 0); //卡片ID-1
cardid(2, 0, 0, 0, 0); //卡片ID-2
cardid(3, 0, 0, 0, 0); //卡片ID-3
```
### CPF_NodeMCU/CPF_NodeMCU.ino
```
#define FIREBASE_HOST "xxxxxx-xxxxx.firebaseio.com" //資料庫連結
#define FIREBASE_AUTH "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" //資料庫api
#define WIFI_SSID "xxxx" //wifi名稱
#define WIFI_PASSWORD "xxxxxxxx" //wifi密碼
```

### 備註
```
telegramBotNodemcu此資料夾內檔案為測試檔案
```
