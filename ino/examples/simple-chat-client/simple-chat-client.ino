#ifdef ESP32
    #include <WiFi.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
#endif

#include <LLMClients.h>

const char* ssid = "myssid";              // Replace with your WiFi SSID
const char* password = "mypassword";      // Replace with your WiFi Password

//add error logging hooks if required: see docs
extern "C" {
    void llmclients_write_last_error(const char *err);
}
void llmclients_write_last_error(const char *err){
  Serial.println(err);
  }
//end logging

String promptText;
LLMClient<GoogleClient> googleClient; //see docs for a list of supported clients

void setup() {
    Serial.begin(115200);
    while (!Serial) {
      ;
    }
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    Serial.println("IP Address: " + WiFi.localIP().toString());

    

    googleClient.begin(
    "your_api_key_your_api_key_your_api_key_",  //replace with you api key
    "gemini-1.5-flash");                        //replace with model name to prompt
    
    //other settings are optional based on your use case. see docs             
    googleClient.setMaxTokens(512);             //hard text limit model can generate
    //googleClient.setTemperature(0.7)            //0.5 - 0.7 is typical
    //googleClient.returnRawResponse()            //library returns json response instead of parsed response. 
    googleClient.setSystemPrompt(
      "You are a helpful assistant");           //use it to steer overall model behaviour.
    //googleClient.retainChatContext(10);         //when enabled, up to 10 most recent prompt message is stored to preserve context

    Serial.println("Chat Client Ready. ");
    Serial.println("\nUser: >");
}

void loop() {

  if (Serial.available() > 0) {
    char incomingChar = Serial.read();
    if (incomingChar == '\n') {
      Serial.println(promptText);
      Serial.println("\nModel: >");
      
      String modelOutput = googleClient.prompt(promptText);
      promptText = "";
      if (modelOutput.isEmpty()) {
          Serial.println("Prompting failed");
      } else {
          Serial.println(modelOutput);
      }
       //discard any input entered when client is not ready for text
      while (Serial.available() > 0) {
        Serial.read();
        }

      //track heap usage
      Serial.print("heap total:largest chunk = ");
      Serial.print(ESP.getFreeHeap());  
      Serial.print(":");
#ifdef ESP32      
      Serial.println(ESP.getMaxAllocHeap());
#elif defined(ESP8266)      
      Serial.print(ESP.getMaxFreeBlockSize());      
#endif      
      Serial.println("\nUser: >");
      
    }
    else {
      promptText += incomingChar;
    }
  }
}
