#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Product {
    int id;
    char name[50];
    float price;
    int qty;
};


int getIntInput(const char *msg);
float getFloatInput(const char *msg);
int isUniqueID(struct Product *p, int n, int id);
void addProduct(struct Product **p, int *n);
void viewProducts(struct Product *p, int n);
void updateQuantity(struct Product *p, int n);
void searchByID(struct Product *p, int n);
void searchByName(struct Product *p, int n);
void searchByPrice(struct Product *p, int n);
void deleteProduct(struct Product **p, int *n);


int getIntInput(const char *msg) {
    char temp[50];
    int valid, num;
    do {
        valid = 1;
        printf("%s", msg);
        scanf("%s", temp);
        for (int i = 0; temp[i] != '\0'; i++) {
            if (temp[i] < '0' || temp[i] > '9') {
                valid = 0;
                break;
            }
        }
        if (valid)
            num = atoi(temp);
        else
            printf("Invalid input! Enter numbers only.\n");
    } while (!valid);
    return num;
}


float getFloatInput(const char *msg) {
    char temp[50];
    int dotCount = 0, valid;
    float num;
    do {
        valid = 1;
        printf("%s", msg);
        scanf("%s", temp);
        for (int i = 0; temp[i] != '\0'; i++) {
            if ((temp[i] < '0' || temp[i] > '9') && temp[i] != '.') {
                valid = 0;
                break;
            }
            if (temp[i] == '.') dotCount++;
            if (dotCount > 1) { valid = 0; break; }
        }
        if (valid)
            num = atof(temp);
        else
            printf("Invalid input! Enter valid price.\n");
    } while (!valid);
    return num;
}


int isUniqueID(struct Product *p, int n, int id) {
    for (int i = 0; i < n; i++) {
        if (p[i].id == id)
            return 0;
    }
    return 1;
}


void addProduct(struct Product **p, int *n) {
    struct Product newp;

    do {
        newp.id = getIntInput("Product ID: ");
        if (!isUniqueID(*p, *n, newp.id))
            printf("Product ID already exists! Try again.\n");
    } while (!isUniqueID(*p, *n, newp.id));

    printf("Product Name: ");
    scanf("%s", newp.name);
    newp.price = getFloatInput("Product Price: ");
    newp.qty = getIntInput("Product Quantity: ");

    *p = realloc(*p, (*n + 1) * sizeof(struct Product));
    (*p)[*n] = newp;
    (*n)++;
    printf("Product added successfully!\n");
}

void viewProducts(struct Product *p, int n) {
    if (n == 0) {
        printf("No products available.\n");
        return;
    }
    printf("\n========= PRODUCT LIST =========\n");
    for (int i = 0; i < n; i++)
        printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
               p[i].id, p[i].name, p[i].price, p[i].qty);
}


void updateQuantity(struct Product *p, int n) {
    int id = getIntInput("Enter Product ID to update quantity: ");
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (p[i].id == id) {
            p[i].qty = getIntInput("Enter new Quantity: ");
            printf("Quantity updated!\n");
            found = 1;
            break;
        }
    }
    if (!found)
        printf("Product not found.\n");
}


void searchByID(struct Product *p, int n) {
    int id = getIntInput("Enter Product ID to search: ");
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (p[i].id == id) {
            printf("Product Found: %d | %s | %.2f | %d\n",
                   p[i].id, p[i].name, p[i].price, p[i].qty);
            found = 1;
            break;
        }
    }
    if (!found)
        printf("Product not found.\n");
}


void searchByName(struct Product *p, int n) {
    char part[50];
    int found = 0;
    printf("Enter name to search (partial allowed): ");
    scanf("%s", part);
    for (int i = 0; i < n; i++) {
        if (strstr(p[i].name, part)) {
            printf("Product: %d | %s | %.2f | %d\n",
                   p[i].id, p[i].name, p[i].price, p[i].qty);
            found = 1;
        }
    }
    if (!found)
        printf("No matching product.\n");
}


void searchByPrice(struct Product *p, int n) {
    float min = getFloatInput("Enter minimum price: ");
    float max = getFloatInput("Enter maximum price: ");
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (p[i].price >= min && p[i].price <= max) {
            printf("Product: %d | %s | %.2f | %d\n",
                   p[i].id, p[i].name, p[i].price, p[i].qty);
            found = 1;
        }
    }
    if (!found)
        printf("No product in this range.\n");
}


void deleteProduct(struct Product **p, int *n) {
    int id = getIntInput("Enter Product ID to delete: ");
    int found = 0;
    for (int i = 0; i < *n; i++) {
        if ((*p)[i].id == id) {
            for (int j = i; j < *n - 1; j++)
                (*p)[j] = (*p)[j + 1];
            (*n)--;
            *p = realloc(*p, (*n) * sizeof(struct Product));
            printf("Product deleted successfully!\n");
            found = 1;
            break;
        }
    }
    if (!found)
        printf("Product not found.\n");
}


int main() {
    struct Product *items = NULL;
    int n, choice;

    n = getIntInput("Enter initial number of products: ");
    items = (struct Product*)calloc(n, sizeof(struct Product));
    if (items == NULL) {
        printf("Memory not allocated!\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("\nEnter details for product %d:\n", i + 1);
        do {
            items[i].id = getIntInput("Product ID: ");
            if (!isUniqueID(items, i, items[i].id))
                printf("Product ID already exists! Try again.\n");
        } while (!isUniqueID(items, i, items[i].id));

        printf("Product Name: ");
        scanf("%s", items[i].name);
        items[i].price = getFloatInput("Product Price: ");
        items[i].qty = getIntInput("Product Quantity: ");
    }

    do {
        printf("\n========= INVENTORY MENU =========\n");
        printf("1. Add New Product\n2. View All Products\n3. Update Quantity\n");
        printf("4. Search Product by ID\n5. Search Product by Name\n");
        printf("6. Search Product by Price Range\n7. Delete Product\n8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addProduct(&items, &n); break;
            case 2: viewProducts(items, n); break;
            case 3: updateQuantity(items, n); break;
            case 4: searchByID(items, n); break;
            case 5: searchByName(items, n); break;
            case 6: searchByPrice(items, n); break;
            case 7: deleteProduct(&items, &n); break;
            case 8:
                free(items);
                printf("Memory released. Exiting...\n");
                return 0;
            default: printf("Invalid choice.Try again.\n");
        }

    } while (1);
}
