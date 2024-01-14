//FIRMWARE UPDATES: If you need to update the firmware after it's assembled and have difficulty accessin the internal micro usb port, start the captive portal and go to http://192.168.4.1/update? to upload new firmware OTA

/* D0 - light sensor input (see circuit diagram below)
 * D1 - one end of color potentiometer
 * D2 - the other end of color potentiometer
 * D3 - blue leg of status LED (with 820 ohm resistor)
 * D4 - LED strip
 * D5 - one end of brightness potentiometer
 * D6 - light sensor input offset (see circuit diagram below. Used to prevent flickering between states around threshold.)
 * D7 - other end of brightness potentiometer
 * D8 - green leg of status LED (with 3.3k resistor)
 * TX - red leg of status LED (with 560 ohm resistor)
 * 
 * 
 *                        GPIO D0     GPIO D6
 *                           |    4.7k   |    100k
 *         ____[CDS CELL]____|___/\/\/\__|___/\/\/\______
 *        |                  |                           |
 *       GND                 V                          GND
 *                        /\/\/\ 100k potentiometer
 *                        |    |
 *                        |    |
 *                       GND  3.3V
 *                       
 *CDS cell should be around 50k around the desired room threshold brightness -- model GL5528 works well                      
 */


#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "src/WiFiManager/WiFiManager.h"         // quotation marks usues library in sketch folder which i can customize the webpage for. Also added LED commands to start red then turn orange when wifi setup starts
#include <PubSubClient.h> //for mqtt

int clientNum=1; //1 is Kenzie, 2 is Liam

#define NUMPIXELS 12
Adafruit_NeoPixel lights(NUMPIXELS, D4, NEO_GRB + NEO_KHZ800);

const char* mqtt_server = "broker.mqtt-dashboard.com";
WiFiClient espClient;
PubSubClient client(espClient);

