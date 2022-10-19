#ifndef ERROR_H
#define ERROR_H

/*
 * Result type inspired by the Rust programming language
 */

enum result_e { 
        RESULT_VOID,
        RESULT_INT, 
        RESULT_ERROR
};

typedef union {
        int i;
} ok_t;

typedef struct {
        enum result_e result;
        ok_t ok;
        char* err;
} result_t;

typedef result_t result_void_t;
typedef result_t result_int_t;

#endif