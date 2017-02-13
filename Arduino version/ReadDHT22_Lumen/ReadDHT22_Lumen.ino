// Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
// Thêm chức năng đọc cảm biến ánh sáng
// Written by ladyada, public domain
// Chỉnh  sửa cho Arduino Uno bởi hocARM.org

#include "DHT.h"

#define DHTPIN 2     // Chân DATA nối với 2
#define LDR_PIN A0    // Chân A0 nối với chân OUT cảm biến as
// Uncomment loại cảm biến bạn sử dụng, nếu DHT11 thì uncomment DHT11 và comment DHT22
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Kết nối
// DHT       | Arduino
//----------------
// VCC(1)    |  5V
// DATA(2)   |  2
// NC(3)     |  x
// GND(4)    |  GND
// Nối trở 10k giữa chân 1 và chân 2

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
  // Đợi chuyển đổi dữ liệu khoảng 2s
  delay(2000);

  float h = dht.readHumidity();
  // Đọc giá trị nhiệt độ C (mặc định)
  float t = dht.readTemperature();
  // Đọc giá trị nhiệt độ F(isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Kiểm tra quá trình đọc thành công hay không
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Tính chỉ số nhiệt độ F (mặc định)
  //  float hif = dht.computeHeatIndex(f, h);
  // Tính chỉ số nhiệt độ C (isFahreheit = false)
  //  float hic = dht.computeHeatIndex(t, h, false);
  // Đọc dữ liệu cảm biến ánh sáng
  int lumen = getLumen(LDR_PIN);
  // IN thông tin ra màn hình
  Serial.print("Do am: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Nhiet do: ");
  Serial.print(t);
  Serial.print(" *C\t");
  Serial.print("Anh sang: ");
  Serial.print(lumen);
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
