idf_component_register(SRCS "espidf-http-rest-api.c" "espidf-llm-clients.c" "constants.c"
                    INCLUDE_DIRS "." "./providers"
                    REQUIRES esp-tls esp_http_client esp_netif protocol_examples_common nvs_flash  esp_event json
                    )         