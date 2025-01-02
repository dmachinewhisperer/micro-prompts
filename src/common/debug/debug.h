#ifndef ____DEBUG_H____
#define ____DEBUG_H____
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void llmclients_lib_write_last_error(const char *error_string);
const char *llmclients_lib_read_last_error();

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
