/* Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
  Thêm chức năng đọc cảm biến ánh sáng
  Thêm chức năng đọc cảm biến độ ẩm đất
  Thêm hiển thị LCD
  Thêm chức năng điều khiển tưới tiêu bằng tay 2 bơm
  Thêm chức năng điều khiển tự động bơm khi thiếu nước
  HocARM NDTR BOT for Arduino by hocARM.org
  -------------------------------------------------
// Kết nối
// DHT       | Arduino Uno
//---------------------------
// VCC(1)    |  5V
// DATA(2)   |  2
// NC(3)     |  x
// GND(4)    |  GND
// Nối trở 10k giữa chân 1 và chân 2
//----------------
//Cảm biến độ ẩm | Arduino
//--------------------------
// VCC(1)    	 |  5V
// GND(2)   	 |  GND
// D0(3)  	     |  x
// A0(4)    	 |  A1
// Cảm biến ánh sáng nối chân A0
//-------------------------------
// 2 nút nhấn sẽ được kết nối với chân A2 và A3
// 2 relay nối với chân 3 và chân 12
*/
#include "DHT.h"
//#include "homephone.h"
#include <LiquidCrystal.h>
#define DHTPIN 2     // Chân DATA nối với PD0
#define LDR_PIN A0    // Chân PE3 nối với chân OUT cảm biến as
#define SOIL_MOIST_1_PIN A1 // Chân PE4 nối với cảm biến độ ẩm
// Relay, nút nhấn
#define PUMPW_ON A2   //Nút có sẵn trên kit
#define PUMPW_PIN 3
#define PUMPS_ON A3   //Nút có sẵn trên kit
#define PUMPS_PIN 12
// Uncomment loại cảm biến bạn sử dụng, nếu DHT11 thì uncomment DHT11 và comment DHT22
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Biến lưu các giá trị cảm biến
int humDHT;
int tempDHT;

int lumen;
int DARK_LIGHT = 40;

int soilMoist;
int DRY_SOIL = 40;
int WET_SOIL = 60;

// Biến lưu trạng thái bơm
boolean pumpWaterStatus = 0;
boolean pumpPesStatus = 0;
int timePumpOn = 10; // Thời gian bật bơm nước
// Biến cho timer
long sampleTimingSeconds = 50; // ==> Thời gian đọc cảm biến (s)
long startTiming = 0;
long elapsedTime = 0;
// Khởi tạo cảm biến
DHT dht(DHTPIN, DHTTYPE);
// Khởi tạo LCD
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
void setup() {
  pinMode(PUMPW_PIN, OUTPUT);
  pinMode(PUMPS_PIN, OUTPUT);
  pinMode(PUMPW_ON, INPUT_PULLUP); // Button
  pinMode(PUMPS_ON, INPUT_PULLUP); // Button
  aplyCmd();
  // Khởi tạo cổng serial baud 115200
  Serial.begin(115200);
  Serial.println("HocARM NDTR Bot!");
  lcd.begin(16, 2);
  lcd.print("HocARM NDTR Bot!*");
  lcd.setCursor(0, 1);
  lcd.print("Xin chao ong chu");
  // Bắt đầu đọc dữ liệu
  readSensors(); // Khởi tạo đọc cảm biến
  startTiming = millis(); // Bắt đầu đếm thời gian
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Vui long cho ...");
}

