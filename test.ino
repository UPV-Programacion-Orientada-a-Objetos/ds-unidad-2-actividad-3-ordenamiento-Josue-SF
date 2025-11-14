#define BAUD_RATE 9600

#define DATA_INTERVAL_MS 50

void setup() {
  
  Serial.begin(BAUD_RATE);
  
  
  while (!Serial) {
    ; 
  }

  
  randomSeed(analogRead(A0));

  delay(2000);
}

void loop() {
  long energy_reading = random(0, 65536);
  Serial.println(energy_reading);
  delay(DATA_INTERVAL_MS);
}