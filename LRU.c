#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASH_BUCKET_COUNT 2027  
#define MAX_LINE_LENGTH 1024

typedef struct Node {
    int key;
    char *value;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct HashEntry {
    int key;
    Node *node;
    struct HashEntry *next;
} HashEntry;

typedef struct LRUCache {
    int capacity;
    int size;
    Node *head;            
    Node *tail;            
    HashEntry *buckets[HASH_BUCKET_COUNT];
} LRUCache;

static char *strdup_safe(const char *src) {
    if (!src) return NULL;
    size_t n = strlen(src) + 1;
    char *dst = malloc(n);
    if (!dst) {
      perror("malloc");
      exit(EXIT_FAILURE); }
    memcpy(dst, src, n);
    return dst;
}


static unsigned int hash_index(int key) {
    unsigned int x = (unsigned int) key;
    x = (x ^ (x >> 16)) * 0x45d9f3b;
    x = (x ^ (x >> 16)) * 0x45d9f3b;
    x = x ^ (x >> 16);
    return x % HASH_BUCKET_COUNT;
}

static void trim_newline(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}


static char *next_token(char **cursor) {
    if (!cursor || !*cursor) return NULL;
    char *p = *cursor;
    while (*p && isspace((unsigned char)*p))
      p++;
    if (!*p) { 
      *cursor = p;
      return NULL;
    }
    char *start = p;
    while (*p && !isspace((unsigned char)*p)) 
      p++;
    if (*p) { 
      *p = '\0'; p++; 
    }
    *cursor = p;
    return start;
}

static int parse_int_strict(const char *s, int *out) {
    if (!s || *s == '\0') return 0;
    const char *p = s;
    if (*p == '-') p++;
    if (*p == '\0') return 0;
    while (*p) {
        if (!isdigit((unsigned char)*p)) return 0;
        p++;
    }
    long v = strtol(s, NULL, 10);
    *out = (int)v;
    return 1;
}

static Node *node_create(int key, const char *value) {
    Node *n = (Node*)malloc(sizeof(Node));
    if (!n) {
      perror("malloc");
      exit(EXIT_FAILURE); 
    }
    n->key = key;
    n->value = strdup_safe(value);
    n->prev = n->next = NULL;
    return n;
}

static void node_free(Node *n) {
    if (!n) return;
    free(n->value);
    free(n);
}


static void detach_node(LRUCache *cache, Node *n) {
    if (!cache || !n) return;
    if (n->prev) n->prev->next = n->next;
    else cache->head = n->next;  
    if (n->next) n->next->prev = n->prev;
    else cache->tail = n->prev; 
    n->prev = n->next = NULL;
}
static void insert_at_head(LRUCache *cache, Node *n) {
    if (!cache || !n) return;
    n->prev = NULL;
    n->next = cache->head;
    if (cache->head)
      cache->head->prev = n;
    cache->head = n;
    if (!cache->tail)
      cache->tail = n;
}

static void move_to_head(LRUCache *cache, Node *n) {
    if (!cache || !n || cache->head == n) return;
    detach_node(cache, n);
    insert_at_head(cache, n);
}

static Node *remove_tail(LRUCache *cache) {
    if (!cache || !cache->tail)
      return NULL;
    Node *t = cache->tail;
    if (t->prev) {
        cache->tail = t->prev;
        cache->tail->next = NULL;
    } else {
        cache->head = cache->tail = NULL;
    }
    t->prev = t->next = NULL;
    return t;
}

static HashEntry *hash_entry_create(int key, Node *node) {
    HashEntry *e = (HashEntry*)malloc(sizeof(HashEntry));
    if (!e) { 
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    e->key = key;
    e->node = node;
    e->next = NULL;
    return e;
}

static void hash_insert(LRUCache *cache, int key, Node *node) {
    unsigned int idx = hash_index(key);
    HashEntry *it = cache->buckets[idx];
    while (it) {
        if (it->key == key) {
            it->node = node;
            return;
        }
        it = it->next;
    }
    HashEntry *newEntry = hash_entry_create(key, node);
    newEntry->next = cache->buckets[idx];
    cache->buckets[idx] = newEntry;
}
static Node *hash_lookup(LRUCache *cache, int key) {
    unsigned int idx = hash_index(key);
    HashEntry *it = cache->buckets[idx];
    while (it) {
        if (it->key == key) return it->node;
        it = it->next;
    }
    return NULL;
}
static void hash_remove(LRUCache *cache, int key) {
    unsigned int idx = hash_index(key);
    HashEntry *it = cache->buckets[idx];
    HashEntry *prev = NULL;
    while (it) {
        if (it->key == key) {
            if (prev) prev->next = it->next;
            else cache->buckets[idx] = it->next;
            free(it);
            return;
        }
        prev = it;
        it = it->next;
    }
}
static void hash_clear(LRUCache *cache) {
    for (int i = 0; i < HASH_BUCKET_COUNT; ++i) {
        HashEntry *it = cache->buckets[i];
        while (it) {
            HashEntry *nx = it->next;
            free(it);
            it = nx;
        }
        cache->buckets[i] = NULL;
    }
}

static LRUCache *create_cache(int capacity) {
    if (capacity <= 0 || capacity > 1000) {
        fprintf(stderr, "Invalid capacity: %d (must be 1..1000)\n", capacity);
        return NULL;
    }
    LRUCache *c = (LRUCache*)malloc(sizeof(LRUCache));
    if (!c) { perror("malloc"); exit(EXIT_FAILURE); }
    c->capacity = capacity;
    c->size = 0;
    c->head = c->tail = NULL;
    for (int i = 0; i < HASH_BUCKET_COUNT; ++i) c->buckets[i] = NULL;
    return c;
}

static void destroy_cache(LRUCache *cache) {
    if (!cache) return;
    
    Node *cur = cache->head;
    while (cur) {
        Node *nx = cur->next;
        node_free(cur);
        cur = nx;
    }
    hash_clear(cache);
    free(cache);
}
static void ensure_capacity(LRUCache *cache) {
    while (cache->size > cache->capacity) {
        Node *ev = remove_tail(cache);
        if (!ev) break;
        hash_remove(cache, ev->key);
        node_free(ev);
        cache->size--;
    }
}
static void cache_put(LRUCache *cache, int key, const char *value) {
    if (!cache) return;
    Node *node = hash_lookup(cache, key);
    if (node) {
        free(node->value);
        node->value = strdup_safe(value);
        move_to_head(cache, node);
        return;
    }
    Node *newNode = node_create(key, value);
    insert_at_head(cache, newNode);
    hash_insert(cache, key, newNode);
    cache->size++;
    if (cache->size > cache->capacity) {
        Node *ev = remove_tail(cache);
        if (ev) {
            hash_remove(cache, ev->key);
            node_free(ev);
            cache->size--;
        }
    }
}

static const char *cache_get(LRUCache *cache, int key) {
    if (!cache) return NULL;
    Node *node = hash_lookup(cache, key);
    if (!node) return NULL;
    move_to_head(cache, node);
    return node->value;
}

static void printSupportedCommands(void) {
    printf("Supported Commands:\n\n");
    printf("1. Create Cache\n");
    printf("   createCache <size>\n");
    printf("   Example: createCache 5\n\n");
    printf("2. Insert or Update\n");
    printf("   put <key> <value>\n");
    printf("   Example: put 101 token_A12\n\n");
    printf("3. Retrieve Value\n");
    printf("   get <key>\n");
    printf("   Example: get 101\n\n");
    printf("4. Exit Program\n");
    printf("   exit\n");
}


int main(void) {
    char line[MAX_LINE_LENGTH];
    LRUCache *cache = NULL;
    printSupportedCommands();
    printf("\n"); 

    while (1) {
        if (!fgets(line, sizeof(line), stdin)) break;
        trim_newline(line);
        char *cursor = line;
        while (*cursor && isspace((unsigned char)*cursor)) 
          cursor++;
        if (*cursor == '\0') 
          continue;
        char *tokenCursor = cursor;
        char *command = next_token(&tokenCursor);
        if (!command) continue;

        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "createCache") == 0) {
            char *sizeTok = next_token(&tokenCursor);
            if (!sizeTok) {
                fprintf(stderr, "Error: createCache requires a size\n");
                continue;
            }
            int cap;
            if (!parse_int_strict(sizeTok, &cap) || cap <= 0 || cap > 1000) {
                fprintf(stderr, "Error: invalid capacity (1..1000 expected)\n");
                continue;
            }
            if (cache) { destroy_cache(cache); cache = NULL; }
            cache = create_cache(cap);
        } else if (strcmp(command, "put") == 0) {
            char *keyTok = next_token(&tokenCursor);
            char *valTok = next_token(&tokenCursor);
            if (!keyTok || !valTok) {
                fprintf(stderr, "Error: put requires <key> <value>\n");
                continue;
            }
            if (!cache) {
                fprintf(stderr, "Error: cache not created. Use createCache <size>\n");
                continue;
            }
            int key;
            if (!parse_int_strict(keyTok, &key)) {
                fprintf(stderr, "Error: invalid key (integer expected)\n");
                continue;
            }
            cache_put(cache, key, valTok);
        } else if (strcmp(command, "get") == 0) {
            char *keyTok = next_token(&tokenCursor);
            if (!keyTok) {
                fprintf(stderr, "Error: get requires <key>\n");
                continue;
            }
            if (!cache) {
                fprintf(stderr, "Error: cache not created. Use createCache <size>\n");
                continue;
            }
            int key;
            if (!parse_int_strict(keyTok, &key)) {
                fprintf(stderr, "Error: invalid key (integer expected)\n");
                continue;
            }
            const char *val = cache_get(cache, key);
            if (val) printf("%s\n", val);
            else printf("NULL\n");
        } else {
            fprintf(stderr, "Unknown command: %s\n", command);
        }
    }

    destroy_cache(cache);
    return 0;
}
