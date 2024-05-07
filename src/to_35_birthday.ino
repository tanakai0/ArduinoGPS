// 35歳の誕生日の12:00までの秒をLCDに表示するプログラム。
// 構造体で受信データを整理して格納
// 2040年以降はUTC年の関係で、うまく働かないかも
// 現在時刻を表示する関数も一応作ってある。
// unsigned long 最大値約42億

// 改良できる点:構造体のupdateを毎回行うのではなくて、10秒は自前のタイマーを利用する。
// 改良できる点:LCDの表示も、毎回clearするのではなくて、必要な箇所だけ変える。

#include<SoftwareSerial.h>
#include<LiquidCrystal.h>
#define RS_PIN 10
#define RW_PIN 9
#define E_PIN 8
#define DB4_PIN 7
#define DB5_PIN 6
#define DB6_PIN 5
#define DB7_PIN 4

SoftwareSerial GPS(14,15);  // RX,TX
LiquidCrystal lcd(RS_PIN, RW_PIN, E_PIN, DB4_PIN, DB5_PIN, DB6_PIN, DB7_PIN);

// gpsで取得した各情報の構造体
// yearについては、いつか使用が変わるかも?  (年は19「97」-20「40」)
struct gps_info{
  char N_or_S;  // north or south
  float lat;  // latitude  [度]([分]は変える。) 
  char E_or_W;  // East or West
  float lon;  // longitude  [度]([分]は変える。) 
  boolean valid;  // valid(1) or invalid(0)
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int number_of_satellites;  //  使用衛星数
  float geoid;  // [m]
  float v;  // [km/hour]
};

void initialize_gps_info(struct gps_info *c){
  c->N_or_S = 'N';
  c->lat = 0.0;
  c->E_or_W='E';
  c->lon=0.0;
  c->valid=0;  // invalid
  c->year=0;
  c->month=0;
  c->day=0;
  c->hour=0;
  c->minute=0;
  c->second=0;
  c->number_of_satellites=0;
  c->geoid=0.0;  // [m]
  c->v=0.0;  // [km/hour]
}

// 受け取ったメッセージによるgps_infoの構造体の更新(時刻は日本の標準時に変える。)
// year は +2000している。
void update_gps_info(struct gps_info *c,char input[]){
  // メッセージの開始記号$で始まっていなかったら終了
  if (input[0]!= '$'){
    return;
  }
  String message = input;
  // GPRMCの時
  int loc = message.indexOf("$GPRMC");
  if (loc >= 0){
    // loc = 0にする
    message = message.substring(loc,message.length());
    c->hour = (message.substring(7,9).toInt()+9)%24;
    c->minute = message.substring(9,11).toInt();
    c->second = message.substring(11,13).toInt();
    //緯度・経度が取得できているかどうか
    if (message[loc+17]!='A') {
      c->valid = 0;
    }
    else{
      c->valid = 1;
      c->N_or_S = message[30];
      c->lat = message.substring(19,21).toInt() + message.substring(21,29).toFloat()/60;
      c->E_or_W = message[44];
      c->lon = message.substring(32,35).toInt() + message.substring(35,43).toFloat()/60;
      int last = message.length()-1; //ヌル文字を除いた文字列の長さ
      c->year = message.substring(last-9,last-7).toInt() + 2000;
      c->month = message.substring(last-11,last-9).toInt();
      c->day = message.substring(last-13,last-11).toInt();
      // UTC日付から日本の日付に変える message.substring(7,9).toInt()+9 > 23なら日付繰り上がり。
      if (message.substring(7,9).toInt()+9 > 23 ){
        change_days(c);
      }
    }
    return;
  }
  
  // GPGGAの時
  loc = message.indexOf("$GPGGA");
  if (loc >= 0) {
    // 位置補足ができているとき
    if (c->valid == 1){
      c->number_of_satellites = message.substring(loc+46,loc+48).toInt();
      c->geoid = message.substring(loc+54,loc+58).toFloat();
    }
    return;
  }
  
  // GPVTGの時
  loc = message.indexOf("$GPVTG");
  if (loc >= 0) {
    // 位置補足ができているとき
    if (c->valid == 1){
      int loc2 = message.indexOf('K');
      c->v = message.substring(loc2-6,loc2-1).toFloat();
    }
  }
}

// UTC日付の繰り上げ yearは下2桁に注意 2000 + 下2桁(year)で計算することとする。
// 2040年以降はうまく働かないかも？
void change_days(struct gps_info *c){
  // day
  int day_limit = 0;
  if (c->month==2){
    // leap year
    if ( ( c->year % 4 == 0 && c->year % 100!=0 ) || c->year % 400 == 0) {
      day_limit = 29;
    }
    // not leap year
    else{
      day_limit = 28;
    }
  }
  else if (c->month==4 || c->month==6 || c->month==9 || c->month==11) {
    day_limit = 30;
  }
  else{
    day_limit = 31;
  }
  if (c->day == day_limit){
    c->day = 1;
    c->month += 1;
    }
  else{
    c->day += 1;
  }

  //month&year
  if (c->month == 13){
    c->month = 1;
    c->year += 1;
  }
}

