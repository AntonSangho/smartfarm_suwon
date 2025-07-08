#include <LiquidCrystal_I2C.h>

#define A_SOIL_HUMI A0
#define O_RGB_R 10
#define O_RGB_G 9
#define O_RGB_B 11
#define O_PWM_1 6
#define O_PWM_2 7
#define LCD_I2C_ADDR 0x27
#define On_Time 2000

#define PUMP_OFF 0
#define PUMP_START 170
#define PUMP_MAX 230

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);

int soilHumidity;
unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 30000; // 1분마다 로그 (30초)

void initPin() {
  pinMode(O_RGB_R, OUTPUT);
  pinMode(O_RGB_G, OUTPUT);
  pinMode(O_RGB_B, OUTPUT);
  digitalWrite(O_RGB_R, LOW);
  digitalWrite(O_RGB_G, LOW);
  digitalWrite(O_RGB_B, LOW);
  pinMode(O_PWM_1, OUTPUT);
  pinMode(O_PWM_2, OUTPUT);
  analogWrite(O_PWM_1, 0);
  digitalWrite(O_PWM_2, 0);
}

void introLcd() {
  lcd.print("Planting Kit");
  lcd.setCursor(0, 1);
  lcd.print("Rev4.0");
}

void initLcd() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  introLcd();
}

void printLcd() {
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Moisture : ");
  lcd.print(soilHumidity);
  lcd.print("%");
  lcd.setCursor(0, 1);
  if (soilHumidity < 20) lcd.print("Need Water");
  else if (soilHumidity < 50) lcd.print("Soil is Dry");
  else if (soilHumidity < 90) lcd.print("Soil is Wet");
  else lcd.print("Enough Water");
}

void calcSoilHumidity() {
  soilHumidity = map(analogRead(A_SOIL_HUMI), 1000, 400, 0, 100);
  if (soilHumidity > 100) soilHumidity = 100;
  else if (soilHumidity < 0) soilHumidity = 0;
}

void writeRGB(bool R, bool G, bool B) {
  digitalWrite(O_RGB_R, R);
  digitalWrite(O_RGB_G, G);
  digitalWrite(O_RGB_B, B);
}

// 데이터 로깅 함수 추가
void logData() {
  unsigned long currentTime = millis();
  
  // CSV 형식으로 데이터 출력
  Serial.print(currentTime / 1000); // 시간(초)
  Serial.print(",");
  Serial.print(soilHumidity);
  Serial.print(",");
  Serial.print(analogRead(A_SOIL_HUMI)); // 원시 센서값
  Serial.print(",");
  
  // 상태 출력
  if (soilHumidity < 20) Serial.println("Need Water");
  else if (soilHumidity < 50) Serial.println("Soil is Dry");
  else if (soilHumidity < 90) Serial.println("Soil is Wet");
  else Serial.println("Enough Water");
}

void setup() {
  Serial.begin(9600);
  initPin();
  initLcd();
  delay(2000);
  writeRGB(HIGH, LOW, HIGH);
  
  // CSV 헤더 출력
  Serial.println("Time(s),Humidity(%),RawValue,Status");
}

void loop() {
  calcSoilHumidity();
  printLcd();
  
  // 정기적으로 데이터 로깅
  unsigned long currentTime = millis();
  if (currentTime - lastLogTime >= LOG_INTERVAL) {
    logData();
    lastLogTime = currentTime;
  }
  
  // 펌프 제어 로직
  if (soilHumidity < 20) {
    delay(2000);
    lcd.clear();
    lcd.noBacklight();
    delay(250);
    
    for (int i = PUMP_START; i < PUMP_MAX; i++) {
      analogWrite(O_PWM_1, i);
      delay(5);
    }
    delay(On_Time);
    analogWrite(O_PWM_1, PUMP_OFF);
    delay(250);
    
    // 급수 이벤트 로깅
    Serial.print(currentTime / 1000);
    Serial.print(",");
    Serial.print(soilHumidity);
    Serial.print(",");
    Serial.print(analogRead(A_SOIL_HUMI));
    Serial.println(",PUMP_ACTIVATED");
  } else {
    analogWrite(O_PWM_1, PUMP_OFF);
  }
  
  delay(1000); // 1초 대기
}