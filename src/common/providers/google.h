#ifndef GOOGLE_H_
#define GOOGLE_H_

#include <stddef.h>
#include "../cJSON/cJSON.h"

#include "../types.h"
/**
API endpoint
const url = 'https://generativelanguage.googleapis.com/v1/models/gemini-pro:generateContent';

// Request body structure
const request = {
  contents: [{
    parts: [
      {
        text: "Your prompt here"
      },
      //Optional image part: provide image uri
      {"file_data":
            {"mime_type": "image/jpeg", "file_uri": '$file_uri'}
        },
      // Optional image part: provide image file path
      {
        inlineData: {
          mimeType: "image/jpeg",
          data: "BASE64_ENCODED_IMAGE_DATA"
        }
      }
    ]
  }],
  generationConfig: {
    temperature: 0.7,
    topK: 40,
    topP: 0.95,
    maxOutputTokens: 2048,
  }
};

//Response body structure
 * {
 *     "candidates": [{
 *         "content": {
 *             "parts": [{
 *                 "text": ""
 *             }],
 *             "role": ""
 *         },
 *         "finishReason": "",
 *         "index": 0,
 *         "safetyRatings": [{
 *             "category": "",
 *             "probability": ""
 *         }]
 *     }],
 *     "usageMetadata": {
 *         "promptTokenCount": 0,
 *         "candidatesTokenCount": 0,
 *         "totalTokenCount": 0
 *     }
 * }
 */


#ifdef __cplusplus
extern "C" {
#endif

char *build_google_request(LLMClientConfig *config);
char *parse_google_response(const char *response);

#ifdef __cplusplus
}
#endif


// Helper function to check if a feature is supported by Google provider
static int is_feature_supported(GlobalFeaturePool feature) {
    for (size_t i = 0; i < provider_google_gemini.num_supported_features; i++) {
        if (provider_google_gemini.supported_features[i] == feature) {
            return 1;
        }
    }
    return 0;
}

char *build_google_request(LLMClientConfig *config) {
    // Validate provider
    if (config->llmconfig.provider != GOOGLE_GEMINI) {
        return NULL;
    }

    // Validate feature support
    if (is_feature_supported(config->llmconfig.feature) == 0) {
        return NULL;
    }

    // Create root object
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return NULL;
    }

    // Create contents array
    cJSON *contents = cJSON_CreateArray();
    if (contents == NULL) {
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToObject(root, "contents", contents);

    // Create first content object
    cJSON *content = cJSON_CreateObject();
    if (content == NULL) {
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToArray(contents, content);

    // Create parts array
    cJSON *parts = cJSON_CreateArray();
    if (parts == NULL) {
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToObject(content, "parts", parts);

    // Add text part
    cJSON *text_part = cJSON_CreateObject();
    if (text_part == NULL) {
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToArray(parts, text_part);
    cJSON_AddStringToObject(text_part, "text", config->llmdata.prompt);

    // Add file part based on feature
    if (config->llmconfig.feature == TEXT_WITH_REMOTE_FILE) {
        cJSON *file_part = cJSON_CreateObject();
        cJSON *file_data = cJSON_CreateObject();
        if (file_part == NULL || file_data == NULL) {
            cJSON_Delete(root);
            return NULL;
        }
        cJSON_AddItemToArray(parts, file_part);
        cJSON_AddItemToObject(file_part, "file_data", file_data);
        cJSON_AddStringToObject(file_data, "mime_type", config->llmdata.mime);
        cJSON_AddStringToObject(file_data, "file_uri", config->llmdata.file.file_uri);
    }
    else if (config->llmconfig.feature == TEXT_WITH_LOCAL_BASE64_ENCODED_FILE) {
        cJSON *file_part = cJSON_CreateObject();
        cJSON *inline_data = cJSON_CreateObject();
        if (file_part == NULL || inline_data == NULL) {
            cJSON_Delete(root);
            return NULL;
        }
        cJSON_AddItemToArray(parts, file_part);
        cJSON_AddItemToObject(file_part, "inlineData", inline_data);
        cJSON_AddStringToObject(inline_data, "mimeType", config->llmdata.mime);
        // Placeholder for base64 data - will be added later
        cJSON_AddStringToObject(inline_data, "data", "BASE64_ENCODED_IMAGE_DATA");
    }

    // Add generation config
    cJSON *gen_config = cJSON_CreateObject();
    if (gen_config == NULL) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON_AddItemToObject(root, "generationConfig", gen_config);
    cJSON_AddNumberToObject(gen_config, "temperature", config->llmconfig.temperature);
    cJSON_AddNumberToObject(gen_config, "topK", config->llmconfig.top_k);
    cJSON_AddNumberToObject(gen_config, "topP", config->llmconfig.top_p);
    cJSON_AddNumberToObject(gen_config, "maxOutputTokens", config->llmconfig.max_tokens);

    // Convert to string
    char *request_str = cJSON_Print(root);
    cJSON_Delete(root);

    //responsibility lies on the caller to free request_str
    return request_str;
}

char *parse_google_response(const char *response) {
    if (response == NULL) {
        return NULL;
    }

    cJSON *root = cJSON_Parse(response);
    if (root == NULL) {
        return NULL;
    }

    // Navigate through the response structure to extract the text
    // Note: This is a placeholder - update according to actual Google API response structure
    cJSON *candidates = cJSON_GetObjectItem(root, "candidates");
    if (candidates != NULL && cJSON_IsArray(candidates)) {
        cJSON *first_candidate = cJSON_GetArrayItem(candidates, 0);
        if (first_candidate != NULL) {
            cJSON *content = cJSON_GetObjectItem(first_candidate, "content");
            if (content != NULL) {
                cJSON *parts = cJSON_GetObjectItem(content, "parts");
                if (parts != NULL && cJSON_IsArray(parts)) {
                    cJSON *first_part = cJSON_GetArrayItem(parts, 0);
                    if (first_part != NULL) {
                        cJSON *text = cJSON_GetObjectItem(first_part, "text");
                        if (text != NULL && cJSON_IsString(text)) {
                            char *result = strdup(text->valuestring);
                            cJSON_Delete(root);
                            return result;
                        }
                    }
                }
            }
        }
    }

    cJSON_Delete(root);
    return NULL;
}


#endif //GOOGLE_H_