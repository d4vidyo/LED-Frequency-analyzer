//FFT
#include "arduinoFFT.h">

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

//Neopixel
#include <FastLED.h>

#define LED_COUNT 64
#define LED_PIN 2

CRGB leds[LED_COUNT];

/*
  These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 1024; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 40000;
const uint8_t amplitude = 100;
const uint8_t lowestband = 20;
const uint8_t highestband = samplingFrequency / 2;


/*These are the input and output vectors
  Input vectors receive computed results from FFT*/
double vReal[samples];
double Real[samples];
volatile double vRealBuffer[samples];
volatile double vFFT[samples];
double Imag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03
#define Audio 34
#define fps 30

/*Input*/
float u[samples];
float Period = 1000000 / samplingFrequency;

/* Timing Stuff*/
long TimingOld[6], TimingNew[6];
long oldTime, newTime;

/*LED stuff*/
double Buckets[LED_COUNT];
double BucketsOld[LED_COUNT];
double BucketsPeak[LED_COUNT];
float R = 0, G = 0, B = 0;
int smallestPosition[samples / 2];
int freque[LED_COUNT];
int Bucketentries[LED_COUNT];
double middleFrequencys[LED_COUNT];
float Maximum[LED_COUNT];

/*OLD
  float BucketAmplitude[64]={396.16, 834.62, 859.69, 714.80, 857.92, 712.57, 535.65, 711.94, 538.18, 538.56, 537.72, 538.66, 409.39, 410.74, 407.79, 409.39, 335.76, 337.05, 336.31, 335.80, 341.94,
                          294.75, 291.30, 305.22, 293.71, 254.86, 252.62, 257.29, 254.26, 234.62, 245.58, 234.68, 247.55, 241.24, 228.47, 239.32, 226.76, 223.12, 204.49, 200.59, 223.26, 207.82,
                          190.25, 197.32, 205.85, 206.74, 204.74, 202.58, 188.04, 192.82, 196.43, 187.38, 185.68, 188.67, 188.73, 182.76, 186.18, 178.90, 165.88, 193.28, 169.95, 171.03, 174.24, 296.27};
*/

/*NEW
  float BucketAmplitude[64] = {5930.33, 5061.44, 4989.55, 4642.48, 8106.07, 4803.11, 7438.56, 7596.22, 7637.06, 8922.90, 7635.20, 7545.49, 7855.98, 7265.73, 7284.55, 7954.88, 6967.67, 6533.06, 7974.01,
                             5721.41, 6453.77, 5743.20, 4836.87, 8339.20, 4494.45, 3823.16, 4599.52, 3524.77, 3769.01, 4222.47, 4285.72, 4053.66, 3550.06, 3040.68, 3758.38, 3141.70, 2819.16, 2852.95,
                             3038.94, 3947.54, 3230.10, 4091.78, 3094.80, 3449.45, 3393.59, 3265.20, 3201.85, 3384.65, 3417.98, 3471.47, 6339.01, 4113.68, 3864.07, 3930.64, 3272.28, 3877.16, 2743.09,
                             2515.11, 2977.76, 3223.32, 2627.26, 2246.24, 2284.94, 13031.66
                            };
*/
/*newNEW
  float BucketAmplitude[64] = {2680.41 , 5008.45 , 4789.11 , 4886.26 , 3795.05 , 4597.22 , 2776.01 , 2206.32 , 3054.51 , 2273.63 , 2607.14 , 1564.74 , 1640.56 , 1651.03 , 1633.64 , 1559.06 , 1753.13 ,
                             978.00 , 1158.34 , 968.60 , 1003.18 , 884.80 , 959.95 , 671.71 , 574.51 , 644.45 , 600.37 , 556.89 , 610.58 , 493.05 , 449.41 , 370.95 , 385.97 , 402.52 , 349.62 , 362.97 ,
                             356.00 , 382.16 , 393.37 , 599.63 , 372.89 , 316.09 , 281.45 , 329.60 , 310.77 , 465.84 , 299.97 , 329.69 , 253.80 , 254.03 , 246.33 , 389.25 , 259.90 , 416.82 , 186.85 , 195.33 ,
                             220.25 , 206.21 , 173.88 , 171.93 , 156.62 , 154.70 , 137.08 , 98.17
                            };
*/
float BucketAmplitude[LED_COUNT];
float BucketNoise[64] = {284.43 , 714.73 , 667.95 , 579.42 , 451.34 , 416.01 , 268.92 , 228.56 , 200.94 , 168.76 , 133.52 , 128.60 , 132.47 , 176.75 , 157.20 , 177.93 , 124.83 , 102.09 , 111.68 , 94.17 ,
                         69.79 , 87.22 , 118.06 , 106.82 , 83.47 , 78.88 , 63.66 , 76.13 , 71.21 , 65.49 , 47.32 , 51.52 , 52.82 , 47.39 , 50.41 , 46.77 , 50.18 , 46.98 , 45.22 , 61.64 , 59.21 , 41.16 ,
                         51.24 , 48.89 , 43.70 , 47.39 , 43.54 , 55.91 , 42.58 , 46.39 , 53.77 , 60.31 , 42.50 , 44.84 , 47.86 , 53.01 , 38.61 , 68.58 , 55.88 , 41.50 , 35.53 , 46.39 , 33.63 , 40.40
                        };




/*Multicore Computing*/
TaskHandle_t Task1, Task2;
float CorePeriod = 1000000 / fps; //time a core has to do its Computation in us
bool checker[7];


