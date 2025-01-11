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
  "system_instruction": {
    "parts": {
      "text": "You are a cat. Your name is Neko."
    }
  },
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
char *parse_google_response(LLMClientConfig *config, const char *response);

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


    //objects that are not bound to a parent immediately after creation 
    //must be tracked in the array to simplify early returns
    cJSON *bin = cJSON_CreateArray();

    // Create root object
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        WRITE_LAST_ERROR("build_google_request: Error creating json root");
        goto cleanup;
    }
    cJSON_AddItemToArray(bin, root);

    //add system prompt if available
    if(config->llmdata.system){
        cJSON *system = cJSON_CreateObject();
        if (system == NULL) {
            WRITE_LAST_ERROR("build_google_request: Error: system == NULL");
            goto cleanup;
        } 
        cJSON_AddItemToObject(root, "system_instruction", system);

        cJSON *system_parts = cJSON_CreateObject();
        if (system_parts==NULL) {
            WRITE_LAST_ERROR("build_google_request: Error: system_parts==NULL");
            goto cleanup;
        }      
        cJSON_AddItemToObject(system, "parts", system_parts);
        cJSON_AddStringToObject(system_parts, "text", config->llmdata.system);

    }
    // Create content object
    cJSON *content = cJSON_CreateObject();
    if (content == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_google_request: Error: content == NULL");
        goto cleanup;
    }
    cJSON_AddItemToArray(bin, content);

    // Create parts array
    cJSON *parts = cJSON_CreateArray();
    if (parts == NULL) {
        WRITE_LAST_ERROR("build_google_request: Error: parts == NULL");
        goto cleanup;
    }
    cJSON_AddItemToObject(content, "parts", parts);

    // Add text part
    cJSON *text_part = cJSON_CreateObject();
    if (text_part == NULL) {
        WRITE_LAST_ERROR("build_google_request: Error: text_part == NULL");
        goto cleanup;
    }
    cJSON_AddItemToArray(parts, text_part);
    cJSON_AddStringToObject(text_part, "text", config->llmdata.prompt);

    // Add generation config
    cJSON *gen_config = cJSON_CreateObject();
    if (gen_config == NULL) {
        WRITE_LAST_ERROR("build_google_request: Error: gen_config == NULL");
        goto cleanup;
    }
    cJSON_AddItemToObject(root, "generationConfig", gen_config);

    // Add fields based on selected feature
    if (config->llmconfig.feature == TEXT_INPUT_WITH_REMOTE_FILE) {
        cJSON *file_part = cJSON_CreateObject();
        if (file_part == NULL) {
            WRITE_LAST_ERROR("build_google_request: Error: file_part == NULL");
            goto cleanup;
        }         
        cJSON_AddItemToArray(parts, file_part);

        cJSON *file_data = cJSON_CreateObject();
        if (file_data == NULL) {
            WRITE_LAST_ERROR("build_google_request: Error: file_data == NULL");
            goto cleanup;
        }         
        cJSON_AddItemToObject(file_part, "file_data", file_data);
        cJSON_AddStringToObject(file_data, "mime_type", config->llmdata.file.mime);
        cJSON_AddStringToObject(file_data, "file_uri", config->llmdata.file.uri);
    }
    else if (config->llmconfig.feature == TEXT_INPUT_WITH_LOCAL_FILE) {
        cJSON *file_part = cJSON_CreateObject();
        if (file_part == NULL) {
            WRITE_LAST_ERROR("build_google_request: Error: file_part == NULL(local)");
            goto cleanup;
        }         
        cJSON_AddItemToArray(parts, file_part);

        cJSON *inline_data = cJSON_CreateObject();
        if (file_data == NULL) {
            WRITE_LAST_ERROR("build_google_request: Error: inline_data == NULL");
            goto cleanup;
        }
        cJSON_AddItemToObject(file_part, "inlineData", inline_data);
        cJSON_AddStringToObject(inline_data, "mimeType", config->llmdata.file.mime);
        const char *base64_file_data = base64_encode(config->llmdata.file.data, 
                                                        config->llmdata.file.nbytes);
        cJSON_AddStringToObject(inline_data, "data", base64_file_data);
        free((void*)base64_file_data);
    }
    else if((config->llmconfig.feature == TEXT_INPUT_WITH_STRUCTURED_OUTPUT) ||
            (config->llmconfig.structured_output > 0) ){
        cJSON *json_response_schema = cJSON_Parse(config->llmconfig.json_response_schema);
        if(json_response_schema !=NULL){
            cJSON_AddStringToObject(gen_config, "response_mime_type", "application/json");
            cJSON_AddItemToObject(gen_config, "response_schema", json_response_schema); 
        }
        else{
            WRITE_LAST_ERROR("build_google_request: Invalid json response_schema");
            goto cleanup;
        }
    }

    // Create messages array
    cJSON *contents = NULL;

    //bundle all previous messages if chatting, else send only message 
    if(config->llmconfig.chat > 0){
      if(config->user_state == NULL){
        config->user_state =  cJSON_CreateArray();
        if(config->user_state ==NULL){
            WRITE_LAST_ERROR("build_google_request: Error creating user_state");
            goto cleanup;
        }
      } 
      contents = (cJSON*)config->user_state; 
      cJSON_AddItemToArray(contents, content);
      cJSON_AddStringToObject(content, "role", "user");

      //trim contents(config->user_state)  to save heap 
      if( cJSON_GetArraySize(contents) > config->llmconfig.chat){
        cJSON_DeleteItemFromArray(contents, 0);
      }      

      //make a deep copy of contents in user_state because contents will 
      //be bound to root which frees all memory when deleted and we need to keep track of previous chat
      config->user_state = cJSON_Duplicate(contents, true);
      if(config->user_state ==NULL){
        //this is a non fatal error, only consequence is that user history will not be stored
        WRITE_LAST_ERROR("build_google_request: Error duplicating user_state");
      }

    } else{
        contents = cJSON_CreateArray();
        if (contents == NULL) {
            WRITE_LAST_ERROR("build_google_request: Error creating user_state");
            goto cleanup;
        }
        cJSON_AddItemToArray(contents, content);
    }

    //bind contents to root
    cJSON_AddItemToObject(root, "contents", contents);

    //set gen configs  
    cJSON_AddNumberToObject(gen_config, "temperature", config->llmconfig.temperature);
    cJSON_AddNumberToObject(gen_config, "topK", config->llmconfig.top_k);
    cJSON_AddNumberToObject(gen_config, "topP", config->llmconfig.top_p);
    cJSON_AddNumberToObject(gen_config, "maxOutputTokens", config->llmconfig.max_tokens);

    // Convert to string
    char *request_str = cJSON_Print(root);
    if(request_str ==NULL){
        WRITE_LAST_ERROR("build_google_request: failed to allocate memory for request_str");
    }
    cJSON_Delete(root);

    //responsibility lies on the caller to free request_str
    return request_str;

