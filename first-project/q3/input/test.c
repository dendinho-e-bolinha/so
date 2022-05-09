#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

char** str_split(char* a_str, const char a_delim) {
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}


int main(int argc, char const *argv[]) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    size_t read;
    char **tokens;

    fp = fopen("cypher.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        //printf("Retrieved line of length %zu:\n", read);
        printf("%s\n", line);

        tokens = str_split(line, ' ');

        char *word, *subs;

        if (tokens) {
            word = *(tokens);
            subs = *(tokens + 1);
            printf("%s | %s \n", word, subs);
            free(*(tokens)); free(*(tokens + 1));
            free(tokens);
        } else {
            perror("");
            exit(EXIT_FAILURE);
        }
    }

    printf("dhsuadhjaskda\n");

    return 0;
}
