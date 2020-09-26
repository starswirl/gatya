#include "TM1637.h"
#include <Servo.h>
  /**
   * TIPS:割り込み関数
   * attachInterrupt(int,fun,mode);
   * int: 割り込み番号
   *  UNOの場合の割込みPIN1は：0(D2ピン)、1(D3ピン)
   * fun: 割り込みする関数
   *  引数も戻り値もなしとする。
   * mode: 割り込み関数を実行するための条件
   *  HIGH：ピンの値がHIGHのとき
   *  LOW：ピンの値がLOWのとき
   *  RISING：ピンの値がHIGHからLOWに変化したとき
   *  FALLING：ピンの値がLOWからHIGHに変化したとき
   *  CHANGE：ピンの値が変化したとき
   */

/**
 * 定数
 */
// 7セグ表示ピン
#define DISP_CLK 2 // 黄色(CLK)
#define DISP_DIO 7 // DIOは標準入出力のこと

// サーボのピン番号
#define SERVO_PIN 9
// スイッチのピン
#define SW_PIN 6 // FIXME: 適当なPINに変える
// コインカウントピン...割り込み関数の為D3だが値は1
#define COIN_PIN 1 // D3

/**
 *  変数
 */
// スイッチの状態
int swStatus = 0;
// コインカウント用
unsigned int coinCount;
// デイスプレイ用配列
int8_t ListDisp[4];

// サーボのインスタンス
Servo servo;

// etc
TM1637 tm1637(DISP_CLK, DISP_DIO);

// チャタリング対策
volatile unsigned long time_prev = 0, time_now;
unsigned long time_chat = 40;

/**
 * メイン
 */
void loop() {
  //displayリセット
  changeDisplay();
  // スイッチの状態確認
  if(digitalRead(SW_PIN) == LOW)pushSw();
}

//---------スイッチ

/**
 * ボタンを押された際の処理
 */
void pushSw(){
  //coinが入ってるかチェック
  if(coinCount > 0){
    // コインカウントを減らす
    coinCount--;
    changeDisplay();
    // ガチャ起動
    gachaAct();
  }else{
    char err_msg[] = "Err";
    // 入ってなかったらエラーを知らせる
    tm1637.displayStr(err_msg);
    delay(2000);
    changeDisplay();
  }
}

//---------コイン（割り込み）

/**
 * コインが投入された際の処理
 */
void coinInsert(){
  time_now = millis(); //現在の割り込み時刻を取得
  if( time_now-time_prev > time_chat){
      coinCount++;
      // コインカウントの表示を変更
      changeDisplay();
  }
  time_prev = time_now; //現在の割り込み時刻を前回の割り込み時刻へコピー
}


//---------ディスプレイ表示

/**
 * ディスプレイ表示などの関数を制御
 */
void changeDisplay(){
  // 画面表示をcoinCountの値にする

  //10進数に変換して1文字ずつ配列に入れなければいけない
  // 1の位
  ListDisp[0] = coinCount % 10 > 0 ? ( coinCount % 10 ) : 0 ;
  // 10の位
  ListDisp[1] = ( coinCount / 10 ) % 10 > 0 ? ( coinCount / 10 ) % 10 : 0 ;
  // 100の位
  ListDisp[2] = ( coinCount / 100 ) % 10 > 0 ? ( coinCount / 100 ) % 10 : 0 ;
  // 1000の位
  ListDisp[3] = ( coinCount / 1000 ) % 10 > 0 ? ( coinCount / 1000 ) % 10 : 0 ;

  //画面出力
  tm1637.display(3, ListDisp[0]);
  tm1637.display(2, ListDisp[1]);
  tm1637.display(1, ListDisp[2]);
  tm1637.display(0, ListDisp[3]);
}

/**
 * セットアップ
 */
void setup() {
  // コインカウントの割り込みピンのプルアップ設定
  pinMode(3,INPUT_PULLUP);
    // コイン投入感知(割り込み設定)
  attachInterrupt(COIN_PIN,coinInsert,FALLING);
  //7セグ表示初期化
  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  // サーボ初期化
  servo.attach(SERVO_PIN);
  servo.write(0);
  // スイッチ入力受付
  pinMode(SW_PIN, INPUT_PULLUP);
  coinCount = 0;
}

//---------メイン系

/**
 *  ガチャガチャ起動
 */
void gachaAct(){
  // music
  // 回転制御
  spin();
  // コインカウントの表示を変更
  changeDisplay();
}

/**
 * 回転制御
 */
void spin(){
    servo.write(180);
  delay(2000);
  servo.write(0);
  delay(1000);
}
