#include <stdio.h>
#include <string.h>


struct Student {
    int rollNumber;
    char name[50];
    float marks1, marks2, marks3;
    float totalMarks;
    float averageMarks;
    char grade;
};


float calculateTotal(float m1, float m2, float m3);
float calculateAverage(float total);
char assignGrade(float avg);
void displayPerformance(char grade);
void printRollNumbersRecursively(struct Student students[], int index, int count);
int isDuplicateRoll(struct Student students[], int currentIndex, int roll);

int main() {
    int numberOfStudents, i;

    printf("Enter number of students: ");
    scanf("%d", &numberOfStudents);

    struct Student students[numberOfStudents];

   
    for (i = 0; i < numberOfStudents; i++) {
        printf("\nEnter details for student %d\n", i + 1);

    
        int roll;
        while (1) {
            printf("Roll Number: ");
            scanf("%d", &roll);
            if (isDuplicateRoll(students, i, roll)) {
                printf("Roll number already exists Please enter a different roll number.\n");
            } else {
                students[i].rollNumber = roll;
                break;
            }
        }

        printf("Name: ");
        scanf("%s", students[i].name);

        printf("Enter marks in 3 subjects: ");
        scanf("%f %f %f", &students[i].marks1, &students[i].marks2, &students[i].marks3);

       
        students[i].totalMarks = calculateTotal(students[i].marks1, students[i].marks2, students[i].marks3);
        students[i].averageMarks = calculateAverage(students[i].totalMarks);

       
        students[i].grade = assignGrade(students[i].averageMarks);
    }

   
    printf("\nStudent Performance\n");
    for (i = 0; i < numberOfStudents; i++) {
        printf("Roll: %d\n", students[i].rollNumber);
        printf("Name: %s\n", students[i].name);
        printf("Total: %.2f\n", students[i].totalMarks);
        printf("Average: %.2f\n", students[i].averageMarks);
        printf("Grade: %c\n", students[i].grade);

       
        if (students[i].averageMarks < 35) {
            continue;
        }

        printf("Performance: ");
        displayPerformance(students[i].grade);
        printf("\n");
    }

   
    printf("List of Roll Numbers (via recursion): ");
    printRollNumbersRecursively(students, 0, numberOfStudents);
    printf("\n");

    return 0;
}

float calculateTotal(float m1, float m2, float m3) {
    return m1 + m2 + m3;
}


float calculateAverage(float total) {
    return total / 3.0;
}


char assignGrade(float avg) {
    if (avg >= 85)
        return 'A';
    else if (avg >= 70)
        return 'B';
    else if (avg >= 50)
        return 'C';
    else if (avg >= 35)
        return 'D';
    else
        return 'F';
}


void displayPerformance(char grade) {
    int stars = 0;
    switch (grade) {
        case 'A': stars = 5; break;
        case 'B': stars = 4; break;
        case 'C': stars = 3; break;
        case 'D': stars = 2; break;
        default: stars = 0; break;
    }

    for (int i = 0; i < stars; i++) {
        printf("*");
    }
}


void printRollNumbersRecursively(struct Student students[], int index, int count) {
    if (index == count)
        return;

    printf("%d ", students[index].rollNumber);
    printRollNumbersRecursively(students, index + 1, count);
}

int isDuplicateRoll(struct Student students[], int currentIndex, int roll) {
    for (int i = 0; i < currentIndex; i++) {
        if (students[i].rollNumber == roll) {
            return 1; 
        }
    }
    return 0; 
}
