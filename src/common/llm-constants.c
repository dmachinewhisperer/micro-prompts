#include <stddef.h>
#include "llm-types.h"

// example default configuration
const LLMClientConfig DEFAULT_LLMCLIENT_CONFIG = {
    .llmconfig = {
        .chat = 0,     // oneshot prompt mode
        .structured_output = 0,             //output type         
        .provider = GOOGLE_GEMINI,                  // default provider
        .base_url = NULL,
        .api_key = NULL,
        .model_name = NULL,
        .version = NULL,
        .max_tokens = MAX_LLM_OUTPUT_TOKENS,
        .temperature = 0.7f,
        .json_response_schema = NULL,
        .api_endpoint = NULL,
        .top_p = 1,
        .top_k = 50
    },
    .llmdata = {                           
        .prompt = NULL,
        .system = NULL,
        .file = {
            .data = NULL,
            .mime = NULL,
            .nbytes = 0,
            .uri = NULL,
            },       
        .response = {
            .return_raw = 0,
        },    
    },
    .user_state = NULL                      // optional user state pointer
};

//model configurations
const ProviderFeaturePool provider_google_gemini = {
    .supported_features = {
        TEXT_INPUT,
        TEXT_INPUT_WITH_REMOTE_FILE,
        TEXT_INPUT_WITH_LOCAL_FILE,
        TEXT_INPUT_WITH_STRUCTURED_OUTPUT,
    },
    .num_supported_features = 4,
    .provider = GOOGLE_GEMINI,
};


const ProviderFeaturePool provider_openai_gpt = {
    .supported_features = {
        TEXT_INPUT,
        TEXT_INPUT_WITH_REMOTE_IMG,
        TEXT_INPUT_WITH_LOCAL_IMG,
        TEXT_INPUT_WITH_LOCAL_AUDIO,
        TEXT_INPUT_WITH_STRUCTURED_OUTPUT,
    },
    .num_supported_features = 4,
    .provider = OPENAI_GPT,
};

const ProviderFeaturePool provider_qroq_ = {
    .supported_features = {
        TEXT_INPUT,
    },
    .num_supported_features = 1,
    .provider = GROK,
};

//helper function to check if feature is supported
int _is_feature_supported(GlobalFeaturePool gfeature, ProviderFeaturePool pfeature) {
    for (size_t i = 0; i < pfeature.num_supported_features; i++) {
        if (pfeature.supported_features[i] == gfeature) {
            return 1;
        }
    }
    return 0;
}