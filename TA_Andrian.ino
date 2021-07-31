/*************************************************************
 * @file        : TA_Andrian.ino
 * @brief       : UNO
 * @revision    : 0.0.0
 * @environment : Arduino 1.8.13
 * **********************************************************/

/************************ Includes **************************/
#include <SoftwareSerial.h>
#include <PZEM004Tv30.h>            // Install library PZEM004Tv30  version 1.1.2

#include <OneWire.h>                // Install library OneWire  version 2.3.5
#include <DallasTemperature.h>      // Install library DallasTemperature    version 3.9.0

/************************ Defines  **************************/
#define PZEM_INPUT_TX_PIN   3
#define PZEM_INPUT_RX_PIN   2

#define PZEM_OUTPUT_TX_PIN  7
#define PZEM_OUTPUT_RX_PIN  6

#define SUHU_DATA_PIN       5

#define RELAY_1_PIN         13
#define RELAY_2_PIN         12
#define RELAY_3_PIN         11
#define RELAY_4_PIN         10

#define RELAY_SUMBER        RELAY_4_PIN
#define RELAY_PROTEKSI      RELAY_1_PIN
#define RELAY_SUHU          RELAY_2_PIN
#define RELAY_FAN           RELAY_3_PIN

#define RELAY_ON            LOW
#define RELAY_OFF           HIGH

/************************ Macros ***************************/
SoftwareSerial serialPzemInput(PZEM_INPUT_RX_PIN, PZEM_INPUT_TX_PIN);
SoftwareSerial serialPzemOutput(PZEM_OUTPUT_RX_PIN, PZEM_OUTPUT_TX_PIN);

PZEM004Tv30 pzemInput(serialPzemInput);
PZEM004Tv30 pzemOutput(serialPzemOutput);

OneWire oneWire(SUHU_DATA_PIN);
DallasTemperature sensorSuhu(&oneWire);

/************************ Structs ***************************/
typedef struct{
    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float pf;
}PzemData_Struct;

typedef struct{
    float suhu;
    PzemData_Struct input;
    PzemData_Struct output;
}SensorData_Struct;

/*********************** Variables **************************/
SensorData_Struct sensorData;

/************************  Setup  ***************************/
void setup(){
    Serial.begin(9600);
    
    sensorSuhu.begin();

    relayInit();
    setSumber(true);                // true = sumber PLN, false = sumber PLTPH
    setProteksi(false);             // true = proteksi ON, false = proteksi OFF
    setFan(false);                  // true = FAN ON, false = FAN OFF

}

/************************   Loop   **************************/
void loop(){
    if(readPzemInput()){
        
    }else{
        
    }

    if(readPzemOutput()){
        
    }else{

    }

    readSuhu();

    delay(2000);
}

bool readPzemInput(){
    PzemData_Struct pzem;
    bool valid = false;
    
    pzem.voltage    = pzemInput.voltage();
    pzem.current    = pzemInput.current();
    pzem.power      = pzemInput.power();
    pzem.energy     = pzemInput.energy();
    pzem.frequency  = pzemInput.frequency();
    pzem.pf         = pzemInput.pf();

    if(isnan(pzem.voltage))         valid = false;
    else if(isnan(pzem.current))    valid = false;
    else if(isnan(pzem.power))      valid = false;
    else if(isnan(pzem.energy))     valid = false;
    else if(isnan(pzem.frequency))  valid = false;
    else if(isnan(pzem.pf))         valid = false;
    else{
        sensorData.input.voltage    = pzem.voltage;
        sensorData.input.current    = pzem.current;
        sensorData.input.power      = pzem.power;
        sensorData.input.energy     = pzem.energy;
        sensorData.input.frequency  = pzem.frequency;
        sensorData.input.pf         = pzem.pf;

        valid = true;
    }

    if(valid){
        Serial.println("------------ INPUT -----------");
        Serial.print(sensorData.input.voltage, 2);      Serial.println("V");
        Serial.print(sensorData.input.current, 3);      Serial.println("A");
        Serial.print(sensorData.input.power, 2);        Serial.println("W");
        Serial.print(sensorData.input.energy, 2);       Serial.println("Wh");
        Serial.print(sensorData.input.frequency, 2);    Serial.println("Hz");
        Serial.print("PF : ");                          Serial.println(sensorData.input.pf, 3);
    }else{
        Serial.println("-- READ INPUT FAILED --");
    }
    Serial.println();

    return valid;
}

