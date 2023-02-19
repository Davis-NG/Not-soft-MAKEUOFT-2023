
int  sensorAnalogPin = A2;    // Select the Arduino input pin to accept the Sound Sensor's analog output 
int  sensorDigitalPin = 3;    // Select the Arduino input pin to accept the Sound Sensor's digital output
int  sound = 0;         // Define variable to store the analog value coming from the Sound Sensor
int  light = 0;         //stores light coming in from light sensor
int  digitalValue;            // Define variable to store the digital value coming from the Sound Sensor
//int  Led13 = 13;              // Define LED port; this is the LED built in to the Arduino (labled L)
                              // When D0 from the Sound Sensor (connnected to pin 7 on the
                              // Arduino) sends High (voltage present), L will light. In practice, you
                              // should see LED13 on the Arduino blink when LED2 on the Sensor is 100% lit.

int button = 13;
int yes = 4;
int no = 5;
int lightSensor = A7;

int adc_id = 0;
int HistoryValue = 0;
char printBuffer[128];

#include <DFRobot_DHT11.h>
#define DHT_SENSOR_TYPE DHT_TYPE_11
#define MAX_STATES 8

static const int DHT_SENSOR_PIN = 2;
DFRobot_DHT11 dht;
//DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

typedef struct state {
  int minSound, maxSound;
  int minWater, maxWater;
  int minTemp, maxTemp;
  int minHum, maxHum;
  int minLig, maxLig;
} State;

State sun = {.minSound = 0, .maxSound = 80, .minWater = 0, .maxWater = 100, .minTemp = -100, .maxTemp = 100, .minHum = 0, .maxHum = 100, .minLig = 50, .maxLig = 1000};
State wind = {.minSound = 80, .maxSound = 1000, .minWater = 0, .maxWater = 100, .minTemp = -100, .maxTemp = 100, .minHum = 0, .maxHum = 100, .minLig = 0, .maxLig = 1000};
State cloud = {.minSound = 0, .maxSound = 80, .minWater = 0, .maxWater = 100, .minTemp = -100, .maxTemp = 100, .minHum = 0, .maxHum = 100, .minLig = 10, .maxLig = 50};
State night = {.minSound = 0, .maxSound = 80, .minWater = 0, .maxWater = 100, .minTemp = -100, .maxTemp = 100, .minHum = 0, .maxHum = 100, .minLig = 0, .maxLig = 10};
State rain = {.minSound = 0, .maxSound = 200, .minWater = 100, .maxWater = 350, .minTemp = 0, .maxTemp = 100, .minHum = 0, .maxHum = 100, .minLig = 0, .maxLig = 50};
State snow = {.minSound = 0, .maxSound = 100, .minWater = 100, .maxWater = 350, .minTemp = -100, .maxTemp = 0, .minHum = 0, .maxHum = 100, .minLig = 0, .maxLig = 50};
State flood = {.minSound = 0, .maxSound = 100, .minWater = 350, .maxWater = 1000, .minTemp = -100, .maxTemp = 100, .minHum = 0, .maxHum = 100, .minLig = 0, .maxLig = 100};
State storm = {.minSound = 200, .maxSound = 1000, .minWater = 100, .maxWater = 350, .minTemp = -100, .maxTemp = 100, .minHum = 0, .maxHum = 100, .minLig = 0, .maxLig = 50};

State stateTable[MAX_STATES] = {flood, storm, rain, snow, wind, sun, night, cloud};
const int FLOOD = 0, STORM = 1, RAIN = 2, SNOW = 3, WIND = 4, SUN = 5, NIGHT = 6, CLOUD = 7;

