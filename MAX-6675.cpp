// IMPLEMENTAÇÃO MAX6675

#include "max6675.h" 

#define pinSO  19   //PINO DIGITAL (SO)
#define pinCS  16   //PINO DIGITAL (CS)
#define pinCLK 18  //PINO DIGITAL (CLK / SCK)

MAX6675 sensorTemp(pinCLK, pinCS, pinSO); 
  
void setup(){
  Serial.begin(115200); 
  delay(100); 
}

void loop(){
   Serial.print(sensorTemp.readCelsius()); 
   Serial.println("C"); 
   delay(5000); 
}