int colors[1025][3] = {{255,0,0},{255,1,0},{255,3,0},{255,4,0},{255,6,0},{255,7,0},{255,9,0},{255,10,0},{255,12,0},{255,13,0},{255,15,0},{255,16,0},{255,18,0},{255,19,0},{255,21,0},{255,22,0},{255,24,0},{255,25,0},{255,27,0},{255,28,0},{255,30,0},{255,31,0},{255,33,0},{255,34,0},{255,36,0},{255,37,0},{255,39,0},{255,40,0},{255,42,0},{255,43,0},{255,45,0},{255,46,0},{255,48,0},{255,49,0},{255,51,0},{255,52,0},{255,54,0},{255,55,0},{255,57,0},{255,58,0},{255,60,0},{255,61,0},{255,63,0},{255,64,0},{255,66,0},{255,67,0},{255,69,0},{255,70,0},{255,72,0},{255,73,0},{255,75,0},{255,76,0},{255,78,0},{255,79,0},{255,81,0},{255,82,0},{255,84,0},{255,85,0},{255,87,0},{255,88,0},{255,90,0},{255,91,0},{255,93,0},{255,94,0},{255,96,0},{255,97,0},{255,99,0},{255,100,0},{255,102,0},{255,103,0},{255,105,0},{255,106,0},{255,108,0},{255,109,0},{255,111,0},{255,112,0},{255,114,0},{255,115,0},{255,117,0},{255,118,0},{255,120,0},{255,121,0},{255,123,0},{255,124,0},{255,126,0},{255,127,0},{255,129,0},{255,130,0},{255,132,0},{255,133,0},{255,135,0},{255,136,0},{255,138,0},{255,139,0},{255,141,0},{255,142,0},{255,144,0},{255,145,0},{255,147,0},{255,148,0},{255,150,0},{255,151,0},{255,153,0},{255,154,0},{255,156,0},{255,157,0},{255,159,0},{255,160,0},{255,162,0},{255,163,0},{255,165,0},{255,166,0},{255,168,0},{255,169,0},{255,171,0},{255,172,0},{255,174,0},{255,175,0},{255,177,0},{255,178,0},{255,180,0},{255,181,0},{255,183,0},{255,184,0},{255,186,0},{255,187,0},{255,189,0},{255,190,0},{255,192,0},{255,193,0},{255,195,0},{255,196,0},{255,198,0},{255,199,0},{255,201,0},{255,202,0},{255,204,0},{255,205,0},{255,207,0},{255,208,0},{255,210,0},{255,211,0},{255,213,0},{255,214,0},{255,216,0},{255,217,0},{255,219,0},{255,220,0},{255,222,0},{255,223,0},{255,225,0},{255,226,0},{255,228,0},{255,229,0},{255,231,0},{255,232,0},{255,234,0},{255,235,0},{255,237,0},{255,238,0},{255,240,0},{255,241,0},{255,243,0},{255,244,0},{255,246,0},{255,247,0},{255,249,0},{255,250,0},{255,252,0},{255,253,0},{255,255,0},{253,255,0},{252,255,0},{250,255,0},{249,255,0},{247,255,0},{246,255,0},{244,255,0},{243,255,0},{241,255,0},{240,255,0},{238,255,0},{237,255,0},{235,255,0},{234,255,0},{232,255,0},{231,255,0},{229,255,0},{228,255,0},{226,255,0},{225,255,0},{223,255,0},{222,255,0},{220,255,0},{219,255,0},{217,255,0},{216,255,0},{214,255,0},{213,255,0},{211,255,0},{210,255,0},{208,255,0},{207,255,0},{205,255,0},{204,255,0},{202,255,0},{201,255,0},{199,255,0},{198,255,0},{196,255,0},{195,255,0},{193,255,0},{192,255,0},{190,255,0},{189,255,0},{187,255,0},{186,255,0},{184,255,0},{183,255,0},{181,255,0},{180,255,0},{178,255,0},{177,255,0},{175,255,0},{174,255,0},{172,255,0},{171,255,0},{169,255,0},{168,255,0},{166,255,0},{165,255,0},{163,255,0},{162,255,0},{160,255,0},{159,255,0},{157,255,0},{156,255,0},{154,255,0},{153,255,0},{151,255,0},{150,255,0},{148,255,0},{147,255,0},{145,255,0},{144,255,0},{142,255,0},{141,255,0},{139,255,0},{138,255,0},{136,255,0},{135,255,0},{133,255,0},{132,255,0},{130,255,0},{129,255,0},{127,255,0},{126,255,0},{124,255,0},{123,255,0},{121,255,0},{120,255,0},{118,255,0},{117,255,0},{115,255,0},{114,255,0},{112,255,0},{111,255,0},{109,255,0},{108,255,0},{106,255,0},{105,255,0},{103,255,0},{102,255,0},{100,255,0},{99,255,0},{97,255,0},{96,255,0},{94,255,0},{93,255,0},{91,255,0},{90,255,0},{88,255,0},{87,255,0},{85,255,0},{84,255,0},{82,255,0},{81,255,0},{79,255,0},{78,255,0},{76,255,0},{75,255,0},{73,255,0},{72,255,0},{70,255,0},{69,255,0},{67,255,0},{66,255,0},{64,255,0},{63,255,0},{61,255,0},{60,255,0},{58,255,0},{57,255,0},{55,255,0},{54,255,0},{52,255,0},{51,255,0},{49,255,0},{48,255,0},{46,255,0},{45,255,0},{43,255,0},{42,255,0},{40,255,0},{39,255,0},{37,255,0},{36,255,0},{34,255,0},{33,255,0},{31,255,0},{30,255,0},{28,255,0},{27,255,0},{25,255,0},{24,255,0},{22,255,0},{21,255,0},{19,255,0},{18,255,0},{16,255,0},{15,255,0},{13,255,0},{12,255,0},{10,255,0},{9,255,0},{7,255,0},{6,255,0},{4,255,0},{3,255,0},{1,255,0},{0,255,0},{0,255,0},{0,255,1},{0,255,3},{0,255,4},{0,255,6},{0,255,7},{0,255,9},{0,255,10},{0,255,12},{0,255,13},{0,255,15},{0,255,16},{0,255,18},{0,255,19},{0,255,21},{0,255,22},{0,255,24},{0,255,25},{0,255,27},{0,255,28},{0,255,30},{0,255,31},{0,255,33},{0,255,34},{0,255,36},{0,255,37},{0,255,39},{0,255,40},{0,255,42},{0,255,43},{0,255,45},{0,255,46},{0,255,48},{0,255,49},{0,255,51},{0,255,52},{0,255,54},{0,255,55},{0,255,57},{0,255,58},{0,255,60},{0,255,61},{0,255,63},{0,255,64},{0,255,66},{0,255,67},{0,255,69},{0,255,70},{0,255,72},{0,255,73},{0,255,75},{0,255,76},{0,255,78},{0,255,79},{0,255,81},{0,255,82},{0,255,84},{0,255,85},{0,255,87},{0,255,88},{0,255,90},{0,255,91},{0,255,93},{0,255,94},{0,255,96},{0,255,97},{0,255,99},{0,255,100},{0,255,102},{0,255,103},{0,255,105},{0,255,106},{0,255,108},{0,255,109},{0,255,111},{0,255,112},{0,255,114},{0,255,115},{0,255,117},{0,255,118},{0,255,120},{0,255,121},{0,255,123},{0,255,124},{0,255,126},{0,255,127},{0,255,129},{0,255,130},{0,255,132},{0,255,133},{0,255,135},{0,255,136},{0,255,138},{0,255,139},{0,255,141},{0,255,142},{0,255,144},{0,255,145},{0,255,147},{0,255,148},{0,255,150},{0,255,151},{0,255,153},{0,255,154},{0,255,156},{0,255,157},{0,255,159},{0,255,160},{0,255,162},{0,255,163},{0,255,165},{0,255,166},{0,255,168},{0,255,169},{0,255,171},{0,255,172},{0,255,174},{0,255,175},{0,255,177},{0,255,178},{0,255,180},{0,255,181},{0,255,183},{0,255,184},{0,255,186},{0,255,187},{0,255,189},{0,255,190},{0,255,192},{0,255,193},{0,255,195},{0,255,196},{0,255,198},{0,255,199},{0,255,201},{0,255,202},{0,255,204},{0,255,205},{0,255,207},{0,255,208},{0,255,210},{0,255,211},{0,255,213},{0,255,214},{0,255,216},{0,255,217},{0,255,219},{0,255,220},{0,255,222},{0,255,223},{0,255,225},{0,255,226},{0,255,228},{0,255,229},{0,255,231},{0,255,232},{0,255,234},{0,255,235},{0,255,237},{0,255,238},{0,255,240},{0,255,241},{0,255,243},{0,255,244},{0,255,246},{0,255,247},{0,255,249},{0,255,250},{0,255,252},{0,255,253},{0,255,255},{0,253,255},{0,252,255},{0,250,255},{0,249,255},{0,247,255},{0,246,255},{0,244,255},{0,243,255},{0,241,255},{0,240,255},{0,238,255},{0,237,255},{0,235,255},{0,234,255},{0,232,255},{0,231,255},{0,229,255},{0,228,255},{0,226,255},{0,225,255},{0,223,255},{0,222,255},{0,220,255},{0,219,255},{0,217,255},{0,216,255},{0,214,255},{0,213,255},{0,211,255},{0,210,255},{0,208,255},{0,207,255},{0,205,255},{0,204,255},{0,202,255},{0,201,255},{0,199,255},{0,198,255},{0,196,255},{0,195,255},{0,193,255},{0,192,255},{0,190,255},{0,189,255},{0,187,255},{0,186,255},{0,184,255},{0,183,255},{0,181,255},{0,180,255},{0,178,255},{0,177,255},{0,175,255},{0,174,255},{0,172,255},{0,171,255},{0,169,255},{0,168,255},{0,166,255},{0,165,255},{0,163,255},{0,162,255},{0,160,255},{0,159,255},{0,157,255},{0,156,255},{0,154,255},{0,153,255},{0,151,255},{0,150,255},{0,148,255},{0,147,255},{0,145,255},{0,144,255},{0,142,255},{0,141,255},{0,139,255},{0,138,255},{0,136,255},{0,135,255},{0,133,255},{0,132,255},{0,130,255},{0,129,255},{0,127,255},{0,126,255},{0,124,255},{0,123,255},{0,121,255},{0,120,255},{0,118,255},{0,117,255},{0,115,255},{0,114,255},{0,112,255},{0,111,255},{0,109,255},{0,108,255},{0,106,255},{0,105,255},{0,103,255},{0,102,255},{0,100,255},{0,99,255},{0,97,255},{0,96,255},{0,94,255},{0,93,255},{0,91,255},{0,90,255},{0,88,255},{0,87,255},{0,85,255},{0,84,255},{0,82,255},{0,81,255},{0,79,255},{0,78,255},{0,76,255},{0,75,255},{0,73,255},{0,72,255},{0,70,255},{0,69,255},{0,67,255},{0,66,255},{0,64,255},{0,63,255},{0,61,255},{0,60,255},{0,58,255},{0,57,255},{0,55,255},{0,54,255},{0,52,255},{0,51,255},{0,49,255},{0,48,255},{0,46,255},{0,45,255},{0,43,255},{0,42,255},{0,40,255},{0,39,255},{0,37,255},{0,36,255},{0,34,255},{0,33,255},{0,31,255},{0,30,255},{0,28,255},{0,27,255},{0,25,255},{0,24,255},{0,22,255},{0,21,255},{0,19,255},{0,18,255},{0,16,255},{0,15,255},{0,13,255},{0,12,255},{0,10,255},{0,9,255},{0,7,255},{0,6,255},{0,4,255},{0,3,255},{0,1,255},{0,0,255},{0,0,255},{1,0,255},{3,0,255},{4,0,255},{6,0,255},{7,0,255},{9,0,255},{10,0,255},{12,0,255},{13,0,255},{15,0,255},{16,0,255},{18,0,255},{19,0,255},{21,0,255},{22,0,255},{24,0,255},{25,0,255},{27,0,255},{28,0,255},{30,0,255},{31,0,255},{33,0,255},{34,0,255},{36,0,255},{37,0,255},{39,0,255},{40,0,255},{42,0,255},{43,0,255},{45,0,255},{46,0,255},{48,0,255},{49,0,255},{51,0,255},{52,0,255},{54,0,255},{55,0,255},{57,0,255},{58,0,255},{60,0,255},{61,0,255},{63,0,255},{64,0,255},{66,0,255},{67,0,255},{69,0,255},{70,0,255},{72,0,255},{73,0,255},{75,0,255},{76,0,255},{78,0,255},{79,0,255},{81,0,255},{82,0,255},{84,0,255},{85,0,255},{87,0,255},{88,0,255},{90,0,255},{91,0,255},{93,0,255},{94,0,255},{96,0,255},{97,0,255},{99,0,255},{100,0,255},{102,0,255},{103,0,255},{105,0,255},{106,0,255},{108,0,255},{109,0,255},{111,0,255},{112,0,255},{114,0,255},{115,0,255},{117,0,255},{118,0,255},{120,0,255},{121,0,255},{123,0,255},{124,0,255},{126,0,255},{127,0,255},{129,0,255},{130,0,255},{132,0,255},{133,0,255},{135,0,255},{136,0,255},{138,0,255},{139,0,255},{141,0,255},{142,0,255},{144,0,255},{145,0,255},{147,0,255},{148,0,255},{150,0,255},{151,0,255},{153,0,255},{154,0,255},{156,0,255},{157,0,255},{159,0,255},{160,0,255},{162,0,255},{163,0,255},{165,0,255},{166,0,255},{168,0,255},{169,0,255},{171,0,255},{172,0,255},{174,0,255},{175,0,255},{177,0,255},{178,0,255},{180,0,255},{181,0,255},{183,0,255},{184,0,255},{186,0,255},{187,0,255},{189,0,255},{190,0,255},{192,0,255},{193,0,255},{195,0,255},{196,0,255},{198,0,255},{199,0,255},{201,0,255},{202,0,255},{204,0,255},{205,0,255},{207,0,255},{208,0,255},{210,0,255},{211,0,255},{213,0,255},{214,0,255},{216,0,255},{217,0,255},{219,0,255},{220,0,255},{222,0,255},{223,0,255},{225,0,255},{226,0,255},{228,0,255},{229,0,255},{231,0,255},{232,0,255},{234,0,255},{235,0,255},{237,0,255},{238,0,255},{240,0,255},{241,0,255},{243,0,255},{244,0,255},{246,0,255},{247,0,255},{249,0,255},{250,0,255},{252,0,255},{253,0,255},{255,0,255},{255,0,253},{255,0,252},{255,0,250},{255,0,249},{255,0,247},{255,0,246},{255,0,244},{255,0,243},{255,0,241},{255,0,240},{255,0,238},{255,0,237},{255,0,235},{255,0,234},{255,0,232},{255,0,231},{255,0,229},{255,0,228},{255,0,226},{255,0,225},{255,0,223},{255,0,222},{255,0,220},{255,0,219},{255,0,217},{255,0,216},{255,0,214},{255,0,213},{255,0,211},{255,0,210},{255,0,208},{255,0,207},{255,0,205},{255,0,204},{255,0,202},{255,0,201},{255,0,199},{255,0,198},{255,0,196},{255,0,195},{255,0,193},{255,0,192},{255,0,190},{255,0,189},{255,0,187},{255,0,186},{255,0,184},{255,0,183},{255,0,181},{255,0,180},{255,0,178},{255,0,177},{255,0,175},{255,0,174},{255,0,172},{255,0,171},{255,0,169},{255,0,168},{255,0,166},{255,0,165},{255,0,163},{255,0,162},{255,0,160},{255,0,159},{255,0,157},{255,0,156},{255,0,154},{255,0,153},{255,0,151},{255,0,150},{255,0,148},{255,0,147},{255,0,145},{255,0,144},{255,0,142},{255,0,141},{255,0,139},{255,0,138},{255,0,136},{255,0,135},{255,0,133},{255,0,132},{255,0,130},{255,0,129},{255,0,127},{255,0,126},{255,0,124},{255,0,123},{255,0,121},{255,0,120},{255,0,118},{255,0,117},{255,0,115},{255,0,114},{255,0,112},{255,0,111},{255,0,109},{255,0,108},{255,0,106},{255,0,105},{255,0,103},{255,0,102},{255,0,100},{255,0,99},{255,0,97},{255,0,96},{255,0,94},{255,0,93},{255,0,91},{255,0,90},{255,0,88},{255,0,87},{255,0,85},{255,0,84},{255,0,82},{255,0,81},{255,0,79},{255,0,78},{255,0,76},{255,0,75},{255,0,73},{255,0,72},{255,0,70},{255,0,69},{255,0,67},{255,0,66},{255,0,64},{255,0,63},{255,0,61},{255,0,60},{255,0,58},{255,0,57},{255,0,55},{255,0,54},{255,0,52},{255,0,51},{255,0,49},{255,0,48},{255,0,46},{255,0,45},{255,0,43},{255,0,42},{255,0,40},{255,0,39},{255,0,37},{255,0,36},{255,0,34},{255,0,33},{255,0,31},{255,0,30},{255,0,28},{255,0,27},{255,0,25},{255,0,24},{255,0,22},{255,0,21},{255,0,19},{255,0,18},{255,0,16},{255,0,15},{255,0,13},{255,0,12},{255,0,10},{255,0,9},{255,0,7},{255,0,6},{255,0,4},{255,0,3},{255,0,1},{255,0,0},{255,0,0},{255,0,0}};

