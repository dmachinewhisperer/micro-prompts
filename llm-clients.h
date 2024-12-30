#ifndef ESP32_LLM_PROMPTING_LIB_H_
#define ESP32_LLM_PROMPTING_LIB_H_
#include <stdint.h>
#include "espidf-http-rest-api.h"

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
    TEXT_ONLY,
    TEXT_WITH_LOCAL_BASE64_ENCODED_FILE,
    TEXT_WITH_REMOTE_FILE,
    AUDIO_TRANSCRIBING,
    AUTO_SPEECH_RECOGNITION,
    FEATURE_COUNT  // Automatically tracks number of features
} GlobalFeaturePool;

// model configuration.
typedef struct {
    uint8_t chat;       //chat > 0 => maintain chat history, else oneshot prompt
    ProviderName provider;
    GlobalFeaturePool feature;
    char *base_url;    // Base URL of the LLM API provider
    char *api_key;     // API key for authentication
    char *model_name;  // Model name or ID (e.g., "gpt-4" or "cohere-llm")
    char *version;     // Optional API version, if applicable (e.g., "v1")
    int max_tokens;    // Maximum number of tokens for the response
    float temperature; // Controls the randomness of the model (0.0-1.0)
    int top_p;         // Nucleus sampling value (0-1)
    int top_k;
} LLMConfig;


// Data feed into the model
typedef struct {
    char *prompt;
    union {
        char *file_uri;
        char *file_dir;
    } file;
    char *mime;
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


//***********************************add more providerconfig below

//constants
extern const LLMClientConfig DEFAULT_LLMCLIENT_CONFIG;
extern const ProviderFeaturePool provider_google_gemini;

//*********************************function prototypes
char* prompt(esp_http_client_handle_t client, LLMClientConfig *llmconfigs);

#endif //ESP32_LLM_PROMPTING_LIB_H_