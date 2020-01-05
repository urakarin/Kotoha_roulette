#if 1
#define LOG_INIT()      Serial.begin(115200)
#define LOG(s)          Serial.print(s)
#define LOGLN(s)        Serial.println(s)
#else
#define LOG_INIT() 
#define LOG(s)     
#define LOGLN(s)   
#endif

const int DURATION_INIT = 40; // msec
const int ROULETTE_PINS[] = {8,9,10,11,12,13};
const int START_SW_PIN = 6;
#define LEN_PINS  (sizeof(ROULETTE_PINS)/sizeof(ROULETTE_PINS[0]))

#define START_SW_GET()    digitalRead(START_SW_PIN)
#define START_SW_IS_ON    false     // スタートスイッチが押されている状態
#define START_SW_IS_OFF   true      // スタートスイッチが離されている状態
bool gStartStatus = START_SW_IS_OFF;   // スタートスイッチの現在の状態
bool gPrevStartStatus = gStartStatus; // スタートスイッチのひとつ前の状態
#define START_SW_OFF2ON   (gPrevStartStatus == START_SW_IS_OFF && gStartStatus == START_SW_IS_ON )
#define START_SW_ON2OFF   (gPrevStartStatus == START_SW_IS_ON  && gStartStatus == START_SW_IS_OFF)


#define RSTS_RUN    0     // 素早く回っている状態
#define RSTS_BRK    1     // 減速中
#define RSTS_STP    2     // 停止中
int gRouletteStatus = RSTS_RUN;


#define LED_ON(index)    digitalWrite(ROULETTE_PINS[index], HIGH)
#define LED_OFF(index)   digitalWrite(ROULETTE_PINS[index], LOW)

void log_led(const int index, const int duration) {
  LOG(index);
  LOG(" ");
  LOG(gStartStatus);
  LOG(" ");
  LOGLN(duration);  
}

// LEDを点滅させる
void pulse(const int index, const int duration) {
  LED_ON(index);
  delay(duration);
  LED_OFF(index);
}


bool check_switch() {
  gPrevStartStatus = gStartStatus;
  gStartStatus = START_SW_GET();
  return gStartStatus;
}

// 停止した時の処理
void finish(const int index) {
  LOG("Finish ");
  LOGLN(index);
  for (int i = 0; i < 10; i++) {
    check_switch();
    if (START_SW_ON2OFF) {
      return;   // スタートスイッチが押されたら処理を中断して戻る
    }
    pulse(index, 100); // 100msec間隔で点滅させる
    delay(100);
  }
  
  LOGLN("ON");
  LED_ON(index);    // 点灯させる
  // スタートスイッチが押されるまで待機
  LOGLN("Wait");
  for (;;) {
    check_switch();
    if (START_SW_ON2OFF) {
      return;   // スタートスイッチが押されたら処理を中断して戻る
    }
    delay(20);
  }
}

// 前準備
void setup() {
  // 乱数の初期化
  randomSeed(analogRead(0));
  
  // LED用のPINをOUTPUTにして、OFFを出力する
  for (int i = 0; i < LEN_PINS; i++) {
    const int pin = ROULETTE_PINS[i];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  pinMode(START_SW_PIN, INPUT); // スタートツイッチは入力とする

  LOG_INIT();
}


// メインループ
void loop() {
  int duration = DURATION_INIT;
  int delta = 0;  // 減速度(msec)
  int index = 0;  // ルーレット番号インデックス 0 ... 5

  for(;;) {
    check_switch();

    switch(gRouletteStatus) {
    case RSTS_RUN:
      if (START_SW_ON2OFF) {
        // 押されていた状態から離された場合
        gRouletteStatus = RSTS_BRK;     // 減速を開始する
        LOGLN("Change state RUN->BRK");
      }
      break;
      
    case RSTS_BRK:
      if (START_SW_ON2OFF) {
        // 押されていた状態から離された場合
        gRouletteStatus = RSTS_RUN;     // 減速を開始する
        LOGLN("Change state BRK->RUN");
        duration = DURATION_INIT;
        delta = 0;
      } else {
        // 減速中なら減速度合いを更新する
        delta = 30 + random(40);  // 減速度合いはランダム 20 ... 39 msec
        duration += delta;        // 速度を更新する   
        if (duration > 1000) {    // これ以上になったら停止とみなす
          gRouletteStatus = RSTS_STP;     // 減速を開始する
          LOGLN("Change state BRK->STP");
        } 
        log_led(index, duration);
      }
      break;
      
    case RSTS_STP:
      // 停止した時の処理
      finish(index);
      LOGLN("Restart");
      gRouletteStatus = RSTS_RUN;     // 初めに戻る
      LOGLN("Change state    ->RUN");
      duration = DURATION_INIT;
      delta = 0;
      LED_OFF(index);   // 消灯させる
      break;
      
    default:
      // ここに来ることはありえない
      break;
    }
    if (gRouletteStatus != RSTS_STP) {
      pulse(index, duration);             // durationの時間だけindex番号のLEDを点灯させる
      index = (index + 1) % LEN_PINS;     // LEDを次の番号へ進める
    }
  }
}
