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
void __attribute__((weak)) llmclients_write_last_error(const char *error_string);

void __attribute__((weak)) llmclients_debug_write(const char *debug_string);

//base64 enc/dec
char *base64_encode(const unsigned char *data, size_t input_length);
unsigned char *base64_decode(const char *data, size_t *output_length);
#ifdef __cplusplus
}
#endif

#define WRITE_LAST_ERROR(err)   llmclients_write_last_error(err)
#define DEBUG_WRITE(err)        llmclients_debug_write(err)


#endif