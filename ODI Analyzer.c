#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "players_data.h" 


typedef enum { ROLE_UNKNOWN = 0, ROLE_BATSMAN = 1, ROLE_BOWLER = 2, ROLE_ALLROUNDER = 3 } RoleType;

typedef struct PlayerNode {
    int playerId;
    char name[64];
    char teamName[64];
    RoleType role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    double performanceIndex;
    struct PlayerNode* next;
} PlayerNode;

typedef struct Team {
    int teamId;
    char name[64];
    int totalPlayers;
    double averageBattingStrikeRate;
    PlayerNode* allPlayersHead;
    PlayerNode* batsmenHead;
    PlayerNode* bowlersHead;
    PlayerNode* allroundersHead;
} Team;

static Team teamsGlobal[16];
static int totalTeams = 0;



static void readLine(char* buf, size_t sz) {
    if (!fgets(buf, (int)sz, stdin)) { buf[0] = '\0'; return; }
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
}


static int parseIntStrict(const char* s, int* out) {
    if (!s || *s == '\0') return 0;
    char* endptr = NULL;
    long v = strtol(s, &endptr, 10);
    if (endptr == s) return 0;
    while (*endptr) { if (!isspace((unsigned char)*endptr)) return 0; endptr++; }
    *out = (int)v;
    return 1;
}


static int parseDoubleStrict(const char* s, double* out) {
    if (!s || *s == '\0') return 0;
    char* endptr = NULL;
    double v = strtod(s, &endptr);
    if (endptr == s) return 0;
    while (*endptr) { if (!isspace((unsigned char)*endptr)) return 0; endptr++; }
    *out = v;
    return 1;
}

static int promptInt(const char* prompt, int minAllowed, int enforceMin) {
    char buf[128]; int v;
    while (1) {
        printf("%s", prompt);
        readLine(buf, sizeof(buf));
        if (!parseIntStrict(buf, &v)) { printf("  Invalid input. Enter a whole number.\n"); continue; }
        if (enforceMin && v < minAllowed) { printf("  Invalid input. Enter a value >= %d.\n", minAllowed); continue; }
        return v;
    }
}

static double promptDouble(const char* prompt, double minAllowed, int enforceMin) {
    char buf[128]; double v;
    while (1) {
        printf("%s", prompt);
        readLine(buf, sizeof(buf));
        if (!parseDoubleStrict(buf, &v)) { printf("  Invalid input. Enter a numeric value (use '.' for decimals).\n"); continue; }
        if (enforceMin && v < minAllowed) { printf("  Invalid input. Enter a value >= %.2f.\n", minAllowed); continue; }
        return v;
    }
}

static void promptString(const char* prompt, char* outBuf, size_t maxLen) {
    printf("%s", prompt);
    readLine(outBuf, maxLen);
    char* s = outBuf; while (*s && isspace((unsigned char)*s)) s++;
    if (s != outBuf) memmove(outBuf, s, strlen(s)+1);
    size_t l = strlen(outBuf); while (l > 0 && isspace((unsigned char)outBuf[l-1])) { outBuf[l-1] = '\0'; l--; }
    if (strlen(outBuf) == 0) strcpy(outBuf, "Unknown");
}
static int ci_cmp(const char* a, const char* b) {
    while (*a && *b) {
        unsigned char ca = (unsigned char) tolower((unsigned char)*a);
        unsigned char cb = (unsigned char) tolower((unsigned char)*b);
        if (ca != cb) return (int)ca - (int)cb;
        a++; b++;
    }
    return (int) tolower((unsigned char)*a) - (int) tolower((unsigned char)*b);
}

