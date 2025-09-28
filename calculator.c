#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 100


int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}


int applyOp(int a, int b, char op, int *error) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) {
                *error = 1;
                return 0;
            }
            return a / b;
    }
    return 0;
}


int evaluate(char* tokens, int *error) {
    int i;
    int values[MAX], valTop = -1;
    char ops[MAX]; int opTop = -1;

    for (i = 0; tokens[i] != '\0'; i++) {
        if (tokens[i] == ' ')
            continue;

        
        else if (isdigit(tokens[i])) {
            int val = 0;
            while (i < strlen(tokens) && isdigit(tokens[i])) {
                val = (val * 10) + (tokens[i] - '0');
                i++;
            }
            values[++valTop] = val;
            i--;
        }

     
        else if (tokens[i] == '+' || tokens[i] == '-' ||
                 tokens[i] == '*' || tokens[i] == '/') {
            while (opTop != -1 && precedence(ops[opTop]) >= precedence(tokens[i])) {
                int b = values[valTop--];
                int a = values[valTop--];
                char op = ops[opTop--];
                values[++valTop] = applyOp(a, b, op, error);
                if (*error) return 0;
            }
            ops[++opTop] = tokens[i];
        }

       
        else {
            *error = 2;
            return 0;
        }
    }

    while (opTop != -1) {
        int b = values[valTop--];
        int a = values[valTop--];
        char op = ops[opTop--];
        values[++valTop] = applyOp(a, b, op, error);
        if (*error) return 0;
    }

    return values[valTop];
}

int main() {
    char expression[MAX];
    printf("Enter expression: ");
    fgets(expression, MAX, stdin);
    expression[strcspn(expression, "\n")] = '\0';

    int error = 0;
    int result = evaluate(expression, &error);

    if (error == 1) {
        printf("Error: Division by zero.\n");
    } else if (error == 2) {
        printf("Error: Invalid expression.\n");
    } else {
        printf("%d\n", result);
    }

    return 0;
}

