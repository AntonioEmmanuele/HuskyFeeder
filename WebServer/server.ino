#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <cstring> 
#include <stdlib.h>
#include <string>
#include<SoftwareSerial.h> 

using namespace std;

#define TXD 15
#define RXD 13

#define HFEED_MODE_MANUAL      0
#define HFEED_MODE_TIMED       1
#define HFEED_MODE_AUTOMATIC   2
#define HFEED_MODE_BLANK       3

#define DEADLINES_UB           16

SoftwareSerial s(RXD,TXD);
//This is the configuration structure for the application
typedef struct HuskyFeed_CFG{
    //  Working mode
    uint8_t     mode;
    //  Quantity of food to be served.
    uint8_t    food_quantity;

    /* Time fields only */ 

    //  Number of deadlines in the list
    uint8_t      deadlines_num;
    uint8_t     deadlines_hours[DEADLINES_UB];
    uint8_t     deadlines_minutes[DEADLINES_UB];
    uint8_t     deadlines_seconds[DEADLINES_UB];
    //Starting hour
    uint8_t     starting_hours;
    uint8_t     starting_minutes;
    uint8_t     starting_seconds;

    //  Tells if the configuration must persist after the last deadline, 
    //  it is periodic if this field is >0
    uint8_t    periodic;
} CFG; 

/* Set these to your desired credentials. */
const char *ssid = "HotspotEmmo"; //Enter your WIFI ssid
const char *password = "HelloBoys"; //Enter your WIFI password
ESP8266WebServer server(80);

void handleRoot();
void handleSave();
void handleCFG (); 

void setup() {
   Serial.begin(9600);
   s.begin(9600);
   Serial.println();
   Serial.print("Configuring access point...");
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println("");
   Serial.println("WiFi connected");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
   server.on ( "/", handleRoot );
   server.on ("/save", handleSave);
   server.on ("/send_cfg", handleCFG);  
   server.begin();
   Serial.println ( "HTTP server started" );  
  
}
void loop() {
 server.handleClient();
} 

void handleRoot() {
 server.send(200, "text/html", "<body data-new-gr-c-s-check-loaded='14.1063.0' data-gr-ext-installed=''>Modalità:&ensp; <input type='text' id='ModalitaInput' size='30' required=''><br><br>Quantità di cibo:&ensp; <input type='text' id='FoodInput' size='30' required=''><br><br>Deadline Ore:&ensp; <input type='text' id='DeadlineOreInput' size='30' required=''><br><br>Deadline Minuti:&ensp; <input type='text' id='DeadlineMinutiInput' size='30' required=''><br><br>Ore Correnti:&ensp; <input type='text' id='OreCorrentiInput' size='30' required=''><br><br>Minuti Correnti:&ensp; <input type='text' id='MinutiCorrentiInput' size='30' required=''><br><br>Num Deadline:&ensp; <input type='text' id='NumDeadlineInput' size='30' required=''><br><br>Periodico:&ensp; <input type='text' id='PeriodicoInput' size='30' required=''><br><br><button name='subject' type='submit' value='Submit' onclick='test()'><script>function test() { var modalita = document.getElementById('ModalitaInput').value; var quantita = document.getElementById('FoodInput').value; var hdeadline = document.getElementById('DeadlineOreInput').value; var mdeadline = document.getElementById('DeadlineMinutiInput').value; var mincurr = document.getElementById('MinutiCorrentiInput').value; var hcurr = document.getElementById('OreCorrentiInput').value; var numdeadline = document.getElementById('NumDeadlineInput').value; var periodico = document.getElementById('PeriodicoInput').value; let data = {}; data['Modalita'] = modalita;  data['Cibo'] = quantita;  data['HDeadline'] = hdeadline;  data['MDeadline'] = mdeadline;  data['HCurr'] = hcurr;  data['MCurr'] = mincurr;  data['NumDeadline'] = numdeadline;  data['Periodico'] = periodico;       var xhr = new XMLHttpRequest(); xhr.open('POST', '/send_cfg'); xhr.setRequestHeader('Content-Type', 'application/json'); xhr.send(JSON.stringify(data)); }</script>Submit</button></body>");
}

void handleSave() {
  Serial.println ("hola"); 
  if (server.arg("pass") != "") {
    Serial.println(server.arg("pass"));
  }
}

