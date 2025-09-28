#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "users.txt"

typedef struct {
    int id;
    char name[50];
    int age;
} User;


void createUser() {
    FILE *file = fopen(FILE_NAME, "a");
    User u;

    printf("Enter ID: ");
    scanf("%d", &u.id);
    printf("Enter Name: ");
    scanf("%s", u.name);
    printf("Enter Age: ");
    scanf("%d", &u.age);

    fprintf(file, "%d %s %d\n", u.id, u.name, u.age);
    fclose(file);
    printf("User added successfully.\n");
}


void readUsers() {
    FILE *file = fopen(FILE_NAME, "r");
    if (!file) {
        printf("No records found.\n");
        return;
    }

    User u;
    printf("\n--- User Records ---\n");
    while (fscanf(file, "%d %s %d", &u.id, u.name, &u.age) != EOF) {
        printf("ID: %d, Name: %s, Age: %d\n", u.id, u.name, u.age);
    }
    fclose(file);
}


void updateUser() {
    FILE *file = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    int id, found = 0;

    printf("Enter ID to update: ");
    scanf("%d", &id);

    User u;
    while (fscanf(file, "%d %s %d", &u.id, u.name, &u.age) != EOF) {
        if (u.id == id) {
            printf("Enter new Name: ");
            scanf("%s", u.name);
            printf("Enter new Age: ");
            scanf("%d", &u.age);
            found = 1;
        }
        fprintf(temp, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(file);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (found) printf("User updated successfully.\n");
    else printf("User not found.\n");
}


void deleteUser() {
    FILE *file = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    int id, found = 0;

    printf("Enter ID to delete: ");
    scanf("%d", &id);

    User u;
    while (fscanf(file, "%d %s %d", &u.id, u.name, &u.age) != EOF) {
        if (u.id != id) {
            fprintf(temp, "%d %s %d\n", u.id, u.name, u.age);
        } else {
            found = 1;
        }
    }

    fclose(file);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (found) printf("User deleted successfully.\n");
    else printf("User not found.\n");
}

int main() {
    int choice;
    while (1) {
        printf("\n1. Create User\n2. Read Users\n3. Update User\n4. Delete User\n5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: createUser(); break;
            case 2: readUsers(); break;
            case 3: updateUser(); break;
            case 4: deleteUser(); break;
            case 5: exit(0);
            default: printf("Invalid choice.\n");
        }
    }
    return 0;
}
