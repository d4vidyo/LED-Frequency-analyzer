#define AUDIO_IN_PIN 34

int PeakTop = 0;
int Peakbottom = 5000;
int u = 0;
void setup() {
  Serial.begin(115200);
}

void loop() {
  u = analogRead(AUDIO_IN_PIN);
  Serial.print(u);
  Serial.print(" , ");
  if (PeakTop < u) {
    PeakTop = u;
  }
  if (Peakbottom > u) {
    Peakbottom = u;
  }
  Serial.print(PeakTop);
  Serial.print(" , ");
  Serial.println(Peakbottom);


}
