#include "LLMClients.h"
//#include "common/providers/google.h"
#include "common/providers/openai.h"
#include "common/utils/utils.h"
#define MAX_HTTP_RESPONSE_LENGTH 3 * 1024



LLMClient::LLMClient() {
    config = DEFAULT_LLMCLIENT_CONFIG;
}

void LLMClient::begin(const char *apiKey, const char *modelName, ProviderName provider) {

    config.llmconfig.api_key = apiKey;
    config.llmconfig.model_name = modelName;
    config.llmconfig.provider = provider;    

#ifdef USE_GOOGLE_CLIENT
    init_google_client();
#endif
#ifdef USE_OPENAI_CLIENT
    init_openai_client();
#endif    
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
    config.llmconfig.structured_output = 1; 
}

//must be called before ::begin() if using different endpoints
void LLMClient::setProviderURL(const char *base, const char *version, const char *endpoint){
    config.llmconfig.base_url = base;
    config.llmconfig.version = version;
    config.llmconfig.api_endpoint = endpoint;
}
void LLMClient::setFileProperties(const char *mime, const char *uri, const char *data, size_t nbytes){
    config.llmdata.file.mime = mime; 
    config.llmdata.file.uri = uri;
    config.llmdata.file.data = (const unsigned char*)data;
    config.llmdata.file.nbytes = nbytes;

}

void LLMClient::retainChatContext(int nchat_msgs){
    config.llmconfig.chat = nchat_msgs; 
}
void LLMClient::returnRawResponse(){
    config.llmdata.response.return_raw = 1; 
}

void LLMClient::___init(String url, const char* headers[][2]){
#ifdef ESP32
        httpClient.begin(url);
#elif defined(ESP8266)
        wifiClient.setInsecure();
        httpClient.begin(wifiClient, url);
#endif

    for (int i = 0; headers[i][0] != NULL && headers[i][1] != NULL; i++) {
        httpClient.addHeader(String(headers[i][0]), String(headers[i][1]));
    }
}

String LLMClient::___request(LLMClientConfig* config,
                       char* (*build_request)(LLMClientConfig*), 
                       char* (*parse_response)(LLMClientConfig*, const char*)){


    String request = String(build_request(config));
    if (request.length() == 0) {
        WRITE_LAST_ERROR("Failed to build request");
        return String();
    }
    DEBUG_WRITE(request);

    int httpCode = httpClient.POST(request);
    String response = "";
    if (httpCode > 0) {
        response = httpClient.getString();
        DEBUG_WRITE("httpCode > 0: ");
    } else {
        WRITE_LAST_ERROR("Error in HTTP request: code <0");
        return String();
    }
    if(config->llmdata.response.return_raw > 0){
        return response;
    }
    char *cstr = parse_response(config, response.c_str());
    if(cstr==NULL){
        return response; 
    }
    String result(cstr);
    free(cstr);
    return response.length() > 0 ? result : String();                         

}

#ifdef USE_GOOGLE_CLIENT
void LLMClient::init_google_client() {
    const char *base_url = "https://generativelanguage.googleapis.com";
    String url = String(base_url) + "/v1beta/models/" +
                                 config.llmconfig.model_name + ":generateContent?key=" + config.llmconfig.api_key;

    const char* headers[][2] = {
    {"Content-Type", "application/json"},
    {NULL, NULL}
    };
    ___init(url, headers);
}
String LLMClient::prompt_google_gemini() {
    
    return  ___request(&config, build_google_request, parse_google_response); 
}
#endif

void LLMClient::init_openai_client(){
    const char *default_base_url = "https://api.openai.com";
    const char *default_version = "v1";
    const char *default_endpoint = "chat/completions";
    
    /** TODO: refactor this block
     * multiple String concat is inefficient and fragments memory
    */
    String url;

    if (config.llmconfig.base_url && 
        config.llmconfig.api_endpoint && 
        config.llmconfig.version) {

        url = String(config.llmconfig.base_url) + "/" + 
                    config.llmconfig.version + "/" + 
                    config.llmconfig.api_endpoint;

    } else {
        if (config.llmconfig.version) {
            url = String(default_base_url) + "/" + 
                        config.llmconfig.version + "/" + 
                        default_endpoint;
        } else {
            url = String(default_base_url) + "/" + 
                        default_version + "/" + 
                        default_endpoint;
        }
    }
    
    char auth_header_value[64];
    snprintf(auth_header_value, sizeof(auth_header_value), "Bearer %s", config.llmconfig.api_key);

    const char* authorization_header = auth_header_value;

    const char* headers[][2] = {
    {"Content-Type", "application/json"},
    {"Authorization", authorization_header},
    {NULL, NULL}
    };

    ___init(url, headers);
    
}

String LLMClient::prompt_openai_gpt(){

    return  ___request(&config, build_openai_request, parse_openai_response); 
}

String LLMClient::prompt(String promptText)
{
    config.llmdata.prompt = promptText.c_str(); 
#ifdef USE_GOOGLE_CLIENT    
    if(config.llmconfig.provider == GOOGLE_GEMINI){
        return prompt_google_gemini();
    }
#endif

#ifdef USE_OPENAI_CLIENT
    if(config.llmconfig.provider == OPENAI_GPT){
        return prompt_openai_gpt();
    }
#endif    
    WRITE_LAST_ERROR("Selected model is not supported");
    return String();
}

LLMClient::~LLMClient() {
    LLMClient::httpClient.end();
}

