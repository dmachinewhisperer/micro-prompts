#include "LLMClients.h"
#include "common/utils/utils.h"
#include "common/providers/google.h"
#include "common/providers/openai.h"

#define MAX_HTTP_RESPONSE_LENGTH 3 * 1024

static void ___init(HTTPClient& httpClient, 
#ifdef ESP8266
                    WiFiClientSecure& wifiClient, 
#endif                    
                    String url, const char* headers[][2]){

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

static String ___request(HTTPClient& httpClient, LLMClientConfig* config,
                       char* (*build_request)(LLMClientConfig*), 
                       char* (*parse_response)(LLMClientConfig*, const char*)){
    if(!config || !build_request || !parse_response )    {
        WRITE_LAST_ERROR("invalid function parameters");
        return String();
    }                
    String request = String(build_request(config));
    if (request.length() == 0) {
        WRITE_LAST_ERROR("Failed to build request");
        return String();
    }
    DEBUG_WRITE(request.c_str());

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


void GoogleClient::init() {
    config.llmconfig.provider = GOOGLE_GEMINI;
    const char *base_url = "https://generativelanguage.googleapis.com";
    String url = String(base_url) + "/v1beta/models/" +
                                 config.llmconfig.model_name + ":generateContent?key=" + config.llmconfig.api_key;

    const char* headers[][2] = {
    {"Content-Type", "application/json"},
    {NULL, NULL}
    };
    ___init(httpClient, 
#ifdef ESP8266    
            wifiClient, 
#endif            
            url, headers);
}
String GoogleClient::prompt() {

    return  ___request(httpClient, &config, build_google_request, parse_google_response); 
}


void OpenaiClient::init(){
    config.llmconfig.provider = OPENAI_GPT;
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

    ___init(httpClient, 
#ifdef ESP8266    
            wifiClient, 
#endif            
            url, headers);
    
}

String OpenaiClient::prompt(){

    return  ___request(httpClient, &config, build_openai_request, parse_openai_response); 
}

