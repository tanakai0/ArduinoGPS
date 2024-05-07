#include <LiquidCrystal.h>;
#define RS_PIN 10
#define RW_PIN 9
#define E_PIN 8
#define DB4_PIN 7
#define DB5_PIN 6
#define DB6_PIN 5
#define DB7_PIN 4

LiquidCrystal lcd(RS_PIN, RW_PIN, E_PIN, DB4_PIN, DB5_PIN, DB6_PIN, DB7_PIN);
byte smiley[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};
int smiley_num=0;

void setup() {
  lcd.begin(16, 2);  //表示領域を指定 lcd.begin(列数,行数)
  lcd.print("hello, world!");
  lcd.createChar(smiley_num, smiley);
  delay(3000);
  lcd.clear(); // 左上に戻りつつ、画面を消す。 lcd.home()はカーソルを左上に戻すだけ。
}

void loop() {
  // set the cursor to (列数,行数):
  lcd.setCursor(0, 0);
  // print from 0 to 9:
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    lcd.print(thisChar);
    delay(500);
  }
  lcd.write(smiley_num); // writeはbyteデータに対して
  delay(500);

  // set the cursor to (16,1):
  lcd.setCursor(16, 1);
  // set the display to automatically scroll:
  lcd.autoscroll();  //新たな文字を出力すると表示位置にある文字が押し出される。
  // print from 0 to 9:
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    lcd.print(thisChar);
    delay(500);
  }
  // turn off automatic scrolling
  lcd.noAutoscroll();  // autoscroll機能の停止

  // clear screen for the next loop:
  lcd.clear();
  delay(5000);
}
