#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "../cJSON/cJSON.h"
#include "../utils/utils.h"
#include "openai.h"

#ifdef BUILD_FOR_TESTING
#include "../tests/test_memory.h" //header for running memory bug tests
#endif

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


char *build_openai_request(LLMClientConfig *config) {

    if(!_is_feature_supported(config->llmconfig.feature, provider_openai_gpt)){
        WRITE_LAST_ERROR("build_openai_request: Selected provider does not support selected feature");
        return NULL;        
    }

    // Ensure max tokens specified is in line with http max response size
    if(config->llmconfig.max_tokens > MAX_LLM_OUTPUT_TOKENS) {
        config->llmconfig.max_tokens = MAX_LLM_OUTPUT_TOKENS;
    }
    // ensure prompt text is specified
    if(config->llmdata.prompt==NULL){
        WRITE_LAST_ERROR("build_openai_request: Error: llmdata.prompt==NULL");
        return NULL;
    }

    //orphan objects that are not yet bound to a parent must be tracked in bin
    //to simplify cleanup upon early returns. BEFORE adding to a parent node, must be detached
    //immediately to prevent circular dependencies (an an unexpected behaviour of cJSON_Print) from
    //this bug report: https://
    cJSON *bin = cJSON_CreateArray();
    if(bin==NULL){
        WRITE_LAST_ERROR("build_openai_request: Error: bin==NULL");
        return NULL; 
    }
    // Create root object
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        WRITE_LAST_ERROR("build_openai_request: Error:root==NULL");
        goto cleanup;
    }
    cJSON_AddItemToArray(bin, root);

    cJSON_AddStringToObject(root, "model", config->llmconfig.model_name);


    // Create user message object
    cJSON *message = cJSON_CreateObject();
    if (message == NULL) {
        WRITE_LAST_ERROR("build_openai_request: Error creating message object");
        goto cleanup;
    }
    cJSON_AddItemToArray(bin, message);

    //add system prompt if available
    cJSON *system_prompt = NULL; 
    if(config->llmdata.system!=NULL){
        system_prompt = cJSON_CreateObject();
        if (system_prompt == NULL) {
            WRITE_LAST_ERROR("build_openai_request: Error: system_prompt==NULL");
            goto cleanup;
        } 
        cJSON_AddItemToArray(bin, system_prompt);
        cJSON_AddStringToObject(system_prompt, "role", "assistant");
        cJSON_AddStringToObject(system_prompt, "content", config->llmdata.system);

    }    
    cJSON_AddStringToObject(message, "role", "user");

    // Create content array for the message
    cJSON *content = cJSON_CreateArray();
    if (content == NULL) {
        WRITE_LAST_ERROR("build_openai_request: Error creating content array");
        goto cleanup;
    }
    cJSON_AddItemToObject(message, "content", content);

    // Add text content
    cJSON *text_content = cJSON_CreateObject();
    if (text_content == NULL) {
        WRITE_LAST_ERROR("build_openai_request: Error creating text content");
        goto cleanup;
    }
    cJSON_AddItemToArray(content, text_content);
    cJSON_AddStringToObject(text_content, "type", "text");
    cJSON_AddStringToObject(text_content, "text", config->llmdata.prompt);


    //add features based on config
    if (config->llmconfig.feature == TEXT_INPUT_WITH_REMOTE_IMG) {
        if(config->llmdata.file.uri==NULL){
            WRITE_LAST_ERROR("build_openai_request: Error: TEXT_INPUT_WITH_REMOTE_IMG: uri==NULL");
            goto cleanup; 
        }
        cJSON *image_content = cJSON_CreateObject();
        if (image_content == NULL) {
            WRITE_LAST_ERROR("build_openai_request: Error: TEXT_INPUT_WITH_REMOTE_IMG: image_content==NULL");
            goto cleanup;
        }        
        cJSON_AddItemToArray(content, image_content);
        cJSON *image_url = cJSON_CreateObject();
        if (image_url == NULL) {
            WRITE_LAST_ERROR("build_openai_request: Error: TEXT_INPUT_WITH_REMOTE_IMG: image_url==NULL");
            goto cleanup;
        }
        cJSON_AddItemToObject(image_content, "image_url", image_url);
        cJSON_AddStringToObject(image_content, "type", "image_url");
        cJSON_AddStringToObject(image_url, "url", config->llmdata.file.uri);

        //controls number of tokens used: see: https://platform.openai.com/docs/guides/vision
        cJSON_AddStringToObject(image_url, "detail", "low");
    }
    else if (config->llmconfig.feature == TEXT_INPUT_WITH_LOCAL_IMG) {
        if((config->llmdata.file.mime == NULL) || (config->llmdata.file.data == NULL) || (config->llmdata.file.nbytes==0)){
            WRITE_LAST_ERROR("build_openai_request: Error: TEXT_INPUT_WITH_LOCAL_IMG: all required file properties(mime,data,nbytes) not set");
            goto cleanup;
        }
        cJSON *image_content = cJSON_CreateObject();
        if (image_content == NULL) {
            WRITE_LAST_ERROR("build_openai_request: Error: TEXT_INPUT_WITH_LOCAL_IMG: image_content == NULL");
            goto cleanup;
        }        
        cJSON_AddItemToArray(content, image_content);
        cJSON *image_url = cJSON_CreateObject();
        if (image_url == NULL) {
            WRITE_LAST_ERROR("build_openai_request: Error: TEXT_INPUT_WITH_LOCAL_IMG: image_url == NULL");
            goto cleanup;
        }
        cJSON_AddItemToObject(image_content, "image_url", image_url);        
        cJSON_AddStringToObject(image_content, "type", "image_url");
 
        
        //attach base64 data
        const char *base64_data = base64_encode(config->llmdata.file.data, 
                                              config->llmdata.file.nbytes);
        char *url = (char*)malloc(strlen(base64_data) + 30); //extra space for prefix
        if((base64_data==NULL) || (url==NULL)){
            WRITE_LAST_ERROR("build_openai_request: Error: TEXT_INPUT_WITH_LOCAL_IMG: base64_data encoding failed");
            if(base64_data) free((void*)base64_data);
            if(url) free((void*)url);
            goto cleanup;
        }
        
        sprintf(url, "data:%s;base64,%s",config->llmdata.file.mime, base64_data);
        cJSON_AddStringToObject(image_url, "url", url);

        //controls number of tokens used: see: https://platform.openai.com/docs/guides/vision
        cJSON_AddStringToObject(image_url, "detail", "low");
        free((void*)base64_data);
        free((void*)url);
    }
    else if (config->llmconfig.feature == TEXT_INPUT_WITH_LOCAL_AUDIO) {
        if((config->llmdata.file.mime == NULL) || (config->llmdata.file.data == NULL) || (config->llmdata.file.nbytes==0)){
            WRITE_LAST_ERROR("build_openai_request: Error: TEXT_INPUT_WITH_LOCAL_AUDIO: required file properties(mime,data,nbytes) not set");
            goto cleanup;
        }        
        cJSON *audio_content = cJSON_CreateObject();
        if (audio_content == NULL) {
            WRITE_LAST_ERROR("build_openai_request: Error; TEXT_INPUT_WITH_LOCAL_AUDIO: audio_content==NULL");
            goto cleanup;
        }      
        cJSON_AddItemToArray(content, audio_content);  
        cJSON *input_audio = cJSON_CreateObject();
        if (input_audio == NULL) {
            WRITE_LAST_ERROR("build_openai_request: Error; TEXT_INPUT_WITH_LOCAL_AUDIO: input_audio==NULL");
            goto cleanup;
        }
        cJSON_AddItemToObject(audio_content, "input_audio", input_audio);
        cJSON_AddStringToObject(audio_content, "type", "input_audio");
        
        //attach base64 data
        const char *base64_data = base64_encode(config->llmdata.file.data, 
                                              config->llmdata.file.nbytes);
        if(base64_data==NULL){
            WRITE_LAST_ERROR("build_openai_request: Error: TEXT_INPUT_WITH_LOCAL_AUDIO: base64_data encoding failed");
            goto cleanup;
        }                                              
        cJSON_AddStringToObject(input_audio, "data", base64_data);

        //openai supports only wav/mp3 as of 01-01-2025
        const char* slash_pos = strchr(config->llmdata.file.mime, '/');
        if(slash_pos==NULL){
          WRITE_LAST_ERROR("build_openai_request: Error: invalid mime type");
          goto cleanup; 
        }
        cJSON_AddStringToObject(input_audio, "format", slash_pos + 1);
        free((void*)base64_data);
    }    

    // Add structured output configuration if needed
    if ((config->llmconfig.feature == TEXT_INPUT_WITH_STRUCTURED_OUTPUT) ||
        (config->llmconfig.structured_output > 0)) {
        cJSON *response_format = cJSON_CreateObject();
        if (response_format == NULL) {
            WRITE_LAST_ERROR("build_openai_request: Error: response_format==NULL");
            goto cleanup;
        }
        cJSON_AddItemToObject(root, "response_format", response_format);
        cJSON_AddStringToObject(response_format, "type", "json_schema");
        
        cJSON *schema = cJSON_Parse(config->llmconfig.json_response_schema);
        if (schema == NULL) {
            WRITE_LAST_ERROR("build_openai_request: Error: Invalid json response schema");
            goto cleanup;
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

        //add system prompt if available
        if(config->user_state ==NULL){
            WRITE_LAST_ERROR("build_openai_request: Error creating user_state");
            goto cleanup;
        }       
        if(system_prompt!=NULL){
            cJSON_DetachItemFromArray(bin, 2); //detach system_prompt
            cJSON_AddItemToArray(config->user_state, system_prompt);
        } 
      }
      cJSON_DetachItemFromArray(bin, 1); //detach message
      messages = (cJSON*)config->user_state; 
      cJSON_AddItemToArray(messages, message);

      //trim messages(config->user_state)  to save heap 
      if( cJSON_GetArraySize(messages) > config->llmconfig.chat){
        cJSON_DeleteItemFromArray(messages, 0);
      }      

      //make a deep copy of messages in user_state because messages will 
      //be bound to root which frees all memory when deleted and we need to keep track of previous chat
      config->user_state = cJSON_Duplicate(messages, true);

      if(config->user_state ==NULL){
        //this is a non fatal error, only consequence is that user history will not be stored
        WRITE_LAST_ERROR("build_openai_request: Error duplicating user_state");
      }
    } 
    else{
        messages = cJSON_CreateArray();
        if (messages == NULL) {
        WRITE_LAST_ERROR("build_openai_request: Error: messages == NULL");
        goto cleanup;
        }
        if(system_prompt!=NULL){
            cJSON_DetachItemFromArray(bin, 2); //detach system_prompt
            cJSON_AddItemToArray(messages, system_prompt);
            
        }
        cJSON_DetachItemFromArray(bin, 1); //detach message
        cJSON_AddItemToArray(messages, message);
        

    }

    cJSON_AddItemToObject(root, "messages", messages);

    // Add generation parameters
    cJSON_AddNumberToObject(root, "max_completion_tokens", config->llmconfig.max_tokens);
    cJSON_AddNumberToObject(root, "temperature", config->llmconfig.temperature);
    cJSON_AddNumberToObject(root, "top_p", config->llmconfig.top_p);
    cJSON_AddNumberToObject(root, "max_tokens", config->llmconfig.max_tokens);

    
    //cJSON_PrintUnformatted
    char *request_str = cJSON_Print(root);
    if(request_str ==NULL){
        WRITE_LAST_ERROR("build_openai_request: Error: failed to allocate memory for request_str");
    }  

    //cJSON_DetachItemFromArray(bin, 0); //detach root
    //cJSON_Delete(root);
    cJSON_Delete(bin);

    //responsibility lies on the caller to free request_str
    return request_str;

