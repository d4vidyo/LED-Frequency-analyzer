void setup() {
  Serial.begin(115200);

}

void loop() {
  int BucketFrequency[64];
  int startingFrequency=20;
  int delta=7;
  for(int i=0; i<64; i++){

    startingFrequency=startingFrequency + delta;
    if(i==0){
      BucketFrequency[i]=startingFrequency;
      }
    else{
      BucketFrequency[i]=BucketFrequency[i-1] + startingFrequency;
    }
  }
  for(int i=0; i<64;i++){
  Serial.print("Bucket ");
  Serial.print(i);
  Serial.print(" = "); 
  Serial.print(BucketFrequency[i]);
  Serial.println("hz"); 
    
  }
  delay(10000);

}
