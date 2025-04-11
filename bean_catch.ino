//2025 04 13　豆つかみ用のプログラム

//----------------------------------------------
//ヘッダーファイルのインクルード
#include <Servo.h>
#include <Adafruit_GFX.h>         // Adafruitのグラフィックスライブラリ
#include <Adafruit_ILI9341.h>     // 液晶表示器 ILI9341 制御用ライブラリ
#include <Fonts/FreeSans18pt7b.h> //フォントを読み込み
#include <Fonts/FreeSans12pt7b.h>
//----------------------------------------------

//----------------------------------------------
//画面表示用の宣言
#define TFT_WIDTH  320  //画面幅
#define TFT_HEIGHT 240  //画面高さ

#define TFT_SCK  18 //液晶表示の　SCK
#define TFT_MOSI 19 //液晶表示の　MOSI
#define TFT_DC   20 //液晶表示の　DC
#define TFT_RST  21 //液晶表示の　RST
#define TFT_CS   22 //液晶表示の　CS

//ILI9341ディスプレイのインスタンスを作成
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI,TFT_DC,TFT_CS,TFT_RST);
//スプライト(メモリ描画領域から一括表示)をcanvasとして準備
//画面表示をtftではなくcanbvasで指定して一括表示させることでチラつきなく表示できる
GFXcanvas16 canvas(TFT_WIDTH,TFT_HEIGHT); //16bitカラースプライト(オフスクリーンバッファ)
//----------------------------------------------

//----------------------------------------------
//サーボクラスの宣言
Servo base_servo;
Servo arm1_servo;
Servo arm2_servo;
Servo hand_servo;
//----------------------------------------------

//----------------------------------------------
//ジョイスティックの入力ピン番号の宣言
const int X1_Pin  = 13;
const int Y1_Pin  = 12;
//const int SW1_Pin = ;
const int X2_Pin  = 15;
//const int Y1_Pin  = ;
const int SW2_Pin = 14;
//----------------------------------------------

//----------------------------------------------
//サーボの信号ピン番号の宣言
const int base_servo_Pin = 2;
const int arm1_servo_Pin = 3;
const int arm2_servo_Pin = 4;
const int hand_servo_Pin = 5;
//----------------------------------------------

//----------------------------------------------
//ジョイスティックのアナログ値の範囲
const int joy_Min = 0;
const int joy_Max = 1023;
//ジョイスティックのプッシュボタンの状態についての変数
int hand_servo_state;
//----------------------------------------------

//----------------------------------------------
//各アクチュエータの機械的な限界角度
const int base_angleMin      = 0;
const int base_angleMax      = 270;
const int arm1_angleMin      = 70;
const int arm1_angleMax      = 170;
const int arm2_angleMin      = 125;
const int arm2_angleMax      = 190;
const int hand_angle_catch   = 60;
const int hand_angle_release = 100;
//----------------------------------------------

//----------------------------------------------
/*ジョイスティックの各信号線の読み取り変数
各ジョイスティックの値を、サーボの角度範囲に置き換える

X1_Pinの信号  → base_servoへの信号
X2_Pinの信号  → arm1_servoへの信号
Y1_Pinの信号  → arm2_servoへの信号
SW2_Pinの信号 → hand_servoへの信号
*/
int base_servo_Pos;
int arm1_servo_Pos;
int arm2_servo_Pos;
int hand_servo_Pos;
int old_hand_servo_Pos = false;
//----------------------------------------------

//----------------------------------------------
//コントローラーでのスタートボタン、可動状態もしくは未稼働状態でのLEDでの出力など
//信号ピンの宣言
const int move_button = 11;
//LED１：未稼働状態　、LED２：稼働状態 を表す
//これらのLEDは、アームの横に設置する
const int led_Pin1    = 0;
const int led_Pin2    = 1;
//LED３: コントローラの起動ボタン
const int led_Pin3    = 10;

//スタートもしくはストップボタンはタクトスイッチで行う
//プルアップ抵抗に設定する
int val,old_val = false;
int move_state;

//制限時間の設定　とりあえず3分で設定している　下行のプログラムではミリ秒に直している
const int limit_time = 3 * 60 * 1000;
//----------------------------------------------

//core0-----------------------------------------
//ジョイスティックとサーボ関連
void setup() {
  //ジョイスティックのピンを入力モードにする
  pinMode(X1_Pin,INPUT);
  pinMode(Y1_Pin,INPUT);
  pinMode(SW1_Pin,INPUT_PULLUP);
  pinMode(X2_Pin,INPUT);
  //pinMode(Y2_Pin,INPUT);
  //pinMode(SW2_Pin,INPUT_PULLUP);

  //サーボを割り当てる
  base_servo.attach(base_servo_Pin);
  arm1_servo.attach(arm1_servo_Pin);
  arm2_servo.attach(arm2_servo_Pin);
  hand_servo.attach(hand_servo_Pin);

}

