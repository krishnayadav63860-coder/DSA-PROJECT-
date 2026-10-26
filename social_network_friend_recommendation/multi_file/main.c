#include "common.h"
#include "graph.h"
#include "storage.h"

static void print_banner(void) {
    printf("\n");
    printf("======================================================================\n");
    printf("   SOCIAL NETWORK FRIEND RECOMMENDATION SYSTEM USING GRAPH & BFS      \n");
    printf("        BTech AIML | Data Structures and Algorithms Laboratory        \n");
    printf("======================================================================\n");
}

static void print_menu(void) {
    printf("\n-------------------------- [ MAIN MENU ] ---------------------------\n");
    printf("  1.  Register New User\n");
    printf("  2.  Add Friendship (Create Undirected Edge)\n");
    printf("  3.  Remove Friendship\n");
    printf("  4.  Delete User Profile (Clean up edges)\n");
    printf("  5.  View a User's Friend List\n");
    printf("  6.  Find Mutual Friends Between Two Users\n");
    printf("  7.  Recommend Friends (BFS Depth 2 + Mutual Scoring + Quick Add)\n");
    printf("  8.  Search User by Name (O(1) Hash Table Lookup)\n");
    printf("  9.  Show Network Statistics & Metrics\n");
    printf("  10. Display Entire Network (Graph Adjacency View)\n");
    printf("  11. File Storage (Save / Load Network Database)\n");
    printf("  12. Load Official Presentation Demo Data\n");
    printf("  0.  Exit System\n");
    printf("--------------------------------------------------------------------\n");
}

/* Helper to prompt and get user ID by name or direct ID input */
static int prompt_user_id(const Graph *g, const char *prompt_label) {
    char input[MAX_NAME_LEN];
    printf("%s: ", prompt_label);
    safe_read_line(input, sizeof(input));

    if (input[0] == '\0') {
        printf("[ERROR] Input cannot be blank.\n");
        return -1;
    }

    /* First, check if exact or case-insensitive name match exists */
    int id = graph_get_user_id(g, input);
    if (id != -1) return id;

    /* If not found by name, check if user typed an integer ID */
    char *endptr;
    long num = strtol(input, &endptr, 10);
    if (*endptr == '\0' && num >= 0 && num < g->userCount) {
        if (!g->users[num].isDeleted) {
            return (int)num;
        }
    }

    printf("[ERROR] User '%s' does not exist in the network.\n", input);
    return -1;
}

static void handle_register_user(Graph *g) {
    char name[MAX_NAME_LEN];
    char city[MAX_INFO_LEN];
    char interests[MAX_INFO_LEN];

    printf("\n--- [1. REGISTER NEW USER] ---\n");
    printf("Enter Username: ");
    safe_read_line(name, sizeof(name));

    if (name[0] == '\0') {
        printf("[ERROR] Registration failed: Username cannot be empty.\n");
        return;
    }

    printf("Enter City (optional, press Enter to skip): ");
    safe_read_line(city, sizeof(city));

    printf("Enter Interests / Hobbies (optional, press Enter to skip): ");
    safe_read_line(interests, sizeof(interests));

    int res = graph_register_user(g, name, city, interests);
    if (res == -2) {
        printf("[ERROR] Registration failed: User '%s' already exists (case-insensitive duplicate).\n", name);
    } else if (res == -1) {
        printf("[ERROR] Registration failed: Invalid name format.\n");
    } else {
        printf("[SUCCESS] User '%s' successfully registered with User ID: %d\n", g->users[res].name, res);
    }
}

static void handle_add_friendship(Graph *g) {
    printf("\n--- [2. ADD FRIENDSHIP] ---\n");
    if (g->activeCount < 2) {
        printf("[INFO] At least 2 active users are needed to create a friendship.\n");
        return;
    }

    int u = prompt_user_id(g, "Enter First Username / ID");
    if (u == -1) return;

    int v = prompt_user_id(g, "Enter Second Username / ID");
    if (v == -1) return;

    if (u == v) {
        printf("[ERROR] Self-friending is not allowed. A user cannot be friends with themselves.\n");
        return;
    }

    if (graph_has_friendship(g, u, v)) {
        printf("[WARNING] Friendship already exists between %s and %s.\n", g->users[u].name, g->users[v].name);
        return;
    }

    if (graph_add_friendship(g, u, v)) {
        printf("[SUCCESS] Friendship established between %s and %s (undirected edge added)!\n",
               g->users[u].name, g->users[v].name);
    } else {
        printf("[ERROR] Failed to add friendship.\n");
    }
}