bool readPzemOutput(){
    PzemData_Struct pzem;
    bool valid = false;
    
    pzem.voltage    = pzemOutput.voltage();
    pzem.current    = pzemOutput.current();
    pzem.power      = pzemOutput.power();
    pzem.energy     = pzemOutput.energy();
    pzem.frequency  = pzemOutput.frequency();
    pzem.pf         = pzemOutput.pf();

    if(isnan(pzem.voltage))         valid = false;
    else if(isnan(pzem.current))    valid = false;
    else if(isnan(pzem.power))      valid = false;
    else if(isnan(pzem.energy))     valid = false;
    else if(isnan(pzem.frequency))  valid = false;
    else if(isnan(pzem.pf))         valid = false;
    else{
        sensorData.output.voltage    = pzem.voltage;
        sensorData.output.current    = pzem.current;
        sensorData.output.power      = pzem.power;
        sensorData.output.energy     = pzem.energy;
        sensorData.output.frequency  = pzem.frequency;
        sensorData.output.pf         = pzem.pf;

        valid = true;
    }

    if(valid){
        Serial.println("------------ OUTPUT -----------");
        Serial.print(sensorData.output.voltage, 2);     Serial.println("V");
        Serial.print(sensorData.output.current, 3);     Serial.println("A");
        Serial.print(sensorData.output.power, 2);       Serial.println("W");
        Serial.print(sensorData.output.energy, 2);      Serial.println("Wh");
        Serial.print(sensorData.output.frequency, 2);   Serial.println("Hz");
        Serial.print("PF : ");                          Serial.println(sensorData.output.pf, 3);
    }else{
        Serial.println("-- READ OUTPUT FAILED --");
    }
    Serial.println();

    return valid;
}

void readSuhu(){
    sensorSuhu.requestTemperatures();
    sensorData.suhu = sensorSuhu.getTempCByIndex(0);

    Serial.println("--------------- SUHU --------------");
    Serial.print("Suhu : ");        Serial.print(sensorData.suhu, 2);    Serial.println(" C");
    Serial.println();
}

void relayInit(){
    pinMode(RELAY_1_PIN, OUTPUT);       digitalWrite(RELAY_1_PIN, RELAY_OFF);
    pinMode(RELAY_2_PIN, OUTPUT);       digitalWrite(RELAY_2_PIN, RELAY_OFF);
    pinMode(RELAY_3_PIN, OUTPUT);       digitalWrite(RELAY_3_PIN, RELAY_OFF);
    pinMode(RELAY_4_PIN, OUTPUT);       digitalWrite(RELAY_4_PIN, RELAY_OFF);
}

void setSumber(bool pln){
    if(pln){    digitalWrite(RELAY_SUMBER, RELAY_OFF);  }   // jika true, Matikan Relay Sumber untuk menggunakan sumber PLN ke LOAD
    else{       digitalWrite(RELAY_SUMBER, RELAY_ON);   }   // jika false, Nyalakn RElay Sumber untuk menggunakan sumber PLTPH ke LOAD
}

void setProteksi(bool protek){
    if(protek){ digitalWrite(RELAY_PROTEKSI, RELAY_ON); }   // jika true, Nyalakan Relay Proteksi untuk memutus LOAD
    else{       digitalWrite(RELAY_PROTEKSI, RELAY_OFF);}   // jika false, Matikan Relay Proteksi untuk menyambungkan LOAD
}

void setFan(bool on){
    if(on){ digitalWrite(RELAY_FAN, RELAY_ON);  }       // jika true, nyalakan FAN
    else{   digitalWrite(RELAY_FAN, RELAY_OFF); }       // jika false, matikan FAN
}