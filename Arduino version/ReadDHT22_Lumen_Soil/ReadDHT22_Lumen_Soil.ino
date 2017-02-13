// Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
// Thêm chức năng đọc cảm biến ánh sáng
// Thêm chức năng đọc cảm biến độ ẩm đất
// Chỉnh  sửa cho Arduino Uno bởi hocARM.org
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
// Cảm biến độ ẩm đất nối chân A1

#include "DHT.h"

#define DHTPIN 2     // Chân DATA nối với 2
#define LDR_PIN A0    // Chân A0 nối với chân OUT cảm biến as
#define SOIL_MOIST_1_PIN A1 // Chân A1 nối với cảm biến độ ẩm

// Uncomment loại cảm biến bạn sử dụng, nếu DHT11 thì uncomment DHT11 và comment DHT22
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
int humDHT;
int tempDHT;
int lumen;
int soilMoist;
// Khởi tạo cảm biến
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Khởi tạo cổng serial baud 115200
  Serial.begin(115200);
  Serial.println("DHTxx test!");
  // Bắt đầu đọc dữ liệu
  dht.begin();
}

void loop() {
  readSensors();
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
