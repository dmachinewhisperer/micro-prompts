#ifndef LLM_CLIENT_UTILS_
#define LLM_CLIENT_UTILS_
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//debugging 
void llmclients_lib_write_last_error(const char *error_string);
const char *llmclients_lib_read_last_error();

//base64 enc/dec
char *base64_encode(const unsigned char *data, size_t input_length);
unsigned char *base64_decode(const char *data, size_t *output_length);
#ifdef __cplusplus
}
#endif

#ifdef LLM_CLIENT_LIB_ENABLE_LOGGING
#define WRITE_LAST_ERROR(err) llmclients_lib_write_last_error(err)
#define READ_LAST_ERROR()   llmclients_lib_read_last_error()
#else
#define WRITE_LAST_ERROR(err)
#define READ_LAST_ERROR()
#endif

#endif