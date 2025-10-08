#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define FILE_NAME "users.txt"
#define TEMP_FILE "temp.txt"
#define MAX_NAME_LEN 100
#define MAX_LINE_LEN 256

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int age;
} User;


void rstrip(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[--len] = '\0';
    }
}
void trim(char *s) {
    rstrip(s);
    
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
   
    int end = (int)strlen(s) - 1;
    while (end >= 0 && isspace((unsigned char)s[end])) s[end--] = '\0';
}


int read_line(const char *prompt, char *buf, size_t size) {
    if (prompt) printf("%s", prompt);
    if (!fgets(buf, (int)size, stdin)) return 0;
    rstrip(buf);
    return 1;
}


int read_int(const char *prompt, int *out) {
    char line[64];
    char *endptr;
    long val;
    while (1) {
        if (!read_line(prompt, line, sizeof(line))) return 0;
        trim(line);
        if (line[0] == '\0') {
            printf("Input cannot be empty. Please enter an integer.\n");
            continue;
        }
        errno = 0;
        val = strtol(line, &endptr, 10);
        if (endptr == line || *endptr != '\0' || errno == ERANGE) {
            printf("Invalid integer. Please try again.\n");
            continue;
        }
        *out = (int)val;
        return 1;
    }
}


int read_int_bounded(const char *prompt, int min, int max, int *out) {
    int v;
    while (1) {
        if (!read_int(prompt, &v)) return 0;
        if (v < min || v > max) {
            printf("Please enter a value between %d and %d.\n", min, max);
            continue;
        }
        *out = v;
        return 1;
    }
}


int parse_csv_line(const char *line_in, User *u) {
    if (!line_in || !u) return 0;
    char line[MAX_LINE_LEN];
    strncpy(line, line_in, sizeof(line)-1);
    line[sizeof(line)-1] = '\0';
    rstrip(line);

    char *p = line;
    char *idtok = strtok(p, ",");
    char *nametok = strtok(NULL, ",");
    char *agetok = strtok(NULL, ",");

    if (!idtok || !nametok || !agetok) return 0;

    trim(idtok); trim(nametok); trim(agetok);

    char *endptr;
    long id = strtol(idtok, &endptr, 10);
    if (endptr == idtok || *endptr != '\0') return 0;
    long age = strtol(agetok, &endptr, 10);
    if (endptr == agetok || *endptr != '\0') return 0;

    u->id = (int)id;
    u->age = (int)age;
    strncpy(u->name, nametok, MAX_NAME_LEN-1);
    u->name[MAX_NAME_LEN-1] = '\0';
    return 1;
}


