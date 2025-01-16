# Micro Prompt for .ino
Micro prompt library adapted for arduino IDE

## Supported Boards
ESP8266, ESP32

## Scoping

## Examples
Arduino
1. Basic Prompting
```
#include <WiFi.h>
#include <LLMClients.h>

//see Supported clients for a list of supported clients
LLMClient<GoogleClient> googleClient; 
void setup(){
    WiFi.begin(yourssid, yourpass);
    googleClient.begin(yourapikey, modelname);  //modename eg. "gemini-1.5-flash"
    String modelOutput = googleClient.prompt("Explain quantum chromodynamics concisely to a layman.");
    Serial.println(modelOutput);
}
void loop(){
    //
}
```

2. File prompting
Various providers support various file formats and mode of passing it to the model. In general there two mode of passing file: By giving the client a uri where the file is or bundling the file data with the prompt. Some clients support none, one or both methods. See here for a list of supported features per supported client. 
Additional lines for file prompting in setup() after client.begin():

Pass the file by uri: 
```
googleClient.setProviderFeature(TEXT_INPUT_WITH_REMOTE_FILE); //see all supported provider features here
googleClient.setFileProperties(
    "audio/wav",                        //mime
    "https://examplefileurl/file.wav",  //file uri
    NULL,
    0,
);
modelOut = googleClient.prompt("Transcribe this file");
```
Pass the file by attaching the data: 
```
//filedata(char*) and nbytes(int) are your file data and size respectively. 
googleClient.setProviderFeature(TEXT_INPUT_WITH_LOCAL_FILE);
googleClient.setFileProperties(
    "audio/wav",    //mime
    NULL, 
    filedata,       //filedata
    nbytes,         //size of filedata in in bytes
);
modelOutput = googleClient.prompt("Transcribe this audio");
```

After attaching a file, in all subsequent prompts it is bundled. The file attached can be a source of great overhead. Switch back to text only prompting by calling: `googleClient.setProviderFeature(TEXT_INPUT);`

3. Structured Output
Some models support getting structured output in form of json, xml etc. To enable:
```
const char *json_schema = "{"name": String}";
googleClient.setJSONResponseSchema(json_string); 
```
4. Retain chat history
You can let the model "rember past prompts" by enabling chat mode. In this mode, previous user requests and model responses are stored and bundled with subsequent prompts.
```
googleClient.retainChatContext(10);
```
10 is the number of chats to return. Note that higher numbers leads to higher heap usage and memory fragmentation.

5. Using a client for another provider. See here for explanation for of this feature
For example, Groq supports openais api format, here is how to use the openai client to prompt models hosted by Groq:
```
LLMClient<OpenaiClient> openaiClient; 
void setup(){
    WiFi.begin(yourssid, yourpass);
    openaiClient.setProviderURL(    //must be called before client.begin()
        "https://groq.com/,
        "v1",
        "chat/completions"
    );
    openaiClient.begin(yourapikey, modelname);          
    String modelOutput = openaiClient.prompt("How do most people define a good life?");
    Serial.println(modelOutput);
}
```
6. Other optional methods
```
    googleClient.setMaxTokens(512);     //maximum tokens to generate
    googleClient.setTemperature(0.7)    //0.5 - 0.7 is typical
    googleClient.returnRawResponse()    //library returns json response instead of parsed response. 
    googleClient.setSystemPrompt("You are a helpful assistant");    //use it to steer overall model behaviour. 
```
