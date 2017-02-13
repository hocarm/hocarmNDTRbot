/* Chương trình đọc nhiệt độ, độ ẩm từ cảm biến DHT
  Thêm chức năng đọc cảm biến ánh sáng
  Thêm chức năng đọc cảm biến độ ẩm đất
  Thêm hiển thị LCD
  Thêm chức năng điều khiển tưới tiêu bằng tay 2 bơm
  Thêm chức năng điều khiển tự động bơm khi thiếu nước
  Thêm chức năng cập nhật dữ liệu qua cloud thingspeak
  HocARM NDTR BOT by hocARM.org
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
   Kết nối
   ESP8266   |  Arduino  | Nguồn ngoài
   VCC       |  x    	 | 3.3v
   GND       |  GND   	 | GND
   CH_PD     |  x    	 | 3.3v
   RX        |  11       | x
   TX        |  10       | x
*/

#include "DHT.h"

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

SoftwareSerial Serial1(10, 11); // RX, TX

#define DHTPIN 2     // Chân DATA nối với PD0
#define LDR_PIN A0    // Chân PE3 nối với chân OUT cảm biến as
#define SOIL_MOIST_1_PIN A1 // Chân PE4 nối với cảm biến độ ẩm
// Relay, nút nhấn
#define PUMPW_ON A2   //Nút có sẵn trên kit
#define PUMPW_PIN 3
#define PUMPS_ON A3   //Nút có sẵn trên kit
#define PUMPS_PIN 12

#define IP "184.106.153.149"// thingspeak.com ip
//#define GREEN_LED 13

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
int timePumpOn = 30; // Thời gian bật bơm nước
// Biến cho timer
long sampleTimingSeconds = 60; // ==> Thời gian đọc cảm biến (s)
long startTiming = 0;
long elapsedTime = 0;

char msg[] = "GET /update?key=xxxxxxxx";	// Thay xxxxxxxx bằng API key của bạn
char cmd[100];
char aux_str[100];
int legth;

