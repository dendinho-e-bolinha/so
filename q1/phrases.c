#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <argp.h>
#include <stdbool.h>

#define READ_END 0
#define WRITE_END 1
#define LINESIZE 256

#define DELIMS ".!?"

char* get_file_content(char *file_name) {
    FILE *file;
    char *text;

    file = fopen(file_name, "r");

    if (!file) {
        perror("Invalid file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    long numbytes = ftell(file);
    fseek(file, 0L, SEEK_SET);

    text = (char*)calloc((size_t)numbytes, sizeof(char));

    fread(text, sizeof(char), (size_t)numbytes, file);

    fclose(file);

    if (!text)
        return NULL;

    return text;
}

bool contains(const char *set, const char el) {
    size_t i;
    for (i = 0; i < strlen(set); ++i)
        if (el == set[i])
            return true;

    return false;
}

size_t count_seps(const char *text) {
    size_t count = 0;
    size_t i;

    for (i = 0; i < strlen(text); ++i)
        if (contains(DELIMS, text[i]))
            count++;

    return count;
}

int main(int argc, char *argv[]) {

    int opt;
    char *file;
    enum { NORMAL_MODE, LIST_MODE } mode = NORMAL_MODE;

    while ((opt = getopt(argc, argv, "l:")) != -1) {
        switch (opt) {
            case 'l':
                mode = LIST_MODE;
                file = optarg;
                break;
            default:
                fprintf(stderr, "Usage: phrases [-l] file\n");
                exit(EXIT_FAILURE);
        }
    }

    if ( (mode == NORMAL_MODE && argc != 2) || (mode == LIST_MODE && argc != 3)) {
        fprintf(stderr, "Usage: phrases [-l] file\n");
        exit(EXIT_FAILURE);
    }

    file = (mode == LIST_MODE) ? file : argv[1];

    char *text = get_file_content(file);
    size_t num_seps = count_seps(text);

    const char **phrases = malloc((num_seps + 1) * sizeof(*phrases));
    char *sep = malloc((num_seps + 1) * sizeof(char));
    size_t len = strlen(text);

    size_t count = 0, i;
    for (i = 0; i < len; ++i) {
        if (contains(DELIMS, text[i])) {
            if (contains(DELIMS, text[i + 1]))
                continue;

            sep[count] = text[i];
            count++;
            text[i] = '\0';
        }       
    }

    phrases[0] = text;

    int curr_sep = 0;

    size_t j;
    for (j = 0; j < len; ++j) {
        if (text[j] == '\0') {
            curr_sep++;
            char *aux = text + j + 1;
            if (*aux == ' ')
                aux++;
            phrases[curr_sep] = aux;
        }
    }

    size_t k;
    for (k = 0; k < count + 1; ++k)
        if (mode == LIST_MODE)
            printf("[%ld] %s%c\n",k + 1, phrases[k], sep[k]);

    if (mode == NORMAL_MODE)
        printf("%ld\n", k++);

    free(sep);
    free(phrases);
    free(text);

    return EXIT_SUCCESS;
}