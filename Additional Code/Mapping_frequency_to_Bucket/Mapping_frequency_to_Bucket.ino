
void setup() {
  Serial.begin(115200);

}

void loop() {
  /* OLD function
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
    }*/
#define Zeile 6
#define Spalte 7
  int aa = 33*4;
  int bb = 52*4;
  int cc = 59*4;
  int dd = 64*4;
  double Matrix[Zeile][Spalte] = { {1, 1, 1, 1, 1, 1, 39},
    {pow(cc, 5), pow(cc, 4), pow(cc, 3), pow(cc, 2), cc, 1, 12000},
    {pow(aa, 5), pow(aa, 4), pow(aa, 3), pow(aa, 2), aa, 1,  4000},
    {pow(bb, 5), pow(bb, 4), pow(bb, 3), pow(bb, 2), bb, 1,  8000},
    {pow(dd, 5), pow(dd, 4), pow(dd, 3), pow(dd, 2), dd, 1, 16000},
    {80, 32, 12, 4, 1, 0, 39}
  };
  double a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
  float x = 0;
  int BucketFrequency[64];
  for (int k = 0; k < Zeile; k++) {
    for (int j = k + 1; j < Zeile; j++) {
      a = Matrix[j][k] / Matrix[k][k];
      for (int i = k; i < Spalte; i++) {
        Matrix[j][i] = Matrix[j][i] - a * Matrix[k][i];
      }
    }
    for (int ii = 0; ii < Zeile; ii++) {
      for (int jj = 0; jj < Spalte; jj++) {
        Matrix[ii][jj] /= 10;
      }
    }
  }
  f = Matrix[5][6] / Matrix[5][5];
  e = (Matrix[4][6] - Matrix[4][5] * f) / Matrix[4][4];
  d = (Matrix[3][6] - Matrix[3][5] * f - Matrix[3][4] * e ) / Matrix[3][3];
  c = (Matrix[2][6] - Matrix[2][5] * f - Matrix[2][4] * e - Matrix[2][3] * d) / Matrix[2][2];
  b = (Matrix[1][6] - Matrix[1][5] * f - Matrix[1][4] * e - Matrix[1][3] * d - Matrix[1][2] * c) / Matrix[1][1];
  a = (Matrix[0][6] - Matrix[0][5] * f - Matrix[0][4] * e - Matrix[0][3] * d - Matrix[0][2] * c - Matrix[0][1] * b) / Matrix[0][0];

  for (int i = 1; i <= 64; i++) {
    BucketFrequency[i - 1] = (a * pow(i, 5)) + (b * pow(i, 4)) + (c * pow(i, 3)) + (d * pow(i, 2)) + (e * i) + f;
  }
  Serial.print(a, 9);
  Serial.print("*x^5+");
  Serial.print(b, 9);
  Serial.print("*x^4+");
  Serial.print(c, 9);
  Serial.print("*x^3+");
  Serial.print(d, 9);
  Serial.print("*x^2+");
  Serial.print(e, 9);
  Serial.print("*x+");
  Serial.println(f, 9);
  for (int i = 0; i < 64; i++) {
    //Serial.print("Bucket ");
    //Serial.print(i);
    //Serial.print(" = ");
    //Serial.print(BucketFrequency[i]);
    //Serial.print("hz");
    //Serial.println(" , ");
  }

  int Diff[64];
  int smallest = 0;
  int smallestPosition[1024];
  for (uint16_t i = 0; i < 1024 / 2; i++) {
    double abscissa = ((i * 1.0 * 40000) / 1024);

    for (int j = 0; j < 64; j++) {
      Diff[j] = abscissa - BucketFrequency[j];

      if (Diff[j] < 0) {
        Diff[j] = -Diff[j];
      }
      if (j == 0) {
        smallest = Diff[j];
      }
      if (smallest >= Diff[j]) {
        smallest = Diff[j];
        smallestPosition[i] = j;
      }
    }/*
    Serial.print("Nr: ");
    Serial.print(i);
    Serial.print(" smallestPosition: ");
    Serial.println(smallestPosition[i]);*/
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
    else if (BucketFrequency[i] < 2 * quarter && BucketFrequency[i] > quarter) {
      Rfix[i] = 255 - 255 * (BucketFrequency[i] - sub[0]) / quarter;
      Gfix[i] = 255;
      Bfix[i] = 0;
      sub[1] = BucketFrequency[i];
    }
    else if (BucketFrequency[i] < 3 * quarter && BucketFrequency[i] > 2 * quarter) {
      Rfix[i] = 0;
      Gfix[i] = 255;
      Bfix[i] = 255 * (BucketFrequency[i] - sub[1]) / quarter;
      sub[2] = BucketFrequency[i];
    }
    else {
      Rfix[i] = 0;
      Gfix[i] = 255 - 255 * (BucketFrequency[i] - sub[2]) / quarter;
      Bfix[i] = 255;
    }
    if (Rfix[i] < 0) {
      Rfix[i] = 0;
    } if (Gfix[i] < 0) {
      Gfix[i] = 0;
    } if (Bfix[i] < 0) {
      Bfix[i] = 0;
    }
  }

  delay(5000);

}
