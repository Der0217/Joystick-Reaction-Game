#include <LiquidCrystal.h>
#include <Keypad.h>
#include <stdlib.h>
#include <time.h> 

//LCD
LiquidCrystal lcd(49, 48, 47, 43, 42, 41);
//蜂鳴器
int BUZZER=A9;
//***搖桿
int joyPinX=A1; //X軸接腳
int joyPinY=A2; //Y軸接腳
int SW=A3;
int value=0;
int xzero=0;
int yzero=0;
int SWstate=0;
//***

//***可變電阻
int VR_Pin=A0;
int VR_Value=0;
//***
enum ACTION{
  //搖桿控制
  Up_A,
  Right_A,
  Down_A,
  Left_A,

};
enum MODE{
  attract, //待機模式
  choose,  //選擇難度模式
  working, //運作模式
  end      //結束模式
};


//***鍵盤配置
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
    {'/', 'E', 'D', 'C'},
    {'B', '3', '6', '9'},
    {'A', '2', '5', '8'},
    {'+', '1', '4', '7'}
};
byte rowPins[ROWS] = {25, 24, 23, 22};
byte colPins[COLS] = {29, 28, 27, 26};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
//***

//***遊戲變數設置
int score=0; //成績
int playState=0; //遊玩狀態
int life=8; //生命
int skip; //模式跳過秒數
bool Go=true; //一次性使用，用於遊戲模式一開始設定LCD
unsigned long startTime=0; // 計時開始的時間
const unsigned long timeLimit = 60000; // 60秒，以毫秒為單位
unsigned long lastUpdateTime = 0; // 上次更新的時間
int queRand[150]; //亂數放置陣列
int view=0; //控制亂數陣列
unsigned long joyDelay = 0; //搖桿延遲
unsigned long clockStartTime=0; //時間開始時間
bool bee=true;  //蜂鳴器開關
bool joyD=true; //搖桿延遲(僅限第一次)
bool hidden=false;//隱藏模式開關
bool joyLock=false;//控制搖桿使用完必須回到中心才能繼續判斷
//***
void setup() {

  pinMode(SW,INPUT_PULLUP); //SW設為提升輸入模式
  
  yzero=analogRead(joyPinY);//讀取搖桿一開始X軸(0,0)值
  xzero=analogRead(joyPinX);//讀取搖桿一開始Y軸(0,0)值
  lcd.begin(32, 2);
  lcd.setCursor(0, 0);
  lcd.print(" JOYSTICK GAMES");
  lcd.setCursor(0, 1);
  lcd.print(" *PRESS E START");
  playState=attract;

  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  pinMode(A15,OUTPUT);
  digitalWrite(A15,LOW);
  randomSeed(analogRead(A0));
  for(int i=0;i<150;i++)
    queRand[i]=random(4);
  Serial.begin(9600);
  pinMode(BUZZER,OUTPUT);
}

void loop() {
  switch(playState){
    case attract:
      Game_Attract();
      break;
    case choose:
      Game_Choose();
      break;
    case working:
      Mode_working();
      break;
    case end:
      Game_End();
      break;
  }
}
  //待機模式，按下E 開始選擇模式
void Game_Attract(){
  char customKey = customKeypad.getKey();
    if(customKey=='E'){
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("  Choose Modes C");
     lcd.setCursor(0, 1);
     lcd.print("A Normal  B Hard");
     playState=choose;
    }
}
  //選擇模式，A->Normal B->Hard
