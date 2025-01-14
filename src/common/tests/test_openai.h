#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "test_memory.h"
#include "../cJSON/cJSON.h"
#include "../providers/openai.h"

static int openai_n_creates = 15; 

void test_openai_gpt() {
    LLMClientConfig llm = DEFAULT_LLMCLIENT_CONFIG;
    llm.llmconfig.provider = OPENAI_GPT;

    // --- TEST CASE 1: TEXT_INPUT ---
    printf("// --- TEST CASE 1: TEXT_INPUT ---\n");
    llm.llmconfig.feature = TEXT_INPUT;

    // Test case 1.1: Valid prompt
    printf("// Test case 1.1: Valid prompt\n");
    llm.llmdata.prompt = "Pray tell, what is the capital of Abuja?";
    run_memory_test(openai_n_creates, &llm,build_openai_request );

    // Test case 1.2: Null prompt
    printf("// Test case 1.2: Null prompt\n");
    llm.llmdata.prompt = NULL;
    run_memory_test(openai_n_creates, &llm,build_openai_request );

    // Test case 1.3: System Prompts
    printf("// Test case 1.3: System Prompts\n");
    llm.llmdata.system = "You are a helpful assistant";
    llm.llmdata.prompt = "What is the capital of Nigeria";
    run_memory_test(openai_n_creates, &llm,build_openai_request );    

    // --- TEST CASE 2: TEXT_INPUT_WITH_LOCAL_FILE ---
    printf("// --- TEST CASE 2: TEXT_INPUT_WITH_LOCAL_FILE ---\n");
    llm.llmconfig.feature = TEXT_INPUT_WITH_LOCAL_IMG;
    llm.llmdata.prompt = "Summarize this text";  // Example prompt

    // Test case 2.1: Valid file data and mime type
    printf("// Test case 2.1: Valid file data and mime type\n");
    unsigned char data[] = {0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03};
    llm.llmdata.file.data = data;
    llm.llmdata.file.nbytes = 12;
    llm.llmdata.file.mime = "application/pdf";
    run_memory_test(openai_n_creates, &llm, build_openai_request); 

    // Test case 2.2: Null file data
    printf("// Test case 2.2: Null file data\n");
    llm.llmdata.file.data = NULL;
    run_memory_test(openai_n_creates, &llm, build_openai_request );

    // Test case 2.3: Null mime type
    printf("// Test case 2.3: Null mime type\n");
    llm.llmdata.file.mime = NULL;
    run_memory_test(openai_n_creates, &llm,build_openai_request );

    // --- TEST CASE 3: TEXT_INPUT_WITH_REMOTE_FILE ---
    printf("// --- TEST CASE 3: TEXT_INPUT_WITH_REMOTE_FILE ---\n");
    llm.llmconfig.feature = TEXT_INPUT_WITH_REMOTE_IMG;
    llm.llmdata.prompt = "Summarize this text";  // Example prompt

    // Test case 3.1: Valid remote file URI and mime type
    printf("// Test case 2.1: Valid file data and mime type\n");
    llm.llmdata.file.uri = "https://myendpoint.com/mypdf";
    llm.llmdata.file.mime = "application/pdf";
    run_memory_test(openai_n_creates, &llm,build_openai_request );

    // Test case 3.2: Null remote file URI
    printf("// Test case 3.2: Null remote file URI\n");
    llm.llmdata.file.uri = NULL;
    run_memory_test(openai_n_creates, &llm,build_openai_request );

    // Test case 3.3: Null mime type for remote file
    printf("// Test case 3.3: Null mime type for remote file\n");
    llm.llmdata.file.mime = NULL;
    run_memory_test(openai_n_creates, &llm,build_openai_request );

    // --- TEST CASE 4: TEXT_INPUT_WITH_STRUCTURED_OUTPUT WITH AUDIO ATTACHMENT ---
    printf("// --- TEST CASE 4: TEXT_INPUT_WITH_STRUCTURED_OUTPUT ---\n");
    llm.llmconfig.feature = TEXT_INPUT_WITH_LOCAL_AUDIO;
    llm.llmdata.prompt = "Please list the key benefits of using cloud computing in 2025."; 
    llm.llmdata.file.data = data; 
    llm.llmdata.file.mime = "audio/wav"; 
    llm.llmconfig.structured_output = 1;
    llm.llmconfig.json_response_schema = "{\n"
        "  \"benefits\": [\n"
        "    {\n"
        "      \"benefit\": \"string\",\n"
        "      \"description\": \"string\"\n"
        "    }\n"
        "  ]\n"
    "}";


    run_memory_test(openai_n_creates, &llm,build_openai_request );

    // --- TEST CASE 5: Response Parsing ---
    printf("// --- TEST CASE 5: Response Parsing ---\n");
    const char *response = 
        "{\n"
        "  \"id\": \"chatcmpl-123456\",\n"
        "  \"object\": \"chat.completion\",\n"
        "  \"created\": 1728933352,\n"
        "  \"model\": \"gpt-4o-2024-08-06\",\n"
        "  \"choices\": [\n"
        "    {\n"
        "      \"index\": 0,\n"
        "      \"message\": {\n"
        "        \"role\": \"assistant\",\n"
        "        \"content\": \"Yes, Indeed. The problem of being faster than light is that you can only live in darkeness.\",\n"
        "        \"refusal\": null\n"
        "      },\n"
        "      \"logprobs\": null,\n"
        "      \"finish_reason\": \"stop\"\n"
        "    }\n"
        "  ],\n"
        "  \"usage\": {\n"
        "    \"prompt_tokens\": 19,\n"
        "    \"completion_tokens\": 10,\n"
        "    \"total_tokens\": 29,\n"
        "    \"prompt_tokens_details\": {\n"
        "      \"cached_tokens\": 0\n"
        "    },\n"
        "    \"completion_tokens_details\": {\n"
        "      \"reasoning_tokens\": 0,\n"
        "      \"accepted_prediction_tokens\": 0,\n"
        "      \"rejected_prediction_tokens\": 0\n"
        "    }\n"
        "  },\n"
        "  \"system_fingerprint\": \"fp_6b68a8204b\"\n"
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
    parsed = parse_openai_response(&llm, response);
    assert(parsed != NULL); 
    printf("%s\n", parsed);
    free((void*)parsed);

    parsed = parse_openai_response(&llm, error_response);
    assert(parsed == NULL); 

    // --- TEST CASE 5: Chat usage and Parsing ---
    printf("// --- TEST CASE 6: Response Parsing ---\n");
    llm.llmconfig.chat =  5;
    llm.llmconfig.feature = TEXT_INPUT;
    llm.llmdata.prompt = "How do I prepare biriyani?";
    run_memory_test(openai_n_creates, &llm,build_openai_request );
    parsed = parse_openai_response(&llm, response);
    assert(parsed != NULL); 
    printf("%s\n", parsed);
    free((void*)parsed);    
    llm.llmdata.prompt = "Is lamb better than chicken for biriyani?";
    run_memory_test(openai_n_creates, &llm,build_openai_request );

    //free user state
    if(llm.user_state){
        cJSON_Delete(llm.user_state);
    }
}