static void handle_remove_friendship(Graph *g) {
    printf("\n--- [3. REMOVE FRIENDSHIP] ---\n");
    int u = prompt_user_id(g, "Enter First Username / ID");
    if (u == -1) return;

    int v = prompt_user_id(g, "Enter Second Username / ID");
    if (v == -1) return;

    if (!graph_has_friendship(g, u, v)) {
        printf("[ERROR] %s and %s are not currently friends.\n", g->users[u].name, g->users[v].name);
        return;
    }

    if (graph_remove_friendship(g, u, v)) {
        printf("[SUCCESS] Friendship removed between %s and %s.\n", g->users[u].name, g->users[v].name);
    } else {
        printf("[ERROR] Failed to remove friendship.\n");
    }
}

static void handle_delete_user(Graph *g) {
    printf("\n--- [4. DELETE USER PROFILE] ---\n");
    int u = prompt_user_id(g, "Enter Username / ID to Delete");
    if (u == -1) return;

    char confirm[16];
    printf("Are you sure you want to delete '%s' and remove all their friendships? (y/n): ", g->users[u].name);
    safe_read_line(confirm, sizeof(confirm));

    if (confirm[0] == 'y' || confirm[0] == 'Y') {
        char deletedName[MAX_NAME_LEN];
        strncpy(deletedName, g->users[u].name, sizeof(deletedName) - 1);
        deletedName[sizeof(deletedName) - 1] = '\0';

        if (graph_delete_user(g, u)) {
            printf("[SUCCESS] User '%s' and all associated graph edges were permanently removed.\n", deletedName);
        } else {
            printf("[ERROR] Failed to delete user.\n");
        }
    } else {
        printf("[INFO] Deletion cancelled.\n");
    }
}

static void handle_view_friends(Graph *g) {
    printf("\n--- [5. VIEW USER FRIEND LIST] ---\n");
    int u = prompt_user_id(g, "Enter Username / ID");
    if (u == -1) return;

    graph_display_friends(g, u);
}

static void handle_find_mutual_friends(Graph *g) {
    printf("\n--- [6. FIND MUTUAL FRIENDS] ---\n");
    int u = prompt_user_id(g, "Enter First User");
    if (u == -1) return;

    int v = prompt_user_id(g, "Enter Second User");
    if (v == -1) return;

    if (u == v) {
        printf("[INFO] Both inputs refer to the same user (%s).\n", g->users[u].name);
        return;
    }

    int *mutualIds = NULL;
    int count = graph_get_mutual_friends(g, u, v, &mutualIds);

    printf("\n============================================================\n");
    printf(" Mutual Friends between %s and %s: (%d total)\n", g->users[u].name, g->users[v].name, count);
    printf("============================================================\n");

    if (count == 0) {
        printf("  No mutual friends found.\n");
    } else {
        for (int i = 0; i < count; i++) {
            int mId = mutualIds[i];
            printf("  %d. %-15s (City: %s)\n", i + 1, g->users[mId].name, g->users[mId].city);
        }
        free(mutualIds);
    }
    printf("============================================================\n");
}

static void handle_recommend_friends(Graph *g) {
    printf("\n--- [7. RECOMMEND FRIENDS (BFS DEPTH 2)] ---\n");
    int u = prompt_user_id(g, "Enter Target Username / ID");
    if (u == -1) return;

    int recCount = 0;
    Recommendation *recs = graph_recommend_friends(g, u, &recCount);

    if (recCount == 0) {
        printf("\n[INFO] No friend recommendations found for %s.\n", g->users[u].name);
        printf("       (Candidates must be at BFS distance 2 with >= 1 mutual friend).\n");
        graph_free_recommendations(recs, recCount);
        return;
    }

    printf("\n========================================================================================\n");
    printf("                   RECOMMENDED FRIENDS FOR: %-15s                      \n", g->users[u].name);
    printf("========================================================================================\n");
    printf(" %-4s | %-15s | %-15s | %-7s | %s\n", "Rank", "Candidate Name", "City", "Mutuals", "Mutual Friend Names");
    printf("----------------------------------------------------------------------------------------\n");

    for (int i = 0; i < recCount; i++) {
        /* Format list of mutual friends names */
        char mutualList[256] = "";
        for (int m = 0; m < recs[i].mutualCount; m++) {
            int mId = recs[i].mutualFriendIds[m];
            if (m > 0) strncat(mutualList, ", ", sizeof(mutualList) - strlen(mutualList) - 1);
            strncat(mutualList, g->users[mId].name, sizeof(mutualList) - strlen(mutualList) - 1);
        }

        int cId = recs[i].candidateId;
        printf(" %-4d | %-15s | %-15s | %-7d | %s\n",
               i + 1, recs[i].candidateName, g->users[cId].city, recs[i].mutualCount, mutualList);
    }
    printf("========================================================================================\n");

    /* Quick Acceptance feature */
    printf("\nAccept a recommendation? Enter Rank # to add friend (0 to return to menu): ");
    int choice = safe_read_int(0, recCount);

    if (choice > 0) {
        int selectedCandidateId = recs[choice - 1].candidateId;
        if (graph_add_friendship(g, u, selectedCandidateId)) {
            printf("[SUCCESS] Friendship established! %s and %s are now friends.\n",
                   g->users[u].name, g->users[selectedCandidateId].name);
        } else {
            printf("[ERROR] Failed to add friendship.\n");
        }
    } else {
        printf("[INFO] No friendship added.\n");
    }

    graph_free_recommendations(recs, recCount);
}

