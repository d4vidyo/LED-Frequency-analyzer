
void setup() {
  Serial.begin(115200);

}

void loop() {
  int BucketFrequency[64];
  int startingFrequency = 20;
  int delta = 7;
  for (int i = 0; i < 64; i++) {

    startingFrequency = startingFrequency + delta;
    if (i == 0) {
      BucketFrequency[i] = startingFrequency;
    }
    else {
      BucketFrequency[i] = BucketFrequency[i - 1] + startingFrequency;
    }
  }
  for (int i = 0; i < 64; i++) {
    Serial.print("Bucket ");
    Serial.print(i);
    Serial.print(" = ");
    Serial.print(BucketFrequency[i]);
    Serial.println("hz");
  }

  int quarter = BucketFrequency[63] / 4;
  int Rfix[64];
  int Gfix[64];
  int Bfix[64];
  int sub[3];
  for (int i = 0; i < 64; i++) {
    if (BucketFrequency[i] < quarter) {
      Rfix[i] = 255;
      Gfix[i] = 255 * BucketFrequency[i] / quarter;
      Bfix[i] = 0;
      sub[0] = BucketFrequency[i];
    }
    if (BucketFrequency[i] < 2 * quarter && BucketFrequency[i] > quarter) {
      Rfix[i] = 255 - 255 * (BucketFrequency[i] - sub[0]) / quarter;
      Gfix[i] = 255;
      Bfix[i] = 0;
      sub[1] = BucketFrequency[i];
    }
    if (BucketFrequency[i] < 3 * quarter && BucketFrequency[i] > 2 * quarter) {
      Rfix[i] = 0;
      Gfix[i] = 255;
      Bfix[i] = 255 * (BucketFrequency[i] - sub[1]) / quarter;
      sub[2] = BucketFrequency[i];
    }
    if (BucketFrequency[i] < 4 * quarter && BucketFrequency[i] > 3 * quarter) {
      Rfix[i] = 0;
      Gfix[i] = 255 - 255 * (BucketFrequency[i] - sub[2]) / quarter;
      Bfix[i] = 255;
    }
    Serial.print(Rfix[i]);
    Serial.print(" , ");
    Serial.print(Gfix[i]);
    Serial.print(" , ");
    Serial.println(Bfix[i]);
  }

  delay(10000);

}
