#include <ADXL345.h>
#include <ADXL345.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
ADXL345 accelerometer;
File main_folder;
File dataFile; 
const int chipSelect = 4; 
int prev_file_indx = 0;
String fileName = "000";
// measurements taken at r_0 (sea level)
const float station_elev = 11.0;
const float p_0 = 1017.3*100.0;

Adafruit_BME280 bme; // start BME sensor

void setup(void) {
  Serial.begin(9600); 
    
  Serial.println("Initialize L3G4200D");
    if (!accelerometer.begin())
  {
    Serial.println("Could not find a valid ADXL345 sensor, check wiring!");
    delay(500);
  }
     // Values for Free Fall detection
  accelerometer.setFreeFallThreshold(0.35); 
  accelerometer.setFreeFallDuration(0.1);  
    // Select INT 1 for get activities
  accelerometer.useInterrupt(ADXL345_INT1);
    checkSetup();
    bool status;
    status = bme.begin();  
    if (!status) {
        while (1);
    }
    // verify SD card is working
    if (!SD.begin(chipSelect)) {
      return;
    }
    pinMode(10,OUTPUT);
    pinMode(8,OUTPUT);
    main_folder = SD.open("/");
    fileName = sd_saver(main_folder);
}
void checkSetup()
{
  Serial.print("Free Fall Threshold = "); Serial.println(accelerometer.getFreeFallThreshold());
  Serial.print("Free Fall Duration = "); Serial.println(accelerometer.getFreeFallDuration());
}

void loop(void) { 
  delay(50);
    
  Vector norm = accelerometer.readNormalize();

  
  Activites activ = accelerometer.readActivites();
 
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println("*C");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println("hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(p_0/100.0));
  Serial.println("m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println("%");

  Serial.println();
  delay(1000);
  // SD save section
  String data_array = "";
  data_array += String(millis()); 
  data_array += ",";
  data_array += String(bme.readTemperature()); // save temp
  data_array += ",";
  data_array += String(bme.readHumidity()); // save humidity
  data_array += ",";
  data_array += String(bme.readPressure()); // save pressure in Pa
  data_array += ",";
  data_array += String(bme.readAltitude(p_0/100.0)+station_elev); 

  // SD Card writing and saving  
  dataFile = SD.open("DATA"+fileName+".csv",FILE_WRITE);
 
  if(dataFile){
    dataFile.println(data_array);
    dataFile.close();
  }
        if ((bme.readAltitude(p_0/100.0)+ 10 <= 100) && (activ.isFreeFall)){   // ejecting altitude(METER)
        digitalWrite(10,HIGH);                  //Drogue parachute deployment
        } 
      if ((bme.readAltitude(p_0/100.0)<= 50)){   // ejecting altitude(METER)
        digitalWrite(8,HIGH);                  //Main parachute deployment
        } 
  delay(100);
}

String sd_saver(File dir){
  while(true){
    
    File entry = dir.openNextFile();
    if (!entry){
      break;
    }
   
    String entry_name = entry.name();
    if ((entry_name.substring(4,7)).toInt()>=prev_file_indx){
      prev_file_indx = (entry_name.substring(4,7)).toInt()+1;
      if (prev_file_indx>=100){
        fileName = String(prev_file_indx);
      } else if (prev_file_indx>=10){
        fileName = "0"+String(prev_file_indx);
      } else{
        fileName = "00"+String(prev_file_indx);
      }

    }
    entry.close();
  }
  return fileName;
}
