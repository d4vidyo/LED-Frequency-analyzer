//FFT
#include "arduinoFFT.h">
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
#define fps 60                        //Sets max. fps. Will go as fast as it can if set to high


//Neopixel
#include <FastLED.h>
#define LED_COUNT 64
#define LED_PIN 2
CRGB leds[LED_COUNT];

//Audio
#define Audio 34                      //Pin used to sample Audio
const uint16_t samples = 1024;        //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 40000;
const uint8_t amplitude = 100;
const uint8_t lowestband = 20;
const uint8_t highestband = samplingFrequency / 2;


/*FFT variables*/
double vReal[samples];
double Real[samples];
volatile double vRealBuffer[samples];
volatile double vFFT[samples];
double Imag[samples];

/*Input*/
float u;                              //Stores sampling Voltage reading

/* Timing Stuff*/
int TimingOld, TimingNew;                     //Framerate controll
int oldTime, newTime;                         //Sampling Frequency controll
int Oldtemp, Newtemp;                         //Temporary variables to troubleshoot
float Period = 1000000 / samplingFrequency;   //Time betweet each sampling reading in microseconds

/*Bucket Variables*/
double Buckets[LED_COUNT];            //Stores the Amplitude of all Frequencys
double BucketsOld[LED_COUNT];         //Stores the Amplitude of all Frequencys from last frame
int BucketFrequency[LED_COUNT];       //Stores the Frequency designatet to each Buckets[] entry
int smallestPosition[samples / 2];    //Stores which Buckets[] variable is closest to vReal[] variable
int Bucketentries[LED_COUNT];         //Stores how many vReal[] variables are put into one Buckets[] variable

float BucketPeakmeasure[samples / 2];  //Only used to update BucketAmplitude[] if necesarry