static double computePerformanceIndex(RoleType role, double battingAverage, double strikeRate, int wickets, double economyRate) {
    if (role == ROLE_BATSMAN) return (battingAverage * strikeRate) / 100.0;
    if (role == ROLE_BOWLER)  return (wickets * 2.0) + (100.0 - economyRate);
    if (role == ROLE_ALLROUNDER) return ((battingAverage * strikeRate) / 100.0) + (wickets * 2.0);
    return 0.0;
}
static PlayerNode* createPlayerNode(int playerId, const char* name, const char* teamName, RoleType role,
                                    int totalRuns, float battingAverage, float strikeRate, int wickets, float economyRate) {
    PlayerNode* n = (PlayerNode*)malloc(sizeof(PlayerNode));
    if (!n) { fprintf(stderr, "Memory allocation error\n"); exit(EXIT_FAILURE); }
    n->playerId = playerId;
    strncpy(n->name, name, sizeof(n->name)-1); n->name[sizeof(n->name)-1] = '\0';
    strncpy(n->teamName, teamName, sizeof(n->teamName)-1); n->teamName[sizeof(n->teamName)-1] = '\0';
    n->role = role;
    n->totalRuns = totalRuns;
    n->battingAverage = battingAverage;
    n->strikeRate = strikeRate;
    n->wickets = wickets;
    n->economyRate = economyRate;
    n->performanceIndex = computePerformanceIndex(role, battingAverage, strikeRate, wickets, economyRate);
    n->next = NULL;
    return n;
}

static void insertAllPlayersHead(Team* t, PlayerNode* node) {
    node->next = t->allPlayersHead;
    t->allPlayersHead = node;
    t->totalPlayers++;
}

static void insertSortedByPerf(PlayerNode** headRef, PlayerNode* node) {
    PlayerNode* cur = *headRef; PlayerNode* prev = NULL;
    node->next = NULL;
    while (cur && cur->performanceIndex >= node->performanceIndex) { prev = cur; cur = cur->next; }
    if (!prev) { node->next = *headRef; *headRef = node; }
    else { node->next = prev->next; prev->next = node; }
}

static void insertPlayerIntoTeam(Team* team, PlayerNode* raw) {
    PlayerNode* full = createPlayerNode(raw->playerId, raw->name, raw->teamName, raw->role,
                                        raw->totalRuns, raw->battingAverage, raw->strikeRate, raw->wickets, raw->economyRate);
    insertAllPlayersHead(team, full);

    PlayerNode* roleCopy = createPlayerNode(raw->playerId, raw->name, raw->teamName, raw->role,
                                            raw->totalRuns, raw->battingAverage, raw->strikeRate, raw->wickets, raw->economyRate);
    if (raw->role == ROLE_BATSMAN) insertSortedByPerf(&team->batsmenHead, roleCopy);
    else if (raw->role == ROLE_BOWLER) insertSortedByPerf(&team->bowlersHead, roleCopy);
    else if (raw->role == ROLE_ALLROUNDER) insertSortedByPerf(&team->allroundersHead, roleCopy);
}

static void recomputeTeamAvgSR(Team* t) {
    double sum = 0.0; int count = 0;
    for (PlayerNode* p = t->allPlayersHead; p; p = p->next) {
        if (p->role == ROLE_BATSMAN || p->role == ROLE_ALLROUNDER) { sum += p->strikeRate; count++; }
    }
    t->averageBattingStrikeRate = (count > 0) ? (sum / count) : 0.0;
}

