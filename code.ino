#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define TdsSensorPin A0

LiquidCrystal_I2C lcd(0x27, 16, 2);

float calibrationFactor = 1.0;
float voltage;
float tdsValue;

// Hardware pins
const int solenoid1 = 2;
const int solenoid2 = 5;
const int solenoid3 = 7; // solenoid3 is used after 5 cycles
const int pump1 = 6; // pump water (now used for all outputs)
const int pump2 = 4; // pump chemical
const int mixer  = 3;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(solenoid1, OUTPUT);
  pinMode(solenoid2, OUTPUT);
  pinMode(solenoid3, OUTPUT); // We keep solenoid3 in the code
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  pinMode(mixer, OUTPUT);

  digitalWrite(solenoid1, HIGH);
  digitalWrite(solenoid2, HIGH);
  digitalWrite(solenoid3, HIGH); // Keep solenoid3 off initially
  digitalWrite(pump1, HIGH);
  digitalWrite(pump2, HIGH);
  digitalWrite(mixer, HIGH);

  lcd.setCursor(0, 0);
  lcd.print("Water Purify Sys");
  delay(3000);

  for (int cycle = 1; cycle <= 5; cycle++) {
    lcd.clear();
    lcd.print("Cycle: ");
    lcd.print(cycle);
    delay(1000);

    // Step 1: Fill tank
    digitalWrite(solenoid1, LOW);
    lcd.clear();
    lcd.print("S1 ON - Fill");
    delay(300000); // 5 mins
    digitalWrite(solenoid1, HIGH);
    lcd.clear();
    lcd.print("S1 OFF");

    // Step 2: Mixer + pump2 (only first cycle)
    if (cycle == 1) {
      digitalWrite(pump2, LOW);
      delay(10000);
      digitalWrite(pump2, HIGH);
      digitalWrite(mixer, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Mixer ON");
      delay(60000);
      digitalWrite(mixer, HIGH);
      lcd.clear();
      lcd.print("Mixer+P2 OFF");
      delay(300000);
    }

    // Step 3: Solenoid2 ON
    digitalWrite(solenoid2, LOW);
    lcd.setCursor(0, 1);
    lcd.print("S2 ON - Wait");
    delay(180000);
    digitalWrite(solenoid2, HIGH);

    // Step 4: Read TDS
    int totalSensorValue = 0;
    int numReadings = 10;

    for (int i = 0; i < numReadings; i++) {
      totalSensorValue += analogRead(TdsSensorPin);
      delay(50);
    }

    float avgSensorValue = totalSensorValue / numReadings;
    voltage = avgSensorValue * (5.0 / 1024.0);
    tdsValue = (133.42 * voltage * voltage * voltage
                - 255.86 * voltage * voltage
                + 857.39 * voltage) * calibrationFactor;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TDS: ");
    lcd.print(tdsValue, 1);
    lcd.print(" ppm");

    Serial.print("TDS Value: ");
    Serial.println(tdsValue);

    delay(5000);

    // Step 5: Pump decision
    digitalWrite(pump1, LOW);
    if (tdsValue < 500 && tdsValue > 0) {
      lcd.clear();
      lcd.print("TDS OK - P1 ON");
    } else {
      lcd.clear();
      lcd.print("Bad TDS! P1 ON");
    }

    delay(15000); // تشغيل المضخة شوية
    digitalWrite(pump1, HIGH);
  }

  // After completing 5 cycles, open solenoid3
  lcd.clear();
  lcd.print("5 Cycles Done");
  delay(2000);

  digitalWrite(solenoid3, LOW); // فتح solenoid3 بعد 5 سايكلز
  lcd.clear();
  lcd.print("S3 Opened - Done");
}

void loop() {
  // Nothing to do
}
