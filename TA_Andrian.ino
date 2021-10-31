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
#define SERIAL_SPEED        9600
#define SERIAL_LEN          400

#define PZEM_INPUT_TX_PIN   3
#define PZEM_INPUT_RX_PIN   2

#define PZEM_OUTPUT_TX_PIN  7
#define PZEM_OUTPUT_RX_PIN  6

#define SUHU_DATA_PIN       5

#define FAN_DATA_PIN        A4

#define RELAY_1_PIN         9
#define RELAY_2_PIN         12
#define RELAY_3_PIN         11
#define RELAY_4_PIN         10

#define RELAY_SUMBER        RELAY_4_PIN
#define RELAY_PROTEKSI      RELAY_1_PIN
#define RELAY_SUHU          RELAY_2_PIN
#define RELAY_INDICATOR     RELAY_3_PIN

#define RELAY_ON            LOW
#define RELAY_OFF           HIGH

#define UPDATE_SENSOR_TIME  1000

#define SUHU_HIGH           40

#define VOLTAGE_THRESHOLD   210
#define CURRENT_THRESHOLD   0.5

/************************ Macros ****************************/
SoftwareSerial serialPzemInput(PZEM_INPUT_RX_PIN, PZEM_INPUT_TX_PIN);
SoftwareSerial serialPzemOutput(PZEM_OUTPUT_RX_PIN, PZEM_OUTPUT_TX_PIN);

PZEM004Tv30 pzemInput(serialPzemInput);
PZEM004Tv30 pzemOutput(serialPzemOutput);

OneWire oneWire(SUHU_DATA_PIN);
DallasTemperature sensorSuhu(&oneWire);

/************************ Structs ***************************/
typedef struct{
    float voltage = 0.0;
    float current = 0.0;
    float power = 0.0;
    float energy = 0.0;
    float frequency = 0.0;
    float pf = 0.0;
}PzemData_Struct;

typedef struct{
    float suhu;
    PzemData_Struct input;
    PzemData_Struct output;
}SensorData_Struct;

/*********************** Variables **************************/
String serial_buff;
bool serial_complete        = false;

SensorData_Struct sensorData;

uint32_t led_time;
uint32_t sensor_time;

bool source_pln;
bool proteksi_on;
bool relay_suhu_on;
bool voltage_low;

bool led_state;

/************************  Setup  ***************************/
void setup(){
    delay(300);

    Serial.begin(SERIAL_SPEED);
    serial_buff.reserve(SERIAL_LEN);

    sensorSuhu.begin();

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(FAN_DATA_PIN, INPUT);
    relayInit();
    setSumber(true);                // true = sumber PLN, false = sumber PLTPH
    setProteksi(false);             // true = proteksi ON, false = proteksi OFF
    setRelaySuhu(false);            // 

    led_time = millis();
    sensor_time = millis();

}

/************************   Loop   **************************/
void loop(){
    if((millis() - led_time) > 200){
        led_time = millis();

        toggleLed();
    }

    if((millis() - sensor_time) > UPDATE_SENSOR_TIME){
        sensor_time = millis();
        
        readPzemInput();
        readPzemOutput();

        readSuhu();

        checkProtection();
    }

    if(serial_complete){
        prosesData();

        serial_complete = false;
    }
}

void checkProtection(){
    if(sensorData.suhu > SUHU_HIGH){    setRelaySuhu(true); }
    else{                               setRelaySuhu(false);}

    if(sensorData.input.voltage < VOLTAGE_THRESHOLD){
        voltage_low = true;
    }else{
        voltage_low = false;
    }

    if(sensorData.output.current > CURRENT_THRESHOLD){
        setProteksi(true);
    }
}

