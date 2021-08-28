/*
                                                        NSYNC 


                          A web xerver that lets you control and monitor your grow op
 
                     This sketch will print the IP address of your WiFi Shield (once connected)
                                              to the Serial monitor. 
                   
 
                               From there, you can open that address in a web browser
  
                               This is written for a network using WPA encryption. For
                                 WEP or WPA, change the Wifi.begin() call accordingly.

                                                       Circuit:
                                              * WiFi shield attached
                                                        * LED 

                                             created  for blackarmy 12 aug 21
                                                                 by acetone 7
 
 */


#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>


// Constants

const char *ssid = "krypton";
const char *password = "9845076686";
const char *msg_toggle_green = "toggleGreen";
const char *msg_toggle_red = "toggleRed";
const char *msg_get_green = "getGreenState";
const char *msg_get_red = "getRedState";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;
const int greenPin = 23;
const int redPin = 22;



// Globals
AsyncWebServer server(3500);
WebSocketsServer webSocket = WebSocketsServer(ws_port);
char msg_buf[10];
int green_state = 0;
int red_state = 0;




/* Functions
 *  
 */

 // Callback: receiving any WebSocket message
 void onWebSocketEvent(uint8_t client_num,
                        WStype_t type,
                        uint8_t * payload,
                        size_t length) {

  // figure out the type of web socket event
  switch(type){
    // client has disconnected 
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected! \n ", client_num);
      break;
    
    // new client has connected
    case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(client_num);
      Serial.printf("[%u] Connection from ",  client_num);
      Serial.println(ip.toString());
    }
    break;

    // handle text messages from client
    case WStype_TEXT:
    {
      // print out raw string message
      Serial.printf("[%u] Received text: %s\n", client_num, payload);

      // toggle green led
      if( strcmp((char *)payload, "toggleGreen") == 0 ) {
        green_state = green_state ? 0 : 1;
        Serial.printf("toggling green led to %u\n", green_state);
        digitalWrite(greenPin, green_state);
      }
      // toggle red led
      else if( strcmp((char *)payload, "toggleRed") == 0 ) {
        red_state = red_state ? 0 : 1;
        Serial.printf("toggling red led to %u\n", red_state);
        digitalWrite(redPin, red_state);
      }

      else if( strcmp((char *)payload, "getGreenState") == 0 ) {
        sprintf(msg_buf, "%d", green_state);
        Serial.printf("sending to [%u]: %s\n", client_num, msg_buf);
        webSocket.sendTXT(client_num, msg_buf);
        
      } 
      else if( strcmp((char *)payload, "getRedState") == 0 ) {
        sprintf(msg_buf, "%d", red_state);
        Serial.printf("sending to [%u]: %s\n", client_num, msg_buf);
        webSocket.sendTXT(client_num, msg_buf);
        
      } 
      else {
        // message not recognized
        Serial.println("[%u] Message not recognized");
      }
      break;
      
    }
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
    
  }


                       
}


// callback: send homepage
void onIndexRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() + 
                  "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/index.html", "text/html");
}

// callback: send style sheets
void onCSSRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() + 
                  "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/style.css", "text/css");
}


// Set your Static IP address

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress local_IP(192, 168, 1, 55);

void setup() {
  // put your setup code here, to run once:
  // start serial port for debugging
  Serial.begin(115200);

  // init leds and turn off all
  pinMode(greenPin, OUTPUT);      // set the green LED pin mode
  pinMode(redPin, OUTPUT);      // set the red LED pin mode
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);

  // make sure file system is cool
  if( !SPIFFS.begin()) {
    Serial.println("eroor mounting SPIFFS");
    while(1);
  }
  
  // connecting to krypton
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  
  Serial.println(WiFi.localIP());

  // on http request for root, provide index.html file
  server.on("/", HTTP_GET, onIndexRequest);

  // on http request for style sheet, provide style.css
  server.on("/style.css", HTTP_GET, onCSSRequest);
  
    
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

}

void loop() {
  // put your main code here, to run repeatedly:

  webSocket.loop();

}
