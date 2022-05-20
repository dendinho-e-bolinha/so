#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#define DELIMS " \n";
#define READ_END 0
#define WRITE_END 1

typedef struct {
    char* word;
    char* cyphered;
} wordStruct;

long getFileBytes(char* filename) {
    FILE* file;

    file = fopen(filename, "r");

    if(!file) {
        perror("Invalid file\n");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);

    return ftell(file);
}

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

size_t numOfLines(char* filename) {
    FILE *fp;
    int count = 0;
    char c;

    fp = fopen(filename, "r");
  
    if (fp == NULL) {
        printf("Could not open file %s", filename);
        exit(EXIT_FAILURE);
    }
  
    for (c = getc(fp); c != EOF; c = getc(fp))
        if (c == '\n') // if the file doesnt end in a new line, the number of lines isnt correct
            count++;
  
    fclose(fp);

    return count;
}

void readCypher(char * content, wordStruct* wordSet, size_t n) {
    size_t idx = 0, cnt = n;
    char delim[] = DELIMS;

    wordStruct words;
    char* token = strtok(content, delim);

    while(token) {
        if(idx % 2 == 0)
            words.word = strdup(token);    
        else
            words.cyphered = strdup(token);

        idx++;

        if(idx % 2 == 0 && idx > 0) {
            wordSet[cnt - n] = words;
            cnt++;
        }

        token = strtok(0, delim);
    }
}

void cypher(char* fileContent, char* result, size_t* bufferSize) {
    char aux[strlen(fileContent)];
    strcpy(aux, fileContent);

    size_t numLines = numOfLines("cypher.txt");
    wordStruct tmp[numLines];
    
    int alreadyWritten = 0, found = 1, helper = 0, bytesUsed = 0;
    size_t incr = 1024;

    readCypher(get_file_content("cypher.txt"), tmp, numLines);

    char* tokens = strtok(aux, " \n");

    while(tokens) {
        alreadyWritten = 0;
        found = 1;
        helper = 0;
        
        for(size_t i = 0; i < numLines; ++i) {
            if(bytesUsed == *bufferSize) {
                *bufferSize += incr;
                result = (char*) realloc(result, *bufferSize);
            }
                
            if(strstr(tokens, tmp[i].word)) {
                strcpy(result, tmp[i].cyphered);
                result += strlen(tmp[i].cyphered);
                tokens += strlen(tmp[i].word);
                strncpy(result, tokens, strlen(tokens));
                helper = 1;
                *result = ' ';
                result++;
                bytesUsed += strlen(tmp[i].cyphered) + strlen(tokens) + 2;

            } else if (strstr(tokens, tmp[i].cyphered)) {
                strcpy(result, tmp[i].word);
                result += strlen(tmp[i].word);
                tokens += strlen(tmp[i].cyphered);
                strncpy(result, tokens, strlen(tokens));
                helper = 1;
                *result = ' ';
                result++;
                bytesUsed += strlen(tmp[i].word) + strlen(tokens) + 2;

            }
            if(i == numLines - 1 && !helper)
                found = 0;

            if (!alreadyWritten && !found) {
                strncpy(result, tokens, strlen(tokens));
                result += strlen(tokens);
                alreadyWritten = 1;
                *result = ' ';
                result++;
                helper = 0;
                bytesUsed += strlen(tokens) + 2;
            }
        }
        tokens = strtok(0, " \n");
    }
    *result = '\0';
}

int main(int argc, char* argv[]) {
    int fd[2];
    int fd2[2];
    pid_t pid;

    char* text = get_file_content("test.txt");
    long numBytes = getFileBytes("test.txt");

    if (argc != 2 && false) {
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
    
    if (pid > 0) { // parent

        close(fd[READ_END]);
        write(fd[WRITE_END], text, numBytes);
        close(fd[WRITE_END]);

        close(fd2[WRITE_END]);
        dup2(fd2[READ_END], STDOUT_FILENO);
        close(fd2[READ_END]);

    } else { // child
        size_t bufferSize = 1024;
        char* result = (char*) calloc(bufferSize, sizeof(char));
        
        close(fd[WRITE_END]);
        read(fd[READ_END], text, numBytes);
        cypher(text, result, &bufferSize);
        close(fd[READ_END]);

        close(fd2[READ_END]);
        write(fd2[WRITE_END], result, bufferSize);
        close(fd2[WRITE_END]);

        free(result);
    }
    
    return EXIT_SUCCESS;
}
