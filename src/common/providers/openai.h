/**
 * // API endpoint 'https://api.openai.com/v1/chat/completions';
 * API Docs: https://platform.openai.com/docs/api-reference/chat/create

// Request body structure chat completion

const request = {
  model: "gpt-4-vision-preview", // or "gpt-4", "gpt-3.5-turbo" etc.
  messages: [
    {
      role: "user",
      content: [
        {
          type: "text",
          text: "Your prompt here"
        },

        //include when using base64 images
        {
          type: "image_url",
          image_url: {
            url: "data:image/jpeg;base64,YOUR_BASE64_IMAGE", // or direct image URL
            detail: "low" //always set to low
          }
        },
        //include when using file_uri i.e config->llmconfig.feature == TEXT_INPUT_WITH_REMOTE_FILE_URL
        {
           type: "image_url",
           image_url: {
              url: "https://upload.wikimedia.org/wikipedia/commons/thumb/d/,
              detail: "low" //always set to low
          }
        },
         //include if prompting with audio
        { 
          type: "input_audio", 
          input_audio: { 
              data: base64str, 
              format: "wav", 
          }
        }
      ]
    }
  ],
  max_completion_tokens: 2048,
  temperature: 0.7,
  top_p: 0.95,
  response_format: {
      "type": "json_schema",
    "json_schema": {some user supplied schema},
    strict: "true" //always true
  },
};

//response for chat completion
{
  "id": "chatcmpl-123456",
  "object": "chat.completion",
  "created": 1728933352,
  "model": "gpt-4o-2024-08-06",
  "choices": [
    {
      "index": 0,
      "message": {
        "role": "assistant",
        "content": "Hi there! How can I assist you today?",
        "refusal": null
      },
      "logprobs": null,
      "finish_reason": "stop"
    }
  ],
  "usage": {
    "prompt_tokens": 19,
    "completion_tokens": 10,
    "total_tokens": 29,
    "prompt_tokens_details": {
      "cached_tokens": 0
    },
    "completion_tokens_details": {
      "reasoning_tokens": 0,
      "accepted_prediction_tokens": 0,
      "rejected_prediction_tokens": 0
    }
  },
  "system_fingerprint": "fp_6b68a8204b"
}

 */

#ifndef OPENAI_H_
#define OPENAI_H_

#include <stddef.h>
#include <string.h>

#include "../cJSON/cJSON.h"
#include "../llm-types.h"
#include "../utils/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

char *build_openai_request(LLMClientConfig *config);
char *parse_openai_response(const char *response);

#ifdef __cplusplus
}
#endif



