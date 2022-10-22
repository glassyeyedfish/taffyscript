#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpc/mpc.h"

#include "result.h"

/* TODO
Bug Fixes:

Granular:
        - Evaluate float expressions.
        - Throw error when mixing types.

Features:
        - Integer, Float, Char
        - Casting
        - Variables
        - Scope Blocks
        - If, else, control flow
        - loop, break, continue, loop if
*/

char success = 1;

result_int_t 
eval_expr(mpc_ast_t* expr) {
        result_int_t r;
        int i;
        int result;

        /* Return value if expression is just a number */
        if (expr->children_num == 0) {
                r.result = RESULT_INT;
                r.ok.i = atoi(expr->contents);
                return r;
        }

        i = 0;

        /* Shift index of expression begins with a paren */
        if (strcmp(expr->children[i]->contents, "(") == 0) i++;

        /* Check if first operand is a subexpression */
        if (expr->children[i]->children_num == 0) {
                result = atoi(expr->children[i]->contents);
        } else {
                r = eval_expr(expr->children[i]);
                if (r.result == RESULT_ERROR) {
                        return r;
                } else {
                        result = r.ok.i;
                }
        }

        /* Loop through and eval rest of the expr */
        for (i += 2; i < expr->children_num; i += 2) {
                int n;
                if (expr->children[i]->children_num == 0) {
                        n = atoi(expr->children[i]->contents);
                } else {
                        r = eval_expr(expr->children[i]);
                        if (r.result == RESULT_ERROR) {
                                return r;
                        } else {
                                n = r.ok.i;
                        }
                }

                if        (strcmp(expr->children[i-1]->contents, "+") == 0) {
                        result += n;
                } else if (strcmp(expr->children[i-1]->contents, "*") == 0) {
                        result *= n;
                } else if (strcmp(expr->children[i-1]->contents, "-") == 0) {
                        result -= n;
                } else if (strcmp(expr->children[i-1]->contents, "/") == 0) {
                        if (n == 0) {
                                r.result = RESULT_ERROR;
                                r.err = "division by zero";
                                return r;
                        } else {
                                result = (int) (result / n);
                        }
                }
        }

        r.result = RESULT_INT;
        r.ok.i = result;
        return r;
}

result_void_t
run_script(mpc_ast_t* t) {
        result_void_t r;
        int i;

        /* Loop through each expression in the script */
        for (i = 1; i < t->children_num - 1; i++) {
                r = eval_expr(t->children[i]);
                if (r.result == RESULT_ERROR) {
                        return r;
                } else {
                        printf("%d\n", r.ok.i);
                }
        }

        r.result = RESULT_VOID;
        return r;
}

result_void_t
parse_script(char* script) {
        result_void_t r;

        /* Literals / Value */
        mpc_parser_t* float_lit_parser = mpc_new("float_lit");
        mpc_parser_t* int_lit_parser = mpc_new("int_lit");
        mpc_parser_t* value_parser = mpc_new("value");


        /* Operations */
        mpc_parser_t* sum_parser = mpc_new("sum");
        mpc_parser_t* prod_parser = mpc_new("prod");


        /* Top */
        mpc_parser_t* expr_parser = mpc_new("expr");
        mpc_parser_t* taffy_parser = mpc_new("taffy");

        mpc_result_t ast;

        mpca_lang(
                MPCA_LANG_DEFAULT,
                "                                                       \
                int_lit         : /[1-9][0-9]*/ ;                       \
                float_lit       : /[1-9][0-9]*.[0-9]+/ ;                \
                                                                        \
                value           : <float_lit>                           \
                                | <int_lit>                             \
                                | '(' <expr> ')'                        \
                                ;                                       \
                                                                        \
                prod            : <value> ( ( '*' | '/' ) <value> )* ;  \
                sum             : <prod> ( ( '+' | '-' ) <prod> )* ;    \
                expr            : <sum> ;                               \
                taffy           : /^/ <expr>+ /$/ ;                     \
                ",
                float_lit_parser,
                int_lit_parser,
                value_parser,
                prod_parser,
                sum_parser,
                expr_parser,
                taffy_parser
        );

        if (mpc_parse("[taffy]", script, taffy_parser, &ast)) {
                mpc_ast_print(ast.output);
                puts("\n====================\n");

                // r = run_script(ast.output);
                // if (r.result == RESULT_ERROR) {
                //         mpc_ast_delete(ast.output);
                //         return r;
                // }

                mpc_ast_delete(ast.output);
        } else {
                mpc_err_print(ast.error);
                mpc_err_delete(ast.error);
        }

        mpc_cleanup(6,
                int_lit_parser,
                float_lit_parser,
                value_parser,
                prod_parser,
                sum_parser,
                expr_parser,
                taffy_parser
        );

        r.result = RESULT_VOID;
        return r;
}

result_void_t
load_script(char* file_name) {
        result_void_t r;
        FILE* fp = fopen(file_name, "r");

        if (fp == NULL) {
                r.result = RESULT_ERROR;
                r.err = "could not open file";
                return r;
        }

        {
                char* script;
                int size;

                int i;
                char c;

                fseek(fp, 0, SEEK_END);
                size = ftell(fp);
                script = (char*) malloc(sizeof(char) * (size + 1));
                rewind(fp);

                c = fgetc(fp);
                for (i = 0; i < size; i++) {
                        script[i] = c;
                        c = fgetc(fp);
                }
                script[size] = '\0';

                r = parse_script(script);
                if (r.result == RESULT_ERROR) {
                        free(script);
                        return r;
                }

                free(script);
        }

        fclose(fp);

        r.result = RESULT_VOID;
        return r;
}


int
main(int argc, char* argv[]) {
        result_t r;

        if (argc != 2) {
                fprintf(stderr, "Usage: taffy [FILE]\n");
        } else {
                r = load_script(argv[1]);
                if (r.result == RESULT_ERROR) {
                        fprintf(stderr, "[taffy] error: %s\n", r.err);
                }
        }

        return 0;
}