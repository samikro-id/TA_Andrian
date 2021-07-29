/*************************************************************
 * @file        : TA_Andrian.ino
 * @brief       : UNO
 * @revision    : 0-0   
 * **********************************************************/

/************************ Includes **************************/
#include <SoftwareSerial.h>
#include <PZEM004Tv30.h>            // Install library PZEM004Tv30  version 1.1.2

#include <OneWire.h>                // Install library OneWire  version 2.3.5
#include <DallasTemperature.h>      // Install library DallasTemperature    version 3.9.0

/************************ Defines  **************************/
#define PZEM_INPUT_TX_PIN   2
#define PZEM_INPUT_RX_PIN   3

#define PZEM_OUTPUT_TX_PIN  5
#define PZEM_OUTPUT_RX_PIN  6

#define SUHU_DATA_PIN       4

#define RELAY_1_PIN         7
#define RELAY_2_PIN         8
#define RELAY_3_PIN         9
#define RELAY_4_PIN         10

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
}PzemData;

typedef struct{
    float suhu;
    PzemData input;
    PzemData output;
}SensorData;

/*********************** Variables **************************/
SensorData sensorData;

/************************  Setup  ***************************/
void setup(){
    Serial.begin(9600);

    sensorSuhu.begin();
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
    PzemData pzem;
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
    PzemData pzem;
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