// Khởi tạo cảm biến
DHT dht(DHTPIN, DHTTYPE);
// Khởi tạo LCD
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
void setup() {
  pinMode(PUMPW_PIN, OUTPUT);
  pinMode(PUMPS_PIN, OUTPUT);
  pinMode(PUMPW_ON, INPUT_PULLUP); // Nut pullup
  pinMode(PUMPS_ON, INPUT_PULLUP); // Nut pullup
  aplyCmd();
  // Khởi tạo cổng serial baud 115200
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial.println("HocARM NDTR Bot!");
  // Bắt đầu đọc dữ liệu
  dht.begin();
  lcd.begin(16, 2);
  lcd.print("HocARM NDTR Bot!*");
  lcd.setCursor(0, 1);
  lcd.print("Xin chao ong chu");
  connectWiFi();
  readSensors(); // Khởi tạo đọc cảm biến
  startTiming = millis(); // Bắt đầu đếm thời gian
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Da ket noi wifi");
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
    updateDataThingSpeak();
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
  //  lcd.clear();            //Xóa màn hình
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
  lcd.print("    BOM.NUOC = ");
  lcd.print(pumpWaterStatus);
  lcd.println("      " );

  lcd.setCursor(1, 0);
  lcd.print("BOM.TRU.SAU = ");
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
void showPumpLCD(void)
{

  lcd.clear();
  lcd.setCursor(0, 1);
  //lcd.setCusor(0, 1);
  lcd.print("    BOM.NUOC = ");
  lcd.print(pumpWaterStatus);
  lcd.println("      " );

  lcd.setCursor(1, 0);
  //lcd.setCusor (1, 0);
  lcd.print("BOM.TRU.SAU = ");
  lcd.print(pumpPesStatus);
  lcd.println("    ");
//  delay(1000);
}
/***************************************************
  Hàm bật bơm nước
****************************************************/
void turnPumpOn()
{
  digitalWrite(PUMPW_PIN, LOW);
  pumpWaterStatus = 1;
  showPumpLCD();
  updateCmdThingSpeak();
  delay (timePumpOn * 1000);
  digitalWrite(PUMPW_PIN, HIGH);
  pumpWaterStatus = 0;
  showPumpLCD();
  updateCmdThingSpeak();
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
    showPumpLCD();
    aplyCmd();
    updateCmdThingSpeak();
  }
  digiValue = debounce(PUMPS_ON);
  if (!digiValue)
  {
    pumpPesStatus = !pumpPesStatus;
    showPumpLCD();
    aplyCmd();
    updateCmdThingSpeak();
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
// Hàm gửi lệnh AT
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout) {

  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // xóa buffer

  delay(100);

  while ( Serial1.available() > 0) Serial1.read();   // đọc input

  Serial1.println(ATcommand);    // Gửi lệnh AT

  x = 0;
  previous = millis();

  // Chờ phản hồi
  do {
    if (Serial1.available() != 0) {
      // Nếu có dữ liệu trong buffer UART, đọc và kiểm tra nó với expected_answer
      response[x] = Serial1.read();
      x++;
      // Nếu đúng thì trả kết quả answer = 1, thoát hàm
      if (strstr(response, expected_answer) != NULL)
      {
        answer = 1;
      }

    }
  } while ((answer == 0) && ((millis() - previous) < timeout)); // Nếu sai thì tiếp tục thử lại cho tới hết thời gian timeout
  Serial.println(response);   // In giá trị nhận được để debug
  return answer;
}
// Hàm gửi lệnh AT 2 để gửi dữ liệu
int8_t sendATcommand2(char* ATcommand, char* expected_answer1,
                      char* expected_answer2, unsigned int timeout) {

  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Khởi tạo lại chuỗi về 0

  delay(100);

  while ( Serial1.available() > 0) Serial1.read();   // Xóa buffer

  Serial1.println(ATcommand);    // Gửi lệnh AT

  x = 0;
  previous = millis();

  // Chờ phản hồi
  do {
    // Nếu có dữ liệu từ UART thì đọc và kiểm tra
    if (Serial1.available() != 0) {
      response[x] = Serial1.read();
      x++;
      // Trả về giá trị 1 nếu nhận được expected_answer1
      if (strstr(response, expected_answer1) != NULL)
      {
        answer = 1;
      }
      // Trả về giá trị 2 nếu nhận được expected_answer2
      else if (strstr(response, expected_answer2) != NULL)
      {
        answer = 2;
      }
    }
  }
  // Đợi time out
  while ((answer == 0) && ((millis() - previous) < timeout));
  Serial.println(response);   // In giá trị nhận được để debug
  return answer;
}
/***************************************************
  Kết nối wifi
****************************************************/
void connectWiFi(void)
{
  sendATcommand("AT", "OK", 5000);                //Kiểm tra kết nối
  sendATcommand("AT+CWMODE=1", "OK", 5000);       //Cấu hình chế độ station
  sendATcommand("AT+CWJAP=\"ten_wifi\",\"mat_khau\"", "OK", 5000);  //Thay ten_wifi và mat_khau của bạn
  sendATcommand("AT+CIPMUX=1", "OK", 5000);       // Bật chế độ đa kết nối
  sendATcommand("AT+CIFSR", "OK", 5000); // Hiển thị ip
  Serial.println("ESP8266 Connected");
}

/***************************************************
  Kết nối với ThingsSpeak.com
****************************************************/
void startThingSpeakCmd(void)
{
  memset(aux_str, '\0', 100);
  snprintf(aux_str, sizeof(aux_str), "AT+CIPSTART=1,\"TCP\",\"%s\",80", IP);
  if (sendATcommand2(aux_str, "OK",  "ERROR", 20000) == 1)
  {
    Serial.println("OK Connected Thingspeak");
  }
}
/***************************************************
  Gửi data lên channel ThingsSpeak.com
****************************************************/
void sendThingSpeakCmd(void)
{
  memset(aux_str, '\0', 100);
  sprintf(aux_str, "AT+CIPSEND=1,%d", legth);
  if (sendATcommand2(aux_str, ">", "ERROR", 20000) == 1)
  {
    Serial.println(cmd);
    sendATcommand2(cmd, "SEND OK", "ERROR", 30000);
  }
}
/***************************************************
  Truyền tất cả dữ liệu lên thingspeak.com và đóng kết nối
****************************************************/
void updateDataThingSpeak(void)
{
  startThingSpeakCmd();
  // Gửi toàn bộ thông tin cảm biến cũng như trạng thái lên thingspeak
  sprintf(cmd, "%s&field1=%d&field2=%d&field3=%d&field4=%d&field5=%d&field6=%d", msg, tempDHT, humDHT, lumen, soilMoist, pumpWaterStatus, pumpPesStatus);
  legth = strlen(cmd) + 2;
  sendThingSpeakCmd();
  sendATcommand("AT+CIPCLOSE=1", "OK", 5000);
}

/***************************************************
  Cập nhật trạng thái bơm lên thingspeak.com
****************************************************/
void updateCmdThingSpeak(void)
{
  for (int i = 0; i < 1; i++)     // Thực hiện 2 lần cho chắc ăn
  {
    startThingSpeakCmd ();
    // Cập nhật trạng thái bơm
    sprintf(cmd, "%s&field5=%d&field6=%d", msg, pumpWaterStatus, pumpPesStatus);
    legth = strlen(cmd) + 2;
    sendThingSpeakCmd();
    sendATcommand("AT+CIPCLOSE=1", "OK", 5000);
  }
}

