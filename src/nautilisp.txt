#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "mpc/mpc.h"
#include "lval.h"

/* Helper function for eval() which evaluates an op. */
lval
eval_op(lval x, char* op, lval y) {
        /* Abort if already hit an error. */
        if (x.type == LVAL_ERR) { 
                return x; 
        } 
        if (y.type == LVAL_ERR) { 
                return y; 
        }

        /* Eval that sucker. */
        if (strcmp(op, "+") == 0) { 
                return lval_num(x.num + y.num); 
        } 
        if (strcmp(op, "-") == 0) { 
                return lval_num(x.num - y.num); 
        } 
        if (strcmp(op, "*") == 0) { 
                return lval_num(x.num * y.num); 
        } 
        if (strcmp(op, "/") == 0) { 
                return y.num == 0
                        ? lval_err(LERR_DIV_ZERO)
                        : lval_num(x.num / y.num); 
        }

        return lval_err(LERR_BAD_OP);
}

/* Evaluate an AST. */
lval
eval(mpc_ast_t* t) {
        char* op;
        lval x;
        int i;

        /* Base case: we hit a number, so return it! */
        if (strstr(t->tag, "number")) {
                long x;
                errno = 0;
                x = strtol(t->contents, NULL, 10);
                return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
        }

        /* Recursive case: we hit an expression. */
        op = t->children[1]->contents;
        x = eval(t->children[2]);

        i = 3;
        while (strstr(t->children[i]->tag, "expr")) {
                x = eval_op(x, op, eval(t->children[i]));
                i++;
        }

        return x;
}

int 
main(void) {

        /* Create Some Parsers */
        mpc_parser_t* number_parser   = mpc_new("number");
        mpc_parser_t* symbol_parser   = mpc_new("symbol");
        mpc_parser_t* sexpr_parser    = mpc_new("sexpr");
        mpc_parser_t* expr_parser     = mpc_new("expr");
        mpc_parser_t* lisp_parser     = mpc_new("lisp");
        
        /* Define them with the following Language */
        mpca_lang(
                MPCA_LANG_DEFAULT,
                "                                        \
                number : /-?[0-9]+/ ;                    \
                symbol : '+' | '-' | '*' | '/' ;         \
                sexpr  : '(' <expr>* ')' ;               \
                expr   : <number> | <symbol> | <sexpr> ; \
                lisp   : /^/ <expr>* /$/ ;               ",
                number_parser, 
                symbol_parser, 
                sexpr_parser, 
                expr_parser, 
                lisp_parser);

        /* REPL */
        puts("Nautilisp v0.9");
        puts("Press Ctrl+C to quit.");

        while(1) {
                char* input;
                mpc_result_t r;

                /* Take some input. */
                putchar('\n');
                input = readline("> ");
                add_history(input);

                /* Try to parse it. */
                if (mpc_parse("<stdin>", input, lisp_parser, &r)) {
                        lval result = eval(r.output);
                        lval_println(result);
                        mpc_ast_delete(r.output);
                } else {
                        mpc_err_print(r.error);
                        mpc_err_delete(r.error);
                }

                free(input);
        }

        mpc_cleanup(
                5, 
                number_parser, 
                symbol_parser, 
                sexpr_parser, 
                expr_parser, 
                lisp_parser);

        return 0;
}