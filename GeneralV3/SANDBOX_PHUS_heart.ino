/* ///TODO
 *Search code for "todo" -- there's some notes
 *get multicolor mode working
 *figure out how more than 2 clients work -- make status LEDs work in that case
 *Rate limits MQTT messages while twisting knob -- maybe one per second? Then on receiver fade between previous color and received color 
 *
*/
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "src/WiFiManager/WiFiManager.h"         // quotation marks usues library in sketch folder which i can customize the webpage for. PLEASE NOTE -- in earlier versions, WiFiManager.cpp had digitalWrite() and analogWrite() lines manually added by Blaine for the status LED. Now that the status LEDs use neopixel, the library version with those lines should NOT be used, otherwise the LED strip will flicker along with other unexpected behavior
#include <PubSubClient.h> //for mqtt

//CLIENT SPECIFIC VARIABLES----------------
//int clientNum=2; //1 is Kenzie, 2 is Liam
char* clientName="USplus09402";

int numOtherClientsInGroup=1;
char* otherClientsInGroup[3]={"PH"};
char* groupName="PHUSSandbox";
//END CLIENT SPECIFIC VARIABLES------------

char groupTopic[70]; //should be large enough
char multiColorTopic[84];

#define NUMPIXELS 12
Adafruit_NeoPixel lights(NUMPIXELS, D4, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel indicators(NUMPIXELS, D3, NEO_RGB + NEO_KHZ800);

const char* mqtt_server = "broker.mqtt-dashboard.com";
WiFiClient espClient;
PubSubClient client(espClient);

//this rediculous and terrifying array is being replaced by getColor()
//int colors[1025][3] = {{255,0,0},{255,1,0},{255,3,0},{255,4,0},{255,6,0},{255,7,0},{255,9,0},{255,10,0},{255,12,0},{255,13,0},{255,15,0},{255,16,0},{255,18,0},{255,19,0},{255,21,0},{255,22,0},{255,24,0},{255,25,0},{255,27,0},{255,28,0},{255,30,0},{255,31,0},{255,33,0},{255,34,0},{255,36,0},{255,37,0},{255,39,0},{255,40,0},{255,42,0},{255,43,0},{255,45,0},{255,46,0},{255,48,0},{255,49,0},{255,51,0},{255,52,0},{255,54,0},{255,55,0},{255,57,0},{255,58,0},{255,60,0},{255,61,0},{255,63,0},{255,64,0},{255,66,0},{255,67,0},{255,69,0},{255,70,0},{255,72,0},{255,73,0},{255,75,0},{255,76,0},{255,78,0},{255,79,0},{255,81,0},{255,82,0},{255,84,0},{255,85,0},{255,87,0},{255,88,0},{255,90,0},{255,91,0},{255,93,0},{255,94,0},{255,96,0},{255,97,0},{255,99,0},{255,100,0},{255,102,0},{255,103,0},{255,105,0},{255,106,0},{255,108,0},{255,109,0},{255,111,0},{255,112,0},{255,114,0},{255,115,0},{255,117,0},{255,118,0},{255,120,0},{255,121,0},{255,123,0},{255,124,0},{255,126,0},{255,127,0},{255,129,0},{255,130,0},{255,132,0},{255,133,0},{255,135,0},{255,136,0},{255,138,0},{255,139,0},{255,141,0},{255,142,0},{255,144,0},{255,145,0},{255,147,0},{255,148,0},{255,150,0},{255,151,0},{255,153,0},{255,154,0},{255,156,0},{255,157,0},{255,159,0},{255,160,0},{255,162,0},{255,163,0},{255,165,0},{255,166,0},{255,168,0},{255,169,0},{255,171,0},{255,172,0},{255,174,0},{255,175,0},{255,177,0},{255,178,0},{255,180,0},{255,181,0},{255,183,0},{255,184,0},{255,186,0},{255,187,0},{255,189,0},{255,190,0},{255,192,0},{255,193,0},{255,195,0},{255,196,0},{255,198,0},{255,199,0},{255,201,0},{255,202,0},{255,204,0},{255,205,0},{255,207,0},{255,208,0},{255,210,0},{255,211,0},{255,213,0},{255,214,0},{255,216,0},{255,217,0},{255,219,0},{255,220,0},{255,222,0},{255,223,0},{255,225,0},{255,226,0},{255,228,0},{255,229,0},{255,231,0},{255,232,0},{255,234,0},{255,235,0},{255,237,0},{255,238,0},{255,240,0},{255,241,0},{255,243,0},{255,244,0},{255,246,0},{255,247,0},{255,249,0},{255,250,0},{255,252,0},{255,253,0},{255,255,0},{253,255,0},{252,255,0},{250,255,0},{249,255,0},{247,255,0},{246,255,0},{244,255,0},{243,255,0},{241,255,0},{240,255,0},{238,255,0},{237,255,0},{235,255,0},{234,255,0},{232,255,0},{231,255,0},{229,255,0},{228,255,0},{226,255,0},{225,255,0},{223,255,0},{222,255,0},{220,255,0},{219,255,0},{217,255,0},{216,255,0},{214,255,0},{213,255,0},{211,255,0},{210,255,0},{208,255,0},{207,255,0},{205,255,0},{204,255,0},{202,255,0},{201,255,0},{199,255,0},{198,255,0},{196,255,0},{195,255,0},{193,255,0},{192,255,0},{190,255,0},{189,255,0},{187,255,0},{186,255,0},{184,255,0},{183,255,0},{181,255,0},{180,255,0},{178,255,0},{177,255,0},{175,255,0},{174,255,0},{172,255,0},{171,255,0},{169,255,0},{168,255,0},{166,255,0},{165,255,0},{163,255,0},{162,255,0},{160,255,0},{159,255,0},{157,255,0},{156,255,0},{154,255,0},{153,255,0},{151,255,0},{150,255,0},{148,255,0},{147,255,0},{145,255,0},{144,255,0},{142,255,0},{141,255,0},{139,255,0},{138,255,0},{136,255,0},{135,255,0},{133,255,0},{132,255,0},{130,255,0},{129,255,0},{127,255,0},{126,255,0},{124,255,0},{123,255,0},{121,255,0},{120,255,0},{118,255,0},{117,255,0},{115,255,0},{114,255,0},{112,255,0},{111,255,0},{109,255,0},{108,255,0},{106,255,0},{105,255,0},{103,255,0},{102,255,0},{100,255,0},{99,255,0},{97,255,0},{96,255,0},{94,255,0},{93,255,0},{91,255,0},{90,255,0},{88,255,0},{87,255,0},{85,255,0},{84,255,0},{82,255,0},{81,255,0},{79,255,0},{78,255,0},{76,255,0},{75,255,0},{73,255,0},{72,255,0},{70,255,0},{69,255,0},{67,255,0},{66,255,0},{64,255,0},{63,255,0},{61,255,0},{60,255,0},{58,255,0},{57,255,0},{55,255,0},{54,255,0},{52,255,0},{51,255,0},{49,255,0},{48,255,0},{46,255,0},{45,255,0},{43,255,0},{42,255,0},{40,255,0},{39,255,0},{37,255,0},{36,255,0},{34,255,0},{33,255,0},{31,255,0},{30,255,0},{28,255,0},{27,255,0},{25,255,0},{24,255,0},{22,255,0},{21,255,0},{19,255,0},{18,255,0},{16,255,0},{15,255,0},{13,255,0},{12,255,0},{10,255,0},{9,255,0},{7,255,0},{6,255,0},{4,255,0},{3,255,0},{1,255,0},{0,255,0},{0,255,0},{0,255,1},{0,255,3},{0,255,4},{0,255,6},{0,255,7},{0,255,9},{0,255,10},{0,255,12},{0,255,13},{0,255,15},{0,255,16},{0,255,18},{0,255,19},{0,255,21},{0,255,22},{0,255,24},{0,255,25},{0,255,27},{0,255,28},{0,255,30},{0,255,31},{0,255,33},{0,255,34},{0,255,36},{0,255,37},{0,255,39},{0,255,40},{0,255,42},{0,255,43},{0,255,45},{0,255,46},{0,255,48},{0,255,49},{0,255,51},{0,255,52},{0,255,54},{0,255,55},{0,255,57},{0,255,58},{0,255,60},{0,255,61},{0,255,63},{0,255,64},{0,255,66},{0,255,67},{0,255,69},{0,255,70},{0,255,72},{0,255,73},{0,255,75},{0,255,76},{0,255,78},{0,255,79},{0,255,81},{0,255,82},{0,255,84},{0,255,85},{0,255,87},{0,255,88},{0,255,90},{0,255,91},{0,255,93},{0,255,94},{0,255,96},{0,255,97},{0,255,99},{0,255,100},{0,255,102},{0,255,103},{0,255,105},{0,255,106},{0,255,108},{0,255,109},{0,255,111},{0,255,112},{0,255,114},{0,255,115},{0,255,117},{0,255,118},{0,255,120},{0,255,121},{0,255,123},{0,255,124},{0,255,126},{0,255,127},{0,255,129},{0,255,130},{0,255,132},{0,255,133},{0,255,135},{0,255,136},{0,255,138},{0,255,139},{0,255,141},{0,255,142},{0,255,144},{0,255,145},{0,255,147},{0,255,148},{0,255,150},{0,255,151},{0,255,153},{0,255,154},{0,255,156},{0,255,157},{0,255,159},{0,255,160},{0,255,162},{0,255,163},{0,255,165},{0,255,166},{0,255,168},{0,255,169},{0,255,171},{0,255,172},{0,255,174},{0,255,175},{0,255,177},{0,255,178},{0,255,180},{0,255,181},{0,255,183},{0,255,184},{0,255,186},{0,255,187},{0,255,189},{0,255,190},{0,255,192},{0,255,193},{0,255,195},{0,255,196},{0,255,198},{0,255,199},{0,255,201},{0,255,202},{0,255,204},{0,255,205},{0,255,207},{0,255,208},{0,255,210},{0,255,211},{0,255,213},{0,255,214},{0,255,216},{0,255,217},{0,255,219},{0,255,220},{0,255,222},{0,255,223},{0,255,225},{0,255,226},{0,255,228},{0,255,229},{0,255,231},{0,255,232},{0,255,234},{0,255,235},{0,255,237},{0,255,238},{0,255,240},{0,255,241},{0,255,243},{0,255,244},{0,255,246},{0,255,247},{0,255,249},{0,255,250},{0,255,252},{0,255,253},{0,255,255},{0,253,255},{0,252,255},{0,250,255},{0,249,255},{0,247,255},{0,246,255},{0,244,255},{0,243,255},{0,241,255},{0,240,255},{0,238,255},{0,237,255},{0,235,255},{0,234,255},{0,232,255},{0,231,255},{0,229,255},{0,228,255},{0,226,255},{0,225,255},{0,223,255},{0,222,255},{0,220,255},{0,219,255},{0,217,255},{0,216,255},{0,214,255},{0,213,255},{0,211,255},{0,210,255},{0,208,255},{0,207,255},{0,205,255},{0,204,255},{0,202,255},{0,201,255},{0,199,255},{0,198,255},{0,196,255},{0,195,255},{0,193,255},{0,192,255},{0,190,255},{0,189,255},{0,187,255},{0,186,255},{0,184,255},{0,183,255},{0,181,255},{0,180,255},{0,178,255},{0,177,255},{0,175,255},{0,174,255},{0,172,255},{0,171,255},{0,169,255},{0,168,255},{0,166,255},{0,165,255},{0,163,255},{0,162,255},{0,160,255},{0,159,255},{0,157,255},{0,156,255},{0,154,255},{0,153,255},{0,151,255},{0,150,255},{0,148,255},{0,147,255},{0,145,255},{0,144,255},{0,142,255},{0,141,255},{0,139,255},{0,138,255},{0,136,255},{0,135,255},{0,133,255},{0,132,255},{0,130,255},{0,129,255},{0,127,255},{0,126,255},{0,124,255},{0,123,255},{0,121,255},{0,120,255},{0,118,255},{0,117,255},{0,115,255},{0,114,255},{0,112,255},{0,111,255},{0,109,255},{0,108,255},{0,106,255},{0,105,255},{0,103,255},{0,102,255},{0,100,255},{0,99,255},{0,97,255},{0,96,255},{0,94,255},{0,93,255},{0,91,255},{0,90,255},{0,88,255},{0,87,255},{0,85,255},{0,84,255},{0,82,255},{0,81,255},{0,79,255},{0,78,255},{0,76,255},{0,75,255},{0,73,255},{0,72,255},{0,70,255},{0,69,255},{0,67,255},{0,66,255},{0,64,255},{0,63,255},{0,61,255},{0,60,255},{0,58,255},{0,57,255},{0,55,255},{0,54,255},{0,52,255},{0,51,255},{0,49,255},{0,48,255},{0,46,255},{0,45,255},{0,43,255},{0,42,255},{0,40,255},{0,39,255},{0,37,255},{0,36,255},{0,34,255},{0,33,255},{0,31,255},{0,30,255},{0,28,255},{0,27,255},{0,25,255},{0,24,255},{0,22,255},{0,21,255},{0,19,255},{0,18,255},{0,16,255},{0,15,255},{0,13,255},{0,12,255},{0,10,255},{0,9,255},{0,7,255},{0,6,255},{0,4,255},{0,3,255},{0,1,255},{0,0,255},{0,0,255},{1,0,255},{3,0,255},{4,0,255},{6,0,255},{7,0,255},{9,0,255},{10,0,255},{12,0,255},{13,0,255},{15,0,255},{16,0,255},{18,0,255},{19,0,255},{21,0,255},{22,0,255},{24,0,255},{25,0,255},{27,0,255},{28,0,255},{30,0,255},{31,0,255},{33,0,255},{34,0,255},{36,0,255},{37,0,255},{39,0,255},{40,0,255},{42,0,255},{43,0,255},{45,0,255},{46,0,255},{48,0,255},{49,0,255},{51,0,255},{52,0,255},{54,0,255},{55,0,255},{57,0,255},{58,0,255},{60,0,255},{61,0,255},{63,0,255},{64,0,255},{66,0,255},{67,0,255},{69,0,255},{70,0,255},{72,0,255},{73,0,255},{75,0,255},{76,0,255},{78,0,255},{79,0,255},{81,0,255},{82,0,255},{84,0,255},{85,0,255},{87,0,255},{88,0,255},{90,0,255},{91,0,255},{93,0,255},{94,0,255},{96,0,255},{97,0,255},{99,0,255},{100,0,255},{102,0,255},{103,0,255},{105,0,255},{106,0,255},{108,0,255},{109,0,255},{111,0,255},{112,0,255},{114,0,255},{115,0,255},{117,0,255},{118,0,255},{120,0,255},{121,0,255},{123,0,255},{124,0,255},{126,0,255},{127,0,255},{129,0,255},{130,0,255},{132,0,255},{133,0,255},{135,0,255},{136,0,255},{138,0,255},{139,0,255},{141,0,255},{142,0,255},{144,0,255},{145,0,255},{147,0,255},{148,0,255},{150,0,255},{151,0,255},{153,0,255},{154,0,255},{156,0,255},{157,0,255},{159,0,255},{160,0,255},{162,0,255},{163,0,255},{165,0,255},{166,0,255},{168,0,255},{169,0,255},{171,0,255},{172,0,255},{174,0,255},{175,0,255},{177,0,255},{178,0,255},{180,0,255},{181,0,255},{183,0,255},{184,0,255},{186,0,255},{187,0,255},{189,0,255},{190,0,255},{192,0,255},{193,0,255},{195,0,255},{196,0,255},{198,0,255},{199,0,255},{201,0,255},{202,0,255},{204,0,255},{205,0,255},{207,0,255},{208,0,255},{210,0,255},{211,0,255},{213,0,255},{214,0,255},{216,0,255},{217,0,255},{219,0,255},{220,0,255},{222,0,255},{223,0,255},{225,0,255},{226,0,255},{228,0,255},{229,0,255},{231,0,255},{232,0,255},{234,0,255},{235,0,255},{237,0,255},{238,0,255},{240,0,255},{241,0,255},{243,0,255},{244,0,255},{246,0,255},{247,0,255},{249,0,255},{250,0,255},{252,0,255},{253,0,255},{255,0,255},{255,0,253},{255,0,252},{255,0,250},{255,0,249},{255,0,247},{255,0,246},{255,0,244},{255,0,243},{255,0,241},{255,0,240},{255,0,238},{255,0,237},{255,0,235},{255,0,234},{255,0,232},{255,0,231},{255,0,229},{255,0,228},{255,0,226},{255,0,225},{255,0,223},{255,0,222},{255,0,220},{255,0,219},{255,0,217},{255,0,216},{255,0,214},{255,0,213},{255,0,211},{255,0,210},{255,0,208},{255,0,207},{255,0,205},{255,0,204},{255,0,202},{255,0,201},{255,0,199},{255,0,198},{255,0,196},{255,0,195},{255,0,193},{255,0,192},{255,0,190},{255,0,189},{255,0,187},{255,0,186},{255,0,184},{255,0,183},{255,0,181},{255,0,180},{255,0,178},{255,0,177},{255,0,175},{255,0,174},{255,0,172},{255,0,171},{255,0,169},{255,0,168},{255,0,166},{255,0,165},{255,0,163},{255,0,162},{255,0,160},{255,0,159},{255,0,157},{255,0,156},{255,0,154},{255,0,153},{255,0,151},{255,0,150},{255,0,148},{255,0,147},{255,0,145},{255,0,144},{255,0,142},{255,0,141},{255,0,139},{255,0,138},{255,0,136},{255,0,135},{255,0,133},{255,0,132},{255,0,130},{255,0,129},{255,0,127},{255,0,126},{255,0,124},{255,0,123},{255,0,121},{255,0,120},{255,0,118},{255,0,117},{255,0,115},{255,0,114},{255,0,112},{255,0,111},{255,0,109},{255,0,108},{255,0,106},{255,0,105},{255,0,103},{255,0,102},{255,0,100},{255,0,99},{255,0,97},{255,0,96},{255,0,94},{255,0,93},{255,0,91},{255,0,90},{255,0,88},{255,0,87},{255,0,85},{255,0,84},{255,0,82},{255,0,81},{255,0,79},{255,0,78},{255,0,76},{255,0,75},{255,0,73},{255,0,72},{255,0,70},{255,0,69},{255,0,67},{255,0,66},{255,0,64},{255,0,63},{255,0,61},{255,0,60},{255,0,58},{255,0,57},{255,0,55},{255,0,54},{255,0,52},{255,0,51},{255,0,49},{255,0,48},{255,0,46},{255,0,45},{255,0,43},{255,0,42},{255,0,40},{255,0,39},{255,0,37},{255,0,36},{255,0,34},{255,0,33},{255,0,31},{255,0,30},{255,0,28},{255,0,27},{255,0,25},{255,0,24},{255,0,22},{255,0,21},{255,0,19},{255,0,18},{255,0,16},{255,0,15},{255,0,13},{255,0,12},{255,0,10},{255,0,9},{255,0,7},{255,0,6},{255,0,4},{255,0,3},{255,0,1},{255,0,0},{255,0,0},{255,0,0}};

int lastColorKnobVal; //use this to determine if the color has changed since last time, re-send if it has

int currentColor=0;
int lastColor=0;
int lastBrightness=0; //stored before going in to dark mode

int brightness=0;
int rawBrightness=0;
int colorKnob=0;
unsigned long analogReadTimer=0;
bool colorReadTurn=true;

bool dualColorMode=false;
unsigned long confirmColorModeTimer=0;;

char sendVal[20]; //array to store value to send (must be long enough to hold the number and our client name)           //OLD COMMENT: //array to store value to send to other heart MUST BE [5] FOR 4 CHAR VALUE!! Due to because of termination char?

unsigned long lastPingSent; //time the last ping was sent
unsigned long lastPingReceived;
int timeout=30000; //time in milliseconds between pings

boolean isDark;

void setup() {

  Serial.begin(9600);

  //set topic variable
  strcpy(groupTopic,"BlaineProjects/RemoteHearts/");
  strcat(groupTopic,groupName);
  strcat(multiColorTopic,groupName);
  
  strcat(groupTopic,"/color");
  strcat(multiColorTopic,"/multicolorMode");
  

  

  pinMode(D0,INPUT); //light sensor input
  pinMode(D6,INPUT); //light sensor offset input (for histerisis)

  isDark=digitalRead(D0);

  indicators.begin();
  indicators.clear();
  indicators.show();
  statusLEDs(255,0,0,0);
  
  //finsish lights setup
  lights.begin();
  lights.clear();
  //startup animation
  for(int i=0;i<200;i++){
    lights.setBrightness(i);
    for(int i=0;i<NUMPIXELS;i++){ //must set the color every time because I'm using setBrightness() as an animation and shouldn't be
      lights.setPixelColor(i, lights.Color(getColor((1024/NUMPIXELS)*i,'r'),getColor((1024/NUMPIXELS)*i,'g'),getColor((1024/NUMPIXELS)*i,'b')));
    }
    lights.show();
    if(i<100){ //slower at lower brigtnesses cause apparent brightness change is bigger per step
      delay(20);
    }else{
      delay(10);
    }
    lastBrightness=i; //leave this variable at wherever we end up according to loop ctr end
  }
  
  setup_wifi();
  /*WiFiManager manager;
  manager.resetSettings();
  manager.autoConnect();*/
  
  
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
    lights.setPixelColor(i, lights.Color(getColor(currentColor,'r'),getColor(currentColor,'g'),getColor(currentColor,'b')));
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


void statusLEDs(int red, int green, int blue, int indicator){
  indicators.setPixelColor(indicator,indicators.Color(red,green,blue));
  indicators.show();
}

void Received_Message(char* topic, byte* payload, unsigned int length) {
/*  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
*/
  Serial.println("Received msg");
  
  if(strcmp(topic,multiColorTopic)==0){
    Serial.println("multicolor topic");
    payload[length] = '\0'; // Add a NULL to the end of the char* to make it a string.
    char* payloadChar = (char *)payload;
    if(strcmp(payloadChar,"true")==0){
      dualColorMode=true;
    }else{
      dualColorMode=false;
    }
  }else if(strcmp(topic,groupTopic)==0){
    
    payload[length] = '\0'; // Add a NULL to the end of the char* to make it a string.

    String strPayload = String((char*)payload);
    String fromClient = strPayload.substring(strPayload.indexOf(',')+1);
    String receivedNumber = strPayload.substring(0,strPayload.indexOf(','));
    /*
    Serial.print("Received number ");
    Serial.print(receivedNumber);
    Serial.print(" from client ");
    Serial.println(fromClient);
    */
    char bufone[15];
    fromClient.toCharArray(bufone,fromClient.length()+1);
    if(strcmp(bufone,clientName)!=0){ //DO NOT PROCESS MESSAGE IF IT IS FROM OURSELVES
      char buf[receivedNumber.length()+1];
      receivedNumber.toCharArray(buf, receivedNumber.length()+1);
  
      int rcvNum = atoi(buf);
      
      if(rcvNum==-1){ //if other heart just came online, ignore the value (-1), but ping it to let it know we're here too and give it our value
        /*
        if(clientNum==1){
          client.publish("KenzieLiamHeart/LiamRcvSANDBOX", itoa(currentColor, sendVal,10));
        }else{client.publish("KenzieLiamHeart/KenzieRcvSANDBOX", itoa(currentColor, sendVal,10));}
        */
        
        itoa(currentColor, sendVal,10);
        strcat(sendVal,",");
        strcat(sendVal,clientName);
        client.publish(groupTopic,sendVal);
      }else{ //only update color from remote heart if it wasn't it's first ping to say it's online
    
        int currentColorRemote; //the color of the other heart
        currentColorRemote = rcvNum; 

        //TODO -- generalize the below for multicolor mode
        if(dualColorMode){
          /*
          if(clientNum==1){ //kenzie on the right -- but receiving so it's backwards
            for(int i=6;i<12;i++){
              lights.setPixelColor(i, lights.Color(getColor(currentColorRemote,'r'),getColor(currentColorRemote,'g'),getColor(currentColorRemote,'b')));
            }
          }else{ //liam on the left -- but receiving so it's backwards
            for(int i=0;i<6;i++){
              lights.setPixelColor(i, lights.Color(getColor(currentColorRemote,'r'),getColor(currentColorRemote,'g'),getColor(currentColorRemote,'b')));
            }
          }
          */
        }else{
          for(int i=0;i<NUMPIXELS;i++){
            lights.setPixelColor(i, lights.Color(getColor(currentColorRemote,'r'),getColor(currentColorRemote,'g'),getColor(currentColorRemote,'b')));
          }
          currentColor=currentColorRemote;
        }
        lights.show();
        
      }
      
      lastPingReceived=millis();
      statusLEDs(0,255,0,0);
    }
  }
}

void reconnect() {
  int retryCtr=0;
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName)) { //name of this client //TODO append some random characters otherwise common names will get kicked from the server
      Serial.println("connected");
      statusLEDs(0,0,255,0);
      // Once connected, publish an announcement...

      /*
      if(clientNum==1){
        client.publish("KenzieLiamHeart/LiamRcvSANDBOX", itoa(-1, sendVal,10)); // -1 indicates we just came online and are requesting other heart's value
        // ... and resubscribe
        client.subscribe("KenzieLiamHeart/KenzieRcvSANDBOX");
      }else{
        client.publish("KenzieLiamHeart/KenzieRcvSANDBOX", itoa(-1, sendVal,10)); 
        // ... and resubscribe
        client.subscribe("KenzieLiamHeart/LiamRcvSANDBOX");
      }
      */
      
      //Serial.println(groupTopic);
      client.subscribe(groupTopic); //subscribe first so that when we send -1 below, we can receive the response right away
      itoa(-1, sendVal,10);
      strcat(sendVal,",");
      strcat(sendVal,clientName);
      client.publish(groupTopic,sendVal); // -1 indicates we just came online and are requesting other heart's value
      
      
    } else {
      statusLEDs(255,0,0,0);
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
    statusLEDs(0,0,255,0);
  }

  if(millis()-lastPingSent>timeout){
    //send a ping

    /*
    if(clientNum==1){
      client.publish("KenzieLiamHeart/LiamRcvSANDBOX", itoa(currentColor, sendVal,10));
    }else{client.publish("KenzieLiamHeart/KenzieRcvSANDBOX", itoa(currentColor, sendVal,10));}
    */
    itoa(currentColor, sendVal,10);
    strcat(sendVal,",");
    strcat(sendVal,clientName);
    client.publish(groupTopic,sendVal); 
    lastPingSent=millis();
  }
}

