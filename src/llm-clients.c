#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "llm-clients.h"
#include "http-rest-api.h"

//#define USE_GOOGLE_CLIENT
#define USE_OPENAI_CLIENT
//#define USE_GROQ_CLIENT

#ifdef USE_GOOGLE_CLIENT
#include "google.h"
#endif 

#ifdef USE_OPENAI_CLIENT
#include "openai.h"
#endif 

#ifdef USE_GROQ_CLIENT
#include "groq.h"
#endif

static const char *TAG = "LLMClient";

extern QueueHandle_t http_event_queue;


typedef struct {
    const char **headers_filter;    // Array of header keys to watch for
    size_t num_headers;             // Number of headers to watch for
    char **response;                 // pointer to buffer where the response body is stored
    //size_t response_size;           // Size of the response body buffer
    char **header_values;           // Array of pointers to store header values
    SemaphoreHandle_t sync;         // Semaphore for signaling completion/syncing with calling funcion
} HTTPQueueHandlerTaskParams;


static void task_http_queue_handler(void *param) {
    HTTPQueueHandlerTaskParams *params = (HTTPQueueHandlerTaskParams *)param;
    http_event_message_t msg;

    while (xQueueReceive(http_event_queue, &msg, portMAX_DELAY)) {
        switch (msg.type) {
            case EVENT_TYPE_HEADER: {
                if ((params->headers_filter != NULL) && (params->header_values !=NULL)) {
                    for (size_t i = 0; i < params->num_headers; i++) {
                        if (strcmp(msg.data.header.key, params->headers_filter[i]) == 0) {
                            // Copy the header value to the respective position
                            params->header_values[i] = strdup(msg.data.header.value);
                            ESP_LOGD(TAG, "Captured Header: %s = %s", msg.data.header.key, msg.data.header.value);
                        }
                    }
                }
                break;
            }

            case EVENT_TYPE_FINISH:
                // Copy the body to the response buffer
                //strncpy(params->response, msg.data.finish.body, params->response_size - 1);
                //params->response[params->response_size - 1] = '\0';
                *(params->response) = msg.data.finish.body;
                ESP_LOGD(TAG, "Response Body Captured");
                xSemaphoreGive(params->sync); // Signal completion
                vTaskDelete(NULL);
                return;

            default:
                ESP_LOGE(TAG, "Unknown event type");
                break;
        }
    }
    xSemaphoreGive(params->sync); // Signal completion even if the loop exits
    vTaskDelete(NULL);
}

static char *___request(esp_http_client_handle_t client, 
                      LLMClientConfig *llmconfigs, 
                      header_mod_t *headers,
                       char* (*build_request)(LLMClientConfig*), 
                       char* (*parse_response)(LLMClientConfig*, const char*)){
    char *request = NULL;
    char *response = NULL;
    SemaphoreHandle_t sync_semaphore = NULL;
    TaskHandle_t task_queue_handler_handle = NULL;

    //the following block configures and starts a task that returns the response body and 
    //any specified headers if needed by the calling application. 
    //this task must be started before making the request as it blocks till completion. 
    //otherwise, there is no queue consumer and the http handler blocks when the queue fills up. 
    //the task releases the sync semaphore and  deletes itself when the body response is complete
    sync_semaphore = xSemaphoreCreateBinary();
    if (sync_semaphore == NULL) {
        ESP_LOGE(TAG, "failed to create sync_semaphore");
        goto cleanup;
    }
    HTTPQueueHandlerTaskParams params = {
        .headers_filter = NULL,
        .num_headers = 0,
        .response = &response,
        .header_values = NULL,
        .sync = sync_semaphore,
    };
    // Launch the task
    if (xTaskCreate(task_http_queue_handler, "HTTPTaskHandler", 
                    1024, &params, tskIDLE_PRIORITY + 1, &task_queue_handler_handle) != pdPASS) {
        ESP_LOGE(TAG, "failed to create queue reading task");
        goto cleanup;
    } 

    request = build_request(llmconfigs);
    if(request==NULL){
        ESP_LOGE(TAG, "build_request failed");
        goto cleanup;
    }
    ESP_LOGE(TAG, "%s", request);
    esp_err_t err = client_request(client, 
                    HTTP_METHOD_POST, 
                    NULL, 
                    headers, 
                    request, 
                    strlen(request));
                   
    if(err!=ESP_OK){
        ESP_LOGE(TAG, "client_request failed");
        goto cleanup;
    } 

    // Wait for the task to process the headers(if specifed) and return respnse
    if (xSemaphoreTake(sync_semaphore, portMAX_DELAY) == pdTRUE) {
        ESP_LOGD(TAG, "Task completed successfully");

    } 

    if(response==NULL){
        ESP_LOGE(TAG, "failed to allocate memory for response");
        goto cleanup;
    }

    free(request); 
    vSemaphoreDelete(sync_semaphore); 

    if(llmconfigs->llmdata.response.return_raw > 0 || parse_response==NULL){
        return response; 
    }
    char *text = parse_openai_response(llmconfigs,response);
    free(response);
    return text;         

//early return
cleanup:
    if(request) free(request);
    if(response) free(response);
    if(sync_semaphore) vSemaphoreDelete(sync_semaphore); 
    if(task_queue_handler_handle) vTaskDelete(task_queue_handler_handle);
    return NULL;    
}