void setup()
{
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(255);

  Serial.begin(115200);
  while (!Serial);
  Serial.println("Ready");



  /*BucketAmplitude Calculation*/
  for (int i = 0; i < LED_COUNT; i++) {
    if (i == 0) {
      BucketAmplitude[i] = 5000;
    }
    else {
      int difference = (4900 / log(64)) * (log(i + 1) - log(i));
      BucketAmplitude[i] =  BucketAmplitude[i - 1] - difference;
    }
    Serial.print(BucketAmplitude[i]);
    Serial.print(" , ");
  }
  Serial.println(" ");



  /*Bucket arrangement*/
  int Diff[64];
  int smallest;

  int delta = 27;
  int del = 7;
  for (int i = 0; i < 64; i++) {

    if (i == 0) {
      freque[i] = delta;
    }
    else {
      freque[i] = freque[i - 1] + delta;
    }
    delta = delta + del;
  }

  for (uint16_t i = 0; i < samples / 2; i++)
  {
    double abscissa = ((i * 1.0 * samplingFrequency) / samples);

    for (int j = 0; j < 64; j++) {
      Diff[j] = abscissa - freque[j];

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
    //Bucketentries[n] = Bucketentries[n] * BucketAmplitude[n] / 10;
  }



  for (int i = 0; i < LED_COUNT; i++) {
    BucketsPeak[i] = 0;
  }

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

  for (;;) {

    //waiting for Core 1
    while (checker[6]) {
      TimingOld[0] = micros();
    }
    checker[6] = 1;

    //Buckets ~500us
    startbuckets();

    //FastLED ~2600us
    startdrawing();

    //Build raw data ~26700us
    startsampling();

    TimingNew[0] = micros();
  }
}


void CodeCore1(void * parameter ) {

  for (;;) {

    TimingOld[1] = micros();

    //Getting Data from Buffer
    for (uint16_t i = 0; i < samples; i++) {
      Real[i] = vRealBuffer[i];
      Imag[i] = 0;
    }

    //Performing FFT ~32100us
    FFT.DCRemoval(Real, samples);
    FFT.Windowing(Real, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(Real, Imag, samples, FFT_FORWARD);
    FFT.ComplexToMagnitude(Real, Imag, samples);
    Real[0] = 0;

    //Writing Data to Buffer
    for (uint16_t i = 0; i < samples / 2; i++) {
      vFFT[i] = Real[i];
    }

    //Delaying next Cycle to match fps setting
    TimingNew[1] = micros();
    while (TimingNew[1] - TimingOld[1] < CorePeriod) {
      TimingNew[1] = micros();
    } checker[6] = 0;
  }
}


void startsampling() {
  for (uint16_t i = 0; i < samples; i++)
  {
    oldTime = micros();

    u[i] = analogRead(Audio);
    vReal[i] = int8_t((amplitude * (u[i]) / 4096) / 2.0);

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
  for (uint8_t Position = 0; Position < LED_COUNT; Position++) {

    if (Position < 26) {
      R = 255;
      G = Position * 100 / 26;
      B = 0;
    }
    if (Position >= 26 && Position < 33) {
      R = 255;
      G = 100 + (Position - 26) * ((255 - 100) / (33 - 26));
      B = 0;
    }
    if (Position >= 33 && Position < 48) {
      R = 255 - (Position - 33) * (255 / (48 - 33));
      G = 255;
      B = 0;
    }
    if (Position >= 48 && Position < 56) {
      R = 0;
      G = 255;
      B = (Position - 48) * (255 / (56 - 48));
    }
    if (Position >= 56 && Position < 64) {
      R = 0;
      G = 255 - (Position - 56) * (255 / (64 - 56));
      B = 255;
    }



    R = R / 10 * (Buckets[Position] / 11) + R / 25;
    G = G / 10 * (Buckets[Position] / 11) + G / 25;
    B = B / 10 * (Buckets[Position] / 11) + B / 25;
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

  //Buckets resetten
  for (int i = 0; i < LED_COUNT; i++) {
    Buckets[i] = 0;
  }
  //Buckets Füllen
  for (int n = 0; n < samples / 2; n++) {
    Buckets[smallestPosition[n]] += vFFT[n];
  }



  //Serial.println(" ");
  //normalising Buckets
  for (int i = 0; i < LED_COUNT; i++) {
    //Buckets[i] = Buckets[i] * 100 / Bucketentries[i];
    Buckets[i] = Buckets[i] / Bucketentries[i];

    if (Buckets[i] < BucketNoise[i] * 1.1) {
      Buckets[i] = 0;
    }
    if (BucketsPeak[i] < Buckets[i]) {
      BucketsPeak[i] = Buckets[i];
    }
    Buckets[i] = 100 * Buckets[i] / BucketAmplitude[i];
    //Serial.print(Buckets[i]);
    //Serial.print(" , ");
    if (Buckets[i] > 10) {
      //Buckets[i]=100 * (log10(Buckets[i])-1);
      if (Buckets[i] > 100) {
        Buckets[i] = 100;
      }
      //Buckets[i] = exp(4.6 * (Buckets[i] / 100));
    }

  }

  for ( int i = 0; i < LED_COUNT - 1 ; i++) {
    if (Buckets[i] < Buckets[i + 1]) {
      Buckets[i] = (Buckets[i] + Buckets[i + 1]) / 2;
    }
  }
  for ( int i = LED_COUNT; i > 0 ; i--) {
    if (Buckets[i] < Buckets[i - 1]) {
      Buckets[i] = (Buckets[i] + Buckets[i - 1]) / 2;
    }
  }

  for ( int i = 0; i < LED_COUNT; i++) {
    if (Buckets[i] < BucketsOld[i]) {
      Buckets[i] = (Buckets[i] + 2 * BucketsOld[i]) / 3;
    }
  }
}


void loop()
{
  delay(1);
}