char *build_openai_request(LLMClientConfig *config) {

    if(!_is_feature_supported(config->llmconfig.feature, provider_openai_gpt)){
        WRITE_LAST_ERROR("build_openai_request: Selected provider does not support selected feature");
        return NULL;        
    }

    // Ensure max tokens specified is in line with http max response size
    if(config->llmconfig.max_tokens > MAX_LLM_OUTPUT_TOKENS) {
        config->llmconfig.max_tokens = MAX_LLM_OUTPUT_TOKENS;
    }

    // Create root object
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        WRITE_LAST_ERROR("build_openai_request: Error creating json root");
        return NULL;
    }

    cJSON_AddStringToObject(root, "model", config->llmconfig.model_name);
  

    // Create user message object
    cJSON *message = cJSON_CreateObject();
    if (message == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_openai_request: Error creating message object");
        return NULL;
    }
    //cJSON_AddItemToArray(messages, message);
    cJSON_AddStringToObject(message, "role", "user");

    // Create content array for the message
    cJSON *content = cJSON_CreateArray();
    if (content == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_openai_request: Error creating content array");
        return NULL;
    }
    cJSON_AddItemToObject(message, "content", content);

    // Add text content
    cJSON *text_content = cJSON_CreateObject();
    if (text_content == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_openai_request: Error creating text content");
        return NULL;
    }
    cJSON_AddItemToArray(content, text_content);
    cJSON_AddStringToObject(text_content, "type", "text");
    cJSON_AddStringToObject(text_content, "text", config->llmdata.prompt);


    //add features based on config
    if (config->llmconfig.feature == TEXT_INPUT_WITH_REMOTE_IMG) {
        cJSON *image_content = cJSON_CreateObject();
        cJSON *image_url = cJSON_CreateObject();
        if (image_content == NULL || image_url == NULL) {
            cJSON_Delete(root);
            WRITE_LAST_ERROR("build_openai_request: Error creating image content");
            return NULL;
        }
        cJSON_AddItemToArray(content, image_content);
        cJSON_AddStringToObject(image_content, "type", "image_url");
        cJSON_AddItemToObject(image_content, "image_url", image_url);
        cJSON_AddStringToObject(image_url, "url", config->llmdata.file.uri);

        //controls number of tokens used: see: https://platform.openai.com/docs/guides/vision
        cJSON_AddStringToObject(image_url, "detail", "low");
    }
    else if (config->llmconfig.feature == TEXT_INPUT_WITH_LOCAL_IMG) {
        cJSON *image_content = cJSON_CreateObject();
        cJSON *image_url = cJSON_CreateObject();
        if (image_content == NULL || image_url == NULL) {
            cJSON_Delete(root);
            WRITE_LAST_ERROR("build_openai_request: Error creating image content");
            return NULL;
        }
        cJSON_AddItemToArray(content, image_content);
        cJSON_AddStringToObject(image_content, "type", "image_url");
        cJSON_AddItemToObject(image_content, "image_url", image_url);
        
        //attach base64 data
        const char *base64_data = base64_encode(config->llmdata.file.data, 
                                              config->llmdata.file.nbytes);
        char *url = malloc(strlen(base64_data) + 30); // Extra space for prefix
        
        sprintf(url, "data:%s;base64,%s",config->llmdata.file.mime, base64_data);
        cJSON_AddStringToObject(image_url, "url", url);

        //controls number of tokens used: see: https://platform.openai.com/docs/guides/vision
        cJSON_AddStringToObject(image_url, "detail", "low");
        free((void*)base64_data);
        free(url);
    }
    else if (config->llmconfig.feature == TEXT_INPUT_WITH_LOCAL_AUDIO) {
        cJSON *audio_content = cJSON_CreateObject();
        cJSON *input_audio = cJSON_CreateObject();
        if (audio_content == NULL || input_audio == NULL) {
            cJSON_Delete(root);
            WRITE_LAST_ERROR("build_openai_request: Error creating audio content");
            return NULL;
        }
        cJSON_AddItemToArray(content, audio_content);
        cJSON_AddStringToObject(audio_content, "type", "input_audio");
        cJSON_AddItemToObject(audio_content, "input_audio", input_audio);
        
        //attach base64 data
        const char *base64_data = base64_encode(config->llmdata.file.data, 
                                              config->llmdata.file.nbytes);
        cJSON_AddStringToObject(input_audio, "data", base64_data);

        //openai supports only wav/mp3 as of 01-01-2025
        const char* slash_pos = strchr(config->llmdata.file.mime, '/');
        if(!slash_pos){
          WRITE_LAST_ERROR("invalid mime type");
          return NULL; 
        }
        cJSON_AddStringToObject(input_audio, "format", slash_pos + 1);
        free((void*)base64_data);
    }    

    // Add structured output configuration if needed
    if ((config->llmconfig.feature == TEXT_INPUT_WITH_STRUCTURED_OUTPUT) ||
        (config->llmconfig.structured_output > 0)) {
        cJSON *response_format = cJSON_CreateObject();
        if (response_format == NULL) {
            cJSON_Delete(root);
            WRITE_LAST_ERROR("build_openai_request: Error creating response format");
            return NULL;
        }
        cJSON_AddItemToObject(root, "response_format", response_format);
        cJSON_AddStringToObject(response_format, "type", "json_schema");
        
        cJSON *schema = cJSON_Parse(config->llmconfig.json_response_schema);
        if (schema == NULL) {
            cJSON_Delete(root);
            WRITE_LAST_ERROR("build_openai_request: Invalid json response schema");
            return NULL;
        }
        cJSON_AddItemToObject(response_format, "json_schema", schema);
        cJSON_AddBoolToObject(response_format, "strict", 1);
    }

    // Create messages array
    cJSON *messages = NULL;

    //bundle all previous messages if chatting, else send only message 
    if(config->llmconfig.chat > 0){
      if(config->user_state == NULL){
        config->user_state =  cJSON_CreateArray();
      } 
      messages = config->user_state; 
      cJSON_AddItemToArray(messages, message);

      //trim messages(config->user_state)  to save heap 
      if( cJSON_GetArraySize(messages) > config.llmconfig.chat){
        cJSON_DeleteItemFromArray(messages, 0);
      }      

      //make a deep copy of messages in user_state because messages will 
      //be bound to root which frees all memory when deleted and we need to keep track of previous chat

      config->user_state = cJSON_Duplicate(messages, true);

    } else{
      messages = cJSON_CreateArray();
      cJSON_AddItemToArray(messages, message);
    }
    if (messages == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("build_openai_request: Error creating user_state");
        return NULL;
    }
    cJSON_AddItemToObject(root, "messages", messages);

    // Add generation parameters
    cJSON_AddNumberToObject(root, "max_completion_tokens", config->llmconfig.max_tokens);
    cJSON_AddNumberToObject(root, "temperature", config->llmconfig.temperature);
    cJSON_AddNumberToObject(root, "top_p", config->llmconfig.top_p);

    //added 
    cJSON_AddNumberToObject(root, "max_tokens", config->llmconfig.max_tokens);

    char *request_str = cJSON_Print(root);
    cJSON_Delete(root);

    return request_str;
}


