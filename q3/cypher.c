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

typedef struct {
    char* word;
    char* cyphered;
} wordStruct;

long getFileBytes(char* filename) {
    FILE* file = fopen(filename, "r");
    if(!file) {
        perror("Invalid file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);

    return ftell(file);
}

char* get_file_content(const char *file_name) {
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

size_t numOfLines(const char* filename) {
    FILE *fp;
    size_t count = 0;
    char c;

    fp = fopen(filename, "r");
  
    if (fp == NULL) {
        printf("Could not open file %s", filename);
        exit(EXIT_FAILURE);
    }
  
    while ((c = (char)getc(fp)) != EOF) {
        if (c == '\n')
            count++;
    }
    
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

size_t replaceWord(char* tokens, char* oldWord, char* newWord, char* result) {
    size_t bytesToMove = 0;

    while(strstr(tokens, oldWord)) {
        strcpy(result, newWord);
        result += strlen(newWord);
        tokens += strlen(oldWord);
        bytesToMove += strlen(newWord);
    }
    strncpy(result, tokens, strlen(tokens));
    result += strlen(tokens);
    bytesToMove += strlen(tokens);

    return bytesToMove; 
}

void cypher(char* fileContent, char* result, size_t* bufferSize) {
    char aux[strlen(fileContent)];
    strcpy(aux, fileContent);

    size_t numLines = numOfLines("cypher.txt");
    wordStruct tmp[numLines];
    
    int alreadyWritten = 0, found = 1, helper = 0;
    size_t bytesUsed = 0, bytesToMove = 0;
    size_t incr = 1024;

    char* cypherFile = get_file_content("cypher.txt");

    readCypher(cypherFile, tmp, numLines);

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
                bytesToMove = replaceWord(tokens, tmp[i].word, tmp[i].cyphered, result);
                alreadyWritten = 1;
                helper = 1;
                result += bytesToMove;
                *result = ' ';
                result++;
                bytesUsed += bytesToMove + 1;
            } else if (strstr(tokens, tmp[i].cyphered)) {
                bytesToMove = replaceWord(tokens, tmp[i].cyphered, tmp[i].word, result);
                alreadyWritten = 1;
                helper = 1;
                result += bytesToMove;
                *result = ' ';
                result++;
                bytesUsed += bytesToMove + 1;
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
                bytesUsed += strlen(tokens) + 1;
            }
        }
        tokens = strtok(0, " \n");
    }
    *(--result) = '\0';

    size_t i;
    for(i = 0; i < numLines; ++i) {
        free(tmp[i].word);
        free(tmp[i].cyphered);
    }
    free(cypherFile);
}

int main(int argc, char* argv[]) {
    int fd[2];
    int fd2[2];
    pid_t pid;

    char text[1024] = {0};
    size_t numBytes = 1024;

    if (argc != 1) {
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
        
        while(read(STDIN_FILENO, text, 1024) > 0) {
            write(fd[WRITE_END], text, numBytes);
            numBytes += 1024;
        }
        
        close(fd[WRITE_END]);
        close(fd2[WRITE_END]);

        char buffer[1024];
        while(read(fd2[READ_END], buffer, 1024) > 0) {
            printf("%s", buffer);
        }

        waitpid(pid, NULL, 0);

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