int id_exists(int id) {
    FILE *f = fopen(FILE_NAME, "r");
    if (!f) return 0; 
    char line[MAX_LINE_LEN];
    User u;
    while (fgets(line, sizeof(line), f)) {
        if (!parse_csv_line(line, &u)) continue;
        if (u.id == id) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}


void createUser() {
    printf("\n-- Create User --\n");
    int id;
  
    while (1) {
        if (!read_int("Enter ID (enter 0 to cancel): ", &id)) {
            printf("Input error - aborting create.\n");
            return;
        }
        if (id == 0) {
            printf("Create cancelled.\n");
            return;
        }
        if (id < 0) {
            printf("ID must be positive.\n");
            continue;
        }
        if (id_exists(id)) {
            printf("ID %d already exists. Please enter a different ID.\n", id);
            continue;
        }
        break;
    }

    char name[MAX_NAME_LEN];
    while (1) {
        if (!read_line("Enter Name: ", name, sizeof(name))) {
            printf("Input error - aborting create.\n");
            return;
        }
        trim(name);
        if (name[0] == '\0') {
            printf("Name cannot be empty.\n");
            continue;
        }
        if (strchr(name, ',')) {
            printf("Name cannot contain comma ',' (storage uses CSV). Please re-enter.\n");
            continue;
        }
        break;
    }

    int age;
    if (!read_int_bounded("Enter Age (0-100): ", 0, 100, &age)) {
        printf("Input error - aborting create.\n");
        return;
    }

    FILE *file = fopen(FILE_NAME, "a");
    if (!file) {
        perror("Failed to open users file for appending");
        return;
    }
    fprintf(file, "%d,%s,%d\n", id, name, age);
    fclose(file);
    printf("User added successfully (ID: %d).\n", id);
}


void readUsers() {
    FILE *file = fopen(FILE_NAME, "r");
    if (!file) {
        printf("\nNo records found.\n");
        return;
    }
    printf("\n--- User Records ---\n");
    char line[MAX_LINE_LEN];
    User u;
    int any = 0;
    while (fgets(line, sizeof(line), file)) {
        if (!parse_csv_line(line, &u)) {
          
            continue;
        }
        printf("ID: %d, Name: %s, Age: %d\n", u.id, u.name, u.age);
        any = 1;
    }
    if (!any) printf("No valid records found.\n");
    fclose(file);
}

void updateUser() {
    FILE *file = fopen(FILE_NAME, "r");
    if (!file) {
        printf("\nNo records found to update.\n");
        return;
    }

    int idToUpdate;
    if (!read_int("Enter ID to update: ", &idToUpdate)) {
        fclose(file);
        printf("Input error - aborting update.\n");
        return;
    }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) {
        perror("Failed to open temporary file");
        fclose(file);
        return;
    }

    char line[MAX_LINE_LEN];
    User u;
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        if (!parse_csv_line(line, &u)) {
           
            continue;
        }
        if (u.id == idToUpdate) {
            found = 1;
            printf("Found user: ID: %d, Name: %s, Age: %d\n", u.id, u.name, u.age);

            char input[MAX_LINE_LEN];
         
            printf("Enter new Name (press Enter to keep \"%s\"): ", u.name);
            if (!fgets(input, sizeof(input), stdin)) {
                printf("Input error - keeping existing values.\n");
            } else {
                rstrip(input);
                trim(input);
                if (input[0] != '\0') {
                    if (strchr(input, ',')) {
                        printf("Name contains comma; keeping old name.\n");
                    } else {
                        strncpy(u.name, input, MAX_NAME_LEN-1);
                        u.name[MAX_NAME_LEN-1] = '\0';
                    }
                }
            }

            printf("Enter new Age (press Enter to keep %d): ", u.age);
            if (!fgets(input, sizeof(input), stdin)) {
                printf("Input error - keeping existing age.\n");
            } else {
                rstrip(input);
                trim(input);
                if (input[0] != '\0') {
                    char *endptr;
                    errno = 0;
                    long newAge = strtol(input, &endptr, 10);
                    if (endptr == input || *endptr != '\0' || errno == ERANGE || newAge < 0 || newAge > 150) {
                        printf("Invalid age input; keeping old age.\n");
                    } else {
                        u.age = (int)newAge;
                    }
                }
            }

           
            fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
        } else {
            
            fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
        }
    }

    fclose(file);
    fclose(temp);

    if (!found) {
        remove(TEMP_FILE);
        printf("User with ID %d not found.\n", idToUpdate);
        return;
    }

    if (remove(FILE_NAME) != 0) {
        perror("Failed to remove original file");
      
        printf("Update failed: could not replace original file. Temp file left for inspection.\n");
        return;
    }
    if (rename(TEMP_FILE, FILE_NAME) != 0) {
        perror("Failed to rename temp file");
        printf("Update may be incomplete.\n");
        return;
    }
    printf("User updated successfully.\n");
}


void deleteUser() {
    FILE *file = fopen(FILE_NAME, "r");
    if (!file) {
        printf("\nNo records found to delete.\n");
        return;
    }

    int idToDelete;
    if (!read_int("Enter ID to delete: ", &idToDelete)) {
        fclose(file);
        printf("Input error - aborting delete.\n");
        return;
    }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) {
        perror("Failed to open temporary file");
        fclose(file);
        return;
    }

    char line[MAX_LINE_LEN];
    User u;
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        if (!parse_csv_line(line, &u)) continue;
        if (u.id == idToDelete) {
            found = 1;
          
        } else {
            fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
        }
    }

    fclose(file);
    fclose(temp);

    if (!found) {
        remove(TEMP_FILE);
        printf("User with ID %d not found.\n", idToDelete);
        return;
    }

    if (remove(FILE_NAME) != 0) {
        perror("Failed to remove original file");
        printf("Delete failed: could not replace original file. Temp file left for inspection.\n");
        return;
    }
    if (rename(TEMP_FILE, FILE_NAME) != 0) {
        perror("Failed to rename temp file");
        printf("Delete may be incomplete.\n");
        return;
    }

    printf("User deleted successfully.\n");
}

int main(void) {
    while (1) {
        printf("\n1. Create User\n2. Read Users\n3. Update User\n4. Delete User\n5. Exit\n");
        int choice;
        if (!read_int("Enter choice: ", &choice)) {
            printf("\nInput error or EOF - exiting.\n");
            break;
        }
        switch (choice) {
            case 1: createUser(); break;
            case 2: readUsers(); break;
            case 3: updateUser(); break;
            case 4: deleteUser(); break;
            case 5: printf("Goodbye.\n"); exit(0);
            default: printf("Invalid choice. Please enter a number between 1 and 5.\n");
        }
    }
    return 0;
}

