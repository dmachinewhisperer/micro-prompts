#ifndef LLM_PROMPTING_LIB_TYPES_H_
#define LLM_PROMPTING_LIB_TYPES_H_
#include <stdint.h>


//the max_tokens specified here is just not arbitary. Http clients are configured with a max  
//response payload size. Say this is 5kb(in the case of espidf http-client bundled with this lib),average 
//lenght of a token in english is around 4, so 1024 * 4 = 4kb reserving the remaining 1kb for error slack 
//.max_tokens  > MAX_LLM_OUTPUT_TOKENS may cause models json response to be cut off and parsing impossible
#define MAX_LLM_OUTPUT_TOKENS   1024


#define MAX_SUPPORTED_FEATURES_PER_PROVIDER 5

//model providers
typedef enum{
    GROK,
    GOOGLE_GEMINI,
    OPENAI_CHATGPT,
    DEEPSEEK,
    HUGGING_FACE,
} ProviderName;

//global feature pool
typedef enum {
    TEXT_INPUT,                                   // Text as input, no other data
    TEXT_INPUT_WITH_LOCAL_BASE64_FILE,            // Text input with a locally base64-encoded file attached
    TEXT_INPUT_WITH_REMOTE_FILE_URL,              // Text input with a remote file URL provided
    AUDIO_INPUT_FOR_CLASSIFICATION,               // Audio file input for classification tasks
    AUDIO_INPUT_FOR_SPEECH_TO_TEXT,               // Audio input for automatic speech recognition (ASR)
    TEXT_INPUT_WITH_STRUCTURED_OUTPUT,            // Text input with a structured/parsed output format  
    TOTAL_FEATURE_COUNT                           // Automatically tracks number of features
} GlobalFeaturePool;

// model configuration.
typedef struct {
    uint8_t chat;       //chat > 0 => maintain chat history, else oneshot prompt
    uint8_t structured_output;
    ProviderName provider;
    GlobalFeaturePool feature;
    char *base_url;    // Base URL of the LLM API provider
    const char *api_key;     // API key for authentication
    const char *model_name;  // Model name or ID (e.g., "gpt-4" or "cohere-llm")
    const char *version;     // Optional API version, if applicable (e.g., "v1")
    const char *json_response_schema; //string representing the how the output should be strctured. 
    int max_tokens;    // Maximum number of tokens for the response
    float temperature; // Controls the randomness of the model (0.0-1.0)
    int top_p;         // Nucleus sampling value (0-1)
    int top_k;
} LLMConfig;


// Data feed into the model
typedef struct {
    const char *prompt;
    struct {
        size_t nbytes;
        const char *uri;
        const char *mime;
        const unsigned char *data;
    } file;
    
} LLMData;

typedef struct{
    LLMConfig llmconfig;
    LLMData llmdata;
    void *user_state;
} LLMClientConfig;


//provider-specific supported features from global pool
//this is a subset of the global pool implemented for the model
typedef struct {
    const char *name;
    GlobalFeaturePool supported_features[MAX_SUPPORTED_FEATURES_PER_PROVIDER];
    size_t num_supported_features;
    ProviderName provider;
} ProviderFeaturePool;


//constants
extern const LLMClientConfig DEFAULT_LLMCLIENT_CONFIG;
extern const ProviderFeaturePool provider_google_gemini;


#endif //ESP32_LLM_PROMPTING_LIB_H_