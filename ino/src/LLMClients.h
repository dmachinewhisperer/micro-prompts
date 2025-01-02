#ifndef LLM_CLIENT_H
#define LLM_CLIENT_H

#ifdef ESP32
    #include <HTTPClient.h>
#elif defined(ESP8266)
    #include <ESP8266HTTPClient.h>
#else
    #error "Platform is Neither ESP32 nor ESP8266. Build Aborted!"
#endif

#include <WiFiClientSecure.h>
#include <common/llm-types.h>


class LLMClient {
private:
    HTTPClient httpClient;
    WiFiClientSecure wifiClient;

    LLMClientConfig config;
    String prompt_google_gemini();

public:
    LLMClient();
    void begin(const char *apiKey, const char *modelName, ProviderName provider);

    
    void setProviderFeature(GlobalFeaturePool feature);
    void setJSONResponseSchema(const char *json);
    void setTemperature(float temperature);
    void setMaxTokens(int maxTokens);
    void setTopP(float topP);
    void setTopK(int topK);

    String prompt(const char *promptText);

    ~LLMClient();
};

#endif // LLM_CLIENT_H
