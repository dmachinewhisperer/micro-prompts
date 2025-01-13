#ifndef GOOGLE_H_
#define GOOGLE_H_
#include "../llm-types.h"

#ifdef __cplusplus
extern "C" {
#endif

char *build_google_request(LLMClientConfig *config);
char *parse_google_response(LLMClientConfig *config, const char *response);

#ifdef __cplusplus
}
#endif
#endif //GOOGLE_H_H