//Values which corespond to 100% of a Frequency
float BucketFFTamplitude[samples / 2] =
{ 6215.35, 6215.35, 6047.04, 5879.84, 5714.83, 5552.93, 5394.93, 5241.38, 5092.66, 4948.89, 4810.03, 4675.85, 4546.01, 4420.08, 4297.63, 4178.23,
  4061.51, 3947.23, 3835.22, 3725.45, 3617.98, 3512.99, 3410.71, 3311.43, 3215.43, 3123.00, 3034.39, 2949.80, 2869.36, 2793.14, 2721.16, 2653.35,
  2589.62, 2529.82, 2473.76, 2421.24, 2372.02, 2325.88, 2282.54, 2241.74, 2203.19, 2166.63, 2131.75, 2098.27, 2065.91, 2034.40, 2003.49, 1972.97,
  1942.65, 1912.38, 1882.05, 1851.63, 1821.08, 1790.45, 1759.80, 1729.23, 1698.86, 1668.82, 1639.27, 1610.32, 1582.12, 1554.75, 1528.31, 1502.86,
  1478.43, 1455.04, 1432.67, 1411.31, 1390.92, 1371.45, 1352.84, 1335.02, 1317.91, 1301.40, 1285.39, 1269.74, 1254.31, 1238.93, 1223.46, 1207.71,
  1191.55, 1174.83, 1157.45, 1139.36, 1120.54, 1101.04, 1080.95, 1060.44, 1039.72, 1019.07, 998.76, 979.15, 960.57, 943.36, 927.84, 914.30, 902.96,
  894.00, 887.48, 883.41, 881.69, 882.12, 884.42, 888.25, 893.19, 898.78, 904.56, 910.07, 914.86, 918.57, 920.88, 921.56, 920.47, 917.56, 912.87,
  906.51, 898.66, 889.56, 879.46, 868.65, 857.43, 846.08, 834.85, 824.00, 813.73, 804.19, 795.53, 787.81, 781.10, 775.38, 770.63, 766.78, 763.72,
  761.34, 759.51, 758.09, 756.93, 755.91, 754.91, 753.84, 752.65, 751.28, 749.71, 747.96, 746.03, 743.94, 741.73, 739.40, 736.97, 734.43, 731.79,
  729.02, 726.11, 723.06, 719.86, 716.55, 713.15, 709.73, 706.37, 703.16, 700.21, 697.61, 695.44, 693.75, 692.56, 691.87, 691.62, 691.71, 692.03,
  692.42, 692.75, 692.87, 692.65, 692.01, 690.88, 689.25, 687.15, 684.65, 681.83, 678.80, 675.69, 672.60, 669.62, 666.83, 664.26, 661.93, 659.82,
  657.88, 656.04, 654.22, 652.34, 650.32, 648.08, 645.55, 642.69, 639.48, 635.92, 632.03, 627.84, 623.42, 618.85, 614.22, 609.62, 605.15, 600.93,
  597.04, 593.56, 590.55, 588.05, 586.06, 584.56, 583.50, 582.80, 582.37, 582.07, 581.79, 581.38, 580.71, 579.67, 578.17, 576.13, 573.51, 570.32,
  566.59, 562.38, 557.78, 552.92, 547.92, 542.91, 538.03, 533.40, 529.11, 525.25, 521.87, 518.98, 516.58, 514.64, 513.10, 511.89, 510.94, 510.15,
  509.44, 508.72, 507.92, 506.97, 505.80, 504.37, 502.65, 500.62, 498.26, 495.59, 492.60, 489.34, 485.83, 482.12, 478.26, 474.30, 470.30, 466.32,
  462.40, 458.59, 454.94, 451.47, 448.21, 445.18, 442.35, 439.74, 437.30, 435.00, 432.80, 430.64, 428.46, 426.21, 423.82, 421.25, 418.46, 415.42,
  412.12, 408.55, 404.75, 400.75, 396.61, 392.39, 388.16, 384.01, 380.02, 376.25, 372.77, 369.64, 366.90, 364.58, 362.69, 361.23, 360.20, 359.58,
  359.34, 359.45, 359.89, 360.63, 361.63, 362.88, 364.35, 366.01, 367.85, 369.84, 371.94, 374.12, 376.34, 378.53, 380.64, 382.60, 384.31, 385.71,
  386.71, 387.23, 387.21, 386.60, 385.36, 383.47, 380.94, 377.80, 374.09, 369.87, 365.21, 360.19, 354.90, 349.44, 343.89, 338.34, 332.88, 327.58,
  322.51, 317.73, 313.30, 309.26, 305.64, 302.46, 299.72, 297.42, 295.53, 294.01, 292.82, 291.88, 291.14, 290.52, 289.93, 289.32, 288.61, 287.74,
  286.69, 285.43, 283.94, 282.22, 280.30, 278.21, 275.99, 273.66, 271.28, 268.90, 266.53, 264.23, 262.02, 259.91, 257.93, 256.09, 254.39, 252.84,
  251.44, 250.19, 249.08, 248.11, 247.26, 246.51, 245.85, 245.25, 244.68, 244.11, 243.49, 242.80, 242.00, 241.06, 239.96, 238.70, 237.26, 235.65,
  233.90, 232.02, 230.05, 228.04, 226.03, 224.07, 222.19, 220.44, 218.86, 217.46, 216.26, 215.27, 214.49, 213.91, 213.51, 213.27, 213.17, 213.17,
  213.25, 213.38, 213.54, 213.69, 213.83, 213.94, 214.00, 213.99, 213.93, 213.79, 213.57, 213.28, 212.91, 212.46, 211.93, 211.32, 210.63, 209.87,
  209.04, 208.15, 207.20, 206.20, 205.16, 204.10, 203.03, 201.97, 200.92, 199.92, 198.96, 198.07, 197.26, 196.54, 195.92, 195.39, 194.96, 194.63,
  194.39, 194.23, 194.14, 194.12, 194.14, 194.21, 194.30, 194.40, 194.51, 194.61, 194.70, 194.79, 194.86, 194.91, 194.96, 195.01, 195.05, 195.10,
  195.15, 195.21, 195.28, 195.35, 195.40, 195.45, 195.46, 195.44, 195.35, 195.20, 194.97, 194.65, 194.23, 193.71, 193.09, 192.37, 191.56, 190.67,
  189.72, 188.72, 187.70, 186.68, 185.69, 184.74, 183.86, 183.08, 182.41, 181.88, 181.49, 181.26, 181.18, 181.27, 181.52, 181.90, 182.41, 183.03,
  183.74, 184.51, 185.31, 186.13, 186.95, 187.74, 188.50, 189.22, 189.90, 190.54, 191.14, 191.72, 192.27, 192.82, 193.36, 193.90, 194.44, 194.99,
  195.54
};

/*float BucketAmplitude[LED_COUNT] = {6021.28, 6021.28, 5350.83, 4720.76, 4164.83, 3702.91, 3336.08, 3048.91,
                                    2818.86, 2626.81, 2461.88, 2318.82, 2192.6, 2075.32, 1957.44, 1831.19,
                                    1693.33, 1545.88, 1394.95, 1248.68, 1115.16, 1000.76, 908.93, 839.61,
                                    789.76, 754.68, 729.84, 712.02, 699.31, 690.18, 682.77, 674.82, 664.35,
                                    650.2, 632.08, 610.18, 584.9, 556.88, 527.35, 498.25, 471.98, 450.6,
                                    434.95, 424.09, 415.55, 406.22, 393.73, 377.3, 358.08, 338.48, 321.24,
                                    308.32, 300.23, 295.87, 292.92, 288.6, 280.5, 267.15, 248.11, 223.72,
                                    194.66, 161.74, 125.96, 88.61
                                   };*/

