#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpc/mpc.h"

int
exec(mpc_ast_t* t) {
        int i;

        for (i = 1; i < t->children_num - 1; i++) {
                int j;
                int result;
                mpc_ast_t* expr;
                
                result = 0;
                expr = t->children[i];

                for (j = 0; j < expr->children_num - 1; j += 2) {
                        result += atoi(expr->children[j]->contents);
                }

                printf("%d\n", result);
        }
        return 0;
}

void
parse_script(char* script) {
        mpc_parser_t* int_lit_parser = mpc_new("int_lit");
        mpc_parser_t* expr_parser = mpc_new("expr");
        mpc_parser_t* taffy_parser = mpc_new("taffy");

        mpc_result_t r;

        mpca_lang(
                MPCA_LANG_DEFAULT,
                "\
                int_lit         : /-?[0-9]+/ ; \
                expr            : <int_lit> ( '+' <int_lit> )* ';' ; \
                taffy           : /^/ <expr>+ /$/ ; \
                ",
                int_lit_parser,
                expr_parser,
                taffy_parser
        );

        if (mpc_parse("[Taffy]", script, taffy_parser, &r)) {
                exec(r.output);
                mpc_ast_delete(r.output);
        } else {
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
        }

        mpc_cleanup(3,
                int_lit_parser,
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