int lastColorKnobVal; //use this to determine if the color has changed since last time, re-send if it has

int currentColor=0;
int lastColor=0;
int lastBrightness=0; //stored before going in to dark mode

int brightness=0;
int rawBrightness=0;
int colorKnob=0;
unsigned long analogReadTimer=0;
bool colorReadTurn=true;

char sendVal[5]; //array to store value to send to other heart MUST BE [5] FOR 4 CHAR VALUE!! Maybe because of termination char?

unsigned long lastPingSent; //time the last ping was sent
unsigned long lastPingReceived;
int timeout=30000; //time in milliseconds between pings

boolean debugMode=false; //set this to true to enable serial communication. This causes the red component of status LED to be tied to tx data, and not behave how it should
boolean isDark;

void setup() {

  if(debugMode){
    //GPIO 1 (TX)
    pinMode(1, FUNCTION_0); 
    Serial.begin(9600);
  }else{
    pinMode(1, FUNCTION_3);
    pinMode(1, OUTPUT); //red 560 ohms  -- NOTE: this is the TX pin and not capable of PWM, so red can only be on/off. But that's ok, because every time we call red so far it's 1023 anyway
  }

  //RGB Status LED config [ Green: connected to other heart, Blue: connected to internet but not other heart, Orange: in setup mode (restart to attempt wifi connection again), Red: could not connect to internet or wifi, retrying now. Restart to enter setup mode to select a different wifi
  //see red setup above in debugMode statement
  pinMode(D8, OUTPUT); //green 3.3k ohms
  pinMode(D3, OUTPUT); //blue 820 ohms

  pinMode(D0,INPUT);
  pinMode(D6,INPUT);

  isDark=digitalRead(D0);
  
  statusLED(1023,0,0);
  
  //pinMode(LED_BUILTIN, OUTPUT); //built in LED is connected as sink, so setting it HIGH turns it off and vice-versa
  //digitalWrite(LED_BUILTIN,HIGH);
  
  //finsish lights setup
  lights.begin();
  lights.clear();
  //startup animation
  for(int i=0;i<200;i++){
    lights.setBrightness(i);
    for(int i=0;i<NUMPIXELS;i++){ //must set the color every time because I'm using setBrightness() as an animation and shouldn't be
      lights.setPixelColor(i, lights.Color(colors[(1024/NUMPIXELS)*i][0],colors[(1024/NUMPIXELS)*i][1],colors[(1024/NUMPIXELS)*i][2]));
    }
    lights.show();
    if(i<100){
      delay(20);
    }else{
      delay(10);
    }
  }
  lastBrightness=200;


  
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  //statusLED(1023,100,0);

  wifiManager.setTimeout(60*5); //if no pages are loaded on the setup AP within this many seconds, reboot in an attempt to connect to the saved network again.
  if(clientNum==1){ //SEARCH WiFiManager.ccp for "Starting Config Portal" BELOW THAT IS THE CODE TO CHANGE STATUS LED FROM RED TO ORANGE
    if(!wifiManager.autoConnect("Kenzie Heart Setup")){
      ESP.restart();
    }
  }else{
    if(!wifiManager.autoConnect("Ko Heart Setup")){
      ESP.restart();
    }
  }
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  //digitalWrite(LED_BUILTIN,LOW);

//END WIFI SETUP, BEGIN MQTT CONNECTION

   

  client.setServer(mqtt_server, 1883);

  client.setCallback(Received_Message);

  //set lights to current knob value, set variables so that the first loop doesn't trigger an update to be sent to remote heart (first we want to RECEIVE the color, not send it)
  pinMode(D5,INPUT); //brightness knob 1
  pinMode(D7,INPUT); //brightness knob 2
  pinMode(D1,OUTPUT); //color knob 1
  pinMode(D2,OUTPUT); //color knob 2
  digitalWrite(D1,HIGH);
  digitalWrite(D2,LOW);
  currentColor=analogRead(A0);
  lastColorKnobVal=currentColor;
  for(int i=0;i<NUMPIXELS;i++){
    lights.setPixelColor(i, lights.Color(colors[currentColor][0],colors[currentColor][1],colors[currentColor][2]));
  }
    pinMode(D1,INPUT); //color knob 1
  pinMode(D2,INPUT); //color knob 2
  pinMode(D5,OUTPUT); //brightness knob 1
  pinMode(D7,OUTPUT); //brightness knob 2
  digitalWrite(D5,HIGH);
  digitalWrite(D7,LOW);
  lights.setBrightness(analogRead(A0)/4);
  lights.show();

}