/*float BucketNoise[64] = {284.43 , 714.73 , 667.95 , 579.42 , 451.34 , 416.01 , 268.92 , 228.56 , 200.94 , 168.76 , 133.52 , 128.60 , 132.47 , 176.75 , 157.20 , 177.93 , 124.83 , 102.09 , 111.68 , 94.17 ,
                         69.79 , 87.22 , 118.06 , 106.82 , 83.47 , 78.88 , 63.66 , 76.13 , 71.21 , 65.49 , 47.32 , 51.52 , 52.82 , 47.39 , 50.41 , 46.77 , 50.18 , 46.98 , 45.22 , 61.64 , 59.21 , 41.16 ,
                         51.24 , 48.89 , 43.70 , 47.39 , 43.54 , 55.91 , 42.58 , 46.39 , 53.77 , 60.31 , 42.50 , 44.84 , 47.86 , 53.01 , 38.61 , 68.58 , 55.88 , 41.50 , 35.53 , 46.39 , 33.63 , 40.40
                        };*/


/*LED stuff*/                         //Stores the fixed color for each LED
int Rfix[LED_COUNT];
int Gfix[LED_COUNT];
int Bfix[LED_COUNT];


/*Multicore Computing*/
TaskHandle_t Task1, Task2;
float CorePeriod = 1000000 / fps; //time a core has to do its Computation in us
bool checker;


void setup()
{

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(255);
  FastLED.show();

  Serial.begin(115200);
  Serial.println("Ready");


  /*designating Frequency for LEDs*/
  //BucketFrequencyManual();
  BucketFrequencyFunction();

  /*aligning FFT results to LED Frequency*/
  vRealtoBucket();

  /*Setting LED colors to corresponding Frequency*/
  LEDColor();

  /*Multicore Computing*/
  xTaskCreatePinnedToCore(
    CodeCore0,
    "Sampling_Math_Drawing",
    1000,
    NULL,
    1,
    &Task1,
    0);

  xTaskCreatePinnedToCore(
    CodeCore1,
    "FFT",
    1000,
    NULL,
    1,
    &Task2,
    1);
}




void CodeCore0(void * parameter ) {
  //Code needs ~29800us
  for (;;) {

    //waiting for Core 1
    while (checker) {
      yield();
    } checker = 1;

    //Buckets ~2200us
    startbuckets();

    //FastLED ~2350us
    startdrawing();

    //Build raw data ~26600us
    startsampling();

  }
}


void CodeCore1(void * parameter ) {
  // Code needs ~30850us
  for (;;) {

    TimingOld = micros();

    //Getting Data from Buffer
    for (uint16_t i = 0; i < samples; i++) {
      Real[i] = vRealBuffer[i];
      Imag[i] = 0;
    }

    //Performing FFT ~30700us
    {
      //DCRemoval ~700us
      FFT.DCRemoval(Real, samples);

      //Windowing ~7150us
      FFT.Windowing(Real, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);

      //Compute ~20150us
      FFT.Compute(Real, Imag, samples, FFT_FORWARD);

      //Complex to Magnitude ~2700us
      FFT.ComplexToMagnitude(Real, Imag, samples / 2);

      Real[0] = Real[1];
    }
    //Writing Data to Buffer
    for (uint16_t i = 0; i < samples / 2; i++) {
      vFFT[i] = Real[i];
    }

    //Delaying next Cycle to match fps setting
    TimingNew = micros();
    while (TimingNew - TimingOld < CorePeriod) {
      TimingNew = micros();
    }
    checker = 0;
  }
}


void startsampling() {
  for (uint16_t i = 0; i < samples; i++) {

    oldTime = micros();
    u = analogRead(Audio);
    vReal[i] = int8_t((amplitude * u / 4096) / 2.0);

    vRealBuffer[i] = vReal[i];
    if (i < LED_COUNT) {
      BucketsOld[i] = Buckets[i];
    }

    newTime = micros();
    while ((newTime - oldTime) < Period) {
      newTime = micros();
    }
  }
}

void startdrawing() {

  float R = 0, G = 0, B = 0;

  for (uint8_t Position = 0; Position < LED_COUNT; Position++) {

    R = Rfix[Position] / 10 * (Buckets[Position] / 11) + Rfix[Position] / 25;
    G = Gfix[Position] / 10 * (Buckets[Position] / 11) + Gfix[Position] / 25;
    B = Bfix[Position] / 10 * (Buckets[Position] / 11) + Bfix[Position] / 25;



    if (R > 255) {
      R = 255;
    }
    if (G > 255) {
      G = 255;
    }
    if (B > 255) {
      B = 255;
    }

    leds[Position] = CRGB(R, G, B);
  }

  FastLED.show();
}