void loop() {
//----------------------------------------------
// アナログ値を読み取るり、サーボの角度範囲に書き換える
  //base_servo
  base_servo_Pos = map(analogRead(X1_Pin),joy_Min,joy_Max,base_angle_Min,base_angle_Max);
  base_servo_Pos = constrain(base_servo_Pos, base_angleMin, base_angleMax);
  //arm1_servo
  arm1_servo_Pos = map(analogRead(X2_Pin),joy_Min,joy_Max,arm1_angle_Min,arm1_angle_Max);
  arm1_servo_Pos = constrain(arm1_servo_Pos, arm1_angleMin, arm1_angleMax);
  //arm2_servo
  arm2_servo_Pos = map(analogRead(Y1_Pin),joy_Min,joy_Max,arm2_angle_Min,arm2_angle_Max);
  arm2_servo_Pos = constrain(arm2_servo_Pos, arm2_angleMin, arm2_angleMax);
  //hand_servo
  hand_servo_Pos = digitalRead(SW2_Pin);
//----------------------------------------------

//----------------------------------------------
//base_servo,arm1_servo,arm2_servoの角度を動かして、モータを回転させる
  base_servo.write(base_servo_Pos);
  arm1_servo.write(arm1_servo_Pos);
  arm2_servo.write(arm2_servo_Pos);
//----------------------------------------------

//----------------------------------------------
//豆をつかむときのハンドのプログラム
  if(hand_servo_Pos == true && old_hand_servo_Pos == false){
    hand_servo_state =  1 - hand_servo_state;
    delay(10);
  }

  if(hand_servo_state == 0){
    hand_servo.write(hand_angle_catch);
  }

  else {
    hand_servo.write(hand_angle_release);
  }

  //old_hand_servo_Posの更新
  old_hand_servo_Pos = hand_servo_Pos;
//----------------------------------------------
}

//core1----------------------------------------------
//豆をつかむ時間を表すタイマーの画面を表示する
void setup1(){
  //ピンの初期設定
  pinMode(move_button,INPUT_PULLUP);
  pinMode(led1_Pin,OUTPUT);
  pinMode(led2_Pin,OUTPUT);
  
  //SPIO初期設定
  SPI.setTX(TFT_MOSI);              //SPI0のTX(MOSI)
  SPI.setSCK(TFT_SCK);              //SPI0のSCK

  //TFT初期設定
  tft.begin();                      //TFTを初期化
  tft.setRotation(3);               //TFTの回転を設定
  canvas.setTextSize(1);            //テキストサイズを指定

  //未稼働状態なので、LED2を点灯させる
  digitalWrite(led1_Pin,HIGH);
  digitalWrite(led2_Pin,LOW);
  delay(500);

  //最初の文字表示
  canvas.setCursor(30,95);          //表座標指定
  canvas.setTextColor(0xFFFF);       //テキスト色
  canvas.setFont(&FreeSans18pt7b);   //フォント指定
  canvas.print("Are you ready?");        //表示内容
  canvas.fillScreen(0x0000);         //背景色
  tft.drawRGBBitmap(0,0,canvas.getBuffer(),TFT_WIDTH,TFT_HEIGHT); //スプライト(メモリ内に描画した画面)をTFTに描画

}

void loop1(){
  val = digitalRead(move_button);
  
  if(val == true && old_val == false){
    move_state = 1 - move_state;
    delay(10);
  }

  //move_buttonを押してから画面にタイマーが起動する
  if(move_state == 0){

    //リモコンのスタートボタンを押してからの経過時間カウント表示
    unsigned long current_time = millis(); //ミリ秒で計算している

    //稼働状態なので、LED2を点灯させる
    digitalWrite(led_Pin1,LOW);
    digitalWrite(led_Pin2,HIGH);
    //コントローラの起動灯の点灯
    digitalWrite(led_Pin3,HIGH);
    

    //スタート！の出力(１秒間)
    canvas.setFont(&FreeSans18pt7b);   //フォント指定
    canvas.setCursor(0,27);            //表示座標取得
    canvas.print("START!");
    tft.drawRGBBitmap(0,0,canvas.getBuffer(),TFT_WIDTH,TFT_HEIGHT);  //スプライト(メモリ内に描画した画面)をTFTに描画

    delay(2000);

    //経過時間の出力
    canvas.setFont(&FreeSans18pt7b);   //フォント指定
    canvas.setCursor(0,27);            //表示座標取得
    canvas.print("Time Limit:");
    canvas.println((limit_time - current_time)/1000);          //あと何秒かをs単位で表示
    tft.drawRGBBitmap(0,0,canvas.getBuffer(),TFT_WIDTH,TFT_HEIGHT);  //スプライト(メモリ内に描画した画面)をTFTに描画

    //終了！の出力
    if((limit_time - current_time)/1000 == 0){

      //未稼働状態なので、LED2を点灯させる
      digitalWrite(led_Pin1,HIGH);
      digitalWrite(led_Pin2,LOW);
      delay(500);

      canvas.setFont(&FreeSans18pt7b);   //フォント指定
      canvas.setCursor(0,27);            //表示座標取得
      canvas.print("Finish!");
      tft.drawRGBBitmap(0,0,canvas.getBuffer(),TFT_WIDTH,TFT_HEIGHT); //スプライト(メモリ内に描画した画面)をTFTに描画
    }
    
    //old_valの更新
    old_val = val;

  }
  
  //もう一度タクトスイッチを押すと、画面には、STOP!が二秒だけ出力されて、その後はAre you ready?に変わる
  else{
    //コントローラの起動灯の消灯
    digitalWrite(led3_Pin,LOW);

    //未稼働状態なので、LED2を点灯させる
    digitalWrite(led_Pin1,HIGH);
    digitalWrite(led_Pin2,LOW);
    delay(500);

    //STOP!を2秒だけ画面に表示
    canvas.setFont(&FreeSans18pt7b);   //フォント指定
    canvas.setCursor(0,27);            //表示座標取得
    canvas.print("STOP!");
    tft.drawRGBBitmap(0,0,canvas.getBuffer(),TFT_WIDTH,TFT_HEIGHT);  //スプライト(メモリ内に描画した画面)をTFTに描画

    delay(2000);

    //Are you ready?を画面に表示
    canvas.setFont(&FreeSans18pt7b);   //フォント指定
    canvas.setCursor(0,27);            //表示座標取得
    canvas.print("Are you ready?");
    tft.drawRGBBitmap(0,0,canvas.getBuffer(),TFT_WIDTH,TFT_HEIGHT);  //スプライト(メモリ内に描画した画面)をTFTに描画
  }
}