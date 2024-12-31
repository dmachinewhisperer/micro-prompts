#ifdef ESP32
    #include <HTTPClient.h>
#elif defined(ESP8266)
    #include <ESP8266HTTPClient.h>
#endif

#include "types.h"

char *prompt_google_gemini()
static char *prompt_google_gemini(HTTPClient &httpclient, LLMClientConfig *llmconfigs) {
    // Add headers
    if (llmconfigs == NULL) {
        Serial.println("LLM configs are uninitialized");
        return NULL;
    }

    httpclient.addHeader("Content-Type", "application/json");

    // Format URL
    const char *base_url = "https://generativelanguage.googleapis.com";

    char google_generate_url[256];
    int len = snprintf(google_generate_url, sizeof(google_generate_url), 
        "%s/v1beta/models/%s:generateContent?key=%s", base_url, 
            llmconfigs->llmconfig.model_name, llmconfigs->llmconfig.api_key);
    if (len < 0 || len >= sizeof(google_generate_url)) {
        Serial.println("Failed to create URL, buffer overflow or error");
        return NULL;
    } else {
        Serial.println("Formatted URL: " + String(google_generate_url));
    }

    httpclient.begin(google_generate_url);

    // Allocate memory for the response
    char *response = (char *)malloc(sizeof(char) * MAX_HTTP_RESPONSE_LENGTH + 1);
    if (response == NULL) {
        Serial.println("Failed to allocate memory for response");
        return NULL;
    }

    // Build the request body
    char *request = build_google_request(llmconfigs);
    if (request == NULL) {
        free(response);
        return NULL;
    }
    Serial.println("Request Payload: " + String(request));

    // Make the POST request
    int httpCode = httpclient.POST(request);

    if (httpCode > 0) {
        String payload = httpclient.getString();
        strncpy(response, payload.c_str(), MAX_HTTP_RESPONSE_LENGTH);
        response[MAX_HTTP_RESPONSE_LENGTH] = '\0'; // Ensure null termination
        Serial.println("Response Code: " + String(httpCode));
        Serial.println("Response: " + payload);
    } else {
        Serial.println("Error in HTTP request: " + String(httpCode));
        free(response);
        response = NULL;
    }

    // Clean up
    free(request);
    //httpclient.end();

    // parse_google_response() is a pure C function and expects a char*
    return response ? parse_google_response(response) : NULL;
}



char* prompt(HTTPClient &httpclient, LLMClientConfig *llmconfigs)
{
    if(llmconfigs->llmconfig.provider == GOOGLE_GEMINI){
        return prompt_google_gemini(httpclient, llmconfigs);
    }

    //else if ...
    return NULL;
}