/**
 * NOTES:
 * 1. esp_http_client_handle_t is a pointer: 
 *      definition: typedef struct esp_http_client *esp_http_client_handle_t
 * 2. Array actual sizes are array[MACRO +1] to allow for /0. Expressions like 
 *      array[MAX_LEN_*] = 'some value' here is therefore correct
 */

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "protocol_examples_utils.h"
#include "esp_tls.h"


#include "esp_system.h"
#include "cJSON.h"
#include "esp_http_client.h"


#include "esp_crt_bundle.h"
#include "freertos/queue.h"

#include "espidf-http-rest-api.h"


#define TAG "api-cli"

extern QueueHandle_t http_event_queue;
/**
 * @brief Handles HTTP events and sends them to a global queue.
 *
 * @param evt A pointer to the HTTP client event structure containing event data.
 *
 * @return An esp_err_t indicating the success or failure of the operation.
 */
static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static int output_len = 0;
    static char buf[MAX_HTTP_RESPONSE_LENGTH + 1]; //accumulate http response body

    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;

        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;

        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;

        case HTTP_EVENT_ON_HEADER: {
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);

            // Create a header event message
            http_event_message_t msg = {0};
            msg.type = EVENT_TYPE_HEADER;
            strncpy(msg.data.header.key, evt->header_key, MAX_HEADER_KEY_LENGTH);
            strncpy(msg.data.header.value, evt->header_value, MAX_HEADER_VALUE_LENGTH);
            msg.data.header.key[MAX_HEADER_KEY_LENGTH] = '\0';
            msg.data.header.value[MAX_HEADER_VALUE_LENGTH] = '\0';
            xQueueSend(http_event_queue, &msg, portMAX_DELAY); // Block until the queue is ready
            break;
        }

        case HTTP_EVENT_ON_FINISH: {
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");

            // Create a completion event message, carries the http response body
            http_event_message_t msg = {0};
            msg.type = EVENT_TYPE_FINISH;
            buf[output_len] = '\0';
            //memcpy(msg.data.finish.body, buf, MAX_HTTP_RESPONSE_LENGTH); //inc. null-term
            strcpy(msg.data.finish.body, buf);
            output_len = 0;

            //ESP_LOGI(TAG, "buf: %s", buf);
            //ESP_LOGI(TAG, "msg.data.finish.body: %s", msg.data.finish.body);
            

            xQueueSend(http_event_queue, &msg, portMAX_DELAY); // Block until the queue is ready
            break;
        }

        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA");

            int copy_len = 0;
            copy_len = MIN(evt->data_len, (MAX_HTTP_RESPONSE_LENGTH - output_len));
            if (copy_len) {
                memcpy(buf + output_len, evt->data, copy_len);
                
            }
            output_len += copy_len;
            break;

        case HTTP_EVENT_DISCONNECTED: {
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");

            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
        }

        default:
            break;
    }

    return ESP_OK;
}


/**
 * @brief Perform an HTTP request with optional inline modifications.
 *
 * @param client      The HTTP client handle.
 * @param method      HTTP method (e.g., HTTP_METHOD_POST). Pass NULL to retain the existing method.
 * @param url         URL for the request. Pass NULL to retain the existing URL.
 * @param header_mods NULL-terminated array of header modifications. Pass NULL for no changes.
 * @param data        Request payload (e.g., for POST). Pass NULL for no payload.
 * @param data_len    Length of the payload. Ignored if data is NULL.
 *
 * @return esp_err_t  ESP_OK on success or an error code on failure.
 */
esp_err_t client_request(esp_http_client_handle_t client,
                  http_method_t method,
                  const char *url,
                  header_mod_t *header_mods,
                  const char *data,
                  size_t data_len) {
    esp_err_t err;

    if(client==NULL){
        return ESP_FAIL;
    }

    // Set HTTP method if specified
    if (method != -1) {
        err = esp_http_client_set_method(client, method);
        if (err != ESP_OK) {
            return err;
        }
    }

    // Set URL if specified
    if (url != NULL) {
        err = esp_http_client_set_url(client, url);
        if (err != ESP_OK) {
            return err;
        }
    }

    // Apply header modifications if provided
    if (header_mods != NULL) {
        for (header_mod_t *mod = header_mods; mod->key != NULL; ++mod) {
            switch (mod->action) {
                case HEADER_ACTION_ADD:
                    if (mod->value != NULL) {
                        err = esp_http_client_set_header(client, mod->key, mod->value);
                        if (err != ESP_OK) {
                            return err;
                        }
                    }
                    break;
                case HEADER_ACTION_REMOVE:
                    err = esp_http_client_delete_header(client, mod->key);
                    //if (err != ESP_OK) {
                    //    return err;
                    //}
                    break;
                default:
                    return ESP_ERR_INVALID_ARG; // Invalid action
            }
        }
    }

    // Set payload if specified
    if (data != NULL && data_len > 0) {
        err = esp_http_client_set_post_field(client, data, data_len);
        if (err != ESP_OK) {
            return err;
        }
    }

    // Perform the HTTP request
    return esp_http_client_perform(client);
}


/**
 * @todo: use defines  to select when certs are used or not
 */
esp_err_t client_init(esp_http_client_handle_t *client, client_config_t *configs) {

    //load ca file
#if 0
    size_t ca_size;    
    uint8_t *google_ca;
    const char *ca_path = "/spiffs/google_root_ca.pem";
    esp_err_t ret = read_file_to_buffer(ca_path, &google_ca, &ca_size, MAX_CA_CERT_LENGHT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ca file");
        return ESP_FAIL;
    }
#endif
    //user_data is a void ptr.
    //point it at the conf struct to track session info 
    esp_http_client_config_t config = {
        .url = configs->url,
        .event_handler = http_event_handler,
        .buffer_size = 2048,
        .timeout_ms = 20000,
        //.auth_type = HTTP_AUTH_TYPE_NONE,
        .user_data = NULL,

        .use_global_ca_store = true,
        .crt_bundle_attach = esp_crt_bundle_attach,
        #if 0
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = (char *)google_ca,
        #endif
    };
    
    *client = esp_http_client_init(&config);
    if (*client == NULL) {
        return ESP_FAIL;
    }
    return ESP_OK;    
}


/**
 * @todo:
 */
void client_deinit(esp_http_client_handle_t client) {
    if (client) {
        
        // Free allocated resources 
        esp_http_client_cleanup(client);
    }
}