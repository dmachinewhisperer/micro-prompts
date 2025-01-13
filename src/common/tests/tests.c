#include "test_google.h"
#include "test_openai.h"

//void llmclients_write_last_error(const char *error_string) {
    //printf("%s", error_string);
//}

int main() {
    //printf("==========RUNNING test_google_gemini");
    //test_google_gemini();
    printf("==========RUNNING test_openai_gpt");
    test_openai_gpt();
    return 0;
}