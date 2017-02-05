// Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
// Thêm chức năng đọc cảm biến ánh sáng
// Written by ladyada, public domain
// Chỉnh  sửa cho TIVA C bởi hocARM.org

#include "DHT.h"

#define DHTPIN PD_0     // Chân DATA nối với PD0
#define LDR_PIN PE_3    // Chân PE3 nối với chân OUT cảm biến as
// Uncomment loại cảm biến bạn sử dụng, nếu DHT11 thì uncomment DHT11 và comment DHT22
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Kết nối
// DHT       | TIVA C
//----------------
// VCC(1)    |  3.3V
// DATA(2)   |  PD0
// NC(3)     |  x
// GND(4)    |  GND

// Kết nối chân 1 của DHT với 3.3V
// Chân 2 kết nối với bất kỳ chân nào của TIVA C
// Chân 4 nối với GND
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
  //  Serial.print(f);
  //  Serial.print(" *F\t");
  //  Serial.print("Heat index: ");
  //  Serial.print(hic);
  //  Serial.print(" *C ");
  //  Serial.print(hif);
  //  Serial.println(" *F");
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
  anaValue = map(anaValue, 4095, 0, 0, 100); //Tối:0  ==> Sáng 100%

  return anaValue;
}
