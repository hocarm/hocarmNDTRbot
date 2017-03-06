// Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
// Written by ladyada, public domain
// Chỉnh  sửa cho ESP8266 bởi hocARM.org

#include "DHT.h"

#define DHTPIN D3      // Chân DATA nối với chân D3

// Uncomment loại cảm biến bạn sử dụng, nếu DHT11 thì uncomment DHT11 và comment DHT22
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Kết nối
// DHT       | ESP8266
//----------------
// VCC(1)    |  3.3V
// DATA(2)   |  D3
// NC(3)     |  x
// GND(4)    |  GND

// Kết nối chân 1 của DHT với 3.3V
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

  // IN thông tin ra màn hình
  Serial.print("Do am: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Nhiet do: ");
  Serial.print(t);
  Serial.println(" *C ");

}