//　gps_infoがうまくできているかの確認用
void kakunin_gps_info(struct gps_info *c){
  Serial.println("------------------------------");
  Serial.println(c->N_or_S);
  Serial.println(c->lat);
  Serial.println(c->E_or_W);
  Serial.println(c->lon);
  Serial.println(c->valid);
  Serial.println(c->year);
  Serial.println(c->month);
  Serial.println(c->day);
  Serial.println(c->hour);
  Serial.println(c->minute);
  Serial.println(c->second);
  Serial.println(c->number_of_satellites);
  Serial.println(c->geoid);
  Serial.println(c->v);
}

/*
// 現在時刻を表示する関数
void print_now(struct gps_info *c){
  String sentence = "";
  if(c->hour < 10){
    sentence.concat('0');
  }
  sentence += String(c->hour) + ':';
  if (c->minute < 10) {
    sentence.concat('0');
  }
  sentence += String(c->minute) + ':';
  if (c->second < 10) {
    sentence.concat('0');
  }
  sentence += String(c->second);
  lcd.clear();
  lcd.print(sentence);
}
*/

int month_day[13] = {0,0,31,59,90,120,151,181,212,243,273,304,334}; // month_day[i] = 1月1日からi月1日までの日数
// 目標年日時までの残り秒数を表示する関数
void print_when(struct gps_info *c,int month_day[],int year,int month,int day,int hour,int minute,int second){
  // 現在の日づけの00:00:00から現時刻までの秒数x
  unsigned long  x = (c->hour)*3600 + (c->minute)*60 + c->second;
  // 目標時点の日づけの00:00:00から目標時刻までの秒数y
  unsigned long y = hour*3600 + minute*60 + second;
  // 現在の日づけの00:00:00から目標時点の日づけの00:00:00までの秒数z
  // まず、何日分day_totalを計算
  // 現在の年の1/1から現在日までの日数day_x
  int day_x = month_day[c->month] + c->day - 1 ;
  // 目標時点の年の1/1から目標日までの日数day_y
  int day_y = month_day[month] + day -1;
  // 現在の年の1/1から目標時点の年の1/1までの日数day_z
  int day_z = (year - c->year) * 365;
  // うるう日の加算
  for(int i=c->year;i<year;i++){
    if ( ( i % 4 == 0 && i % 100!=0 ) || i % 400 == 0) {
      day_z ++;
    }
  }
  int day_total = day_z - day_x + day_y;
  if(day_total < 0){
    lcd.clear();
    lcd.print("Time over...");
    return;
  }
  else if(day_total == 0){
    if(x-y >= 0){
      lcd.clear();
      lcd.print("Time over...");
    }
  }
  else{
    unsigned long z = day_total * 86400;
    // 目的の秒数 remain_second
    unsigned long remain_second = z - x + y;
    String sentence = String(remain_second);
    sentence += 's';
    lcd.clear();
    lcd.print(sentence);
  }
}
  
struct gps_info gps_info,*gps_info_p = &gps_info;
unsigned long tm = 0;  // LCDの表示間隔を1秒にするための変数。おそらくオーバーフローしてもうまく動き続けるかも。
unsigned long tm_old = 0;  // 上と合わせて使う。

void setup() {
  Serial.begin(9600);
  while(!Serial){
    ;
  }
  GPS.begin(9600);
  initialize_gps_info(gps_info_p);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Hello!");
  delay(1000);
  lcd.clear();
}

void loop() {
  char gps_message[128]; // gpsから受け取るNMEA形式のデータ1メッセージ（ヌル文字含めて128文字以下）
  int i=0;
  // NMEAの1つのメッセージは改行で終わる。そのため、改行が来たら、もしくは128文字目であれば、メッセージ受け取り完了とする。
  do {
    if (GPS.available()){
      // gps_messageのi番目に1byteつまりASCIIの1文字を格納する。
      gps_message[i] = GPS.read();
      i++;
    }
    if (i > 127) {
      break;
    }
  }while(gps_message[i-1] != '\n');
  gps_message[i-1] = '\0';
  update_gps_info(gps_info_p,gps_message);
  // kakunin_gps_info(gps_info_p);
  tm = millis();
  if(tm_old + 1000 < tm){
    tm_old = tm;
    print_when(gps_info_p,month_day,2032,6,6,12,0,0);
  }
  
}
