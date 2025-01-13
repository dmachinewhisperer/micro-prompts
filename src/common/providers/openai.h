#ifndef OPENAI_H_
#define OPENAI_H_

#include "../llm-types.h"

#ifdef __cplusplus
extern "C" {
#endif

char *build_openai_request(LLMClientConfig *config);
char *parse_openai_response(LLMClientConfig *config, const char *response);

#ifdef __cplusplus
}
#endif
#endif //OPENAI_H_