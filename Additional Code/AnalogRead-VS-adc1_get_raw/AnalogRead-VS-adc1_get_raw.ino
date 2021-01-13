#include <driver/adc.h>

#define AUDIO_IN_PIN 34
#define samples 1024

double analogValue[samples];
double TimeOld;
double TimeNew;

void setup() {
  Serial.begin(115200);
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
}

void loop() {

  TimeOld = micros();
  for (int i = 0; i < 1024; i++) {
    analogValue[i] = analogRead(AUDIO_IN_PIN);
  }
  TimeNew = micros();
  Serial.print("Time needed for analogRead: ");
  Serial.print(TimeNew-TimeOld);
  Serial.print(" us. ");

  TimeOld = micros();
  for (int i = 0; i < 1024; i++) {
    analogValue[i] = adc1_get_raw(ADC1_CHANNEL_6);
  }
  TimeNew = micros();
  Serial.print("Time needed for adc_get_raw: ");
  Serial.print(TimeNew-TimeOld);
  Serial.println(" us. ");
  delay(500);

}