void prosesData(){
    int index, index2, index3;

    index = serial_buff.indexOf("|");

    if(serial_buff.substring(0, index) == "GET"){
        index++;
        if(serial_buff.substring(index) == "DATA"){
            serial_buff = "";
            serial_buff = "DATA|" + String(sensorData.suhu, 0)
                        + "|" + String(sensorData.input.voltage, 1) 
                        + "|" + String(sensorData.input.current, 3)
                        + "|" + String(sensorData.input.power, 1)
                        + "|" + String(sensorData.input.frequency)
                        + "|" + String(sensorData.input.pf, 3)
                        + "|" + String(sensorData.input.energy, 1)

                        + "|" + String(sensorData.output.voltage, 1) 
                        + "|" + String(sensorData.output.current, 3)
                        + "|" + String(sensorData.output.power, 1)
                        + "|" + String(sensorData.output.frequency, 1)
                        + "|" + String(sensorData.output.pf, 3)
                        + "|" + String(sensorData.output.energy, 1)
                        + "|" + String(source_pln)
                        + "|" + String(proteksi_on)
                        + "|" + String(getFan())
                        + "|" + String(relay_suhu_on)
                        + "|" + String(voltage_low)
                        ;

            Serial.println(serial_buff);
        }
        else{

        }
    }
    else if(serial_buff.substring(0, index) == "SET"){
        index++;
        index2 = serial_buff.indexOf("|", index);

        if(serial_buff.substring(index, index2) == "SOURCE"){
            index2++;
            if(serial_buff.substring(index2) == "PLN"){
                setSumber(true);

                Serial.println(serial_buff.substring(index));
            }
            else if(serial_buff.substring(index2) == "PLTPH"){
                setSumber(false);
                Serial.println(serial_buff.substring(index));
            }          
        }
        else if(serial_buff.substring(index, index2) == "PROTEC"){
            index2++;
            if(serial_buff.substring(index2) == "ON"){
                setProteksi(true);

                Serial.println(serial_buff.substring(index));
            }
            else if(serial_buff.substring(index2) == "OFF"){
                setProteksi(false);
                Serial.println(serial_buff.substring(index));
            }
        }
    }

    serial_buff = "";
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

    if(isnan(pzem.voltage) || isnan(pzem.current)|| isnan(pzem.power) 
        || isnan(pzem.energy) || isnan(pzem.frequency) || isnan(pzem.pf)){
        sensorData.input.voltage    = 0;
        sensorData.input.current    = 0;
        sensorData.input.power      = 0;
        sensorData.input.energy     = 0;
        sensorData.input.frequency  = 0;
        sensorData.input.pf         = 0;
    }
    else{
        sensorData.input.voltage    = pzem.voltage;
        sensorData.input.current    = pzem.current;
        sensorData.input.power      = pzem.power;
        sensorData.input.energy     = pzem.energy;
        sensorData.input.frequency  = pzem.frequency;
        sensorData.input.pf         = pzem.pf;

        valid = true;
    }

    // if(valid){
    //     Serial.println("------------ INPUT -----------");
    //     Serial.print(sensorData.input.voltage, 2);      Serial.println("V");
    //     Serial.print(sensorData.input.current, 3);      Serial.println("A");
    //     Serial.print(sensorData.input.power, 2);        Serial.println("W");
    //     Serial.print(sensorData.input.energy, 2);       Serial.println("Wh");
    //     Serial.print(sensorData.input.frequency, 2);    Serial.println("Hz");
    //     Serial.print("PF : ");                          Serial.println(sensorData.input.pf, 3);
    // }else{
    //     Serial.println("-- READ INPUT FAILED --");
    // }
    // Serial.println();

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

    if(isnan(pzem.voltage) || isnan(pzem.current)|| isnan(pzem.power) 
        || isnan(pzem.energy) || isnan(pzem.frequency) || isnan(pzem.pf)){
        sensorData.output.voltage    = 0;
        sensorData.output.current    = 0;
        sensorData.output.power      = 0;
        sensorData.output.energy     = 0;
        sensorData.output.frequency  = 0;
        sensorData.output.pf         = 0;
    }
    else{
        sensorData.output.voltage    = pzem.voltage;
        sensorData.output.current    = pzem.current;
        sensorData.output.power      = pzem.power;
        sensorData.output.energy     = pzem.energy;
        sensorData.output.frequency  = pzem.frequency;
        sensorData.output.pf         = pzem.pf;

        valid = true;
    }

    // if(valid){
    //     Serial.println("------------ OUTPUT -----------");
    //     Serial.print(sensorData.output.voltage, 2);     Serial.println("V");
    //     Serial.print(sensorData.output.current, 3);     Serial.println("A");
    //     Serial.print(sensorData.output.power, 2);       Serial.println("W");
    //     Serial.print(sensorData.output.energy, 2);      Serial.println("Wh");
    //     Serial.print(sensorData.output.frequency, 2);   Serial.println("Hz");
    //     Serial.print("PF : ");                          Serial.println(sensorData.output.pf, 3);
    // }else{
    //     Serial.println("-- READ OUTPUT FAILED --");
    // }
    // Serial.println();

    return valid;
}