void Game_Choose(){
  char customKey = customKeypad.getKey();
      if(customKey=='A'){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Mode: Normal");
        playState=working;
        skip=5;
      }else if(customKey=='B'){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Mode: Hard");
        playState=working;
        skip=3;
      }else if(customKey=='C'){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Mode: Hidden");
        playState=working;
        hidden=true;
      }
}
//結束模式
void Game_End(){
  char customKey = customKeypad.getKey();
    if(customKey=='E'){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Choose Modes C");
      lcd.setCursor(0, 1);
      lcd.print("A Normal  B Hard");
      clearSet();
    }
}
//遊戲模式
void Mode_working(){
  if(Go){
    lcd.clear();
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("  Ready...");
    delay(2000);
    lcd.setCursor(10, 0);
    lcd.print("Go!");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Score:");
    lcd.setCursor(6, 1);
    lcd.print(score);
    lcd.setCursor(9, 1);
    lcd.print("Time:");
    Go=false;

    startTime = millis();
    clockStartTime=millis();

    for(int i=2;i<10;i++){
      pinMode(i,OUTPUT);
      analogWrite(i,200);
    }
    showQToLcd(queRand[0]);
  }
  if(hidden){
    skip=analogRead(VR_Pin)/100;
    if(skip<1)
      skip=1;
    Serial.print(skip);
    Serial.print("  ");
  }
  unsigned long nowTime = millis();
  unsigned long clockTime =millis();
  handleJoyInput(nowTime);
  countdown_60s(clockTime);
}
//搖桿輸入判斷正錯
void handleJoyInput(unsigned long nowTime){
  if (nowTime - startTime >= skip * 1000 || (nowTime - startTime < skip * 1000 && joyCheck(queRand[view],nowTime))) {
    if(nowTime - startTime >= skip * 1000)
      errorLife();
      view++;
      if(life!=0)    
        showQToLcd(queRand[view]);
      startTime = millis();                                                                  
  }
}
//扣命方法  
void errorLife(){
   life--;
    switch(life){
      case 0:
        digitalWrite(2,HIGH);
        showEnd();        
        playState=end;
        break;
      default:
        digitalWrite(life+2,HIGH);    
        break;
    }
}

//搖桿判定
int joyCheck(int randValue,unsigned long nowTime){
  int valueX,valueY,number=-1;
  bool ch=false;
  bool sh=true;
  //搖桿
  if(joyD){
    delay(500);
    joyD=false;
  }
  if(nowTime-joyDelay>=200){
    joyDelay=nowTime;
    if(sh){
      valueX=analogRead(joyPinX)-xzero;//讀取移動後X軸值
      valueY=analogRead(joyPinY)-yzero;//讀取移動後Y軸值
      sh=false;    
    }
    if(valueX<-300 && valueY<550 && valueY>-550 && !joyLock){
      //up 
      number=Up_A;
      ch=true;
      joyLock=true;
    }else if(valueX>300 && valueY<550 && valueY>-550 && !joyLock){
      //down
      number=Down_A;
      ch=true;
      joyLock=true;
    }else if(valueY>300 && valueX<550 &&valueX>-550 && !joyLock){
      //left
      number=Left_A;
      ch=true;
      joyLock=true;
    }else if(valueY<-300 && valueX<550 &&valueX>-550 && !joyLock){
      //right
      number=Right_A;
      ch=true;
      joyLock=true;
    }else{
      //原點情況
      joyLock=false;
    }

    if(ch){
      if(number==randValue){
        score++;
        lcd.setCursor(6, 1);
        lcd.print(score);
      }else{
        errorLife();
      }
      return 1;
    }else
      return 0;
  }else
    return 0;
  
}
//LCD 倒數60秒
void countdown_60s(unsigned long clockTime) {
  unsigned long elapsedTime = clockTime - clockStartTime; // 計算已經過去的時間
    // 在時間限制內，執行倒數60秒的程式碼
    unsigned long remainingTime = (timeLimit - elapsedTime) / 1000; // 換算成秒
    // 檢查是否已經過了一秒
    if (clockTime - lastUpdateTime >= 1000) {
      if(remainingTime<10){
        lcd.setCursor(15, 1);
        lcd.print(" ");
      }
      lcd.setCursor(14, 1);
      lcd.print(remainingTime);
      lastUpdateTime = clockTime; // 更新上次更新時間
    }

    if (remainingTime ==0) {
      showEnd();      
      playState = end;
    }
}

//清除設定
void clearSet(){
  startTime = 0;
  lastUpdateTime = 0;
  score=0;
  life=8;
  Go=true;
  skip=-1;
  playState=choose;
  bee=true;
  joyD=true;
  hidden=false;
  joyLock=false;
}

//顯示題目
void showQToLcd(int randValue){
  lcd.setCursor(0, 0);
  Serial.print(randValue);
  switch(randValue){
    case 0:
      lcd.print("       UP       ");
      break;
    case 1:
      lcd.print("     Right      ");
      break;
    case 2:
      lcd.print("      Down      ");
      break;
    case 3:
      lcd.print("      Left      ");
      break;
  }
}
void showEnd(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    Score     ");
  lcd.setCursor(10, 0);
  lcd.print(score);
  lcd.setCursor(0, 1);
  lcd.print("*PRESS E RESTART");
  //蜂鳴器發出叫聲
  digitalWrite(BUZZER,HIGH);
  delay(500);
  digitalWrite(BUZZER,LOW);

}

