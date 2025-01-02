#include "LLMClients.h"
#include "common/providers/google.h"

#define MAX_HTTP_RESPONSE_LENGTH 3 * 1024

LLMClient::LLMClient() {
    config = DEFAULT_LLMCLIENT_CONFIG;
}

void LLMClient::begin(const char *apiKey, const char *modelName, ProviderName provider) {

    config.llmconfig.api_key = apiKey;
    config.llmconfig.model_name = modelName;
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

void LLMClient::setJSONResponseSchema(const char *json) {
    config.llmconfig.json_response_schema = json;
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
    char *cstr = parse_google_response(response.c_str());
    String result(cstr);
    free(cstr);
    return response.length() > 0 ? result : String();
}

String LLMClient::prompt(const char *promptText) {

    config.llmdata.prompt = promptText;

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
