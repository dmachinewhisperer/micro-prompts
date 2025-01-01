#ifdef ESP32
    //#include <WiFi.h>
    //#include <HTTPClient.h>
#elif defined(ESP8266)
    //#include <ESP8266WiFi.h>
    ///#include <WiFiClientSecure.h>
    //#include <ESP8266HTTPClient.h>
#endif

#include "LLMClients.h"
#include "common/providers/google.h"

#define MAX_HTTP_RESPONSE_LENGTH 3 * 1024

LLMClient::LLMClient() {
    config = DEFAULT_LLMCLIENT_CONFIG;
}

void LLMClient::begin(const String &apiKey, const String &modelName, ProviderName provider) {

    strncpy(api_key_buffer, apiKey.c_str(), API_KEY_MAX_LEN - 1);
    api_key_buffer[API_KEY_MAX_LEN - 1] = '\0';
    strncpy(model_name_buffer, modelName.c_str(), MODEL_NAME_MAX_LEN - 1);
    model_name_buffer[MODEL_NAME_MAX_LEN - 1] = '\0';

    config.llmconfig.api_key = api_key_buffer;
    config.llmconfig.model_name = model_name_buffer;
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
        httpClient.begin(google_generate_url);
    #elif defined(ESP8266)
        wifiClient.setInsecure();
        httpClient.begin(wifiClient, google_generate_url);
    #endif

    // build_google_request() returns char* (C-style)
    httpClient.addHeader("Content-Type", "application/json");
    String request = String(build_google_request(&config));
    if (request.length() == 0) {
        Serial.println("Failed to build request");
        httpClient.end();
        return String();
    }
    Serial.println(google_generate_url);
    Serial.println(request);

    // Make the POST request
    int httpCode = httpClient.POST(request);
    String response = "";
    if (httpCode > 0) {
        response = httpClient.getString();
        Serial.print("httpCode > 0: ");
        Serial.println(httpCode);
    } else {
        Serial.print("Error in HTTP request: code = ");
        Serial.println(httpCode);
    }

    httpClient.end();
    // parse_google_response() returns char* (C-style)
    return response.length() > 0 ? String(parse_google_response(response.c_str())) : String();
}

String LLMClient::prompt(const String &promptText) {
    strncpy(prompt_buffer, promptText.c_str(), PROMPT_TEXT_MAX_LEN - 1);
    prompt_buffer[PROMPT_TEXT_MAX_LEN - 1] = '\0';

    config.llmdata.prompt = prompt_buffer;

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
