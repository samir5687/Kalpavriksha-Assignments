#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define MAX_NAME 50
#define LINE 1024

typedef struct FreeBlock {
    int idx;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct Node {
    char name[MAX_NAME+1];
    int is_dir;
    struct Node *parent;
    struct Node *child;
    struct Node *next;
    struct Node *prev;
    int *blocks;
    int blocks_count;
    int size_bytes;
} Node;

static unsigned char disk[NUM_BLOCKS][BLOCK_SIZE];
static FreeBlock *free_head = NULL;
static FreeBlock *free_tail = NULL;
static Node *root = NULL;
static Node *cwd = NULL;


static void free_push_tail(int i) {
    FreeBlock *n = malloc(sizeof(FreeBlock));
    if (!n) { puts("Memory error"); exit(1); }
    n->idx = i;
    n->next = NULL;
    n->prev = NULL;
    if (!free_tail) {
        free_head = n;
        free_tail = n;
        return;
    }
    free_tail->next = n;
    n->prev = free_tail;
    free_tail = n;
}
static int free_pop_head(void) {
    if (!free_head)
    return -1;
    FreeBlock *n = free_head;
    int idx = n->idx;
    free_head = n->next;
    if (free_head) 
    free_head->prev = NULL;
    else 
    free_tail = NULL;
    free(n);
    return idx;
}
static int count_free(void) {
    int c = 0;
    FreeBlock *t = free_head;
    while (t){
        c++; t = t->next;
        }
    return c;
}


static Node* find_child(Node *d, const char *name) {
    if (!d || !d->is_dir || !d->child) 
    return NULL;
    Node *it = d->child;
    do {
        if (strcmp(it->name, name) == 0) 
        return it;
        it = it->next;
    } while (it != d->child);
    return NULL;
}
static void insert_child(Node *d, Node *n) {
    n->parent = d;
    n->child = NULL;
    n->next = n;
    n->prev = n;
    n->blocks = NULL;
    n->blocks_count = 0;
    n->size_bytes = 0;
    if (!d->child) {
        d->child = n;
        return;
    }
    Node *h = d->child;
    Node *t = h->prev;
    t->next = n;
    n->prev = t;
    n->next = h;
    h->prev = n;
}
static void unlink_node(Node *n) {
    if (!n || !n->parent) 
    return;
    Node *p = n->parent;
    if (p->child == n) {
        if (n->next == n) p->child = NULL;
        else
        p->child = n->next;
    }
    n->prev->next = n->next;
    n->next->prev = n->prev;
    n->next = n;
    n->prev = n;
    n->parent = NULL;
}

static void cmd_mkdir(char *name) {
    if (!name || !*name) {
        puts("Name required");
        return;
        }
    if (find_child(cwd, name)) {
        printf("'%s' exists\n", name); 
        return; 
        
    }
    Node *n = malloc(sizeof(Node));
    if (!n) {
        puts("Memory error"); 
        exit(1);
        }
    memset(n, 0, sizeof(Node));
    strncpy(n->name, name, MAX_NAME);
    n->is_dir = 1;
    insert_child(cwd, n);
    printf("Directory '%s' created\n", name);
}
static void cmd_create(char *name) {
    if (!name || !*name) {
        puts("Name required");
        return;
        }
    if (find_child(cwd, name)) { 
        printf("'%s' exists\n", name); 
        return; 
        
    }
    Node *n = malloc(sizeof(Node));
    if (!n) { 
        puts("Memory error"); 
        exit(1);
        }
    memset(n, 0, sizeof(Node));
    strncpy(n->name, name, MAX_NAME);
    n->is_dir = 0;
    insert_child(cwd, n);
    printf("File '%s' created\n", name);
}
static void cmd_write(char *name, char *content) {
    if (!name || !*name) {
        printf("Usage: write <file> \"text\"\n");
        return; }
    Node *f = find_child(cwd, name);
    if (!f) { 
        printf("File '%s' not found\n", name);
        return;
        }
    if (f->is_dir) { 
        printf("'%s' is directory\n", name); 
        return; 
        
    }
    if (f->blocks && f->blocks_count > 0) {
        for (int i = 0; i < f->blocks_count; i++) 
        free_push_tail(f->blocks[i]);
        free(f->blocks);
        f->blocks = NULL;
        f->blocks_count = 0;
        f->size_bytes = 0;
    }
    int len = (int)strlen(content);
    int need = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (need == 0) 
    need = 1;
    if (count_free() < need) {
        puts("Disk full. Write failed"); 
        return;
        }
    f->blocks = malloc(sizeof(int) * need);
    for (int i = 0; i < need; i++) {
        int idx = free_pop_head();
        f->blocks[i] = idx;
        int off = i * BLOCK_SIZE;
        int towrite = BLOCK_SIZE;
        if (off + towrite > len) 
        towrite = len - off;
        if (towrite > 0) 
        memcpy(disk[idx], content + off, towrite);
        else disk[idx][0] = '\0';
    }
    f->blocks_count = need;
    f->size_bytes = len;
    printf("Wrote %d bytes in %d blocks\n", len, need);
}
static void cmd_read(char *name) {
    if (!name || !*name) { 
        printf("Usage: read <file>\n");
        return; 
        
    }
    Node *f = find_child(cwd, name);
    if (!f) {
        printf("File '%s' not found\n", name); 
        return;
        }
    if (f->is_dir){
    printf("'%s' is directory\n", name);
    return;
    }
    if (!f->blocks || f->blocks_count == 0) {
        puts("(empty)");
        return;
        }
    int rem = f->size_bytes;
    for (int i = 0; i < f->blocks_count; i++) {
        int toread = rem < BLOCK_SIZE ? rem : BLOCK_SIZE;
        if (toread > 0) 
        fwrite(disk[f->blocks[i]], 1, toread, stdout);
        rem -= toread;
    }
    puts("");
}
static void cmd_delete(char *name) {
    if (!name || !*name) {
        puts("Usage: delete <file>");
        return; 
        
    }
    Node *f = find_child(cwd, name);
    if (!f) { 
        printf("File '%s' not found\n", name);
        return;
        }
    if (f->is_dir) {
        printf("'%s' is directory\n", name);
        return; 
        
    }
    if (f->blocks) {
        for (int i = 0; i < f->blocks_count; i++) free_push_tail(f->blocks[i]);
        free(f->blocks);
    }
    unlink_node(f);
    free(f);
    puts("File deleted");
}
static void cmd_rmdir(char *name) {
    if (!name || !*name) {
        puts("Usage: rmdir <dir>");
        return; 
        
    }
    Node *d = find_child(cwd, name);
    if (!d) { 
        printf("Directory '%s' not found\n", name);
        return;
        }
    if (!d->is_dir) { 
        printf("'%s' is not directory\n", name); 
        return;
        }
    if (d->child) {
        puts("Directory not empty"); 
        return;
        }
    unlink_node(d);
    free(d);
    puts("Directory removed");
}
static void cmd_ls(void) {
    if (!cwd->child) {
        puts("(empty)");
        return; 
        
    }
    Node *it = cwd->child;
    do {
        if (it->is_dir) printf("%s/\n", it->name);
        else printf("%s\n", it->name);
        it = it->next;
    } while (it != cwd->child);
}
static void cmd_cd(char *arg) {
    if (!arg || !*arg) {
        puts("Usage: cd <dir>");
        return; 
        
    }
    if (strcmp(arg, "/") == 0) { cwd = root; puts("Moved to /"); return; }
    if (strcmp(arg, "..") == 0) {
        if (cwd->parent) {
            cwd = cwd->parent;
            Node *tmp = cwd;
            char parts[100][MAX_NAME+1];
            int pc = 0;
            while (tmp && tmp->parent) {
                strncpy(parts[pc], tmp->name, MAX_NAME);
                pc++;
                tmp = tmp->parent;
            }
            if (pc == 0) puts("/");
            else {
                printf("/");
                for (int i = pc - 1; i >= 0; i--) {
                    printf("%s", parts[i]);
                    if (i > 0) printf("/");
                }
                puts("");
            }
        } else puts("Already at root");
        return;
    }
    Node *t = find_child(cwd, arg);
    if (!t) { 
        printf("No such dir '%s'\n", arg); 
    return;
    }
    if (!t->is_dir) { printf("'%s' is not dir\n", arg); return; }
    cwd = t;
    Node *tmp = cwd;
    char parts[100][MAX_NAME+1];
    int pc = 0;
    while (tmp && tmp->parent) {
        strncpy(parts[pc], tmp->name, MAX_NAME);
        pc++;
        tmp = tmp->parent;
    }
    if (pc == 0) puts("/");
    else {
        printf("/");
        for (int i = pc - 1; i >= 0; i--) {
            printf("%s", parts[i]);
            if (i > 0) printf("/");
        }
        puts("");
    }
}
static void cmd_pwd(void) {
    Node *tmp = cwd;
    char parts[100][MAX_NAME+1];
    int pc = 0;
    while (tmp && tmp->parent) {
        strncpy(parts[pc], tmp->name, MAX_NAME);
        pc++;
        tmp = tmp->parent;
    }
    if (pc == 0) puts("/");
    else {
        printf("/");
        for (int i = pc - 1; i >= 0; i--) {
            printf("%s", parts[i]);
            if (i > 0) printf("/");
        }
        puts("");
    }
}
static void cmd_df(void) {
    int freec = count_free();
    int used = NUM_BLOCKS - freec;
    double pct = (double) used / (double) NUM_BLOCKS * 100.0;
    printf("Total Blocks: %d\nUsed Blocks: %d\nFree Blocks: %d\nDisk Usage: %.2f%%\n",
           NUM_BLOCKS, used, freec, pct);
}


static void init_vfs(void) {
    free_head = NULL;
    free_tail = NULL;
    for (int i = 0; i < NUM_BLOCKS; i++) free_push_tail(i);
    root = malloc(sizeof(Node));
    if (!root) { puts("Memory error"); exit(1); }
    memset(root, 0, sizeof(Node));
    strcpy(root->name, "/");
    root->is_dir = 1;
    root->parent = NULL;
    root->child = NULL;
    root->next = root;
    root->prev = root;
    cwd = root;
}
static void cleanup_vfs(void) {
    if (root->child) {
        Node *it = root->child;
        Node **arr = NULL;
        int cnt = 0;
        do {
            arr = realloc(arr, (cnt + 1) * sizeof(Node *));
            arr[cnt++] = it;
            it = it->next;
        } while (it != root->child);
        for (int i = 0; i < cnt; i++) {
            Node *n = arr[i];
            if (n->is_dir) {
                if (n->child) {
                    Node *s = n->child;
                    Node **sarr = NULL;
                    int sc = 0;
                    do {
                        sarr = realloc(sarr, (sc + 1) * sizeof(Node *));
                        sarr[sc++] = s;
                        s = s->next;
                    } while (s != n->child);
                    for (int j = 0; j < sc; j++) {
                        if (!sarr[j]->is_dir && sarr[j]->blocks) {
                            for (int b = 0; b < sarr[j]->blocks_count; b++) free_push_tail(sarr[j]->blocks[b]);
                            free(sarr[j]->blocks);
                        }
                        free(sarr[j]);
                    }
                    free(sarr);
                }
                free(n);
            } else {
                if (n->blocks) {
                    for (int b = 0; b < n->blocks_count; b++) free_push_tail(n->blocks[b]);
                    free(n->blocks);
                }
                free(n);
            }
        }
        free(arr);
    }
    free(root);
    FreeBlock *fb = free_head;
    while (fb) {
        FreeBlock *nx = fb->next;
        free(fb);
        fb = nx;
    }
    free_head = free_tail = NULL;
}


static void parse_line(char *line) {
    char *p = line;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
    if (!*p) return;
    char cmd[64];
    int i = 0;
    while (p[i] && p[i] != ' ' && p[i] != '\t' && p[i] != '\n') i++;
    strncpy(cmd, p, i);
    cmd[i] = '\0';
    char *args = p + i;
    while (*args == ' ' || *args == '\t') args++;
    if (strcmp(cmd, "mkdir") == 0) {
        char name[MAX_NAME+1];
        if (sscanf(args, "%50s", name) == 1) cmd_mkdir(name);
        else puts("mkdir <name>");
        return;
    }
    if (strcmp(cmd, "create") == 0) {
        char name[MAX_NAME+1];
        if (sscanf(args, "%50s", name) == 1) cmd_create(name);
        else puts("create <name>");
        return;
    }
    if (strcmp(cmd, "write") == 0) {
        char fname[MAX_NAME+1];
        int pos = 0;
        while (args[pos] && args[pos] != ' ' && args[pos] != '\t') pos++;
        if (pos == 0) { puts("write <file> \"text\""); return; }
        strncpy(fname, args, pos);
        fname[pos] = '\0';
        char *rest = args + pos;
        while (*rest == ' ' || *rest == '\t') rest++;
        if (*rest == '"') {
            rest++;
            char *end = strchr(rest, '"');
            if (!end) { puts("Missing closing quote"); return; }
            char content[LINE];
            int clen = (int)(end - rest);
            if (clen >= LINE) clen = LINE - 1;
            strncpy(content, rest, clen);
            content[clen] = '\0';
            cmd_write(fname, content);
        } else {
            puts("write <file> \"text\"");
        }
        return;
    }
    if (strcmp(cmd, "read") == 0) {
        char name[MAX_NAME+1];
        if (sscanf(args, "%50s", name) == 1) cmd_read(name);
        else puts("read <file>");
        return;
    }
    if (strcmp(cmd, "delete") == 0) {
        char name[MAX_NAME+1];
        if (sscanf(args, "%50s", name) == 1) cmd_delete(name);
        else puts("delete <file>");
        return;
    }
    if (strcmp(cmd, "rmdir") == 0) {
        char name[MAX_NAME+1];
        if (sscanf(args, "%50s", name) == 1) cmd_rmdir(name);
        else puts("rmdir <dir>");
        return;
    }
    if (strcmp(cmd, "ls") == 0) { cmd_ls(); return; }
    if (strcmp(cmd, "cd") == 0) {
        char arg[MAX_NAME+1];
        if (sscanf(args, "%50s", arg) == 1) cmd_cd(arg);
        else puts("cd <dir>");
        return;
    }
    if (strcmp(cmd, "pwd") == 0) { cmd_pwd(); return; }
    if (strcmp(cmd, "df") == 0) { cmd_df(); return; }
    if (strcmp(cmd, "exit") == 0) { cleanup_vfs(); puts("Exiting..."); exit(0); }
    printf("Unknown: %s\n", cmd);
}


int main(void) {
    init_vfs();
    puts("Short VFS ready. Type 'exit' to quit.");
    char line[LINE];
    while (1) {
        Node *t = cwd;
        char parts[100][MAX_NAME+1];
        int pc = 0;
        while (t && t->parent) {
            strncpy(parts[pc], t->name, MAX_NAME);
            pc++;
            t = t->parent;
        }
        if (pc == 0) printf("/ > ");
        else {
            printf("/");
            for (int i = pc - 1; i >= 0; i--) {
                printf("%s", parts[i]);
                if (i > 0) printf("/");
            }
            printf(" > ");
        }
        if (!fgets(line, sizeof(line), stdin)) break;
        parse_line(line);
    }
    cleanup_vfs();
    return 0;
}
