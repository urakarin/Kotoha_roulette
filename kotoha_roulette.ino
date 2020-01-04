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

#define START_SW_STS()    (digitalRead(START_SW_PIN))
#define START_SW_ON()     (digitalRead(START_SW_PIN) == false)
#define START_SW_OFF()    (digitalRead(START_SW_PIN) == true)

// LEDを点灯する
void pulse(const int index, const int duration) {
  const int pin = ROULETTE_PINS[index];
  digitalWrite(pin, HIGH);
  delay(duration);
  digitalWrite(pin, LOW);  

  bool start_sw = START_SW_STS();
  LOG(index);
  LOG(" ");
  LOG(start_sw);
  LOG(" ");
  LOGLN(duration);
}

// 停止した時の処理
void finish(const int index) {
  const int pin = ROULETTE_PINS[index];

  LOG("Finish ");
  LOGLN(index);
  for (int i = 0; i < 10; i++) {
    pulse(index, 100); // 100msec間隔で点滅させる
    delay(100);
  }
  
  LOGLN("ON");
  digitalWrite(pin, HIGH); // 点灯させる
  // スタートスイッチが押されるまで待機
  LOGLN("Wait");
  for (;;) {
    if (START_SW_ON()) {
      break;
    }
    delay(20);
  }
  LOGLN("Restart");
  delay(1000);
  digitalWrite(pin, LOW); // 消灯させる
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
  // put your main code here, to run repeatedly:
  int duration = DURATION_INIT;
  int delta = 0;  // 減速度(msec)
  int index = 0;  // ルーレット番号インデックス 0 ... 5
  bool breaking = false;  // 停止に向かって減速中ならtrue

  for(;;) {
    // 点灯→消灯
    pulse(index, duration);

    // スイッチの状態を確認する。押されていたら減速を開始する
    if (START_SW_ON() && !breaking) {
      LOGLN("stop");
      breaking = true;
    }
    if (breaking) {
      // 減速中なら減速度合いを更新する
      delta = 30 + random(40);  // 減速度合いはランダム 20 ... 39 msec
    }
    // 速度を更新する
    duration += delta;
    if (duration > 1000) {  // 1500msec以上になったら停止とみなす
      // 停止した時の処理
      finish(index);
      breaking = false;
      duration = DURATION_INIT;
      delta = 0;
    } 

    // LEDを次の番号へ進める
    index = (index + 1) % LEN_PINS;  
  }
}
