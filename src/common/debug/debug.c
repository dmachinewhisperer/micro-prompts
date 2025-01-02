#include "debug.h"

static char *last_error = NULL;

void llmclients_lib_write_last_error(const char *error_string) {
    if (error_string == NULL) {
        return;
    }

    size_t len = strlen(error_string) + 1;
    char *new_error = (char *)malloc(len);

    if (new_error == NULL) {
        return;
    }

    strcpy(new_error, error_string);

    if (last_error != NULL) {
        free(last_error);
    }

    last_error = new_error;
}

const char *llmclients_lib_read_last_error() {
    return last_error;
}
