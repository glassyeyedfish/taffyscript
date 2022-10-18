#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpc/mpc.h"

/* TODO
Bug Fixes:
        - Debug eval_expr

Features:
        - Integer, Float, Char
        - Casting
        - Variables
        - Scope Blocks
        - If, else, control flow
        - loop, break, continue, loop if
*/


int 
eval_expr(mpc_ast_t* expr) {
        int i;
        int result = atoi(expr->children[1]->contents);

        for (i = 3; i < expr->children_num - 1; i++) {
                int n;
                if (strstr(expr->children[i]->tag, "int_lit")) {
                        n = atoi(expr->children[i]->contents);
                } else if (strstr(expr->children[i]->tag, "expr")) {
                        n = eval_expr(expr->children[i]);
                }

                if        (strcmp(expr->children[i-1]->contents, "+") == 0) {
                        result += n;
                } else if (strcmp(expr->children[i-1]->contents, "*") == 0) {
                        result *= n;
                } else if (strcmp(expr->children[i-1]->contents, "-") == 0) {
                        result -= n;
                } else if (strcmp(expr->children[i-1]->contents, "/") == 0) {
                        if (n == 0) {
                                fprintf(stderr, "[taffy] error: division by zero");
                                exit(EXIT_FAILURE);
                        } else {
                                result = (int) (result / n);
                        }
                }
        }

        return result;
}

int
run_script(mpc_ast_t* t) {
        int i;

        /* Loop through each expression in the script */
        for (i = 1; i < t->children_num - 1; i++) {
                printf("%d\n", eval_expr(t->children[i]));
        }
        return 0;
}

void
parse_script(char* script) {
        /* Integer literal */
        mpc_parser_t* int_lit_parser = mpc_new("int_lit");

        /* Operator */
        mpc_parser_t* opnd_parser = mpc_new("opnd");
        mpc_parser_t* oper_parser = mpc_new("oper");

        /* Top-level expression */
        mpc_parser_t* expr_parser = mpc_new("expr");
        mpc_parser_t* taffy_parser = mpc_new("taffy");

        mpc_result_t r;

        mpca_lang(
                MPCA_LANG_DEFAULT,
                "\
                int_lit         : /-?[0-9]+/ ;\
                opnd            : <int_lit> | <expr> ;\
                oper            : '+' | '*' | '-' | '/' ;\
                expr            : '(' <opnd> ( <oper> <opnd> )* ')' ;\
                taffy           : /^/ <expr>+ /$/ ;\
                ",
                int_lit_parser,
                opnd_parser,
                oper_parser,
                expr_parser,
                taffy_parser
        );

        if (mpc_parse("[taffy]", script, taffy_parser, &r)) {
                mpc_ast_print(r.output);
                run_script(r.output);
                mpc_ast_delete(r.output);
        } else {
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
        }

        mpc_cleanup(4,
                int_lit_parser,
                opnd_parser,
                oper_parser,
                expr_parser,
                taffy_parser
        );
}

void
load_script(char* file_name) {
        bool success = true;
        FILE* fp = fopen(file_name, "r");

        if (fp == NULL) {
                fprintf(stderr, "Error: could not open file!");
                success = false;
        }

        if (success) {
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

                parse_script(script);

                free(script);
        }

        fclose(fp);
}


int
main(int argc, char* argv[]) {

        if (argc != 2) {
                fprintf(stderr, "Usage: taffy [FILE]");
        } else {
                load_script(argv[1]);
        }

        return 0;
}