// Thêm thư viện có sẵn LCD
#include <LiquidCrystal.h>
 
//Khởi tạo với các chân
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
 
void setup() {
  //Khai báo LCD 16x2
  lcd.begin(16, 2);
  //In ra màn hình HocARM.org
  lcd.print("HocARM.org!");
}
 
void loop() {
  // đặt con trỏ vào cột 0, dòng 1
  lcd.setCursor(0, 1);
  // In ra dong chu Hello
  lcd.print(" Hello");
}