cleanup:
    cJSON_Delete(bin);    
}

char *parse_google_response(LLMClientConfig *config, const char *response){
    if (response == NULL) {
        WRITE_LAST_ERROR("parse_google_response: response==NULL");
        return NULL;
    }

    cJSON *root = cJSON_Parse(response);
    if (root == NULL) {
        goto cleanup;
    }

    cJSON *candidates = cJSON_GetObjectItem(root, "candidates");
    if (!candidates || !cJSON_IsArray(candidates)) {
        goto cleanup;
    }

    cJSON *first_candidate = cJSON_GetArrayItem(candidates, 0);
    if (!first_candidate) {
        goto cleanup; 
    }

    cJSON *content = cJSON_GetObjectItem(first_candidate, "content");
    if (!content) {
        goto cleanup;
    }

    cJSON *parts = cJSON_GetObjectItem(content, "parts");
    if (!parts || !cJSON_IsArray(parts)) {
        goto cleanup;
    }

    cJSON *first_part = cJSON_GetArrayItem(parts, 0);
    if (!first_part) {
        goto cleanup;
    }

    cJSON *text = cJSON_GetObjectItem(first_part, "text");
    if (!text || !cJSON_IsString(text)) {
        goto cleanup;
    }

    char *result = strdup(text->valuestring);

    //store model response if chatting
    if(config->llmconfig.chat > 0 && config->user_state){
        cJSON *_content = cJSON_Duplicate(content, true);
        if(_content==NULL){
            //non fatal error
            WRITE_LAST_ERROR("parse_google_response: storing model repohse failed");
        }
        cJSON_AddItemToArray((cJSON*)config->user_state, _content);
    }
    cJSON_Delete(root);
    return result;

cleanup:
    WRITE_LAST_ERROR("parse_google_response: incorrect json format");
    cJSON_Delete(root);
    return NULL;    
}


#endif //GOOGLE_H_