void statusLED(int red, int green, int blue){
  int dimmingFactor=4;   //WARNING: increasing this too high causes interference with LED strip which flickers
  //analogWrite(1/dimmingFactor,red);
  if(red>0 && !debugMode){ //red now uses the TX pin which cannot do analogWrite. But digital is ok for our use right now
    digitalWrite(1,HIGH);
  }else if(!debugMode){
    digitalWrite(1,LOW);
  }
  analogWrite(D8,green/dimmingFactor);
  analogWrite(D3,blue/dimmingFactor);
}

void Received_Message(char* topic, byte* payload, unsigned int length) {
/*  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
*/
  payload[length] = '\0'; // Add a NULL to the end of the char* to make it a string.
  int rcvNum = atoi((char *)payload); 
  
  if(rcvNum==-1){ //if other heart just came online, ignore the value (-1), but ping it to let it know we're here too and give it our value
    delay(500); //give client time to subscribe to topic
    if(clientNum==1){
      client.publish("KenzieLiamHeart/LiamRcv", itoa(currentColor, sendVal,10));
    }else{client.publish("KenzieLiamHeart/KenzieRcv", itoa(currentColor, sendVal,10));}
  }else{ //only update color from remote heart if it wasn't it's first ping to say it's online
    
    currentColor = atoi((char *)payload); 
  
    for(int i=0;i<NUMPIXELS;i++){
      lights.setPixelColor(i, lights.Color(colors[currentColor][0],colors[currentColor][1],colors[currentColor][2]));
    }
    lights.show();
    
  }
  
  lastPingReceived=millis();
  statusLED(0,1023,0);
  
}