void confirmColorMode(){ //every day, re-publish the current color mode to the MQTT broker since it only retains the last message for 3 days
  //TODO generalize this function
  if(millis()-confirmColorModeTimer>60000*60*24 && client.connected()){
    char* tempDualColorMode;
    if(dualColorMode){
      tempDualColorMode="true";
    }else{
      tempDualColorMode="false";
    }
    client.publish("KenzieLiamHeart/dualColorSANDBOX",tempDualColorMode,true);
    confirmColorModeTimer=millis();
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
  confirmColorMode();

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

    /*
    if(clientNum==1){
      client.publish("KenzieLiamHeart/LiamRcvSANDBOX", itoa(currentColor, sendVal,10));
    }else{client.publish("KenzieLiamHeart/KenzieRcvSANDBOX", itoa(currentColor, sendVal,10));}
    */
    itoa(currentColor, sendVal,10);
    strcat(sendVal,",");
    strcat(sendVal,clientName);
    client.publish(groupTopic,sendVal);
    
    lastColorKnobVal=colorKnob;
    //Serial.print("sending new color to other heart: ");
    //Serial.println(currentColor);
  }

  if(dualColorMode){
    //TODO generalize the below for multicolor mode
    /*
    if(clientNum==1){ //kenzie on the right
      for(int i=0;i<6;i++){
        lights.setPixelColor(i, lights.Color(getColor(currentColor,'r'),getColor(currentColor,'g'),getColor(currentColor,'b')));
      }
    }else{ //Liam on the left
      for(int i=6;i<12;i++){
        lights.setPixelColor(i, lights.Color(getColor(currentColor,'r'),getColor(currentColor,'g'),getColor(currentColor,'b')));
      }
    }
    */
  }else{
    for(int i=0;i<NUMPIXELS;i++){
      lights.setPixelColor(i, lights.Color(getColor(currentColor,'r'),getColor(currentColor,'g'),getColor(currentColor,'b')));
    }
  }
  lights.show();
  
  delay(5); //mqtt disconnects if lights.show() is called too frequently (delay of >=5ms works)
  //NOTE FROM TWO YEAR WISER BLAINE: I think the actual issue is not with mqtt or show(), it's doing analogread() more often than 5ms which kills wifi on ESP8266
  //NOTE: in another project I found that limiting analogread() to once per 5ms is generally safe. However, here it's doing it twice per 5ms, so I will try reducing it more to see if that fixes the intermittant disconnect issue. Seems unlikely though since in other project it affected it on order of seconds, not days/weeks like kenzie and liam are reporting
  //theoretically this is no longer necessary due to limiting it at the beginning of loop() but I'm keeping it just to be safe (only change one thing at a time)  
}









