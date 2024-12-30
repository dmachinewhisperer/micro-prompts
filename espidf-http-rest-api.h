#ifndef ESPIDF_REST_API_CALL
#define ESPIDF_REST_API_CALL


#include "esp_http_client.h"

#define MAX_HEADER_KEY_LENGTH 128
#define MAX_HEADER_VALUE_LENGTH 256
#define MAX_HTTP_RESPONSE_LENGTH 1024


// Global queue handle
//QueueHandle_t http_event_queue;

typedef esp_http_client_method_t http_method_t;

//event types enqueued
typedef enum {
    EVENT_TYPE_HEADER,  // Header key-value pairs
    EVENT_TYPE_FINISH   // Data transfer completion
} event_type_t;

//HTTP event messages
typedef struct {
    event_type_t type; 
    union {
        struct { 
            char key[MAX_HEADER_KEY_LENGTH + 1];
            char value[MAX_HEADER_VALUE_LENGTH + 1];
        } header;
        struct { 
            char body[MAX_HTTP_RESPONSE_LENGTH + 1];
        } finish;
    } data;
} http_event_message_t;

#define QUEUE_LENGTH 10
#define QUEUE_ITEM_SIZE sizeof(http_event_message_t)

//header actions
typedef enum {
    HEADER_ACTION_ADD,
    HEADER_ACTION_REMOVE
} header_action_t;

//header config/modifications
typedef struct {
    const char *key;        // Header key
    const char *value;      // Header value (NULL for removal)
    header_action_t action; // Action: Add or Remove
} header_mod_t;


typedef struct {
    char *url;  
    char *cert;
} client_config_t;

esp_err_t client_request(esp_http_client_handle_t client,
                  http_method_t method,
                  const char *url,
                  header_mod_t *header_mods,
                  const char *data,
                  size_t data_len);

esp_err_t client_init(esp_http_client_handle_t *client, client_config_t *configs);
void client_deinit(esp_http_client_handle_t client);                  
#endif //ESPIDF_REST_API_CALL