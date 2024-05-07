// GPSで取得できるデータの解析
// 2040年以降はUTC年の関係で、うまく働かないかも

#include<SoftwareSerial.h>

SoftwareSerial GPS(14,15);  // RX,TX

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
  
struct gps_info gps_info,*gps_info_p = &gps_info;

void setup() {
  Serial.begin(9600);
  while(!Serial){
    ;
  }
  GPS.begin(9600);
  initialize_gps_info(gps_info_p);
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
  kakunin_gps_info(gps_info_p);
}



/*
 * char gps_message[128]; // gpsから受け取るNMEA形式のデータ1メッセージ（ヌル文字含めて128文字以下）
int i = 0;
int n = 0;

void loop() {
  if (GPS.available()) {
    // gps_messageのi番目に1byteつまりASCIIの1文字を格納する。
    gps_message[i] = GPS.read();
    // NMEAの1つのメッセージは改行で終わる。そのため、改行が来たら、もしくは128文字目であれば、メッセージ受け取り完了とする。
    if (i >= 127 || gps_message[i] == '\n') {
      gps_message[i] = '\0';
      i = 0;
      Serial.println(n)
      Serial.println(gps_message);
    }
    else{
      i++;
    }
  }
}
*/
