#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "users.txt"

typedef struct {
    int id;
    char name[50];
    int age;
} User;

int idExists(int id) {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL)
        return 0;

    User temp;
    while (fscanf(file, "%d %s %d", &temp.id, temp.name, &temp.age) == 3) {
        if (temp.id == id) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

void createUser() {
    FILE *file;
    User u;
    int valid = 0;

    printf("\n--- Create User ---\n");

    while (!valid) {
        printf("Enter ID: ");
        if (scanf("%d", &u.id) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');  
            continue;
        }
        if (u.id <= 0) {
            printf("ID must be positive.\n");
            continue;
        }
        if (idExists(u.id)) {
            printf("A user with ID %d already exists. Try another.\n", u.id);
            continue;
        }
        valid = 1;
    }

    printf("Enter Name: ");
    scanf("%s", u.name);

    while (1) {
        printf("Enter Age: ");
        if (scanf("%d", &u.age) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }
        if (u.age < 0 || u.age > 100) {
            printf("Please enter a valid age (0–100).\n");
            continue;
        }
        break;
    }

    file = fopen(FILE_NAME, "a");
    if (file == NULL) {
        printf("Error: could not open file for writing.\n");
        return;
    }

    fprintf(file, "%d %s %d\n", u.id, u.name, u.age);
    fclose(file);
    printf("User added successfully!\n");
}

void readUsers() {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        printf("\nNo records found.\n");
        return;
    }

    User u;
    printf("\n--- User Records ---\n");
    int found = 0;

    while (fscanf(file, "%d %s %d", &u.id, u.name, &u.age) == 3) {
        printf("ID: %d, Name: %s, Age: %d\n", u.id, u.name, u.age);
        found = 1;
    }

    if (!found)
        printf("No valid user data found.\n");

    fclose(file);
}

void updateUser() {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        printf("\nNo records found to update.\n");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Error: could not open temporary file.\n");
        fclose(file);
        return;
    }

    int id, found = 0;
    User u;

    printf("Enter ID to update: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        fclose(file);
        fclose(temp);
        remove("temp.txt");
        while (getchar() != '\n');
        return;
    }

    while (fscanf(file, "%d %s %d", &u.id, u.name, &u.age) == 3) {
        if (u.id == id) {
            found = 1;
            printf("Enter new Name (current: %s): ", u.name);
            scanf("%s", u.name);
            printf("Enter new Age (current: %d): ", u.age);
            if (scanf("%d", &u.age) != 1) {
                printf("Invalid age. Keeping old value.\n");
                while (getchar() != '\n');
            }
        }
        fprintf(temp, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(file);
    fclose(temp);

    if (!found) {
        printf("User not found.\n");
        remove("temp.txt");
        return;
    }

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);
    printf("User updated successfully.\n");
}

void deleteUser() {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        printf("\nNo records found to delete.\n");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Error: could not open temporary file.\n");
        fclose(file);
        return;
    }

    int id, found = 0;
    User u;

    printf("Enter ID to delete: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid input. Please enter a valid number.\n");
        while (getchar() != '\n');
        fclose(file);
        fclose(temp);
        remove("temp.txt");
        return;
    }

    while (fscanf(file, "%d %s %d", &u.id, u.name, &u.age) == 3) {
        if (u.id == id) {
            found = 1;
            continue; 
        }
        fprintf(temp, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(file);
    fclose(temp);

    if (!found) {
        printf("User not found.\n");
        remove("temp.txt");
        return;
    }

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);
    printf("User deleted successfully.\n");
}

int main() {
    int choice;

    while (1) {
        printf("\n1. Create User\n2. Read Users\n3. Update User\n4. Delete User\n5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); 
            continue;
        }

        switch (choice) {
            case 1: createUser(); break;
            case 2: readUsers(); break;
            case 3: updateUser(); break;
            case 4: deleteUser(); break;
            case 5: printf("Goodbye!\n"); exit(0);
            default: printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}
