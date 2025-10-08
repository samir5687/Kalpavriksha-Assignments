#include <stdio.h>
#include <stdlib.h>

#define MAX_LENGTH 100


int myIsDigit(unsigned char ch) {
    return (ch >= '0' && ch <= '9');
}


int myStrLen(const char *str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}


void myRemoveNewline(char *str) {
    int len = myStrLen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}


int getPrecedence(char operator) {
    if (operator == '+' || operator == '-') return 1;
    if (operator == '*' || operator == '/') return 2;
    return 0;
}


int performOperation(int operand1, int operand2, char operator, int *errorFlag) {
    switch (operator) {
        case '+': return operand1 + operand2;
        case '-': return operand1 - operand2;
        case '*': return operand1 * operand2;
        case '/':
            if (operand2 == 0) {
                *errorFlag = 1; 
                return 0;
            }
            return operand1 / operand2;
        default:
            *errorFlag = 2; 
            return 0;
    }
}


int evaluateExpression(char *expression, int *errorFlag) {
    int valueStack[MAX_LENGTH], valueTop = -1;
    char operatorStack[MAX_LENGTH];
    int operatorTop = -1;

    for (int index = 0; expression[index] != '\0'; index++) {
        if (expression[index] == ' ')
            continue;

     
        else if (myIsDigit((unsigned char)expression[index])) {
            int number = 0;
            while (index < myStrLen(expression) && myIsDigit((unsigned char)expression[index])) {
                number = (number * 10) + (expression[index] - '0');
                index++;
            }
            valueStack[++valueTop] = number;
            index--; 
        }

       
        else if (expression[index] == '+' || expression[index] == '-' ||
                 expression[index] == '*' || expression[index] == '/') {

            while (operatorTop != -1 &&
                   getPrecedence(operatorStack[operatorTop]) >= getPrecedence(expression[index])) {

                int operand2 = valueStack[valueTop--];
                int operand1 = valueStack[valueTop--];
                char operator = operatorStack[operatorTop--];

                valueStack[++valueTop] = performOperation(operand1, operand2, operator, errorFlag);
                if (*errorFlag) return 0;
            }

            operatorStack[++operatorTop] = expression[index];
        }

     
        else {
            *errorFlag = 2;
            return 0;
        }
    }

   
    while (operatorTop != -1) {
        int operand2 = valueStack[valueTop--];
        int operand1 = valueStack[valueTop--];
        char operator = operatorStack[operatorTop--];

        valueStack[++valueTop] = performOperation(operand1, operand2, operator, errorFlag);
        if (*errorFlag) return 0;
    }

    return valueStack[valueTop];
}

int main() {
    char inputExpression[MAX_LENGTH];
    printf("Enter expression: ");
    fgets(inputExpression, MAX_LENGTH, stdin);

    myRemoveNewline(inputExpression);

    int errorFlag = 0;
    int result = evaluateExpression(inputExpression, &errorFlag);

    if (errorFlag == 1)
        printf("Error: Division by zero.\n");
    else if (errorFlag == 2)
        printf("Error: Invalid expression.\n");
    else
        printf("%d\n", result);

    return 0;
}