void reconnect() {
  int retryCtr=0;
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    char* clientName;
    if(clientNum==1){
      clientName="KenzieHeart";
    }else{clientName="LiamHeart";}
    if (client.connect(clientName)) { //name of this client
      Serial.println("connected");
      statusLED(0,0,1023);
      // Once connected, publish an announcement...
      if(clientNum==1){
        client.publish("KenzieLiamHeart/LiamRcv", itoa(-1, sendVal,10)); // -1 indicates we just came online and are requesting other heart's value
        // ... and resubscribe
        client.subscribe("KenzieLiamHeart/KenzieRcv");
      }else{
        client.publish("KenzieLiamHeart/KenzieRcv", itoa(-1, sendVal,10)); 
        // ... and resubscribe
        client.subscribe("KenzieLiamHeart/LiamRcv");
      }
    } else {
      statusLED(1023,0,0);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in  seconds");
      // Wait 5 seconds before retrying
      delay(5000);

      retryCtr++;
      if(retryCtr>18){ //if disconnected from wifi for over 1.5 minutes (actually around 3 mins when factoring attempt time), restart to try to reconnect (since if left on for long enough, it may disconnect and not reconnect automatically -- unknown why. Firmware bug? MQTT issue?)
        ESP.restart();
      }
    }
  }
}