void readSuhu(){
    sensorSuhu.requestTemperatures();
    sensorData.suhu = sensorSuhu.getTempCByIndex(0);

    // Serial.println("--------------- SUHU --------------");
    // Serial.print("Suhu : ");        Serial.print(sensorData.suhu, 2);    Serial.println(" C");
    // Serial.println();
}

void relayInit(){
    pinMode(RELAY_1_PIN, OUTPUT);       digitalWrite(RELAY_1_PIN, RELAY_OFF);
    pinMode(RELAY_2_PIN, OUTPUT);       digitalWrite(RELAY_2_PIN, RELAY_OFF);
    pinMode(RELAY_3_PIN, OUTPUT);       digitalWrite(RELAY_3_PIN, RELAY_OFF);
    pinMode(RELAY_4_PIN, OUTPUT);       digitalWrite(RELAY_4_PIN, RELAY_OFF);
}

void setSumber(bool pln){
    if(pln){                    // jika true, Matikan Relay Sumber untuk menggunakan sumber PLN ke LOAD    
        digitalWrite(RELAY_SUMBER, RELAY_OFF);
        digitalWrite(RELAY_INDICATOR, RELAY_OFF);
        source_pln = true;
    }   
    else{                       // jika false, Nyalakn RElay Sumber untuk menggunakan sumber PLTPH ke LOAD      
        digitalWrite(RELAY_SUMBER, RELAY_ON);   
        digitalWrite(RELAY_INDICATOR, RELAY_ON);   
        source_pln = false;
    }   
}

void setProteksi(bool protek){
    if(protek){                     // jika true, Nyalakan Relay Proteksi untuk memutus LOAD
        digitalWrite(RELAY_PROTEKSI, RELAY_ON); 
        proteksi_on = true;
    }   
    else{                           // jika false, Matikan Relay Proteksi untuk menyambungkan LOAD       
        digitalWrite(RELAY_PROTEKSI, RELAY_OFF);
        proteksi_on = false;
    }   
}

bool getFan(){
    if(digitalRead(FAN_DATA_PIN) == LOW){
        return true;
    }     
    else{
        return false;
    }
}

void setRelaySuhu(bool on){
    if(on){                         // jika true, nyalakan RELAY SUHU
        digitalWrite(RELAY_SUHU, RELAY_ON);  
        relay_suhu_on = true;
    }       
    else{                           // jika false, matikan RELAY SUHU   
        digitalWrite(RELAY_SUHU, RELAY_OFF); 
        relay_suhu_on = false;
    }       
}

/******* Serial Interrupt Event Callback ********/
void serialEvent(){
  while(Serial.available()){
    char inChar = (char) Serial.read();
    if(inChar == '\n'){
      serial_complete = true;
    }else if(inChar == '\r'){
      // do nothing
    }else{
      if(!serial_complete){
        serial_buff += inChar;
      }
    }
  }
}

void toggleLed(){
  if(led_state){
    digitalWrite(LED_BUILTIN, LOW);
    led_state = false;
  }else{
    digitalWrite(LED_BUILTIN, HIGH);
    led_state = true;
  }
}

/**
 * command list
 * -> GET|DATA
 * -> SET|SOURCE|PLN
 * -> SET|SOURCE|PLTPH
 * -> SET|PROTEC|ON
 * -> SET|PROTEC|OFF
 */