/* Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
  Thêm chức năng đọc cảm biến ánh sáng
  Thêm chức năng đọc cảm biến độ ẩm đất
  Thêm hiển thị LCD
  Thêm chức năng điều khiển tưới tiêu/ cấp nhiệt bằng tay bơm và đèn
  HocARM NDTR BOT for ESP8266 by hocARM.org
  -------------------------------------------------
  // Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
  // Thêm chức năng đọc cảm biến ánh sáng
  // Thêm chức năng đọc cảm biến độ ẩm đất
  // Kết nối
  // DHT       | ESP8266
  //---------------------------
  // VCC(1)    |  3.3V
  // DATA(2)   |  D3
  // NC(3)     |  x
  // GND(4)    |  GND
  // Nối trở 10k giữa chân 1 và chân 2
  //----------------
  //Cảm biến độ ẩm | ESP8266
  //--------------------------
  // VCC(1)    	 |  3.3V
  // GND(2)   	 |  GND
  // D0(3)  	 |  x
  // A0(4)    	 |  A0
*/
#include "DHT.h"

#define DHTPIN D3     // Chân DATA nối với D3
//#define LDR_PIN A0    // Chân PE3 nối với chân OUT cảm biến as
#define SOIL_MOIST_1_PIN A0 // Chân PE4 nối với cảm biến độ ẩm
// Relay, nút nhấn
#define PUMP_ON_BUTTON D0   //Nút có sẵn trên kit
#define LAMP_ON_BUTTON D1   //Nút có sẵn trên kit
#define SENSORS_READ_BUTTON D4

#define PUMP_PIN D6   //Bom
#define LAMP_PIN D7   //Den
// Uncomment loại cảm biến bạn sử dụng, nếu DHT11 thì uncomment DHT11 và comment DHT22
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Biến lưu các giá trị cảm biến
int humDHT;
int tempDHT;
int lumen;
int soilMoist;
// Biến lưu trạng thái bơm
boolean pumpStatus = 0;
boolean lampStatus = 0;

int timePumpOn = 10; // Thời gian bật bơm nước
// Biến cho timer
long sampleTimingSeconds = 50; // ==> Thời gian đọc cảm biến (s)
long startTiming = 0;
long elapsedTime = 0;
// Khởi tạo cảm biến
DHT dht(DHTPIN, DHTTYPE);

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
  readSensors(); // Khởi tạo đọc cảm biến
  startTiming = millis(); // Bắt đầu đếm thời gian
}

void loop() {
  // Khởi tạo timer
  elapsedTime = millis() - startTiming;
  readLocalCmd();
  if (elapsedTime > (sampleTimingSeconds * 1000))
  {
    readSensors();
    printData();
    startTiming = millis();
  }
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
  soilMoist = getSoilMoist();        //Đọc cảm biến độ ẩm đất
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
  //  Serial.print("Anh sang: ");
  //  Serial.print(lumen);
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
    readSensors();
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