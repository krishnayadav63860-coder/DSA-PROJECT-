#include "storage.h"

bool storage_save_network(const Graph *g, const char *filename) {
    if (!g || !filename || filename[0] == '\0') return false;

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("[ERROR] Failed to open file for saving");
        return false;
    }

    fprintf(fp, "# Social Network Friend Recommendation System Database\n");
    fprintf(fp, "# Format: USERS_COUNT, followed by Name|City|Interests\n");
    fprintf(fp, "# Followed by FRIENDSHIPS_COUNT, followed by User1|User2\n");
    fprintf(fp, "USERS_COUNT: %d\n", g->activeCount);

    /* Write active users */
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].isDeleted) {
            fprintf(fp, "%s|%s|%s\n", g->users[i].name, g->users[i].city, g->users[i].interests);
        }
    }

    /* Count unique undirected friendships */
    int friendshipCount = 0;
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].isDeleted) {
            AdjListNode *curr = g->adjLists[i].head;
            while (curr) {
                if (curr->destId > i && !g->users[curr->destId].isDeleted) {
                    friendshipCount++;
                }
                curr = curr->next;
            }
        }
    }

    fprintf(fp, "FRIENDSHIPS_COUNT: %d\n", friendshipCount);
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].isDeleted) {
            AdjListNode *curr = g->adjLists[i].head;
            while (curr) {
                if (curr->destId > i && !g->users[curr->destId].isDeleted) {
                    fprintf(fp, "%s|%s\n", g->users[i].name, g->users[curr->destId].name);
                }
                curr = curr->next;
            }
        }
    }

    fclose(fp);
    return true;
}

bool storage_load_network(Graph *g, const char *filename) {
    if (!g || !filename || filename[0] == '\0') return false;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return false;
    }

    char line[512];
    int expectedUsers = -1;
    int expectedFriendships = -1;
    int usersLoaded = 0;
    int friendshipsLoaded = 0;

    /* Temporary buffer while parsing */
    graph_clear(g);

    while (fgets(line, sizeof(line), fp)) {
        /* Strip newline characters */
        line[strcspn(line, "\r\n")] = '\0';
        trim_whitespace(line);

        /* Skip empty lines and comment lines */
        if (line[0] == '\0' || line[0] == '#') continue;

        if (strncmp(line, "USERS_COUNT:", 12) == 0) {
            expectedUsers = atoi(line + 12);
            continue;
        }

        if (strncmp(line, "FRIENDSHIPS_COUNT:", 18) == 0) {
            expectedFriendships = atoi(line + 18);
            continue;
        }

        /* If we are still reading users */
        if (expectedFriendships == -1) {
            /* Parse Name|City|Interests */
            char *pipe1 = strchr(line, '|');
            if (pipe1) {
                *pipe1 = '\0';
                char *name = line;
                char *rest = pipe1 + 1;
                char *pipe2 = strchr(rest, '|');
                char *city = rest;
                char *interests = "";
                if (pipe2) {
                    *pipe2 = '\0';
                    interests = pipe2 + 1;
                }
                graph_register_user(g, name, city, interests);
                usersLoaded++;
            }
        } else {
            /* Reading friendships: UserA|UserB */
            char *pipe = strchr(line, '|');
            if (pipe) {
                *pipe = '\0';
                char *userA = line;
                char *userB = pipe + 1;
                trim_whitespace(userA);
                trim_whitespace(userB);

                int idA = graph_get_user_id(g, userA);
                int idB = graph_get_user_id(g, userB);
                if (idA != -1 && idB != -1) {
                    graph_add_friendship(g, idA, idB);
                    friendshipsLoaded++;
                }
            }
        }
    }

    fclose(fp);
    (void)expectedUsers;
    (void)expectedFriendships;
    return (usersLoaded > 0 || friendshipsLoaded > 0 || expectedUsers == 0);
}
