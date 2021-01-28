/*

	Example of use of the FFT libray
        Copyright (C) 2014 Enrique Condes

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
  In this example, the Arduino simulates the sampling of a sinusoidal 1000 Hz
  signal with an amplitude of 100, sampled at 5000 Hz. Samples are stored
  inside the vReal array. The samples are windowed according to Hamming
  function. The FFT is computed using the windowed samples. Then the magnitudes
  of each of the frequencies that compose the signal are calculated. Finally,
  the frequency with the highest peak is obtained, being that the main frequency
  present in the signal.
*/
//FastFourierTransformation
#include "arduinoFFT.h">

//Neopixel
#include <FastLED.h>

#define LED_COUNT 64
#define LED_PIN 2

CRGB leds[LED_COUNT];

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 1024; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 40000;
const uint8_t amplitude = 100;
const uint8_t lowestband = 20;
const uint8_t highestband = samplingFrequency/2;


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
float Period = 1000000/samplingFrequency;

/* Timing Stuff*/
long TimingOld[6], TimingNew[6];
long oldTime, newTime;

/*LED stuff*/
double Buckets[LED_COUNT];
double BucketsOld[LED_COUNT];
float R=0,G=0,B=0;
int smallestPosition[samples/2];
int freque[LED_COUNT];
int Bucketentries[LED_COUNT];
double middleFrequencys[LED_COUNT];
float Maximum[LED_COUNT];

/*OLD
float BucketAmplitude[64]={396.16, 834.62, 859.69, 714.80, 857.92, 712.57, 535.65, 711.94, 538.18, 538.56, 537.72, 538.66, 409.39, 410.74, 407.79, 409.39, 335.76, 337.05, 336.31, 335.80, 341.94,
                          294.75, 291.30, 305.22, 293.71, 254.86, 252.62, 257.29, 254.26, 234.62, 245.58, 234.68, 247.55, 241.24, 228.47, 239.32, 226.76, 223.12, 204.49, 200.59, 223.26, 207.82,
                          190.25, 197.32, 205.85, 206.74, 204.74, 202.58, 188.04, 192.82, 196.43, 187.38, 185.68, 188.67, 188.73, 182.76, 186.18, 178.90, 165.88, 193.28, 169.95, 171.03, 174.24, 296.27};
*/

/*NEW*/
float BucketAmplitude[64]={5930.33, 5061.44, 4989.55, 4642.48, 8106.07, 4803.11, 7438.56, 7596.22, 7637.06, 8922.90, 7635.20, 7545.49, 7855.98, 7265.73, 7284.55, 7954.88, 6967.67, 6533.06, 7974.01,
                          5721.41, 6453.77, 5743.20, 4836.87, 8339.20, 4494.45, 3823.16, 4599.52, 3524.77, 3769.01, 4222.47, 4285.72, 4053.66, 3550.06, 3040.68, 3758.38, 3141.70, 2819.16, 2852.95,
                          3038.94, 3947.54, 3230.10, 4091.78, 3094.80, 3449.45, 3393.59, 3265.20, 3201.85, 3384.65, 3417.98, 3471.47, 6339.01, 4113.68, 3864.07, 3930.64, 3272.28, 3877.16, 2743.09,
                          2515.11, 2977.76, 3223.32, 2627.26, 2246.24, 2284.94, 13031.66};


/*Multicore Computing*/
TaskHandle_t Task1, Task2;
float CorePeriod = 1000000/fps; //time a core has to do its Computation in us
bool checker[7];