void handleCFG ()
{
    //Serial.println ("Triggered"); 
    
     // allocate the memory for the document
      const size_t CAPACITY = JSON_OBJECT_SIZE(1);
      StaticJsonDocument<400> doc;
      CFG config;

      for (int i=0; i<DEADLINES_UB; i++) {
        config.deadlines_hours[i] = 0;
        config.deadlines_minutes[i] = 0;
        config.deadlines_seconds[i] = 0;
      }
      config.starting_seconds = 0;
      config.starting_hours = 0; 
      config.starting_minutes = 0; 
      config.deadlines_num = 0; 
            
      deserializeJson(doc, server.arg("plain"));

      //serializeJsonPretty (doc, Serial); 
      
      const String modalita = doc["Modalita"];
      const int qta = doc["Cibo"];
      const string hdeadline  = doc["HDeadline"];
      const string mdeadline = doc["MDeadline"];
      const int hcurr = doc["HCurr"];
      const int mcurr = doc["MCurr"];
      const int numdeadline = doc["NumDeadline"];
      const String periodico = doc["Periodico"];
      int deadline_ora [10] = {0};
      int deadline_min [10] = {0};

      //Serial.println ("Mode"); 
      if (modalita == "MANUAL") {
        config.mode = HFEED_MODE_MANUAL;
      }
      else if (modalita == "TIMED") {
        config.mode = HFEED_MODE_TIMED;
      }
      else if (modalita == "AUTOMATIC") {
        config.mode = HFEED_MODE_AUTOMATIC;
      }
      else {
        config.mode = HFEED_MODE_BLANK;
      }
    
    //Serial.println ("Deadline"); 

    string delimiter = ",";

    string hdeadlinetmp = hdeadline; 
    string mdeadlinetmp = mdeadline; 
    int start=0,end=0,i=0;
    if (hdeadlinetmp != "") {
      start = 0; 
      end = hdeadlinetmp.find(delimiter);
      i = 0;
      while (end != -1) {
        config.deadlines_hours[i] = std::__cxx11::stoi(hdeadlinetmp.substr(start, end - start));
        start = end + delimiter.size();
        end = hdeadlinetmp.find(delimiter, start);
        i++;
      }
      config.deadlines_hours[i] = std::__cxx11::stoi(hdeadlinetmp.substr(start, end - start));
    }

    if (mdeadlinetmp != "") {
      start = 0; 
      end = mdeadlinetmp.find(delimiter);
      i = 0;
      while (end != -1) {
        config.deadlines_minutes[i] = std::__cxx11::stoi(mdeadlinetmp.substr(start, end - start));
        start = end + delimiter.size();
        end = mdeadlinetmp.find(delimiter, start);
        i++;
      }
      config.deadlines_minutes[i] = std::__cxx11::stoi(mdeadlinetmp.substr(start, end - start));
    }
    
//Serial.println ("Hours"); 

      config.starting_hours = hcurr; 
      config.starting_minutes = mcurr; 
      config.deadlines_num = numdeadline; 
      config.food_quantity = qta;

//Serial.println ("Periodico"); 
      if (periodico == "Sì") {
        config.periodic = 1;
      }
      else {
        config.periodic = 0;
      }

      char to_send [sizeof(config)];

      memcpy (to_send, (const char*) &config, sizeof(config)); 

    for (int i=0; i < sizeof(config); i++) {
        s.write (to_send[i]); 
    }

/*
      Serial.println(modalita); 
      Serial.println(qta); 
      Serial.println(numdeadline); 
      for (int i=0; i<numdeadline; i++) {
        Serial.print(config.deadlines_hours[i]);
        Serial.print (" ");
        Serial.print(config.deadlines_minutes[i]);
        Serial.println (""); 
      }
      Serial.println (hcurr); 
      Serial.println (mcurr); 
      Serial.println (periodico); 
*/
      Serial.println(config.mode);
      Serial.println(config.food_quantity);
      Serial.println(config.deadlines_num);
      for (int i=0; i<config.deadlines_num; i++) {
        Serial.print(config.deadlines_hours[i]);
        Serial.print (" ");
        Serial.print(config.deadlines_minutes[i]);
        Serial.print (" ");
        Serial.print(config.deadlines_seconds[i]);
        Serial.println (" ");
      }
      Serial.println(config.starting_hours);
      Serial.println(config.starting_minutes);
      Serial.println(config.starting_seconds);
      Serial.println(config.periodic);
      
      server.send ( 200, "text/json", "{success:true}" );
     handleRoot();
}
