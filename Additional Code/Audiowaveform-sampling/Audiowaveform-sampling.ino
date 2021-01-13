#define AUDIO_IN_PIN 34
#define samples 1024
#define SamplingFrequency 40000

double oldTime;
double newTime;
double Period = 1000000 / SamplingFrequency;

double AnalogValue[samples];

void setup() {
  Serial.begin(115200);
}

void loop() {

  /*
    Performing a Serial.println after every analog read takes to much time and distorts the data recorded.
    There will be a Jump in Values after samples amount of datapoints if you look at the Serial plotter.
  */

  oldTime = micros();
  for (uint16_t i = 0; i < samples; i++) {
    AnalogValue[i] = analogRead(AUDIO_IN_PIN);
    newTime = micros();
    while (newTime - oldTime < Period) {
      newTime = micros();
    }
  }
  for (uint16_t i = 0; i < samples; i++) {
    Serial.println(AnalogValue[i]);
  }

}
