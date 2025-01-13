#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "test_memory.h"
#include "../cJSON/cJSON.h"
#include "../providers/google.h"

static int google_n_creates = 14; 

void test_google_gemini() {
    LLMClientConfig llm = DEFAULT_LLMCLIENT_CONFIG;
    llm.llmconfig.provider = GOOGLE_GEMINI;

    // --- TEST CASE 1: TEXT_INPUT ---
    printf("// --- TEST CASE 1: TEXT_INPUT ---\n");
    llm.llmconfig.feature = TEXT_INPUT;

    // Test case 1.1: Valid prompt
    printf("// Test case 1.1: Valid prompt\n");
    llm.llmdata.prompt = "Pray tell, what is the capital of Abuja?";
    run_memory_test(google_n_creates, &llm,build_google_request );

    // Test case 1.2: Null prompt
    printf("// Test case 1.2: Null prompt\n");
    llm.llmdata.prompt = NULL;
    run_memory_test(google_n_creates, &llm,build_google_request );

    // Test case 1.3: System Prompts
    printf("// Test case 1.3: System Prompts\n");
    llm.llmdata.system = "You are a helpful assistant";
    llm.llmdata.prompt = "What is the capital of Nigeria?";
    run_memory_test(google_n_creates, &llm,build_google_request );    

    // --- TEST CASE 2: TEXT_INPUT_WITH_LOCAL_FILE ---
    printf("// --- TEST CASE 2: TEXT_INPUT_WITH_LOCAL_FILE ---\n");
    llm.llmconfig.feature = TEXT_INPUT_WITH_LOCAL_FILE;
    llm.llmdata.prompt = "Summarize this text";  // Example prompt

    // Test case 2.1: Valid file data and mime type
    printf("// Test case 2.1: Valid file data and mime type\n");
    unsigned char pdf[] = {0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03};
    llm.llmdata.file.data = pdf;
    llm.llmdata.file.nbytes = 12;
    llm.llmdata.file.mime = "application/pdf";
    run_memory_test(google_n_creates, &llm, build_google_request); 

    // Test case 2.2: Null file data
    printf("// Test case 2.2: Null file data\n");
    llm.llmdata.file.data = NULL;
    run_memory_test(google_n_creates, &llm, build_google_request );

    // Test case 2.3: Null mime type
    printf("// Test case 2.3: Null mime type\n");
    llm.llmdata.file.mime = NULL;
    run_memory_test(google_n_creates, &llm,build_google_request );

    // --- TEST CASE 3: TEXT_INPUT_WITH_REMOTE_FILE ---
    printf("// --- TEST CASE 3: TEXT_INPUT_WITH_REMOTE_FILE ---\n");
    llm.llmconfig.feature = TEXT_INPUT_WITH_REMOTE_FILE;
    llm.llmdata.prompt = "Summarize this text";  // Example prompt

    // Test case 3.1: Valid remote file URI and mime type
    printf("// Test case 2.1: Valid file data and mime type\n");
    llm.llmdata.file.uri = "https://myendpoint.com/mypdf";
    llm.llmdata.file.mime = "application/pdf";
    run_memory_test(google_n_creates, &llm,build_google_request );

    // Test case 3.2: Null remote file URI
    printf("// Test case 3.2: Null remote file URI\n");
    llm.llmdata.file.uri = NULL;
    run_memory_test(google_n_creates, &llm,build_google_request );

    // Test case 3.3: Null mime type for remote file
    printf("// Test case 3.3: Null mime type for remote file\n");
    llm.llmdata.file.mime = NULL;
    run_memory_test(google_n_creates, &llm,build_google_request );

    // --- TEST CASE 4: TEXT_INPUT_WITH_STRUCTURED_OUTPUT ---
    printf("// --- TEST CASE 4: TEXT_INPUT_WITH_STRUCTURED_OUTPUT ---\n");
    llm.llmconfig.feature = TEXT_INPUT_WITH_REMOTE_FILE;
    llm.llmdata.prompt = "Please list the key benefits of using cloud computing in 2025."; 
    llm.llmdata.file.uri = "https://myendpoint.com"; 
    llm.llmdata.file.mime = "application/pdf"; 
    llm.llmconfig.structured_output = 1;
    llm.llmconfig.json_response_schema = "{\n"
        "  \"benefits\": [\n"
        "    {\n"
        "      \"benefit\": \"string\",\n"
        "      \"description\": \"string\"\n"
        "    }\n"
        "  ]\n"
    "}";


    run_memory_test(google_n_creates, &llm,build_google_request );

    // --- TEST CASE 5: Response Parsing ---
    printf("// --- TEST CASE 5: Response Parsing ---\n");
    const char *response = 
        "{\n"
        "    \"candidates\": [\n"
        "        {\n"
        "            \"content\": {\n"
        "                \"parts\": [\n"
        "                    {\n"
        "                        \"text\": \"The capital of Abuja is Abuja.\"\n"
        "                    }\n"
        "                ],\n"
        "                \"role\": \"assistant\"\n"
        "            },\n"
        "            \"finishReason\": \"stop\",\n"
        "            \"index\": 0,\n"
        "            \"safetyRatings\": [\n"
        "                {\n"
        "                    \"category\": \"safety\",\n"
        "                    \"probability\": \"0.95\"\n"
        "                }\n"
        "            ]\n"
        "        }\n"
        "    ],\n"
        "    \"usageMetadata\": {\n"
        "        \"promptTokenCount\": 10,\n"
        "        \"candidatesTokenCount\": 8,\n"
        "        \"totalTokenCount\": 18\n"
        "    }\n"
        "}";
    
    const char *error_response = 
        "{\n"
        "    \"error\": {\n"
        "        \"code\": 400,\n"
        "        \"message\": \"Invalid input. Missing required fields.\",\n"
        "        \"details\": [\n"
        "            {\n"
        "                \"type\": \"missing_field\",\n"
        "                \"field\": \"prompt\"\n"
        "            }\n"
        "        ]\n"
        "    }\n"
        "}";

    const char *parsed;
    parsed = parse_google_response(&llm, response);
    assert(parsed != NULL); 
    printf("%s\n", parsed);
    free((void*)parsed);

    parsed = parse_google_response(&llm, error_response);
    assert(parsed == NULL); 

    // --- TEST CASE 5: Response Parsing ---
    printf("// --- TEST CASE 5: Response Parsing ---\n");
    llm.llmconfig.chat =  5;
    llm.llmconfig.feature = TEXT_INPUT;
    llm.llmdata.prompt = "How do I prepare biriyani?";
    run_memory_test(google_n_creates, &llm,build_google_request );
    parsed = parse_google_response(&llm, response);
    assert(parsed != NULL); 
    printf("%s\n", parsed);
    free((void*)parsed);    
    llm.llmdata.prompt = "Is lamb better than chicken for biriyani?";
    run_memory_test(google_n_creates, &llm,build_google_request );

    //free user state
    if(llm.user_state){
        cJSON_Delete(llm.user_state);
    }
}

