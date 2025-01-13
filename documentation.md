# Documentation 

## Table of Contents

- [Objects](#objects)
- [Constants](#constants)
- [ESP-IDF APIs](#esp-idf-apis)
- [Arduino APIs](#arduino-apis)
- [Features supported by available providers](#features-supported-by-available-providers)
- [Supporting other providers](#supporting-other-providers)
- [Debugging The Library](#debugging)
- [Examples](#examples)
- [Porting and Baremetal Usage](#porting-and-baremetal-usage)

## Objects

### 1. `ProviderName` (Enum)
Enum representing different providers:

- `GROK`
- `GOOGLE_GEMINI`
- `OPENAI_GPT`

---

### 2. `GlobalFeaturePool` (Enum)
Enum representing all features available across all providers. Providers implement a subset of the features in this enum(see the provider feature section). 

- `TEXT_INPUT`  
  Description: Text as input, no other data.

- `TEXT_INPUT_WITH_LOCAL_FILE`  
  Description: Text input with a locally base64-encoded file attached.

- `TEXT_INPUT_WITH_REMOTE_FILE`  
  Description: Text input with a remote file URL provided.

- `AUDIO_INPUT_FOR_CLASSIFICATION`  
  Description: Audio file input for classification tasks.

- `AUDIO_INPUT_FOR_SPEECH_TO_TEXT`  
  Description: Audio input for automatic speech recognition (ASR).

- `TEXT_INPUT_WITH_STRUCTURED_OUTPUT`  
  Description: Text input with a structured/parsed output format.

- `TEXT_INPUT_WITH_REMOTE_IMG`  
  Description: Text input with a remote image.

- `TEXT_INPUT_WITH_LOCAL_IMG`  
  Description: Text input with a locally stored image.

- `TEXT_INPUT_WITH_LOCAL_AUDIO`  
  Description: Text input with a locally stored audio file.

---

### 3. `LLMConfig` (Struct)
Structure for model configuration:

- **`chat` (uint8_t)**  
  Description: If greater than 0, maintain chat history; else, one-shot prompt.

- **`structured_output` (uint8_t)**  
  Description: Flag for structured output.

- **`provider` (ProviderName)**  
  Description: The provider of the LLM (e.g., `GROK`, `GOOGLE_GEMINI`, `OPENAI_GPT`).

- **`feature` (GlobalFeaturePool)**  
  Description: A feature from the global feature pool (e.g., `TEXT_INPUT`, `AUDIO_INPUT_FOR_CLASSIFICATION`).

- **`base_url` (const char *)**  
  Description: Base URL of the LLM API provider.

- **`api_key` (const char *)**  
  Description: API key for authentication.

- **`model_name` (const char *)**  
  Description: Model name or ID (e.g., "gpt-4" or "cohere-llm").

- **`version` (const char *)**  
  Description: Optional API version (e.g., "v1").

- **`api_endpoint` (const char *)**  
  Description: API endpoint for providers that implement OpenAI API format.

- **`json_response_schema` (const char *)**  
  Description: String representing the structure of the output.

- **`max_tokens` (int)**  
  Description: Maximum number of tokens for the response.

- **`temperature` (float)**  
  Description: Controls the randomness of the model (0.0 - 1.0).

- **`top_p` (int)**  
  Description: Nucleus sampling value (0-1).

- **`top_k` (int)**  
  Description: Top-k value for sampling.

---

### 4. `LLMData` (Struct)
Structure for model data:

- **`prompt` (const char *)**  
  Description: The prompt to send to the model.

- **`system` (const char *)**  
  Description: System-level prompt for guiding model behavior.

- **`file` (Struct)**  
  Contains data related to a file attachment:
  - **`nbytes` (size_t)**  
    Description: The size of the file in bytes.
  - **`uri` (const char *)**  
    Description: URI pointing to the file.
  - **`mime` (const char *)**  
    Description: MIME type of the file. Different providers support different mimes
  - **`data` (const unsigned char *)**  
    Description: Raw file data.

- **`response` (Struct)**  
  Contains response configuration:
  - **`return_raw` (uint8_t)**  
    Description: If greater than 0, raw model output JSON is returned (user needs to parse the result).

---

### 5. `LLMClientConfig` (Struct)
Structure for the overall client configuration:

- **`llmconfig` (LLMConfig)**  
  Description: The model configuration.

- **`llmdata` (LLMData)**  
  Description: The model data.

- **`user_state` (void *)**  
  Description: A pointer for user-defined state, allowing for custom handling.

---

## Constants. 

1. `DEFAULT_LLMCLIENT_CONFIG` is an intializer that can be used to init a new LLMClientConfig object. It sets the client up for `TEXT_INPUT` prompting using `GOOGLE_GEMINI` provider. All other fields of the object are set to `0` (type int ) or `NULL` (type const char*);

## ESP-IDF APIs
### `char* prompt(esp_http_client_handle_t client, LLMClientConfig *llmconfigs);`
- **Description**: 
  - Returns parsed or raw model reponse based on the `llmconfigs` settings.
- **Arguments**:
  - `client`: Initialized http client.
  - `llmconfigs`: `LLMClientConfig` object representing model generation settings.   
  - `endpoint`: The API endpoint to use for communication with the provider.


## Arduino APIs

The main object `LLMClientConfig` is encapsulated in a C++ class to provide an Arduino-like interface. Getters and setters are used to read from and write to the object.

### `LLMClient` Class

### Public Methods

#### `void setProviderURL(const char *base, const char *version, const char *endpoint);`
- **Description**: 
  - Optional. For when to use, [See](#features-supported-by-available-providers)
  - It must be called before `begin()` if needed.
- **Arguments**:
  - `base`: The base URL of the provider's API.
  - `version`: The version of the API.
  - `endpoint`: The API endpoint to use for communication with the provider.

#### `void begin(const char *apiKey, const char *modelName, ProviderName provider);`
- **Description**: 
  - Initializes the client with the specified API key, model name, and provider.
- **Arguments**:
  - `apiKey`: The API key for authentication.
  - `modelName`: The name of the model (e.g., `"gpt-4"` or `"cohere-llm"`).
  - `provider`: The provider to use (e.g., `GROK`, `GOOGLE_GEMINI`, `OPENAI_GPT` from the `ProviderName` enum).

#### `void setFileProperties(const char *mime, const char *uri, const char *data, size_t nbytes);`
- **Description**: 
  - Sets the file properties when prompting with a file.
  - This method configures the MIME type, URI, data, and size of the file to be included in the prompt.
- **Arguments**:
  - `mime`: The MIME type of the file (e.g., `"image/jpeg"`, `"application/json"`).
  - `uri`: The URI pointing to the file location.
  - `data`: The raw data of the file.
  - `nbytes`: The size of the file data in bytes.

#### `void setProviderFeature(GlobalFeaturePool feature);`
- **Description**: 
  - Sets the feature to be used for the provider. See [supported provider features](#) for the list of available features.
- **Arguments**:
  - `feature`: A feature from the `GlobalFeaturePool` enum (e.g., `TEXT_INPUT`, `AUDIO_INPUT_FOR_CLASSIFICATION`).

#### `void setJSONResponseSchema(const char *json);`
- **Description**: 
  - Supplies a JSON schema if structured output is needed from the model.
- **Arguments**:
  - `json`: A string representing the desired JSON schema format.

#### `void setTemperature(float temperature);`
- **Description**: 
  - Sets the temperature for the model response, which controls the randomness (between 0.0 and 1.0).
- **Arguments**:
  - `temperature`: A floating-point value controlling the randomnes.

#### `void setMaxTokens(int maxTokens);`
- **Description**: 
  - Sets the maximum number of tokens that the model is allowed to generate for the response.
- **Arguments**:
  - `maxTokens`: An integer specifying the maximum token limit.

#### `void setTopP(float topP);`
- **Description**: 
  - Sets the `top_p` value for nucleus sampling. 
- **Arguments**:
  - `topP`: A floating-point value (between 0 and 1) representing the cumulative probability mass.

#### `void setTopK(int topK);`
- **Description**: 
  - Sets the `top_k` value for sampling. The model will select from the top `k` most likely tokens.
- **Arguments**:
  - `topK`: An integer specifying the number of tokens to sample from.

#### `void retainChatContext(int nchat_msgs);`
- **Description**: 
  - Retains the last `nchat_msgs` chat messages and includes them with the payload on each prompt. This is useful when interacting with the model in a conversational context.
- **Arguments**:
  - `nchat_msgs`: The number of chat messages to retain.

#### `void returnRawResponse();`
- **Description**: 
  - Configures the library to return the raw JSON response from the model instead of the parsed text response. This can be useful when debugging or when you want to handle the raw JSON manually.

#### `String prompt(String promptText);`
- **Description**: 
  - Prompts the model with the given text. This method returns the parsed model output.
  - If parsing fails (e.g., due to an unexpected JSON response or resource exhaustion), or if `returnRawResponse()` is enabled, the raw JSON response will be returned instead.
- **Arguments**:
  - `promptText`: The input text that you want to send to the model.
- **Returns**:
  - A `String` containing the model's parsed output or raw JSON response if configured.

---

## Features Supported by Available Providers

### Google
- `TEXT_INPUT`
- `TEXT_INPUT_WITH_LOCAL_FILE`  
  Description: Local file is base64 encoded and sent. See Google documentation for supported file formats.
- `TEXT_INPUT_WITH_REMOTE_FILE`  
  Description: File can be attached via URI.
- `TEXT_INPUT_WITH_STRUCTURED_OUTPUT`

### OpenAI
- `TEXT_INPUT`
- `TEXT_INPUT_WITH_STRUCTURED_OUTPUT`  
  Description: Returns structured output.
- `TEXT_INPUT_WITH_LOCAL_AUDIO`  
  Description: Base64 encoded audio input.
- `TEXT_INPUT_WITH_LOCAL_IMG`  
  Description: Base64 encoded local image.
- `TEXT_INPUT_WITH_REMOTE_IMG`  
  Description: File URI for remote image input.

### Groq
- `TEXT_INPUT`


## Supporting Other Providers

Some providers support the API format of other providers. For example, Grok supports the OpenAI API format. When using a model from provider X that implements provider Y's API format, you should configure the client as follows:

1. **Set the Provider URL for Provider X**  
   Supply the URL for provider X. The client will then communicate with provider X, but it will behave as though you're prompting a provider Y model.

2. **Configure the Parameters for Provider Y**  
   After setting the provider URL, configure subsequent parameters as if you're using provider Y's model. For example, set `providerName` to the provider Y model.

### ESP-IDF Configuration
For ESP-IDF, set the following fields in `LLMClientConfig`:
- `LLMClientConfig.llmdata.base_url`
- `LLMClientConfig.llmdata.version`
- `LLMClientConfig.llmdata.api_endpoint`

If any of these fields are missing or null, the client defaults to calling the original provider.

### Arduino Configuration
For Arduino, use the following method to set the provider URL:
- Call `setProviderURL()` and supply parameters.  
  **Note**: This must be called before `begin()`.

## Debugging

The library provides two functions marked as weak, with empty bodies. These functions allow you to implement custom error handling and debugging output in your application.

### Functions

- `void llmclients_write_last_error(const char *err)`  
  **Description**: Implement this function to handle errors in the library. The error message will be passed as the `err` parameter.
  
- `void llmclients_debug_write(const char *err)`  
  **Description**: Implement this function if you want to see debug messages. The debug information will be passed as the `err` parameter.

### Example Implementations

#### ESP-IDF
```c
void llmclients_write_last_error(const char *err) {
    ESP_LOGE("llm-clients-lib", "%s", err);
}
```
#### Arduino
```cpp
void llmclients_write_last_error(const char *err) {
    Serial.println(err);
}
```

## Examples 

## Porting and Baremetal Usage
The code in ./common is platform independent and provides code templates for building and parsing api requests of the supported providers. The platform dependent part of the libary provides hardware depended functionalities like http client and resource management so these functions can be used directly although you have to handle the hardware dependent parts yourself. A few things to keep in mind:

- char build_\*_request(LLMClientConfig* configs): constructs a json request structure using the configuration passed to it via configs. It returns a dynamically allocated json request structure. It is the callers responsibility to free it using free()

- char parse_\*_response(LLMClientConfig* configs, char*response): parses the response from the respective providerr configured in configs. Returns the dynamically allocated string and must be freed using free()

- LLMClientConfig.user_state is used to track the messages between the model and user when chat is enabled. This memory is dynamically allocated and is a cJSON object. Free it with cJSON_Delete() when chat is no longer needed or done with prompting.
