#ifndef LLM_CLIENT_H
#define LLM_CLIENT_H

#ifdef ESP32
    #include <WiFi.h>
    #include <HTTPClient.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
#else
    #error "Platform is Neither ESP32 nor ESP8266. Build Aborted!"
#endif

#include <common/types.h>

class LLMClient {
private:
    WiFiClient wifiClient;
    HTTPClient httpClient;

    LLMClientConfig config;
    String prompt_google_gemini();

public:
    LLMClient();
    void begin(const String &apiKey, const String &modelName, ProviderName provider);

    void setProviderFeature(GlobalFeaturePool feature);
    void setTemperature(float temperature);
    void setMaxTokens(int maxTokens);
    void setTopP(float topP);
    void setTopK(int topK);

    String prompt(const String &promptText);

    ~LLMClient();
};

#endif // LLM_CLIENT_H
