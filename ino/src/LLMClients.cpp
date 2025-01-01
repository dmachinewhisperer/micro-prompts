#ifdef ESP32
    #include <WiFi.h>
    #include <HTTPClient.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
#else
    #error "Platform is Neither ESP32 nor ESP8266. Build Aborted!"
#endif

#include "LLMClients.h"
#include "common/providers/google.h"

#define MAX_HTTP_RESPONSE_LENGTH 3 * 1024

LLMClient::LLMClient() {
    config = DEFAULT_LLMCLIENT_CONFIG;
}

void LLMClient::begin(const String &apiKey, const String &modelName, ProviderName provider) {
    config.llmconfig.api_key = apiKey.c_str();  
    config.llmconfig.model_name = modelName.c_str();
    config.llmconfig.provider = provider;
}

void LLMClient::setTemperature(float temp) {
    config.llmconfig.temperature = temp;
}

void LLMClient::setMaxTokens(int maxTokens) {
    config.llmconfig.max_tokens = maxTokens;
}

void LLMClient::setProviderFeature(GlobalFeaturePool feature) {
    config.llmconfig.feature = feature;
}

String LLMClient::prompt_google_gemini() {
    const char *base_url = "https://generativelanguage.googleapis.com";
    String google_generate_url = String(base_url) + "/v1beta/models/" +
                                 config.llmconfig.model_name + ":generateContent?key=" + config.llmconfig.api_key;

    #ifdef ESP32
        HTTPClient httpclient;
        httpclient.begin(google_generate_url);
    #elif defined(ESP8266)
        HTTPClient httpclient;
        //wifiClient.setInsecure();
        httpclient.begin(wifiClient, google_generate_url);
    #endif

    // build_google_request() returns char* (C-style)
    String request = String(build_google_request(&config));
    if (request.length() == 0) {
        Serial.println("Failed to build request");
        httpclient.end();
        return String();
    }

    // Make the POST request
    int httpCode = httpclient.POST(request);
    String response = "";
    if (httpCode > 0) {
        response = httpclient.getString();
    } else {
        Serial.println("Error in HTTP request");
    }

    httpclient.end();

    // parse_google_response() returns char* (C-style)
    return response.length() > 0 ? String(parse_google_response(response.c_str())) : String();
}

String LLMClient::prompt(const String &promptText) {
    config.llmdata.prompt = promptText.c_str(); // Use const char* here

    switch (config.llmconfig.provider) {
        case GOOGLE_GEMINI:
            return prompt_google_gemini();
        default:
            Serial.println("Unsupported provider");
            return String();
    }
}

LLMClient::~LLMClient() {
    
}
