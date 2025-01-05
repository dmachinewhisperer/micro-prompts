#ifndef GROK_H_
#define GROK_H_


//#include "openai.h"
#include "../llm-types.h"
#include "../utils/utils.h"




//audio requests are not handled by chat completions api and must be done seperately. 
char *build_qrok_request_audio(LLMClientConfig *config){
    return NULL; 
}
/**
* Groq API response format for audio transcriptions. 
{
  "text": "Your transcribed text appears here...",
  "x_groq": {
    "id": "req_unique_id"
  }
}
*/
char *parse_groq_response_audio(LLMClientConfig *config){
    return NULL; 
}

char *build_groq_request(LLMClientConfig *config){
    //max_tokens

    if(!_is_feature_supported(config->llmconfig.feature, provider_groq_)){
        WRITE_LAST_ERROR("build_groq_request: Selected provider does not support selected feature");
        return NULL;        
    }
    if(config->llmconfig.feature == TEXT_INPUT){

        //groq support openais api calling style for chat completions
        //return build_openai_request(config);
    }
    
    

}
#endif