/** TODO: handle refusal, contained in the response message.refusal. (done)
 * **/
char *parse_openai_response(LLMClientConfig *config, const char *response){
//char *parse_openai_response(const char *response) {
    if (response == NULL) {
        return NULL;
    }

    cJSON *root = cJSON_Parse(response);
    if (root == NULL) {
        WRITE_LAST_ERROR("parse_openai_response: Invalid JSON response");
        return NULL;
    }

    // Get choices array
    cJSON *choices = cJSON_GetObjectItem(root, "choices");
    if (choices == NULL || !cJSON_IsArray(choices)) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("parse_openai_response: No choices in response");
        return NULL;
    }

    // Get first choice
    cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
    if (first_choice == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("parse_openai_response: Empty choices array");
        return NULL;
    }

    // Get message
    cJSON *message = cJSON_GetObjectItem(first_choice, "message");
    if (message == NULL) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("parse_openai_response: No message in choice");
        return NULL;
    }

    // Check for refusal
    cJSON *refusal = cJSON_GetObjectItem(message, "refusal");
    if (refusal != NULL && !cJSON_IsNull(refusal)) {
        char *refusal_str = cJSON_Print(refusal);
        WRITE_LAST_ERROR(refusal_str);
        free(refusal_str);
        cJSON_Delete(root);
        return NULL;
    }

    // Get content
    cJSON *content = cJSON_GetObjectItem(message, "content");
    if (content == NULL || !cJSON_IsString(content)) {
        cJSON_Delete(root);
        WRITE_LAST_ERROR("parse_openai_response: No content in message");
        return NULL;
    }

    //store model response if chatting
    if(config->llmconfig.chat > 0 && config->user_state){
        cJSON_AddItemToArray(config->user_state, cJSON_Duplicate(message, true));
    }

    // Copy content
    char *result = strdup(content->valuestring);
    cJSON_Delete(root);
    return result;
}

#endif //OPENAI_H_