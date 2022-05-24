#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/wait.h>

#define DELIMS " \n";
#define READ_END 0
#define WRITE_END 1

#define BUFFER_SIZE 1024

typedef struct {
    char* word;
    char* cyphered;
} word_struct;

long get_file_bytes(FILE* file) {
    if (!file) {
        return -1;
    }

    long curr = ftell(file);
    if (fseek(file, 0L, SEEK_END) < 0) {
        return -1;
    }
    
    long size = ftell(file);
    if (fseek(file, curr, SEEK_SET) < 0) {
        return -1;
    }

    return size;
}

char* get_file_content(FILE * file) {
    if (!file) {
        return NULL;
    }

    long numbytes = get_file_bytes(file);
    if (numbytes < 0) {
        return NULL;
    }

    char *text = (char*) calloc((size_t) numbytes + 1, sizeof(char));
    if (text == NULL) {
        return NULL;    
    }

    fread(text, sizeof(char), (size_t) numbytes, file);
    text[numbytes] = '\0';
    
    return text;
}

size_t get_num_of_lines(FILE* file) {
    if (!file) {
        return 0;
    }

    long curr = ftell(file);

    if (fseek(file, 0, SEEK_SET) < 0) {
        return 0;
    }

    size_t lines = 0;
    
    char c;
    while ((c = getc(file)) != EOF) {
        if (c == '\n')
            lines++;
    }
    
    if (fseek(file, curr, SEEK_SET) < 0) {
        return 0;
    }

    return lines;
}

size_t read_substitutions(char *content, word_struct* word_set, size_t n) {
    size_t idx = 0, cnt = 0;
    char delim[] = DELIMS;

    word_struct words = { NULL, NULL };
    char* token = strtok(content, delim);

    while(token) {
        if (strlen(token) == 0) {
            break;
        }

        if(idx % 2 == 0) {
            words.word = strdup(token);    
        } else {
            words.cyphered = strdup(token);

            word_set[cnt] = words;
            cnt++;
        }

        idx++;
        token = strtok(NULL, delim);
    }

    return cnt;
}

char* ensure_buffer_capacity(char* buffer, size_t *current_capacity, size_t desired_capacity) {
    while (desired_capacity >= *current_capacity) {
        *current_capacity += BUFFER_SIZE;

        char *temp = realloc(buffer, (*current_capacity) * sizeof(char));
        if (temp == NULL) {
            free(buffer);
            return NULL;
        }

        buffer = temp;
        memset(buffer + (*current_capacity) - BUFFER_SIZE, 0, BUFFER_SIZE);
    }

    return buffer;
}

char* apply_cypher(char* content, word_struct* substitutions, size_t n) {
    size_t size = 0, max_size = BUFFER_SIZE;

    char *result = (char*) malloc(max_size * sizeof(char));
    if (result == NULL) {
        return NULL;
    }

    memset(result, 0, max_size);

    while (true) {
        char *next_substr = NULL, *next_target = NULL, *next_replacement = NULL;
        for (size_t i = 0; i < n; i++) {
            char *first = substitutions[i].word;
            char *second = substitutions[i].cyphered;

            char *substr = strstr(content, first);
            if (substr != NULL && (next_substr == NULL || substr < next_substr)) {
                next_substr = substr;
                next_target = first;
                next_replacement = second;
            }

            substr = strstr(content, second);
            if (substr != NULL && (next_substr == NULL || substr < next_substr)) {
                next_substr = substr;
                next_target = second;
                next_replacement = first;
            }
        }

        if (next_substr == NULL) {
            break;
        }

        size_t bytes = next_substr - content + strlen(next_replacement); 
        result = ensure_buffer_capacity(result, &max_size, size + bytes);
        if (result == NULL) {
            return NULL;
        }

        strncat(result, content, next_substr - content);
        size += next_substr - content;

        strcat(result, next_replacement);
        size += strlen(next_replacement);

        content = next_substr + strlen(next_target);
    }

    size_t bytes = strlen(content); 
    result = ensure_buffer_capacity(result, &max_size, size + bytes);
    if (result == NULL) {
        return NULL;
    }

    strcat(result, content);
    return result;
}

char* cypher(char* file_content) {
    FILE *cypher_file = fopen("cypher.txt", "r");
    if (cypher_file == NULL) {
        return NULL;
    }

    size_t num_lines = get_num_of_lines(cypher_file) + 1;
    word_struct tmp[num_lines];

    char* cypher_content = get_file_content(cypher_file);
    fclose(cypher_file);

    if (cypher_content == NULL) {
        return NULL;
    }

    num_lines = read_substitutions(cypher_content, tmp, num_lines);
    free(cypher_content);

    char *result = apply_cypher(file_content, tmp, num_lines);

    for (size_t i = 0; i < num_lines; i++) {
        free(tmp[i].word);
        free(tmp[i].cyphered);
    }

    return result;
}

int main(int argc, char* argv[]) {
    int input[2], output[2];
    if (pipe(input) < 0 || pipe(output) < 0) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) { // parent
        close(input[READ_END]);

        char buffer[BUFFER_SIZE];

        int numbytes;
        while((numbytes = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
            write(input[WRITE_END], buffer, numbytes);
        }
        
        close(input[WRITE_END]);
        close(output[WRITE_END]);

        int wstatus;
        waitpid(pid, &wstatus, 0);

        if (WIFEXITED(wstatus)) {
            while((numbytes = read(output[READ_END], buffer, BUFFER_SIZE)) > 0) {
                write(STDOUT_FILENO, buffer, numbytes);
            }

            close(output[READ_END]);
        } else {
            close(output[READ_END]);

            printf("An error occured while cyphering the text...\n");
            return EXIT_FAILURE;
        }

    } else { // child
        close(input[WRITE_END]);

        char *text = (char*) calloc(BUFFER_SIZE, sizeof(char));
        if (text == NULL) {
            return EXIT_FAILURE;
        }

        size_t len = 0, capacity = BUFFER_SIZE;

        int numbytes;
        while ((numbytes = read(input[READ_END], text + len, BUFFER_SIZE)) > 0) {
            len += numbytes;

            text = ensure_buffer_capacity(text, &capacity, len + BUFFER_SIZE);
            if (text == NULL) {
                return EXIT_FAILURE;
            }
        }
        
        close(input[READ_END]);
        
        char *result = cypher(text);
        free(text);
        
        if (result == NULL) {
            return EXIT_FAILURE;
        }

        close(output[READ_END]);
        write(output[WRITE_END], result, strlen(result) + 1);
        close(output[WRITE_END]);

        free(result);
    }
    
    return EXIT_SUCCESS;
}
