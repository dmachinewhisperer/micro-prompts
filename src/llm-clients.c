#include "google.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "llm-clients.h"
#include "http-rest-api.h"

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


static char *prompt_google_gemini(esp_http_client_handle_t client, LLMClientConfig *llmconfigs){
    char *request = NULL;
    char *response = NULL;
    SemaphoreHandle_t sync_semaphore = NULL;
    TaskHandle_t task_queue_handler_handle = NULL;

    if (client == NULL){
        ESP_LOGE(TAG, "http_client uninit'ed");
        return NULL;
    }    
    header_mod_t hmods[] = {
        {"Content-Type", "application/json", HEADER_ACTION_ADD},
        {NULL, NULL, 0}
    };
    
    //format url
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

    //response memory management is the responsibility of the function caller
    //response = (char*)malloc(sizeof(char) * MAX_HTTP_RESPONSE_LENGTH + 1);
    //if(response==NULL){
    //    ESP_LOGE(TAG, "malloc'ing space for response failed");
    //    return NULL;
    //}

    //the following block configures and starts a task that returns the response body and 
    //any specified headers if needed by the calling application. 
    //this task must be started before making the request as it blocks till completion. 
    //otherwise, there is no queue consumer and the http handler blocks when the queue fills up. 
    //the task releases the sync semaphore and  deletes itself when the body response is complete
    
    sync_semaphore = xSemaphoreCreateBinary();
    if (sync_semaphore == NULL) {
        ESP_LOGE(TAG, "Failed to create semaphore");
        goto cleanup;
    }
    HTTPQueueHandlerTaskParams params = {
        .headers_filter = NULL,
        .num_headers = 0,
        .response = &response,
        //.response_size = MAX_HTTP_RESPONSE_LENGTH,
        .header_values = NULL,
        .sync = sync_semaphore,
    };
    // Launch the task
    if (xTaskCreate(task_http_queue_handler, "HTTPTaskHandler", 
                    1024, &params, tskIDLE_PRIORITY + 1, &task_queue_handler_handle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create queue reading task");
        goto cleanup;
    } 

    request = build_google_request(llmconfigs);
    if(request==NULL){
        goto cleanup;
    }
    ESP_LOGE(TAG, "%s", request);
    esp_err_t err = client_request(client, 
                    HTTP_METHOD_POST, 
                    NULL, 
                    hmods, 
                    request, 
                    strlen(request));
                   
    if(err!=ESP_OK){
        goto cleanup;
    } 

    // Wait for the task to process the headers(if specifed) and return respnse
    if (xSemaphoreTake(sync_semaphore, portMAX_DELAY) == pdTRUE) {
        ESP_LOGD(TAG, "Task completed successfully");

    } 

    if(response==NULL){
        ESP_LOGD(TAG, "Failed to allocate memory for response.");
        goto cleanup;
    }
    char *text = parse_google_response(response);

    //clean up
    free(request); 
    free(response);
    vSemaphoreDelete(sync_semaphore);    
    return text;         

//early return
cleanup:
    if(request) free(request);
    if(response) free(response);
    if(sync_semaphore) vSemaphoreDelete(sync_semaphore); 
    if(task_queue_handler_handle) vTaskDelete(task_queue_handler_handle);
    return NULL;
    
}


char* prompt(esp_http_client_handle_t client, LLMClientConfig *llmconfigs)
{
    if(llmconfigs->llmconfig.provider == GOOGLE_GEMINI){
        return prompt_google_gemini(client, llmconfigs);
    }

    //else if ...
    return NULL;
}