#ifdef USE_GOOGLE_CLIENT
static char *prompt_google_gemini(esp_http_client_handle_t client, LLMClientConfig *llmconfigs){

    if (client == NULL){
        ESP_LOGE(TAG, "http_client uninit'ed");
        return NULL;
    }    
    header_mod_t headers[] = {
        {"Content-Type", "application/json", HEADER_ACTION_ADD},
        {NULL, NULL, 0}
    };
    
    //format URL
    const char *base_url = "https://generativelanguage.googleapis.com";

    char google_generate_url[256];
    int len = snprintf(google_generate_url, sizeof(google_generate_url), 
        "%s/v1beta/models/%s:generateContent?key=%s", base_url, 
            llmconfigs->llmconfig.model_name, llmconfigs->llmconfig.api_key);    
    if (len < 0 || len >= sizeof(google_generate_url)) {
        ESP_LOGE(TAG, "len = %d: Failed to create URL, buffer overflow or error", len);
        return NULL;
    }
    else{
        ESP_LOGD(TAG, "FORMATTED_URL: %s", google_generate_url);
    }
    
    esp_http_client_set_url(client, google_generate_url);
    return ___request(client, llmconfigs, headers, build_google_request, parse_google_response);

}

#endif

#ifdef USE_OPENAI_CLIENT
static char *prompt_openai_gpt(esp_http_client_handle_t client, LLMClientConfig *llmconfigs){

    if (client == NULL){
        ESP_LOGE(TAG, "http_client uninit'ed");
        return NULL;
    }    

    char auth_header_value[64];
    snprintf(auth_header_value, sizeof(auth_header_value), "Bearer %s", llmconfigs->llmconfig.api_key);

    header_mod_t headers[] = {
        {"Content-Type", "application/json", HEADER_ACTION_ADD},
        {"Authorization", auth_header_value, HEADER_ACTION_ADD},
        {NULL, NULL, 0}
    };

    // Default values
    const char *default_base_url = "https://api.openai.com";
    const char *default_version = "v1";
    const char *default_endpoint = "chat/completions";
    
    // Prepare the URL string
    char openai_generate_url[128];
    int len; 

    if (llmconfigs->llmconfig.base_url && 
        llmconfigs->llmconfig.api_endpoint && 
        llmconfigs->llmconfig.api_endpoint) {

        len = snprintf(openai_generate_url, sizeof openai_generate_url, "%s/%s/%s", 
                    llmconfigs->llmconfig.base_url, 
                    llmconfigs->llmconfig.version, 
                    llmconfigs->llmconfig.api_endpoint);
    } else {
        // If base_url or api_endpoint is not specified, use defaults
        if (llmconfigs->llmconfig.version) {
            // If version is supplied
            len = snprintf(openai_generate_url, sizeof openai_generate_url, "%s/%s/%s", 
                     default_base_url, 
                     llmconfigs->llmconfig.version, 
                     default_endpoint);
        } else {
            // Default to v1 and chat/completions
            len = snprintf(openai_generate_url, sizeof openai_generate_url, "%s/%s/%s", 
                     default_base_url, 
                     default_version, 
                     default_endpoint);
        }
    }

    if (len < 0 || len >= sizeof(openai_generate_url)) {
        ESP_LOGE(TAG, "len = %d: Failed to create URL, buffer overflow or error", len);
        return NULL;
    }
    else{
        ESP_LOGD(TAG, "FORMATTED_URL: %s", openai_generate_url);
    }
    
    esp_http_client_set_url(client, openai_generate_url);
    return ___request(client, llmconfigs, headers, build_openai_request, parse_openai_response);

}

#endif



#ifdef USE_GROQ_CLIENT
static char *prompt_qroq_(esp_http_client_handle_t client, LLMClientConfig *llmconfigs){
    llmconfigs->llmconfig.base_url = "https://api.groq.com";
    llmconfigs->llmconfig.api_endpoint = "openai/chat/completions";
    llmconfigs->llmconfig.version = "v1";

    //llmconfigs->llmdata.response.return_raw = 1; 

    return prompt_openai_gpt(client, llmconfigs); 
}
#endif

char* prompt(esp_http_client_handle_t client, LLMClientConfig *llmconfigs)
{
#ifdef USE_GOOGLE_CLIENT    
    if(llmconfigs->llmconfig.provider == GOOGLE_GEMINI){
        return prompt_google_gemini(client, llmconfigs);
    }
#endif

#ifdef USE_OPENAI_CLIENT
    if(llmconfigs->llmconfig.provider == OPENAI_GPT){
        return prompt_openai_gpt(client, llmconfigs);
    }
#endif    
    WRITE_LAST_ERROR("Selected model is not supported");
    return NULL;
}
