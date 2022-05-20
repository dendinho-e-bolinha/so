#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <argp.h>
#include <stdbool.h>
#include <assert.h>


#define READ_END 0
#define WRITE_END 1
#define LINESIZE 256

#define DELIM " "

char* get_file_content(char *file_name) {
    FILE *file;
    char *text;
    long numbytes;

    file = fopen(file_name, "r");

    if (!file) {
        perror("Invalid file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    numbytes = ftell(file);
    fseek(file, 0L, SEEK_SET);

    text = (char*)calloc(numbytes, sizeof(char));

    fread(text, sizeof(char), numbytes, file);

    fclose(file);

    if (!text)
        return NULL;

    return text;
}


char* replaceWord(const char *s, const char* oldW, const char* newW) {
    char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
  
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;
  
            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }
  
    // Making new string of enough length
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);
  
    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
  
    result[i] = '\0';
    return result;
}

bool contains(char *set, char el) {
    size_t i;
    for (i = 0; i < strlen(set); ++i)
        if (el == set[i])
            return true;

    return false;
}

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

char* cypher(char* text) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    size_t read;
    char **tokens;
    char* cyphered_text;

    fp = fopen("cypher.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    strncpy(cyphered_text, text, sizeof(text));

    while ((read = getline(&line, &len, fp)) != -1) {
        //printf("Retrieved line of length %zu:\n", read);
        printf("%s\n", line);

        tokens = str_split(line, ' ');

        char *word, *subs;

        if (tokens) {
            word = *(tokens);
            subs = *(tokens + 1);
            cyphered_text = replaceWord(cyphered_text, word, subs);
            printf("%s | %s \n", word, subs);
            free(*(tokens)); free(*(tokens + 1));
            free(tokens);
        } else {
            perror("");
            exit(EXIT_FAILURE);
        }
    }

    return cyphered_text;
}


int main(int argc, char *argv[]) {

    int fd[2];
    int fd2[2];
    pid_t pid;

    if (argc != 2) {
        printf("Wrong number of arguments passed: %d\n", argc - 1);
        exit(EXIT_FAILURE);
    } 
    
    if (pipe(fd) < 0 || pipe(fd2) < 0) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    if ((pid = fork()) < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        char *file_content = get_file_content(argv[1]);

        close(fd[READ_END]);
        write(fd[WRITE_END], file_content, sizeof(file_content));
        close(fd[WRITE_END]);

        close(fd2[WRITE_END]);
        dup2(fd2[READ_END], STDOUT_FILENO);
        close(fd2[READ_END]);
    } else {
        char file_content[500];

        close(fd[WRITE_END]);
        read(fd[READ_END], file_content, sizeof(file_content));
        char* cyphered_text = cypher(file_content);
        close(fd[READ_END]);

        close(fd2[READ_END]);
        write(fd2[WRITE_END], cyphered_text, sizeof(cyphered_text));
        close(fd2[WRITE_END]);

        printf("sadasdas: %s", cyphered_text);

    }

    return EXIT_SUCCESS;
}