void loop() {
  // Khởi tạo timer
  elapsedTime = millis() - startTiming;
  readLocalCmd();
  
  if (elapsedTime > (sampleTimingSeconds * 1000))
  {
    readSensors();
    printData();
    showDataLCD();
    autoControlPlantation();
    startTiming = millis();
  }
}
int getLumen(int anaPin)
{
  int anaValue = 0;
  for (int i = 0; i < 10; i++) // Đọc giá trị cảm biến 10 lần và lấy giá trị trung bình
  {
    anaValue += analogRead(anaPin);
    delay(50);
  }

  anaValue = anaValue / 10;
  anaValue = map(anaValue, 1023, 0, 0, 100); //Tối:0  ==> Sáng 100%

  return anaValue;
}
int getSoilMoist()
{
  int i = 0;
  int anaValue = 0;
  for (i = 0; i < 10; i++)  //
  {
    anaValue += analogRead(SOIL_MOIST_1_PIN); //Đọc giá trị cảm biến độ ẩm đất
    delay(50);   // Đợi đọc giá trị ADC
  }
  anaValue = anaValue / (i);
  anaValue = map(anaValue, 1023, 0, 0, 100); //Ít nước:0%  ==> Nhiều nước 100%
  return anaValue;
}
void readSensors(void)
{
  tempDHT = dht.readTemperature();   //Đọc nhiệt độ DHT22
  humDHT = dht.readHumidity();       //Đọc độ ẩm DHT22
  lumen = getLumen(LDR_PIN);         //Đọc ánh sáng
  soilMoist = getSoilMoist();        //Đọc cảm biến độ ẩm đất
}
void showDataLCD(void)
{
  lcd.setCursor(0, 1);
  lcd.print(" DO.AM% = ");
  lcd.print(humDHT);
  lcd.println("  % " );

  lcd.setCursor(1, 0);
  lcd.print(" NH.DO = ");
  lcd.print(tempDHT);
  lcd.println(" *C ");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(" AM.DAT% = ");
  lcd.print(soilMoist);
  lcd.println("  %   " );

  lcd.setCursor(1, 0);
  lcd.print("A.SANG% = ");
  lcd.print(lumen);
  lcd.println(" %  ");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("  BOM.NC = ");
  lcd.print(pumpWaterStatus);
  lcd.println("      " );

  lcd.setCursor(1, 0);
  lcd.print("BOM.SAU = ");
  lcd.print(pumpPesStatus);
  lcd.println("    ");
  delay(1000);
}
void printData(void)
{
  // IN thông tin ra màn hình
  Serial.print("Do am: ");
  Serial.print(humDHT);
  Serial.print(" %\t");
  Serial.print("Nhiet do: ");
  Serial.print(tempDHT);
  Serial.print(" *C\t");
  Serial.print("Anh sang: ");
  Serial.print(lumen);
  Serial.print(" %\t");
  Serial.print("Do am dat: ");
  Serial.print(soilMoist);
  Serial.println(" %");
}
/***************************************************
  Hàm bật bơm nước
****************************************************/
void turnPumpOn()
{
  digitalWrite(PUMPW_PIN, LOW);
  pumpWaterStatus = 1;
  showDataLCD();
  delay (timePumpOn * 1000);
  digitalWrite(PUMPW_PIN, HIGH);
  pumpWaterStatus = 0;
  showDataLCD();
}
/****************************************************************
  Hàm đọc trạng thái bơm và kiểm tra nút nhấn
  (Nút nhấn mặc định là mức "CAO"):
****************************************************************/
void readLocalCmd()
{
  int digiValue = debounce(PUMPW_ON);
  if (!digiValue)
  {
    pumpWaterStatus = !pumpWaterStatus;
    showDataLCD();
    aplyCmd();
  }
  digiValue = debounce(PUMPS_ON);
  if (!digiValue)
  {
    pumpPesStatus = !pumpPesStatus;
    showDataLCD();
    aplyCmd();
  }
}
/***************************************************
  Thực hiện điều khiển các bơm
****************************************************/
void aplyCmd()
{
  if (pumpWaterStatus == 1) digitalWrite(PUMPW_PIN, LOW);
  if (pumpWaterStatus == 0) digitalWrite(PUMPW_PIN, HIGH);

  if (pumpPesStatus == 1) digitalWrite(PUMPS_PIN, LOW);
  if (pumpPesStatus == 0) digitalWrite(PUMPS_PIN, HIGH);
}
/***************************************************
  Hàm kiểm tra trạng thái phím bấm
****************************************************/
boolean debounce(int pin)
{
  boolean state;
  boolean previousState;
  const int debounceDelay = 60;

  previousState = digitalRead(pin);
  for (int counter = 0; counter < debounceDelay; counter++)
  {
    delay(1);
    state = digitalRead(pin);
    if (state != previousState)
    {
      counter = 0;
      previousState = state;
    }
  }
  return state;
}
/***************************************************
   Chức năng tự động tưới tiêu
****************************************************/
void autoControlPlantation()
{
  //--------------------------------- BƠM NƯỚC ------//
  if (soilMoist < DRY_SOIL && lumen > DARK_LIGHT)
  {
    turnPumpOn();
  }
}
