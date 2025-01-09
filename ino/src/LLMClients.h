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

//#define USE_GOOGLE_CLIENT
#define USE_OPENAI_CLIENT

class LLMClient {
private:
    HTTPClient httpClient;
    WiFiClientSecure wifiClient;

    LLMClientConfig config;

    void ___init(String url, const char* headers[][2]);
    String ___request(LLMClientConfig* config,
                       char* (*build_request)(LLMClientConfig*), 
                       char* (*parse_response)(LLMClientConfig*, const char*));

#ifdef USE_GOOGLE_CLIENT 
    void init_google_client();       
    String prompt_google_gemini();
#endif
#ifdef USE_OPENAI_CLIENT
    void init_openai_client();
    String prompt_openai_gpt();
#endif
public:
    LLMClient();
    void begin(const char *apiKey, const char *modelName, ProviderName provider);

    void setFileProperties(const char *mime, const char *uri, const char *data, size_t nbytes);
    void setProviderURL(const char *base, const char *version, const char *endpoint);
    void setProviderFeature(GlobalFeaturePool feature);
    void setJSONResponseSchema(const char *json);
    void setTemperature(float temperature);
    void setMaxTokens(int maxTokens);
    void setTopP(float topP);
    void setTopK(int topK);
    void retainChatContext(int nchat_msgs);
    void returnRawResponse();
    String prompt(String promptText);

    ~LLMClient();
};

#endif // LLM_CLIENT_H
