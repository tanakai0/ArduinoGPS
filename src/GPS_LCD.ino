// GPS& LCD
// example of LCD output
//   11:17:58
//   N43.12345  <-> E141.12345   (switch per some interval)
//

#include<SoftwareSerial.h>
#include<LiquidCrystal.h>
#define RS_PIN 10
#define RW_PIN 9
#define E_PIN 8
#define DB4_PIN 7
#define DB5_PIN 6
#define DB6_PIN 5
#define DB7_PIN 4

SoftwareSerial GPS(14,15);  //RX,TX
LiquidCrystal lcd(RS_PIN, RW_PIN, E_PIN, DB4_PIN, DB5_PIN, DB6_PIN, DB7_PIN);
char gps_input[128];
char gps_time_jp_output[9];  // ex. 12:33:04 + '\0'
char gps_lat_output[13];  // ex. N:41.1234567 + '\0'
char gps_lon_output[14];  // ex. E:141.1234567 + '\0'
String gps_value;
String gps_time_jp;
String gps_lat;
String gps_lon;
unsigned long tm = 0;
unsigned long tm_old = 0;
int i = 0;
int lat_lon_flag = 0; // switch print(latitude) or print(longtitude) per lat_lon_interval seconds
int lat_lon_interval = 3;
int lat_lon_interval_2 = 2*lat_lon_interval;

void setup() {
  GPS.begin(9600);
  Serial.begin(9600);
  lcd.begin(16, 2);  //表示領域を指定
  lcd.print("hello, world!");
  delay(1000);
  lcd.clear();
}

void loop() {
  if (GPS.available()){
    gps_input[i] = GPS.read();
    if(i > 127 || gps_input[i] == '\n'){
      gps_input[i] = '\0';
      i = 0;
      gps_value = gps_input;
      // search $GPRMC
      int loc = gps_value.indexOf("$GPRMC");
      if(loc >= 0){
        // get hour,minute,second
        gps_value = gps_value.substring(loc+7,loc+13);
        // change UTC to Japan time and make ??:??:?? + '\0'
        gps_time_jp = ((gps_value.substring(0,2).toInt()+9)%24);
        if(gps_time_jp.length() != 2){
          gps_time_jp = " " + gps_time_jp;
        }
        gps_time_jp.concat(":");
        gps_time_jp.concat(gps_value.substring(2,4));
        gps_time_jp.concat(":");
        gps_time_jp.concat(gps_value.substring(4,6));
        gps_time_jp.toCharArray(gps_time_jp_output,9);
        gps_time_jp_output[8] = '\0';
      }

      //get latitude,longtitude & DMM -> DEG
      gps_value = gps_input;
      if(gps_value[17]=='A'){  // this flag represents which it got latitude & longtitude

        // latitude
        gps_lat = gps_value[30];
        gps_lat.concat(gps_value.substring(19,21));
        gps_lat.concat(".");
        String temp = gps_value.substring(21,23) + gps_value.substring(24,29);
        int count = 0;  // the number of 0
        for(int j=0;j<temp.length();j++){
          if(temp[j]=='0'){
            count++;
          }
          else{
            break;
          }
        }
        temp = (temp.toInt()/60);
        for(int j=0;j<count;j++){
          gps_lat.concat("0");
        }
        gps_lat.concat(temp);
        gps_lat.toCharArray(gps_lat_output,13);
        gps_lat_output[12] = '\0';
        
        //longtitude
        gps_lon = gps_value[44];
        gps_lon.concat(gps_value.substring(32,35));
        gps_lon.concat(".");
        temp = gps_value.substring(35,37) + gps_value.substring(38,43);
        count = 0;  // the number of 0
        for(int j=0;j<temp.length();j++){
          if(temp[j]=='0'){
            count++;
          }
          else{
            break;
            }
        }
        temp = (temp.toInt()/60);
        for(int j=0;j<count;j++){
          gps_lon.concat("0");
        }
        gps_lon.concat(temp);
        gps_lon.toCharArray(gps_lon_output,14);
        gps_lon_output[13] = '\0';
      }
    }
    else{
      i++;
    }
  }

  if(gps_time_jp.length() == 0){  // while searching GPS
    lcd.setCursor(0, 0);
    lcd.print("GPS searching...");
  }
  else{
    tm = millis();
    if(tm_old + 1000 < tm){
      tm_old = tm;
      lcd.clear();
      lcd.print(gps_time_jp_output);
      if(gps_lat.length() != 0){
        lcd.setCursor(0,1);
        if(lat_lon_flag < lat_lon_interval){
          lcd.print(gps_lat_output);
          lat_lon_flag += 1;
        }
        else{
          lcd.print(gps_lon_output);
          lat_lon_flag += 1;
          lat_lon_flag %= lat_lon_interval_2;
        }
      }
    }
  }
}
