/* Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
  Thêm chức năng đọc cảm biến ánh sáng
  Thêm chức năng đọc cảm biến độ ẩm đất
  Thêm hiển thị LCD
  Thêm chức năng điều khiển tưới tiêu bằng tay 2 bơm
  HocARM NDTR BOT for ESP8266 by hocARM.org
  -------------------------------------------------
  // Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
  // Thêm chức năng đọc cảm biến ánh sáng
  // Thêm chức năng đọc cảm biến độ ẩm đất
  // Chỉnh  sửa cho Arduino Uno bởi hocARM.org
  // Kết nối
  // DHT       | ESP8266
  //---------------------------
  // VCC(1)    |  3.3V
  // DATA(2)   |  2
  // NC(3)     |  x
  // GND(4)    |  GND
  // Nối trở 10k giữa chân 1 và chân 2
  //----------------
  //Cảm biến độ ẩm | ESP8266
  //--------------------------
  // VCC(1)       |  3.3V
  // GND(2)       |  GND
  // D0(3)        |  x
  // A0(4)        |  A1
*/
#include "DHT.h"
//#include "homephone.h"
//#include <LiquidCrystal.h>
/* TIMER */
#include <SimpleTimer.h>

#define DHTPIN D3     // Chân DATA nối với D3
//#define LDR_PIN A0    // Chân PE3 nối với chân OUT cảm biến as
#define SOIL_MOIST_1_PIN A0 // Chân PE4 nối với cảm biến độ ẩm
// Relay, nút nhấn
#define PUMP_ON_BUTTON D0   
#define LAMP_ON_BUTTON D1   
#define SENSORS_READ_BUTTON D4

#define PUMP_PIN D6   //Bom
#define LAMP_PIN D7   //Den
// Uncomment loại cảm biến bạn sử dụng, nếu DHT11 thì uncomment DHT11 và comment DHT22
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

/* Thông số cho chế độ tự động */
#define DRY_SOIL      66
#define WET_SOIL      88
#define COLD_TEMP     15
#define HOT_TEMP      24
#define TIME_PUMP_ON  15
#define TIME_LAMP_ON  15

/* TIMER */
#define READ_BUTTONS_TM   1L  // Tương ứng với giây
#define READ_SOIL_HUM_TM  10L //Đọc cảm biến ẩm đất
#define READ_AIR_DATA_TM  2L  //Đọc DHT
#define DISPLAY_DATA_TM   10L
#define AUTO_CTRL_TM      60L //Chế độ tư động

// Biến lưu các giá trị cảm biến
float humDHT = 0;
float tempDHT = 0;
//int lumen;
int soilMoist = 0;
// Biến lưu trạng thái bơm
boolean pumpStatus = 0;
boolean lampStatus = 0;

int timePumpOn = 10; // Thời gian bật bơm nước
// Biến cho timer
long sampleTimingSeconds = 50; // ==> Thời gian đọc cảm biến (s)
long startTiming = 0;
long elapsedTime = 0;
// Khởi tạo timer
SimpleTimer timer;

// Khởi tạo cảm biến
DHT dht(DHTPIN, DHTTYPE);
// Khởi tạo LCD
//homephone lcd (PD_1, PD_2, PD_3, PE_1, PE_2);
//LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

void setup() {
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(PUMP_ON_BUTTON, INPUT_PULLUP);
  pinMode(LAMP_ON_BUTTON, INPUT_PULLUP);
  pinMode(SENSORS_READ_BUTTON, INPUT_PULLUP);
  aplyCmd();
  // Khởi tạo cổng serial baud 115200
  Serial.begin(115200);
  Serial.println("HocARM NDTR Bot!");
  // Bắt đầu đọc dữ liệu
  dht.begin();
  startTimers();
}

void loop() {
  timer.run(); // Chạy SimpleTimer
}
void getSoilMoist(void)
{
  int i = 0;
  soilMoist = 0;
  for (i = 0; i < 10; i++)  //
  {
    soilMoist += analogRead(SOIL_MOIST_1_PIN); //Đọc giá trị cảm biến độ ẩm đất
    delay(50);   // Đợi đọc giá trị ADC
  }
  soilMoist = soilMoist / (i);
  soilMoist = map(soilMoist, 1023, 0, 0, 100); //Ít nước:0%  ==> Nhiều nước 100%
}

void getDhtData(void)
{

  tempDHT = dht.readTemperature();
  humDHT = dht.readHumidity();
  if (isnan(humDHT) || isnan(tempDHT))   // Kiểm tra kết nối lỗi thì thông báo.
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
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
  Serial.print(" %\t");
  Serial.print("Do am dat: ");
  Serial.print(soilMoist);
  Serial.println(" %");
}

/****************************************************************
  Hàm đọc trạng thái bơm và kiểm tra nút nhấn
  (Nút nhấn mặc định là mức "CAO"):
****************************************************************/
void readLocalCmd()
{
  boolean digiValue = debounce(PUMP_ON_BUTTON);
  if (!digiValue)
  {
    pumpStatus = !pumpStatus;
    aplyCmd();
  }

  digiValue = debounce(LAMP_ON_BUTTON);
  if (!digiValue)
  {
    lampStatus = !lampStatus;
    aplyCmd();
  }

  digiValue = debounce(SENSORS_READ_BUTTON);
  if (!digiValue)
  {
    getDhtData();
    getSoilMoist();
    printData();
  }
}
/***************************************************
  Thực hiện điều khiển các bơm
****************************************************/
void aplyCmd()
{
  if (pumpStatus == 1) digitalWrite(PUMP_PIN, LOW);
  if (pumpStatus == 0) digitalWrite(PUMP_PIN, HIGH);

  if (lampStatus == 1) digitalWrite(LAMP_PIN, LOW);
  if (lampStatus == 0) digitalWrite(LAMP_PIN, HIGH);
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
* Chế độ tự động dựa trên thông số cảm biến
****************************************************/
void autoControlPlantation(void)
{ 
  if (soilMoist < DRY_SOIL) 
  {
    turnPumpOn();
  }

  if (tempDHT < COLD_TEMP) 
  {
    turnLampOn();
  }
}
/***************************************************
* Bật bơm trong thời gian định sẵn
****************************************************/
void turnPumpOn()
{
  pumpStatus = 1;
  aplyCmd();
  delay (TIME_PUMP_ON*1000);
  pumpStatus = 0;
  aplyCmd();
}

/***************************************************
* Bật đèn trong thời gian định sẵn
****************************************************/
void turnLampOn()
{
  lampStatus = 1;
  aplyCmd();
  delay (TIME_LAMP_ON*1000);
  lampStatus = 0;
  aplyCmd();
}

/***************************************************
  Khởi động Timers
****************************************************/
void startTimers(void)
{
  timer.setInterval(READ_BUTTONS_TM * 1000, readLocalCmd);
  timer.setInterval(READ_AIR_DATA_TM * 1000, getDhtData);
  timer.setInterval(READ_SOIL_HUM_TM * 1000, getSoilMoist);
  timer.setInterval(AUTO_CTRL_TM * 1000, autoControlPlantation);
  timer.setInterval(DISPLAY_DATA_TM*1000, printData);
}