void setup_wifi() {
  WiFiManager manager;
  manager.setDebugOutput(false);
  //manager.resetSettings();
  
  Serial.println("Attempting to connect to saved network");
  WiFi.begin();
  unsigned long startTime=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-startTime<14000){
    Serial.print(".");
    delay(500);
    if(WiFi.status()==WL_NO_SSID_AVAIL || WiFi.status()==WL_CONNECT_FAILED){
      break;
    }
  }
  Serial.println();

  if(WiFi.status()!=WL_CONNECTED){ //launch wifi manager in this block
    //String networkName=strcat(strcat(clientName,"'s Heart Setup - "),WiFi.macAddress().c_str());
    String networkName=strcat(clientName,"'s Heart Setup");
    Serial.print("Connection to saved network failed, starting config AP on: ");
    Serial.println(networkName);

    //set lights to indicate that we are in config mode
    statusLEDs(255,50,0,0);

    // Switch wifiManager config portal IP from default 192.168.4.1 to 8.8.8.8. This ensures auto-load on some android devices which have 8.8.8.8 hard-coded in the OS.
    manager.setAPStaticIPConfig(IPAddress(8,8,8,8), IPAddress(8,8,8,8), IPAddress(255,255,255,0));

    manager.setTimeout(60*5); //if no pages are loaded on the setup AP within this many seconds, reboot in an attempt to connect to the saved network again.
    if(!manager.autoConnect(networkName.c_str(),"")){
      Serial.println("WifiManager portal timeout. Resetting now to attempt connection again. Will launch AP again on reboot if connection fails again");
      Serial.println("\n\n");
      ESP.restart();
    }
    //ESP.restart(); //temporary workaround to mitigate the flicker issue. When using WiFiManager to connect to a new network, the LED strip will flicker and flash until reboot unless D8 has digitalWrite() called every loop
  }

  Serial.print("Succesfully connected to ");
  Serial.println(WiFi.SSID());
  

}