void pingAndStatus(){
  if(millis()-lastPingReceived>timeout+5000){
    //other heart is offline
    statusLED(0,0,1023);
  }

  if(millis()-lastPingSent>timeout){
    //send a ping
    if(clientNum==1){
      client.publish("KenzieLiamHeart/LiamRcv", itoa(currentColor, sendVal,10));
    }else{client.publish("KenzieLiamHeart/KenzieRcv", itoa(currentColor, sendVal,10));}
    lastPingSent=millis();
  }
}

void loop(){
  if(millis()-analogReadTimer>10){ //ensuring we don't do analogread() too often, which crashes ESP8266. >5 should be ok, doing >10 to be safe
      if(!colorReadTurn){
      //read the two potentiometers by sequentially switching them on and off
      pinMode(D1,INPUT); //color knob 1
      pinMode(D2,INPUT); //color knob 2
      pinMode(D5,OUTPUT); //brightness knob 1
      pinMode(D7,OUTPUT); //brightness knob 2
      digitalWrite(D5,HIGH);
      digitalWrite(D7,LOW);
      delay(1); //give voltage time to stabalize (I didn't do this last time and I'm not sure if it's necessary)
      rawBrightness = analogRead(A0);
    }else{
      pinMode(D5,INPUT); //brightness knob 1
      pinMode(D7,INPUT); //brightness knob 2
      pinMode(D1,OUTPUT); //color knob 1
      pinMode(D2,OUTPUT); //color knob 2
      digitalWrite(D1,HIGH);
      digitalWrite(D2,LOW);
      delay(1); //give voltage time to stabalize (I didn't do this last time and I'm not sure if it's necessary)
      colorKnob = analogRead(A0);
    }
    colorReadTurn=!colorReadTurn;
    analogReadTimer=millis();
  }
  brightness=rawBrightness; //because brightness is modified every loop, it doesn't hold it's value in non-analogRead cycles. rawBrightness retains it's og value
  

/*  Serial.print(brightness);
  Serial.print("\t");
  Serial.println(colorKnob);
*/
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  pingAndStatus();
  
  if(isDark && digitalRead(D0) && digitalRead(D6)){ //if it's dark, dim the lights and ignore the brightness knob
    lights.setBrightness(5);
    lights.show();
    isDark=false;
  }else if(!isDark && !digitalRead(D0) && !digitalRead(D6)){
    lights.setBrightness(lastBrightness);
    isDark=true;
  }

  //BRIGHTNESS KNOB HANDLING
  if(!digitalRead(D0)){ //only do this if it's not dark
    brightness=(brightness/4)-7; //minus 7 because adc on esp is junk and min rawvalue is ~14 plus noise
    if(brightness<1){ //going to 0 breaks it for some weird reason.... (stays off for several seconds before turning back on)
      brightness=1;
    }
    if(brightness>lastBrightness+5 || brightness<lastBrightness-5){
      if(brightness<5){
        brightness=5; //below this there's not enough bits to produce meaningful colors. Also good to keep min brightness above fixed "lighs off" brightness
      }
      lights.setBrightness(brightness);
      lastBrightness=brightness; 
      //lights.show(); //this was only necessary in button version i think
    }
  }

  //COLOR KNOB HANDLING
  /*int colorKnob=0;
  for(int i=0;i<10;i++){ //averaging so we don't get noise spikes up and down that cause an unnecessary transmit
    colorKnob=(colorKnob+analogRead(A0));
  }
  colorKnob=colorKnob/10;*/ //OLD AVERAGING

  if(colorKnob>lastColorKnobVal+9 || colorKnob<lastColorKnobVal-9){ //if color changed since last send, send again //TODO: use averaging instead
    int stepsToChange=colorKnob-lastColorKnobVal; //change light color index by this many steps, and wrap around if out of array bounds. also set currentColor
   
    currentColor+=stepsToChange;

   //wrap value if it goes beyond bounds
    if(currentColor>1024){
      currentColor=currentColor-1024;
    }if(currentColor<0){
      currentColor=1024+currentColor;
    }

    
    if(clientNum==1){
      client.publish("KenzieLiamHeart/LiamRcv", itoa(currentColor, sendVal,10));
    }else{client.publish("KenzieLiamHeart/KenzieRcv", itoa(currentColor, sendVal,10));}
    
    lastColorKnobVal=colorKnob;
    //Serial.print("sending new color to other heart: ");
    //Serial.println(currentColor);
  }

  for(int i=0;i<NUMPIXELS;i++){
    lights.setPixelColor(i, lights.Color(colors[currentColor][0],colors[currentColor][1],colors[currentColor][2]));
  }
  lights.show();
  
  delay(5); //mqtt disconnects if lights.show() is called too frequently (delay of >=5ms works)
  //NOTE FROM TWO YEAR WISER BLAINE: I think the actual issue is not with mqtt or show(), it's doing analogread() more often than 5ms which kills wifi on ESP8266
  //NOTE: in another project I found that limiting analogread() to once per 5ms is generally safe. However, here it's doing it twice per 5ms, so I will try reducing it more to see if that fixes the intermittant disconnect issue. Seems unlikely though since in other project it affected it on order of seconds, not days/weeks like kenzie and liam are reporting
  //theoretically this is no longer necessary due to limiting it at the beginning of loop() but I'm keeping it just to be safe (only change one thing at a time)
}