void setup()
{
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(255);
  
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Ready");
  delay(100);



    /*Bucket arrangement*/
    int Diff[64];
    int smallest;
 
    int delta=20;
    int del=7;
    for(int i=0; i<64; i++){
  
      delta=delta + del;
      if(i==0){
        freque[i]=delta;
        }
      else{
        freque[i]=freque[i-1] + delta;
      }
    }
    
    for (uint16_t i = 0; i < samples/2; i++)
    {
      double abscissa = ((i * 1.0 * samplingFrequency) / samples);
      
      for(int j=0; j<64;j++){
        Diff[j]=abscissa - freque[j];   
       
        if (Diff[j] < 0){
          Diff[j]=-Diff[j];
        }
        
        if(j==0){smallest = Diff[j];}
        if(smallest >= Diff[j]){
          smallest = Diff[j];
          smallestPosition[i]=j;
          
        }   
      }
      
    }
    for(int n=0; n<LED_COUNT; n++){
      for(int i=0; i<samples/2; i++){
        if(smallestPosition[i]==n){
          Bucketentries[n]++;
        }
      }
      Bucketentries[n]=Bucketentries[n]*BucketAmplitude[n]/10;
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




void CodeCore0(void * parameter ){

  for(;;){
   
    //waiting for Core 1
    while(checker[6]){
      TimingOld[0]=micros();
    }
    checker[6]=1;
    
    //Buckets ~500us   
    startbuckets();
    
    //FastLED ~2600us 
    startdrawing();
   
    //Build raw data ~26700us        
    startsampling(); 
    
    //Writing Data to Buffer
    for(uint16_t i=0; i<samples; i++){
      vRealBuffer[i]=vReal[i];
    }
    for(uint16_t i=0; i<LED_COUNT; i++){
      BucketsOld[i]=Buckets[i];
    }
    TimingNew[0]=micros();
  }  
}


void CodeCore1(void * parameter ){

  for(;;){
    
    TimingOld[1]=micros();

    //Getting Data from Buffer
    for(uint16_t i=0; i<samples; i++){
      Real[i]=vRealBuffer[i];
      Imag[i]=0;
    }

    //Performing FFT ~32100us
    FFT.Windowing(Real, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  
    FFT.Compute(Real, Imag, samples, FFT_FORWARD); 
    FFT.ComplexToMagnitude(Real, Imag, samples); 
    Real[0]=0; 
    Real[1]=0;
    Real[2]=0; 

    //Writing Data to Buffer
    for(uint16_t i=0; i< samples; i++){
      vFFT[i]=Real[i];  
    }  

    //Delaying next Cycle to match fps setting
    TimingNew[1]=micros();
    while(TimingNew[1]-TimingOld[1] < CorePeriod){
      TimingNew[1]=micros();
    }checker[6]=0;
  }    
}


void startsampling(){
  for (uint16_t i = 0; i < samples; i++)
      {
        oldTime=micros();
        u[i]=analogRead(Audio);
        
        vReal[i] = int8_t((amplitude * (u[i]-2125)/4096) / 2.0);         
          
        newTime=micros();
        while((newTime-oldTime) < Period){    
        newTime=micros();
        }
      }
}

void startdrawing(){
  for(uint8_t Position = 0; Position < LED_COUNT; Position++){
    
        if(Position < 26){
          R=255;
          G=Position * 100/26;
          B=0;
        }
        if(Position >= 26 && Position < 33){
          R=255;
          G=100 + (Position - 26) * ((255-100)/(33-26));
          B=0;
        }
        if(Position >=33 && Position < 48){
          R=255 - (Position -33) * (255/(48-33));
          G=255;
          B=0;
        }
        if(Position >=48 && Position < 56){
          R=0;
          G=255;
          B=(Position-48) * (255/(56-48));
        }
        if(Position >=56 && Position <64){
          R=0;
          G=255 - (Position-56) * (255/(64-56));
          B=255;
        }
    
    
        
        R=R/10 * (Buckets[Position]/11) +R/25;
        G=G/10 * (Buckets[Position]/11) +G/25;
        B=B/10 * (Buckets[Position]/11) +B/25;
        if(R > 255){R=255;}
        if(G > 255){G=255;}
        if(B > 255){B=255;}
        
        leds[LED_COUNT-1-Position] = CRGB(R, G, B);
          
        }
       FastLED.show();
}

void startbuckets(){
  
      //Buckets resetten
      for(int i =0; i<LED_COUNT; i++){
        Buckets[i]=0;
      }
      //Buckets Füllen
      for(int n=0; n< samples/2; n++){
        Buckets[smallestPosition[n]]+=vFFT[n];
      }    
    
      //normalising Buckets
      for(int i = 0; i < LED_COUNT; i++){
        Buckets[i]=Buckets[i]*100/Bucketentries[i];
        if(Buckets[i] < 4){
          Buckets[i] = 0;
        }
        if(Buckets[i] > 10){
          //Buckets[i]=100 * (log10(Buckets[i])-1);
          if(Buckets[i]>80){
            Buckets[i]=80;
          }
          Buckets[i]=exp(4.6 * (Buckets[i]/80));
        }
        
     }
     
     for( int i=0; i<LED_COUNT -1 ;i++){
        if(Buckets[i]<Buckets[i+1]){
          Buckets[i]=(Buckets[i]+Buckets[i+1])/2;
        }        
     }
     for( int i=LED_COUNT; i>0 ;i--){
        if(Buckets[i]<Buckets[i-1]){
          Buckets[i]=(Buckets[i]+Buckets[i-1])/2;
        }        
     }
     
     for( int i=0; i<LED_COUNT; i++){
      if(Buckets[i] < BucketsOld[i]){
          Buckets[i]=(Buckets[i] + 2*BucketsOld[i])/3;
        }
     }
}


void loop()
{
  delay(1);
}
