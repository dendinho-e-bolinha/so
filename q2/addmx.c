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
    if (fscanf(file, "%lux%lu", &(m->lines), &(m->cols)) < 2) {
        return ERROR;
    }

    if (m->cols == 0 || m->lines == 0) {
        return ERROR;
    }

    return OK;
}

int read_matrix_body(struct matrix *m, FILE *file) {
    size_t len = m->cols * m-> lines;

    int *values = mmap(NULL, len * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, 0, 0);
    if (values == MAP_FAILED) {
        return ERROR;
    }
    
    size_t read = 0;

    int curr;
    while(read < len && fscanf(file, "%d", &curr)  == 1) {
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
        fclose(afile);
        fclose(bfile);
        fprintf(stderr, "File not found");
        return EXIT_FAILURE;
    }

    struct matrix a;
    struct matrix b;
    
    if (read_matrix(&a, afile) == ERROR || read_matrix(&b, bfile) == ERROR) {
        fclose(afile);
        fclose(bfile);
        return EXIT_FAILURE;
    }

    fclose(afile);
    fclose(bfile);

    if (a.lines != b.lines || a.cols != b.cols) {
        fprintf(stderr, "The matrices have incompatible structures.");
        return EXIT_FAILURE;
    }

    int *result = mmap(NULL, a.cols * a.lines * sizeof(int), PROT_WRITE, MAP_SHARED | MAP_ANON, 0, 0);

    size_t col;
    bool is_parent = true;
    for (col = 0; col < a.cols; col++) {
        pid_t pid = fork();
        if (pid == 0) {
            is_parent = false;
            break;
        }
    }

    if (is_parent) {
        int status;
        for (size_t i = 0; i < a.cols; i++) {
            waitpid(-1, &status, 0);
        }

        printf("%lux%lu\n", a.lines, a.cols);

        for (size_t line = 0; line < a.lines; line++) {
            for (size_t column = 0; column < a.cols; column++) {
                if (column > 0) {
                    putchar(' ');
                }
                
                printf("%d", result[column + line * a.cols]);
            }

            putchar('\n');
        }

        return EXIT_SUCCESS;
    } else {
        for (size_t line = 0; line < a.lines; line++) {
            size_t pos = col + line * a.cols;
            result[pos] = a.values[pos] + b.values[pos];
        }
    }

    size_t size = a.cols * a.lines * sizeof(int);
    munmap(a.values, size);
    munmap(b.values, size);
    munmap(result, size);

    return EXIT_SUCCESS;
}
