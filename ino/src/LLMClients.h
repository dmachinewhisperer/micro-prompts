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
#include "common/cJSON/cJSON.h"

template<typename ClientType>
class LLMClient;

class LLMClientBase {
protected:
    HTTPClient httpClient;
#ifdef ESP8266    
    WiFiClientSecure wifiClient;
#endif    
    LLMClientConfig config;
public:
    virtual void init() = 0;
    virtual String prompt() = 0;
    virtual ~LLMClientBase() = default;
};

class GoogleClient : public LLMClientBase {
private:
    friend class LLMClient<GoogleClient>;
public:
    void init() override;
    String prompt() override;
};

class OpenaiClient : public LLMClientBase {
private:
    friend class LLMClient<OpenaiClient>;
public:
    void init() override;
    String prompt() override;
};

template <typename ClientType>
class LLMClient {
    static_assert(std::is_base_of<LLMClientBase, ClientType>::value, 
                                "ClientType must inherit from LLMClientBase");
private:
    ClientType client;
public:
    LLMClient() {
        client.config = DEFAULT_LLMCLIENT_CONFIG;
    }

    void begin(const char *apiKey, const char *modelName) {
        client.config.llmconfig.api_key = apiKey;
        client.config.llmconfig.model_name = modelName;  
        client.init();
    }

    void setTemperature(float temp) {
        client.config.llmconfig.temperature = temp;
    }
     
    void setMaxTokens(int maxTokens) {
        client.config.llmconfig.max_tokens = maxTokens;
    }

     
    void setProviderFeature(GlobalFeaturePool feature) {
        client.config.llmconfig.feature = feature;
    }
     
    void setJSONResponseSchema(const char *json) {
        client.config.llmconfig.json_response_schema = json;
        client.config.llmconfig.structured_output = 1; 
    }
    
    void setProviderURL(const char *base, const char *version, const char *endpoint){
        client.config.llmconfig.base_url = base;
        client.config.llmconfig.version = version;
        client.config.llmconfig.api_endpoint = endpoint;
    }
     
    void setFileProperties(const char *mime, const char *uri, const char *data, size_t nbytes){
        client.config.llmdata.file.mime = mime; 
        client.config.llmdata.file.uri = uri;
        client.config.llmdata.file.data = (const unsigned char*)data;
        client.config.llmdata.file.nbytes = nbytes;

    }
     
    void retainChatContext(int nchat_msgs){
        client.config.llmconfig.chat = nchat_msgs; 
    }

    void returnRawResponse(){
        client.config.llmdata.response.return_raw = 1; 
    }
  
    void setSystemPrompt(const char *system){
        client.config.llmdata.system = system; 
    }

    String prompt(String promptText)
    {
        client.config.llmdata.prompt = promptText.c_str(); 
        return client.prompt();
    }
     
    ~LLMClient() {
        client.httpClient.end();
        if(client.config.user_state!=NULL){
            cJSON_Delete((cJSON*)client.config.user_state);
        }
    }

};

#endif // LLM_CLIENT_H
