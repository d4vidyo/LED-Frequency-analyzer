//FFT
#include "arduinoFFT.h">
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
#define fps 60                        //Sets max. fps 
//will go as fast as it can if set to high

//Neopixel
#include <FastLED.h>
#define LED_COUNT 64
#define LED_PIN 2
CRGB leds[LED_COUNT];

//Audio
#define Audio 34                      //Pin used to sample Audio

/*
  These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 1024; //This value MUST ALWAYS be a power of 2
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
float Period = 1000000 / samplingFrequency;   //Time betweet each sampling reading in microseconds

/*Bucket Variables*/
double Buckets[LED_COUNT];            //Stores the Amplitude of all Frequencys
double BucketsOld[LED_COUNT];         //Stores the Amplitude of all Frequencys from last frame
int BucketFrequency[LED_COUNT];       //Stores the Frequency designatet to each Buckets[] entry
int smallestPosition[samples / 2];    //Stores which Buckets[] variable is closest to vReal[] variable
int Bucketentries[LED_COUNT];         //Stores how many vReal[] variables are put into one Buckets[] variable

                                      //Values which corespond to 100% of a Frequency
float BucketAmplitude[LED_COUNT] = {6021.28, 6021.28, 5350.83, 4720.76, 4164.83, 3702.91, 3336.08, 3048.91,
                                    2818.86, 2626.81, 2461.88, 2318.82, 2192.6, 2075.32, 1957.44, 1831.19,
                                    1693.33, 1545.88, 1394.95, 1248.68, 1115.16, 1000.76, 908.93, 839.61,
                                    789.76, 754.68, 729.84, 712.02, 699.31, 690.18, 682.77, 674.82, 664.35,
                                    650.2, 632.08, 610.18, 584.9, 556.88, 527.35, 498.25, 471.98, 450.6,
                                    434.95, 424.09, 415.55, 406.22, 393.73, 377.3, 358.08, 338.48, 321.24,
                                    308.32, 300.23, 295.87, 292.92, 288.6, 280.5, 267.15, 248.11, 223.72,
                                    194.66, 161.74, 125.96, 88.61
                                   };
                                 
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
    }
    checker = 1;

    //Buckets ~800us
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

    leds[LED_COUNT - 1 - Position] = CRGB(R, G, B);
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
    Buckets[smallestPosition[n]] += vFFT[n];
  }

  //normalising Buckets
  for (int i = 0; i < LED_COUNT; i++) {

    Buckets[i] = Buckets[i] / Bucketentries[i];
    Buckets[i] = 100 * Buckets[i] / BucketAmplitude[i];
    /*if (Buckets[i] > 10) {
      Buckets[i] = 100 * (log10(Buckets[i]) - 1);
      }*/
    if (Buckets[i] > 100) {
      Buckets[i] = 100;
    }

  }


  for ( int i = 0; i < LED_COUNT; i++) {
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
