//Example program on how to prompt with frames from a video feed and 
//automate the processing of the model output using structured output using the LLMClient library
//MJPEG video format encodes each frame in jpeg standard independent of surrounding frames  so they are 
//simpler to process. This example reads jpeg frames stored in a file system and instructs a model to run inference 
//and return a structured output. Note that the extent to which the model output correponds to the desired output depends
//on the quality of the model your providers hosts. 

//Before compiling sketch, download this tool and follow the instructions to install
//Upload the /data folder in the sketch directory to the esp32 then compile and run

#include <WiFi.h>
#include <SPIFFS.h>
#include <LLMClients.h>

#include "image_names.h"

#define MAX_FILE_SIZE (8 * 1024)          //8kb
char filedata[MAX_FILE_SIZE];             //preallocated space for file contents
const char* ssid = "myssid";              // Replace with your WiFi SSID
const char* password = "mypassword";      // Replace with your WiFi Password

String modelOutput; 
int image_index = 0; 

/**add error logging hooks if required: see docs**/
extern "C" {
    void llmclients_write_last_error(const char *err);
}
void llmclients_write_last_error(const char *err){
  Serial.println(err);
  }
/**end logging**/

int MIN(size_t a, size_t b){
  
}
LLMClient<GoogleClient> googleClient; //see docs for a list of supported clients
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully!"); 
     
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.println("IP Address: " + WiFi.localIP().toString() );

  googleClient.begin(
  "your_api_key_your_api_key_your_api_key_",  //replace with you api key
  "gemini-1.5-flash");                        //replace with model name to prompt
  
  googleClient.setSystemPrompt("You are a helpful assistant identifying a cctv footage");    
  googleClient.setProviderFeature(TEXT_INPUT_WITH_LOCAL_FILE);
  const char* json_schema = R"(
      {
        "type": "object",
        "properties": {
          "no_of_people": { "type": "integer" },
          "action_of_people": { "type": "string" },
          "concise_description": { "type": "string" }
        }
      }
      )";

  googleClient.setJSONResponseSchema(json_schema);     
}

void loop() {
  while(image_files[image_index]){
    File file = SPIFFS.open(image_files[image_index], "r");
    if (file) {
      size_t filesize = file.size();
    size_t nbytes = filesize<MAX_FILE_SIZE?filesize:MAX_FILE_SIZE;

    size_t index = 0;
    while (file.available()&& index<MAX_FILE_SIZE) {
      filedata[index] = file.read();
      index++;
    }
    file.close();
      googleClient.setFileProperties(
        "image/jpeg",     //mime
        NULL, 
        filedata,         //filedata
        nbytes           //size of filedata in in bytes
        );
      modelOutput = googleClient.prompt("What do you see in the image? Return your response strictly in the"
                                        "json schema supplied with this request with fields: no of people, action of people in the"
                                        "image, one line concise description of the scene");
      if(modelOutput.isEmpty()){
        Serial.print("image name: ");
        Serial.println(image_files[image_index]);
        Serial.println(modelOutput);
        Serial.println();
        }
      else{
        Serial.println("Prompting failed");
        }  
      image_index++; 
      
    }else{
      Serial.println("failed to read file");        
    }

    delay(100); //guard to not execeed  api quota 
  }  
   
  while(1){
    ;
    }
}
