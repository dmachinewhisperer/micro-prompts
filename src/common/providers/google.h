#ifndef GOOGLE_H_
#define GOOGLE_H_

#include <stddef.h>
#include <string.h>

#include "../cJSON/cJSON.h"
#include "../llm-types.h"
#include "../utils/utils.h"

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


char *build_google_request(LLMClientConfig *config) {
   
    // Validate feature support
    if(_is_feature_supported(config->llmconfig.feature, provider_google_gemini) ==0){
        WRITE_LAST_ERROR("build_google_request: Selected provider does not support selected feature");
        return NULL;        
    }

   // ensure max tokens specified is in line with http max response size
    if(config->llmconfig.max_tokens > MAX_LLM_OUTPUT_TOKENS){
        config->llmconfig.max_tokens = MAX_LLM_OUTPUT_TOKENS;
    }
    // Create root object
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        WRITE_LAST_ERROR("build_google_request: Error creating json root");
        return NULL;
    }

    // Create contents array
    cJSON *contents = cJSON_CreateArray();
    if (contents == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_google_request: Error creating json contents");
        return NULL;
    }
    cJSON_AddItemToObject(root, "contents", contents);

    // Create first content object
    cJSON *content = cJSON_CreateObject();
    if (content == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_google_request: Error creating json content");
        return NULL;
    }
    cJSON_AddItemToArray(contents, content);

    // Create parts array
    cJSON *parts = cJSON_CreateArray();
    if (parts == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_google_request: Error creating json parts");
        return NULL;
    }
    cJSON_AddItemToObject(content, "parts", parts);

    // Add text part
    cJSON *text_part = cJSON_CreateObject();
    if (text_part == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_google_request: Error creating json text_part");
        return NULL;
    }
    cJSON_AddItemToArray(parts, text_part);
    cJSON_AddStringToObject(text_part, "text", config->llmdata.prompt);

    // Add generation config
    cJSON *gen_config = cJSON_CreateObject();
    if (gen_config == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_google_request: Error creating json gen_config");
        return NULL;
    }

    // Add fields based on selected feature
    if (config->llmconfig.feature == TEXT_INPUT_WITH_REMOTE_FILE_URL) {
        cJSON *file_part = cJSON_CreateObject();
        cJSON *file_data = cJSON_CreateObject();
        if (file_part == NULL || file_data == NULL) {
            cJSON_Delete(root);
            WRITE_LAST_ERROR("build_google_request: Error creating json file_part || file_data");
            return NULL;
        }
        cJSON_AddItemToArray(parts, file_part);
        cJSON_AddItemToObject(file_part, "file_data", file_data);
        cJSON_AddStringToObject(file_data, "mime_type", config->llmdata.file.mime);
        cJSON_AddStringToObject(file_data, "file_uri", config->llmdata.file.uri);
    }
    else if (config->llmconfig.feature == TEXT_INPUT_WITH_LOCAL_BASE64_FILE) {
        cJSON *file_part = cJSON_CreateObject();
        cJSON *inline_data = cJSON_CreateObject();
        if (file_part == NULL || inline_data == NULL) {
            cJSON_Delete(root);
            WRITE_LAST_ERROR("build_google_request: Error creating json content");
            return NULL;
        }
        cJSON_AddItemToArray(parts, file_part);
        cJSON_AddItemToObject(file_part, "inlineData", inline_data);
        cJSON_AddStringToObject(inline_data, "mimeType", config->llmdata.file.mime);
        const char *base64_file_data = base64_encode(config->llmdata.file.data, 
                                                        config->llmdata.file.nbytes);
        cJSON_AddStringToObject(inline_data, "data", base64_file_data);
        free(base64_file_data);
    }
    else if((config->llmconfig.feature == TEXT_INPUT_WITH_STRUCTURED_OUTPUT) ||
            (config->llmconfig.structured_output > 0) ){
        cJSON *json_response_schema = cJSON_Parse(config->llmconfig.json_response_schema);
        if(json_response_schema !=NULL){
            cJSON_AddStringToObject(gen_config, "response_mime_type", "application/json");
            cJSON_AddItemToObject(gen_config, "response_schema", json_response_schema); 
        }
        else{
            cJSON_Delete(root);
            WRITE_LAST_ERROR("build_google_request: Invalid json response_schema");
            return NULL;
        }
    }


    //set gen configs  
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
    WRITE_LAST_ERROR(response);
    return NULL;
}


#endif //GOOGLE_H_