static int binarySearchTeamById(int id) {
    int lo = 0, hi = totalTeams - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (teamsGlobal[mid].teamId == id) return mid;
        if (teamsGlobal[mid].teamId < id) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

static void initializeFromHeader(void) {
    totalTeams = teamCount;
    for (int i = 0; i < totalTeams; ++i) {
        teamsGlobal[i].teamId = i + 1;
        strncpy(teamsGlobal[i].name, teams[i], sizeof(teamsGlobal[i].name)-1);
        teamsGlobal[i].name[sizeof(teamsGlobal[i].name)-1] = '\0';
        teamsGlobal[i].totalPlayers = 0;
        teamsGlobal[i].averageBattingStrikeRate = 0.0;
        teamsGlobal[i].allPlayersHead = NULL;
        teamsGlobal[i].batsmenHead = NULL;
        teamsGlobal[i].bowlersHead = NULL;
        teamsGlobal[i].allroundersHead = NULL;
    }

    for (int i = 0; i < playerCount; ++i) {
        const Player* ph = &players[i];
        RoleType r = ROLE_UNKNOWN;
        if (ci_cmp(ph->role, "Batsman") == 0) r = ROLE_BATSMAN;
        else if (ci_cmp(ph->role, "Bowler") == 0) r = ROLE_BOWLER;
        else if (ci_cmp(ph->role, "All-rounder") == 0 || ci_cmp(ph->role, "Allrounder") == 0) r = ROLE_ALLROUNDER;

        PlayerNode tmp;
        tmp.playerId = ph->id;
        strncpy(tmp.name, ph->name, sizeof(tmp.name)-1); tmp.name[sizeof(tmp.name)-1] = '\0';
        strncpy(tmp.teamName, ph->team, sizeof(tmp.teamName)-1); tmp.teamName[sizeof(tmp.teamName)-1] = '\0';
        tmp.role = r;
        tmp.totalRuns = ph->totalRuns;
        tmp.battingAverage = ph->battingAverage;
        tmp.strikeRate = ph->strikeRate;
        tmp.wickets = ph->wickets;
        tmp.economyRate = ph->economyRate;
        tmp.performanceIndex = computePerformanceIndex(r, tmp.battingAverage, tmp.strikeRate, tmp.wickets, tmp.economyRate);
        tmp.next = NULL;

        int tidx = -1;
        for (int t = 0; t < totalTeams; ++t) {
            if (strcmp(teamsGlobal[t].name, ph->team) == 0) { tidx = t; break; }
        }
        if (tidx >= 0) insertPlayerIntoTeam(&teamsGlobal[tidx], &tmp);
    }

    for (int t = 0; t < totalTeams; ++t) recomputeTeamAvgSR(&teamsGlobal[t]);
}

static const char* roleToString(RoleType r) {
    if (r == ROLE_BATSMAN) return "Batsman";
    if (r == ROLE_BOWLER) return "Bowler";
    if (r == ROLE_ALLROUNDER) return "All-rounder";
    return "Unknown";
}

static void showTeamList(void) {
    printf("Available Teams (ID : Name):\n");
    for (int i = 0; i < totalTeams; ++i) printf("  %2d : %s\n", teamsGlobal[i].teamId, teamsGlobal[i].name);
}

static void displayPlayersOfTeam(int teamId) {
    int idx = binarySearchTeamById(teamId);
    if (idx == -1) { printf("Team ID %d not found.\n", teamId); return; }
    Team* t = &teamsGlobal[idx];
    printf("\nPlayers of Team %s:\n", t->name);
    printf("====================================================================================\n");
    printf("ID   Name                          Role         Runs   Avg    SR    Wkts  ER    PerfIdx\n");
    printf("====================================================================================\n");
    int count = 0; for (PlayerNode* p = t->allPlayersHead; p; p = p->next) count++;
    if (count == 0) { printf("  No players.\n"); return; }
    PlayerNode** arr = (PlayerNode**)malloc(sizeof(PlayerNode*) * count);
    int k = 0; for (PlayerNode* p = t->allPlayersHead; p; p = p->next) arr[k++] = p;
    for (int i = 0; i < count-1; ++i)
        for (int j = i+1; j < count; ++j)
            if (arr[i]->playerId > arr[j]->playerId) { PlayerNode* tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp; }
    for (int i = 0; i < count; ++i) {
        PlayerNode* p = arr[i];
        printf("%-4d %-30s %-12s %6d %6.1f %6.1f %6d %5.1f %9.2f\n",
               p->playerId, p->name, roleToString(p->role),
               p->totalRuns, p->battingAverage, p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
    }
    free(arr);
    printf("====================================================================================\n");
    printf("Total Players: %d\n", t->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n\n", t->averageBattingStrikeRate);
}

static int teamCompare(const void* a, const void* b) {
    const Team* ta = a; const Team* tb = b;
    if (ta->averageBattingStrikeRate < tb->averageBattingStrikeRate) return 1;
    if (ta->averageBattingStrikeRate > tb->averageBattingStrikeRate) return -1;
    return 0;
}
static void displayTeamsByAvgStrikeRate(void) {
    Team* tmp = (Team*)malloc(sizeof(Team) * totalTeams);
    if (!tmp) return;
    memcpy(tmp, teamsGlobal, sizeof(Team) * totalTeams);
    qsort(tmp, totalTeams, sizeof(Team), teamCompare);
    printf("\nTeams Sorted by Average Batting Strike Rate (Batsmen + All-rounders):\n");
    printf("==============================================================\n");
    printf("ID   Team Name             Avg Bat SR    Total Players\n");
    printf("==============================================================\n");
    for (int i = 0; i < totalTeams; ++i) printf("%-4d %-22s %-12.2f %-6d\n", tmp[i].teamId, tmp[i].name, tmp[i].averageBattingStrikeRate, tmp[i].totalPlayers);
    printf("==============================================================\n\n");
    free(tmp);
}

static void displayTopKPlayersOfTeamByRole(int teamId, RoleType role, int K) {
    int idx = binarySearchTeamById(teamId);
    if (idx == -1) { printf("Team ID %d not found.\n", teamId); return; }
    Team* t = &teamsGlobal[idx];
    PlayerNode* head = (role == ROLE_BATSMAN) ? t->batsmenHead : (role == ROLE_BOWLER ? t->bowlersHead : t->allroundersHead);
    printf("\nTop %d %s(s) of Team %s:\n", K, roleToString(role), t->name);
    printf("====================================================================================\n");
    printf("ID   Name                          Role         Runs   Avg    SR    Wkts  ER    PerfIdx\n");
    printf("====================================================================================\n");
    int printed = 0;
    for (PlayerNode* p = head; p && printed < K; p = p->next, ++printed) {
        printf("%-4d %-30s %-12s %6d %6.1f %6.1f %6d %5.1f %9.2f\n",
               p->playerId, p->name, roleToString(p->role),
               p->totalRuns, p->battingAverage, p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
    }
    if (printed == 0) printf("  No players of that role in the team.\n");
}

typedef struct { PlayerNode* node; int teamIndex; } HeapEntry;
typedef struct { HeapEntry* arr; int size; int cap; } MaxHeap;

static MaxHeap* heapCreate(int cap) {
    MaxHeap* h = (MaxHeap*)malloc(sizeof(MaxHeap));
    h->arr = (HeapEntry*)malloc(sizeof(HeapEntry) * cap);
    h->size = 0; h->cap = cap;
    return h;
}
static void heapPush(MaxHeap* h, HeapEntry e) {
    if (h->size >= h->cap) { h->cap *= 2; h->arr = (HeapEntry*)realloc(h->arr, sizeof(HeapEntry) * h->cap); }
    int i = h->size++; h->arr[i] = e;
    while (i > 0) {
        int p = (i-1)/2;
        if (h->arr[p].node->performanceIndex < h->arr[i].node->performanceIndex) { HeapEntry tmp = h->arr[p]; h->arr[p] = h->arr[i]; h->arr[i] = tmp; i = p; }
        else break;
    }
}
static HeapEntry heapPop(MaxHeap* h) {
    HeapEntry out = {NULL, -1};
    if (h->size == 0) return out;
    out = h->arr[0];
    h->arr[0] = h->arr[--h->size];
    int i = 0;
    while (1) {
        int l = 2*i + 1, r = 2*i + 2, largest = i;
        if (l < h->size && h->arr[l].node->performanceIndex > h->arr[largest].node->performanceIndex) largest = l;
        if (r < h->size && h->arr[r].node->performanceIndex > h->arr[largest].node->performanceIndex) largest = r;
        if (largest != i) { HeapEntry tmp = h->arr[i]; h->arr[i] = h->arr[largest]; h->arr[largest] = tmp; i = largest; }
        else break;
    }
    return out;
}
static void heapDestroy(MaxHeap* h) { if (!h) return; free(h->arr); free(h); }

static void displayAllPlayersAcrossTeamsByRole(RoleType role) {
    MaxHeap* heap = heapCreate(totalTeams > 0 ? totalTeams : 4);
    for (int t = 0; t < totalTeams; ++t) {
        PlayerNode* head = (role == ROLE_BATSMAN) ? teamsGlobal[t].batsmenHead : (role == ROLE_BOWLER ? teamsGlobal[t].bowlersHead : teamsGlobal[t].allroundersHead);
        if (head) heapPush(heap, (HeapEntry){ head, t });
    }
    printf("\nAll %s(s) across teams (by Performance Index):\n", roleToString(role));
    printf("================================================================================================================\n");
    printf("ID   Name                          Team               Role         Runs   Avg    SR    Wkts  ER    PerfIdx\n");
    printf("================================================================================================================\n");
    int any = 0;
    while (heap->size > 0) {
        HeapEntry e = heapPop(heap);
        PlayerNode* p = e.node;
        printf("%-4d %-30s %-18s %-12s %6d %6.1f %6.1f %6d %5.1f %9.2f\n",
               p->playerId, p->name, teamsGlobal[e.teamIndex].name, roleToString(p->role),
               p->totalRuns, p->battingAverage, p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
        any = 1;
        if (p->next) heapPush(heap, (HeapEntry){ p->next, e.teamIndex });
    }
    if (!any) printf("  No players found for this role across teams.\n");
    heapDestroy(heap);
}

static void addPlayerInteractive(void) {
    printf("\nAdd Player to Team\n");
    showTeamList();
    int teamId = promptInt("Enter Team ID to add player: ", 1, 1);
    int tidx = binarySearchTeamById(teamId);
    if (tidx == -1) { printf("Team ID %d not found.\n", teamId); return; }

    int playerId = promptInt("Player ID: ", 1, 1);
    char name[64]; promptString("Name: ", name, sizeof(name));

    int roleChoice;
    while (1) {
        roleChoice = promptInt("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 1);
        if (roleChoice >= 1 && roleChoice <= 3) break;
        printf("  Invalid role. Choose 1, 2 or 3.\n");
    }
    RoleType role = (roleChoice == 1 ? ROLE_BATSMAN : (roleChoice == 2 ? ROLE_BOWLER : ROLE_ALLROUNDER));

    int totalRuns = promptInt("Total Runs: ", 0, 1);
    double battingAvg = promptDouble("Batting Average: ", 0.0, 1);
    double strikeRate = promptDouble("Strike Rate: ", 0.0, 1);
    int wickets = promptInt("Wickets: ", 0, 1);
    double economy = promptDouble("Economy Rate: ", 0.0, 1);

    PlayerNode raw;
    raw.playerId = playerId;
    strncpy(raw.name, name, sizeof(raw.name)-1); raw.name[sizeof(raw.name)-1] = '\0';
    strncpy(raw.teamName, teamsGlobal[tidx].name, sizeof(raw.teamName)-1); raw.teamName[sizeof(raw.teamName)-1] = '\0';
    raw.role = role;
    raw.totalRuns = totalRuns;
    raw.battingAverage = (float)battingAvg;
    raw.strikeRate = (float)strikeRate;
    raw.wickets = wickets;
    raw.economyRate = (float)economy;
    raw.performanceIndex = computePerformanceIndex(role, raw.battingAverage, raw.strikeRate, raw.wickets, raw.economyRate);
    raw.next = NULL;

    insertPlayerIntoTeam(&teamsGlobal[tidx], &raw);
    recomputeTeamAvgSR(&teamsGlobal[tidx]);

    printf("Player added successfully to Team %s!\n", teamsGlobal[tidx].name);
}

static int promptMenuChoice(void) { return promptInt("Enter your choice: ", 1, 1); }

int main(void) {
    initializeFromHeader();

    while (1) {
        printf("\n==============================================================================\n");
        printf("ICC ODI Player Performance Analyzer\n");
        printf("==============================================================================\n");
        printf("1. Add Player to Team\n");
        printf("2. Display Players of a Specific Team\n");
        printf("3. Display Teams by Average Batting Strike Rate\n");
        printf("4. Display Top K Players of a Specific Team by Role\n");
        printf("5. Display all Players of specific role Across All Teams by performance index\n");
        printf("6. Exit\n");
        printf("==============================================================================\n");

        int choice = promptMenuChoice();

        if (choice == 1) addPlayerInteractive();
        else if (choice == 2) { 
          showTeamList();
          int tid = promptInt("Enter Team ID: ", 1, 1);
          displayPlayersOfTeam(tid); }
        else if (choice == 3) displayTeamsByAvgStrikeRate();
        else if (choice == 4) {
          showTeamList(); 
          int tid = promptInt("Enter Team ID: ", 1, 1); 
          int r = promptInt("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 1); 
          int k = promptInt("Enter number of players (K): ", 1, 1);
          RoleType role = (r==1?ROLE_BATSMAN:(r==2?ROLE_BOWLER:ROLE_ALLROUNDER));
          displayTopKPlayersOfTeamByRole(tid, role, k); }
        else if (choice == 5) {
          int r = promptInt("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 1);
          RoleType role = (r==1?ROLE_BATSMAN:(r==2?ROLE_BOWLER:ROLE_ALLROUNDER)); 
          displayAllPlayersAcrossTeamsByRole(role); }
        else if (choice == 6) { printf("Goodbye!\n"); break; }
        else printf("Invalid choice. Please select between 1 and 6.\n");
    }

    return 0;
}