//takes in a number 0-1024 (the potentiometer range) and returns the red, green, or blue component of that "index" of the rainbow
//component should be 'r', 'g', or 'b'
int getColor(int color,char component){
  int r;
  int g;
  int b;

  if(color==0){
    r=255;
    g=0;
    b=0;
  }else if(color<170){
    r=255;
    b=0;
    g=(color/170.0)*255;
  }else if(color<170*2){
    color=color-170;
    b=0;
    g=255;
    r=((170-color)/170.0)*255;
  }else if(color<170*3){
    color=color-170*2;
    r=0;
    g=255;
    b=(color/170.0)*255;
  }else if(color<170*4){
    color=color-170*3;
    r=0;
    b=255;
    g=((170-color)/170.0)*255;
  }else if(color<170*5){
    color=color-170*4;
    g=0;
    b=255;
    r=(color/170.0)*255;
  }else if(color<1025){
    color=color-170*5;
    g=0;
    r=255;
    b=((173-color)/173.0)*255; //173 cause this loop absorbs the remainder of 1024/6
  }else{ //should never be called, but if someone calls for a number outside of the range, we should return black instead of some strange number
    r=0;
    g=0;
    b=0;
  }
  if(component=='r'){
    return r;
  }else if(component=='g'){
    return g;
  }else if(component=='b'){
    return b;
  }else{
    return 0; //return 0 if they call for a component other than r/g/b
  }
}
