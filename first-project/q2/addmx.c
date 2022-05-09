#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdbool.h>

#define OK 0
#define ERROR 1

struct matrix {
    unsigned long lines, cols;
    int* values;  
};

int read_matrix_header(struct matrix* m, FILE *file) {
    if (fscanf(file, "%lux%lu", &(m->cols), &(m->lines)) == EOF) {
        return ERROR;
    }

    if (m->cols == 0 || m->lines == 0) {
        return ERROR;
    }

    return OK;
}

int read_matrix_body(struct matrix *m, FILE *file) {
    size_t len = m->cols * m-> lines;

    int *values = mmap(NULL, len, PROT_READ, MAP_SHARED | MAP_ANON, 0, 0);
    if (values == MAP_FAILED) {
        return ERROR;
    }
    
    size_t read = 0;

    int curr;
    while(read < len && fscanf(file, "%d", &curr) != EOF){
        values[read] = curr;
        read++; 
    }

    if (read != len) {
        munmap(values, len);
        return ERROR;
    }

    m->values = values;
    return OK;
}

int read_matrix(struct matrix* m, FILE *file) {
    if (read_matrix_header(m, file) != OK) {
        return ERROR;
    }

    if (read_matrix_body(m, file) != OK) {
        return ERROR;
    }

    return OK;
}

// int add_matrix(struct matrix* ma, struct matri* mb) {
//     if (read_matrix_header())
// }

int main(int argc, char const *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: addmx file1 file2\n");
    }
    
    FILE *afile = fopen(argv[1], "r");
    FILE *bfile = fopen(argv[2], "r"); 

    if (afile == NULL || bfile == NULL) {
        fprintf(stderr, "File not found");
        return EXIT_FAILURE;
    }

    struct matrix a;
    struct matrix b;
    
    if (read_matrix(&a, afile) == ERROR || read_matrix(&b, bfile) == ERROR) {
        return EXIT_FAILURE;
    }

    if (a.lines != b.lines || a.cols != b.cols) {
        fprintf(stderr, "The matrixes have incompatible structures.");
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < a.cols * a.lines; i++) {
        printf("%d ", a.values[i]);
    }

    return EXIT_SUCCESS;
}
