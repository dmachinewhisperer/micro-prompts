#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include "test_memory.h"
//google

//to simulate failure of the cJSON_Create* functions(which hardly fails in practice), the functions are wrapped 
//up in __cJSON_Create*() wrappers. The wrapper function encountered inbuild_*_request  reads the first element of 
//the array below and succeeds if it is 1, else it fails.the function increments a counter to point to the second element 
//and the subsequent __*Create* function repeats.Each call to build_*_request exhausts a contiguous portion of the array equal
// to the number of __*Create* it encounters to ensure all possible (early) returns paths does not introduce memory bugs, we generate 
//all possible permutations of success/fail in the contiguos port. The array below must be at least as large as the number of __*Create* 
//function in the build_*_request fucntion with the largest __*Create*()'s

#define NENTRIES 20
static char paths[NENTRIES] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static int next = 0; 

cJSON *___cJSON_CreateObject(){
    cJSON *obj;
    if(paths[next]==0){
        obj =  NULL;
    } else{
        obj = cJSON_CreateObject();
    }
    if(next >= NENTRIES){
        next = 0; 
    } else{
        next++;
    }
    return obj;
}

cJSON *___cJSON_CreateArray(){
    cJSON *obj;
    if(!paths[next]){
        obj =  NULL;
    } else{
        obj = cJSON_CreateArray();
    }
    if(next >= NENTRIES){
        next = 0; 
    } else{
        next++;
    }
    return obj;
}

static unsigned int _exp(int base, int e) {
    int result = 1;
    for (int i = 0; i < e; i++) {
        result *= base;
    }
    return result;
}

static void show_test_pattern(int n_creates){
    for(int i = 0; i<n_creates; i++){
        printf("%d, ",paths[i]);
    }
    printf("\n");
}

static void generate_test_pattern(unsigned int n) {
    if(n>_exp(2,NENTRIES) -1) {
        printf("%s: n = %d", "number bits greater that paths[]", n);
        return; 
    } 
    int i = 0;
    while (n) {
        paths[i++] = n & 1;
        n = n >> 1;          
    }
}

//for a given llm configuration, this function tests all possible 
//returns paths of the build_*_request functions to ensure no memory leakage is possible
void run_memory_test(int n_creates, LLMClientConfig *config, char* (*build_request)(LLMClientConfig*)){
    char *out;
    unsigned int i = 0; 
    unsigned int total_patterns = _exp(2,n_creates);
    while(i <total_patterns-1){
        next = 0;
        generate_test_pattern(i);
        out = build_request(config);
        if(out){
            free(out);
        }
        /**
        if(out!=NULL){
            printf("%d: ", i);
            show_test_pattern();
            //printf("\n%s",out);
            free(out);
        }
        **/
        i++;
    }

    //print the output for the last combination: 2**N - 1. 
    //note that there are other combinations that will yield valid output
    //as there are execution pathways that cause the function to return earlier 
    //with valid output and not transverse the entire bit array. e.g feature = TEXT_INPUT
    generate_test_pattern(total_patterns-1);
    out = build_request(config);
    if(out==NULL){
        printf("%d: ", i);
        show_test_pattern(n_creates);
    }else{
        printf("%s\n", out);
        show_test_pattern(n_creates);
        free(out);
    }    
 
}