cleanup:
    cJSON_Delete(bin);    
    return NULL;    
}


/** TODO: handle refusal, contained in the response message.refusal. (done)
 * **/
char *parse_openai_response(LLMClientConfig *config, const char *response){
    if (response == NULL) {
        WRITE_LAST_ERROR("parse_openai_response: Error: reponse is NULL");
        return NULL;
    }

    cJSON *root = cJSON_Parse(response);
    if (root == NULL) {
        WRITE_LAST_ERROR("parse_openai_response: Error: cJSON_Parse(response) failed");
        return NULL;
    }

    // Get choices array
    cJSON *choices = cJSON_GetObjectItem(root, "choices");
    if (choices == NULL || !cJSON_IsArray(choices)) {
        goto cleanup;
    }

    // Get first choice
    cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
    if (first_choice == NULL) {
        goto cleanup;
    }

    // Get message
    cJSON *message = cJSON_GetObjectItem(first_choice, "message");
    if (message == NULL) {
        goto cleanup;
    }

/*    // Check for refusal
    cJSON *refusal = cJSON_GetObjectItem(message, "refusal");
    if (refusal != NULL && !cJSON_IsNull(refusal)) {
        char *refusal_str = cJSON_Print(refusal);
        WRITE_LAST_ERROR(refusal_str);
        free(refusal_str);
        cJSON_Delete(root);
        return NULL;
    }
*/
    // Get content
    cJSON *content = cJSON_GetObjectItem(message, "content");
    if (content == NULL || !cJSON_IsString(content)) {
        goto cleanup;
    }

    //store model response if chatting
    if(config->llmconfig.chat > 0 && config->user_state){
        cJSON_AddItemToArray((cJSON*)config->user_state, cJSON_Duplicate(message, true));
    }

    // Copy content
    char *result = strdup(content->valuestring);
    cJSON_Delete(root);
    return result;
cleanup:
    WRITE_LAST_ERROR("parse_openai_response: Error: incorrect json format");
    cJSON_Delete(root);
    return NULL;     
}