static void handle_search_user(Graph *g) {
    printf("\n--- [8. SEARCH USER BY NAME (HASH TABLE)] ---\n");
    char name[MAX_NAME_LEN];
    printf("Enter Username to Search: ");
    safe_read_line(name, sizeof(name));

    if (name[0] == '\0') {
        printf("[ERROR] Username cannot be empty.\n");
        return;
    }

    int id = graph_get_user_id(g, name);
    if (id == -1) {
        printf("[INFO] User '%s' NOT FOUND in the social network hash table.\n", name);
        return;
    }

    printf("\n============================================================\n");
    printf("                  USER PROFILE DETAILS                      \n");
    printf("============================================================\n");
    printf("  User ID       : %d\n", g->users[id].id);
    printf("  Username      : %s\n", g->users[id].name);
    printf("  City          : %s\n", g->users[id].city);
    printf("  Interests     : %s\n", g->users[id].interests);
    printf("  Total Friends : %d\n", g->adjLists[id].degree);
    printf("  Lookup Status : Found via djb2 Hash Table (O(1) average)\n");
    printf("============================================================\n");
}

static void handle_file_storage(Graph *g) {
    printf("\n--- [11. FILE STORAGE & PERSISTENCE] ---\n");
    printf("  1. Save Current Network to File\n");
    printf("  2. Load Network from File\n");
    int subChoice = safe_read_int(1, 2);

    char filename[128];
    printf("Enter filename (default: network.txt): ");
    safe_read_line(filename, sizeof(filename));
    if (filename[0] == '\0') {
        strcpy(filename, "network.txt");
    }

    if (subChoice == 1) {
        if (storage_save_network(g, filename)) {
            printf("[SUCCESS] Social network saved successfully to '%s'.\n", filename);
        } else {
            printf("[ERROR] Could not save network to '%s'.\n", filename);
        }
    } else {
        char confirm[16];
        printf("Loading from file will replace the current in-memory network. Continue? (y/n): ");
        safe_read_line(confirm, sizeof(confirm));
        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            if (storage_load_network(g, filename)) {
                printf("[SUCCESS] Network successfully loaded from '%s'!\n", filename);
                printf("          Active Users: %d\n", g->activeCount);
            } else {
                printf("[ERROR] Failed to load from '%s'. Please check file existence/format.\n", filename);
            }
        } else {
            printf("[INFO] Load cancelled.\n");
        }
    }
}

int main(void) {
    Graph *network = graph_create(INITIAL_GRAPH_CAPACITY);
    print_banner();

    int choice = -1;
    while (choice != 0) {
        print_menu();
        choice = safe_read_int(0, 12);

        switch (choice) {
            case 1:
                handle_register_user(network);
                break;
            case 2:
                handle_add_friendship(network);
                break;
            case 3:
                handle_remove_friendship(network);
                break;
            case 4:
                handle_delete_user(network);
                break;
            case 5:
                handle_view_friends(network);
                break;
            case 6:
                handle_find_mutual_friends(network);
                break;
            case 7:
                handle_recommend_friends(network);
                break;
            case 8:
                handle_search_user(network);
                break;
            case 9:
                graph_show_stats(network);
                break;
            case 10:
                graph_display_network(network);
                break;
            case 11:
                handle_file_storage(network);
                break;
            case 12:
                graph_load_demo_data(network);
                break;
            case 0:
                printf("\nExiting Social Network System. Freeing all allocated memory...\n");
                break;
            default:
                printf("[ERROR] Unknown menu option.\n");
                break;
        }
    }

    graph_free(network);
    printf("All resources cleanly released. Thank you!\n");
    return 0;
}
