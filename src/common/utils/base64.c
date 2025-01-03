#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64_encode(const unsigned char *data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = malloc(output_length + 1);
    if (encoded_data == NULL) return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_chars[triple & 0x3F];
    }

    // Add padding
    if (input_length % 3) {
        for (i = 0; i < 3 - (input_length % 3); i++) {
            encoded_data[output_length - 1 - i] = '=';
        }
    }

    encoded_data[output_length] = '\0';
    return encoded_data;
}

unsigned char *base64_decode(const char *data, size_t *output_length) {
    size_t input_length = strlen(data);
    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    size_t i, j;
    uint32_t sextet_a, sextet_b, sextet_c, sextet_d;
    uint32_t triple;

    for (i = 0, j = 0; i < input_length;) {
        sextet_a = data[i] == '=' ? 0 : strchr(base64_chars, data[i]) - base64_chars;
        sextet_b = data[i + 1] == '=' ? 0 : strchr(base64_chars, data[i + 1]) - base64_chars;
        sextet_c = data[i + 2] == '=' ? 0 : strchr(base64_chars, data[i + 2]) - base64_chars;
        sextet_d = data[i + 3] == '=' ? 0 : strchr(base64_chars, data[i + 3]) - base64_chars;

        triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;

        i += 4;
    }

    return decoded_data;
}

/**
int main() {
    const char *test = "Hello, World!";
    char *encoded = base64_encode((unsigned char *)test, strlen(test));
    printf("Encoded: %s\n", encoded);

    size_t decoded_length;
    unsigned char *decoded = base64_decode(encoded, &decoded_length);
    printf("Decoded: %.*s\n", (int)decoded_length, decoded);

    free(encoded);
    free(decoded);
    return 0;
}
*/