void startbuckets() {

  //reseting Buckets
  for (int i = 0; i < LED_COUNT; i++) {
    Buckets[i] = 0;
  }
  //filling buckets
  for (int n = 0; n < samples / 2; n++) {
    Buckets[smallestPosition[n]] += (100 * vFFT[n] / BucketFFTamplitude[n]);
  }

  //normalising Buckets
  for (int i = 0; i < LED_COUNT; i++) {

    Buckets[i] = Buckets[i] / Bucketentries[i];
    //Buckets[i] = 100 * Buckets[i] / BucketAmplitude[i];
    /*if (Buckets[i] > 10) {
      Buckets[i] = 100 * (log10(Buckets[i]) - 1);
      }*/

    if (Buckets[i] > 100) {
      Buckets[i] = 100;
    }
  }

  double Bucketsbuffer[LED_COUNT];
  for (int i = 0; i < LED_COUNT; i++) {
    Bucketsbuffer[i] = Buckets[i];
  }
  for (int i = LED_COUNT / 2; i < LED_COUNT - 1; i++) {
    if (Buckets[i] > Bucketsbuffer[i - 1] && Buckets[i] > Bucketsbuffer[i + 1]) {
      Buckets[i - 1] /= 2;
      Buckets[i] *= 1.2;
      if (Buckets[i] > 100) {
        Buckets[i] = 100;
      }
      //Buckets[i] = 100 * (log10(Buckets[i]) - 1);
      Buckets[i + 1] /= 2;
    }
  }

  for (int i = 0; i < LED_COUNT; i++) {
    if (Buckets[i] < BucketsOld[i]) {
      Buckets[i] = (Buckets[i] + 2 * BucketsOld[i]) / 3;
    }
  }
}

void BucketFrequencyFunction() {

#define Zeile 6
#define Spalte 7
  double Matrix[Zeile][Spalte] = { {1, 1, 1, 1, 1, 1, 39},
    {pow(59, 5), pow(59, 4), pow(59, 3), pow(59, 2), 59, 1, 12000},
    {pow(33, 5), pow(33, 4),  pow(33, 3), pow(33, 2), 33, 1,  4000},
    {pow(50, 5), pow(50, 4), pow(50, 3), pow(50, 2), 50, 1,  8000},
    {pow(64, 5), pow(64, 4), pow(64, 3), pow(64, 2), 64, 1, 16000},
    {80, 32, 12, 4, 1, 0, 39}
  };
  double a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
  float x = 0;

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

  for (int i = 1; i <= LED_COUNT; i++) {
    BucketFrequency[i - 1] = (a * pow(i, 5)) + (b * pow(i, 4)) + (c * pow(i, 3)) + (d * pow(i, 2)) + (e * i) + f;
  }
}

void BucketFrequencyManual() {

  int startingFrequency = lowestband;
  int delta = 7;

  for (int i = 0; i < LED_COUNT; i++) {

    if (i == 0) {
      BucketFrequency[i] = startingFrequency;
    }
    else {
      BucketFrequency[i] = BucketFrequency[i - 1] + startingFrequency;
    }
    startingFrequency = startingFrequency + delta;
  }
}

void vRealtoBucket() {

  for (uint16_t i = 0; i < samples / 2; i++) {

    int Diff[LED_COUNT];
    int smallest;
    double abscissa = ((i * 1.0 * samplingFrequency) / samples);
    for (int j = 0; j < LED_COUNT; j++) {
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
    }
  }

  for (int n = 0; n < LED_COUNT; n++) {
    for (int i = 0; i < samples / 2; i++) {
      if (smallestPosition[i] == n) {
        Bucketentries[n]++;
      }
    }
  }
}

void LEDColor() {

  int quarter = BucketFrequency[LED_COUNT - 1] / 4;
  int sub[3];
  for (int i = 0; i < LED_COUNT; i++) {
    if (BucketFrequency[i] <= quarter) {
      Rfix[i] = 255;
      Gfix[i] = 255 * BucketFrequency[i] / quarter;
      Bfix[i] = 0;
      sub[0] = BucketFrequency[i];
    }
    else if (BucketFrequency[i] <= 2 * quarter && BucketFrequency[i] > quarter) {
      Rfix[i] = 255 - 255 * (BucketFrequency[i] - sub[0]) / quarter;
      Gfix[i] = 255;
      Bfix[i] = 0;
      sub[1] = BucketFrequency[i];
    }
    else if (BucketFrequency[i] <= 3 * quarter && BucketFrequency[i] > 2 * quarter) {
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
}

void loop()
{
  delay(1);
}