void setup()
{
  Serial.begin(9600);               // The IDE settings for Serial Monitor/Plotter (preferred) must match this speed
  randomSeed(analogRead(5));
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("YEHUDA");
  pinMode(sensorDigitalPin,INPUT);  // Define pin 7 as an input port, to accept digital input
  pinMode(sensorAnalogPin, INPUT);
  //pinMode(Led13,OUTPUT);            // Define LED13 as an output port, to indicate digital trigger reached
  pinMode(DHT_SENSOR_PIN, INPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(lightSensor, INPUT);
  pinMode(yes, INPUT_PULLUP);
  pinMode(no, INPUT_PULLUP);
}

void loop(){
  int state = -1;
  if (digitalRead(button) == LOW){
    delay(50); 
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    // // set the cursor to column 0, line 1
    // // (note: line 1 is the second row, since counting begins with 0):
    // lcd.setCursor(0, 1);
    // // print the number of seconds since reset:
    // lcd.print(millis()  / 1000);
    sound = analogRead(sensorAnalogPin); // Read the value of the analog interface A0 assigned to digitalValue 
    light = analogRead(lightSensor);
    Serial.println(light);
    digitalValue=digitalRead(sensorDigitalPin); // Read the value of the digital interface 7 assigned to digitalValue 
    Serial.println(sound); // Send the analog value to the serial transmit interface
    
    /*if(digitalValue==HIGH)      // When the Sound Sensor sends signla, via voltage present, light LED13 (L)
    {
      digitalWrite(Led13,HIGH);
    }
    else
    {
      digitalWrite(Led13,LOW);
    }*/
    
    delay(50);                  // Slight pause so that we don't overwhelm the serial interface
    int value = analogRead(adc_id); // get adc value

      if(((HistoryValue>=value) && ((HistoryValue - value) > 10)) || ((HistoryValue<value) && ((value - HistoryValue) > 10)))
      {
        sprintf(printBuffer,"ADC%d level is %d\n",adc_id, value);
        Serial.print(printBuffer);
        HistoryValue = value;
      }

      int temperature, humidity;
    dht.read(DHT_SENSOR_PIN);
    temperature = dht.temperature;
    humidity = dht.humidity;

    Serial.print( "T = ");
    Serial.print(temperature);
    Serial.print( " deg. C, H = ");
    Serial.print(humidity);
    Serial.println( "%" );

    //value is water level
    //sound, humidity, temperature self-explanatory

    for(int i = 0; i < MAX_STATES; i++){
      State curState = stateTable[i];
      bool matchTemp = ((temperature >= curState.minTemp) && (temperature <= curState.maxTemp));
      bool matchSound = ((sound >= curState.minSound) && (sound <= curState.maxSound));
      bool matchLig = ((light >= curState.minLig) && (light <= curState.maxLig));
      bool matchHum = ((humidity >= curState.minHum) && (humidity <= curState.maxHum));
      bool matchWater = ((value >= curState.minWater) && (value <= curState.maxWater));
      if(matchTemp && matchSound && matchLig && matchHum && matchWater){
        state = i;
        Serial.print("State is ");
        Serial.println(i);
        break;
      }
    }

    switch(state) {
      case FLOOD:         
        lcd.setCursor(0, 0);
        lcd.print("It's flooding,");
        lcd.setCursor(0, 1);
        lcd.print("[YES]      [NO]");
        break;
      case STORM:
        lcd.setCursor(0, 0);
        lcd.print("It's thundering");
        lcd.setCursor(0, 1);
        lcd.print("[YES]      [NO]");
        break;
      case RAIN:
        lcd.setCursor(0, 0);
        lcd.print("It's raining");
        lcd.setCursor(0, 1);
        lcd.print("[YES]      [NO]");
        break;
      case SNOW:
        lcd.setCursor(0, 0);
        lcd.print("It's snowing");
        lcd.setCursor(0, 1);
        lcd.print("[YES]      [NO]");
        break;
      case WIND:
        lcd.setCursor(0, 0);
        lcd.print("It's windy");
        lcd.setCursor(0, 1);
        lcd.print("[YES]      [NO]");
        break;
      case SUN:
        lcd.setCursor(0, 0);
        lcd.print("It's sunny");
        lcd.setCursor(0, 1);
        lcd.print("[YES]      [NO]");
        break;
      case NIGHT:
        lcd.setCursor(0, 0);
        lcd.print("It's night");
        lcd.setCursor(0, 1);
        lcd.print("[YES]      [NO]");
        break;
      case CLOUD:
        lcd.setCursor(0, 0);
        lcd.print("It's cloudy");
        lcd.setCursor(0, 1);
        lcd.print("[YES]      [NO]");
        break;
      default:
        lcd.setCursor(0, 0);
        lcd.print("Something broke");
        break;
    }

    /*if (value >= 350){
      lcd.setCursor(0, 0);
      lcd.print("It's flooding,");
      lcd.setCursor(0, 1);
      lcd.print("SWIM");
    }else if (value >= 100 && temperature >= 0){
      if(sound <= 100){
        lcd.setCursor(0, 0);
        lcd.print("It's raining");
        lcd.setCursor(0, 1);
        lcd.print("HIDE");
      }else{
        lcd.setCursor(0, 0);
        lcd.print("It's thundering");
        lcd.setCursor(0, 1);
        lcd.print("HIDE");
      }
    }else if (value >= 100 && temperature < 0){
      lcd.setCursor(0, 0);
      lcd.print("It's snowing");
      lcd.setCursor(0, 1);
      lcd.print("FALL");
    }else{
      lcd.setCursor(0, 0);
      lcd.print("It's dry,");
      lcd.setCursor(0, 1);
      lcd.print("CRY");
    }

    delay(6000);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("                ");

    if (temperature >= 20 && temperature <= 30){
      lcd.setCursor(0, 0);
      lcd.print("It's lukewarm");
      lcd.setCursor(0, 1);
      lcd.print("Be meh");
    }else if (temperature < 20 && temperature >= 0){
      lcd.setCursor(0, 0);
      lcd.print("Itsa wee nippy");
      lcd.setCursor(0, 1);
      lcd.print("Be meh");
    }else if (temperature < 0){
      lcd.setCursor(0, 0);
      lcd.print("mo");
      lcd.setCursor(0, 1);
      lcd.print("Be meh");
    }else{
      lcd.setCursor(0, 0);
      lcd.print("mo");
      lcd.setCursor(0, 1);
      lcd.print("Be meh");
    }

    // lcd.setCursor(0, 1);
    // lcd.print(humidity);
    // lcd.setCursor(2, 1);
    // lcd.print("%");*/

    while(digitalRead(button) == LOW);
    while(digitalRead(yes) == HIGH && digitalRead(no) == HIGH);
    lcd.setCursor(0, 1);
    lcd.print("UPDATED.   [NEW]");

    if(digitalRead(no) == LOW) {
      switch(state) {
        case FLOOD:         
        stateTable[FLOOD].minWater += 10;
        stateTable[STORM].maxWater += 10;
        stateTable[RAIN].maxWater += 10;
        stateTable[SNOW].maxWater += 10;
        break;
      case STORM: 
        stateTable[STORM].minSound += 10;
        stateTable[RAIN].maxSound += 10;
        break;
      case RAIN:
        if(random(1)){
          stateTable[RAIN].minWater += 5;
          stateTable[WIND].maxWater += 5;
          stateTable[CLOUD].maxWater += 5;
          stateTable[SUN].maxWater += 5;
          stateTable[NIGHT].maxWater += 5;
        } else {
          stateTable[RAIN].minTemp -= 2;
          stateTable[SNOW].maxTemp -= 2;
        }
        break;
      case SNOW:
        if(random(1)){
          stateTable[SNOW].minWater += 5;
          stateTable[WIND].maxWater += 5;
          stateTable[CLOUD].maxWater += 5;
          stateTable[SUN].maxWater += 5;
          stateTable[NIGHT].maxWater += 5;
        } else {
          stateTable[RAIN].minTemp += 2;
          stateTable[SNOW].maxTemp += 2;
        }
        break;
      case WIND:
        stateTable[WIND].minSound += 5;
        stateTable[CLOUD].maxSound += 5;
        stateTable[SUN].maxSound += 5;
        stateTable[NIGHT].maxSound += 5;
        break;
      case SUN:
        //stateTable[FLOOD].maxLig += 5;
        stateTable[STORM].maxLig += 5;
        stateTable[RAIN].maxLig += 5;
        stateTable[SNOW].maxLig += 5;
        stateTable[CLOUD].maxLig += 5;
        stateTable[SUN].minLig += 5;
        Serial.print("Sun min light = ");
        Serial.println(stateTable[SUN].minLig);
        break;
      case NIGHT:
        stateTable[CLOUD].minLig -= 2;
        stateTable[NIGHT].maxLig -= 2;
        break;
      case CLOUD:
        if(random(1)){
          stateTable[CLOUD].minLig += 2;
          stateTable[NIGHT].maxLig += 2;
        } else {
          stateTable[CLOUD].maxLig -= 2;
          stateTable[SUN].minLig -= 2;
        }
        break;
      default:
        break;
      }
    }
  }  
}

