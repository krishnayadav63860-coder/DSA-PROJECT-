# Project Report: Social Network Friend Recommendation System Using Graph, BFS and Hashing

**Course:** B.Tech Artificial Intelligence and Machine Learning (AIML)  
**Subject:** Data Structures and Algorithms (DSA)  
**Implementation Standard:** C (C11, GCC Compatible)  

---

## 1. Project Overview
In modern digital social networks (such as LinkedIn, Facebook, and Instagram), connecting users with mutual acquaintances is central to user engagement and network growth. This project implements a complete, final, and robust console-based **Social Network Friend Recommendation System** in C from first principles, without using third-party libraries.

The system represents the social network as an undirected, unweighted graph where each user profile corresponds to a vertex and each mutual friendship corresponds to a bidirectional edge. To scale dynamically, the graph utilizes an **Adjacency List** backed by dynamic arrays and linked lists. To achieve instantaneous user record retrieval in $O(1)$ average time, a custom **Hash Table** using the industry-standard **djb2** hash algorithm with separate chaining collision resolution is implemented. Friend recommendations are generated using **Breadth-First Search (BFS)** bounded to a depth of 2 (friends-of-friends), scored by the count of mutual friends, and ranked in descending order with alphabetical tie-breaking.

---

## 2. Objectives
1. **Model Real-World Social Topologies:** Construct a scalable, dynamic graph using adjacency lists capable of handling an arbitrary number of users without arbitrary upper limits.
2. **High-Performance Lookups:** Implement a string-keyed hash table with separate chaining and case-insensitive handling to map usernames to internal integer vertex indices in $O(1)$ average time.
3. **Graph Traversal via BFS:** Design a dynamic First-In, First-Out (FIFO) queue to execute Breadth-First Search up to depth 2, discovering non-connected friends-of-friends.
4. **Ranking & Recommendation Engine:** Compute the set intersection of mutual friendships for candidate recommendations, sort them in descending order of mutual connection count, and break ties alphabetically.
5. **Interactive Friendship Formation:** Enable direct acceptance of recommended candidates to automatically establish new edges in the graph.
6. **Data Persistence & Integrity:** Provide disk storage mechanisms to serialize and deserialize the entire social graph into human-readable text files (`network.txt`).
7. **Robust Software Engineering:** Implement strict input sanitization (handling non-numeric menu choices, empty inputs, spaces, and duplicate keys), eliminate dangling pointers, prevent memory leaks through exhaustive resource deallocation, and ensure zero compiler warnings under `-Wall -Wextra -std=c11`.

---

## 3. How the System Works (Step-by-Step Flow)

```
[Start Program]
       │
       ▼
[Initialize Dynamic Graph & djb2 Hash Table]
       │
       ▼
[Display Interactive Console Menu (Options 0 to 12)]
       │
       ├─► 1. Register User ───────► Trim name, check hash table for duplicate, allocate vertex slot.
       ├─► 2. Add Friendship ──────► Resolve user IDs, validate non-self, insert undirected edge (u <-> v).
       ├─► 3. Remove Friendship ───► Remove node from u's list and v's list, update vertex degrees.
       ├─► 4. Delete User Profile ─► Disconnect from all neighbor adjacency lists, free chains, tombstone vertex.
       ├─► 5. View Friends ────────► Traverse target user's adjacency list, print friend profiles.
       ├─► 6. Mutual Friends ──────► Intersect neighbor sets of user A and user B, display common friends.
       ├─► 7. Recommend Friends ───► Run BFS (depth <= 2), filter distance 2 nodes, rank by mutuals, allow quick add.
       ├─► 8. Search User (O(1)) ──► Compute djb2 hash, traverse chain, display user profile & degree.
       ├─► 9. Network Metrics ─────► Calculate active vertices, total edges (sum of degrees / 2), density, max degree.
       ├─► 10. Display Network ────► Print entire graph adjacency list representation.
       ├─► 11. Save / Load File ───► Serialize/Deserialize active users and friendships to/from 'network.txt'.
       ├─► 12. Load Demo Data ─────► Initialize demo users (Rahul, Amit, Priya, Neha, Karan) and relationships.
       └─► 0. Exit System ─────────► Recursively free all graph nodes, hash chains, arrays, and queues.
```

---

## 4. Graph Representation & ASCII Topology Diagram

### Demo Dataset:
- **Registered Users:** Rahul (ID 0), Amit (ID 1), Priya (ID 2), Neha (ID 3), Karan (ID 4)
- **Friendships (Undirected Edges):**
  - Rahul $\longleftrightarrow$ Amit
  - Rahul $\longleftrightarrow$ Neha
  - Amit $\longleftrightarrow$ Priya
- **Isolated Node:** Karan (Registered user with 0 friendships)

### ASCII Graph Topology:

```
        [ Neha ]
           │
           │ (Friendship)
           │
       [ Rahul ] ◄──────────────► [ Amit ] ◄──────────────► [ Priya ]
       (Depth 0)                    (Depth 1)                  (Depth 2)
       Target User                 Mutual Friend               Recommended
                                                               Candidate!
                                                               (Mutuals: Amit)

       [ Karan ] (Isolated Vertex, Degree = 0, Disconnected)
```

### Dynamic Adjacency List Structure in Memory:

```
 Vertex Index       User Profile            Adjacency List (Linked Chain)
 ════════════════════════════════════════════════════════════════════════════
   [0]            Rahul (Delhi)       ──► [Amit (1)] ──► [Neha (3)] ──► NULL
   [1]            Amit (Bangalore)    ──► [Rahul (0)] ──► [Priya (2)] ──► NULL
   [2]            Priya (Mumbai)      ──► [Amit (1)] ──► NULL
   [3]            Neha (Delhi)        ──► [Rahul (0)] ──► NULL
   [4]            Karan (Pune)        ──► NULL (No edges)
```

---

## 5. Data Structures Used and Their Purpose

| Data Structure | Implementation Details | Purpose & Justification |
| :--- | :--- | :--- |
| **Dynamic Graph** | Dynamic array of `User` structs + dynamic array of `AdjList` | Stores the entire network. Dynamically grows via `realloc()` when the number of registered users exceeds capacity. Undirected edges are represented symmetrically. |
| **Adjacency List** | Singly-linked list of `AdjListNode` per vertex | Space-efficient graph storage requiring $O(V + E)$ memory instead of $O(V^2)$ required by adjacency matrices. Ideal for sparse social networks. |
| **Hash Table** | Fixed/dynamic bucket array with separate chaining linked lists | Maps case-insensitive usernames to internal integer vertex indices. Provides $O(1)$ average time complexity for user search and duplicate verification. |
| **Hash Function** | djb2 string hash algorithm ($hash \times 33 + c$) | Distributes alphanumeric string keys uniformly across buckets with minimal collision clustering. |
| **FIFO Queue** | Singly-linked list with `front` and `rear` pointers | Drives Breadth-First Search (BFS) level-order traversal. Ensures strictly $O(1)$ enqueue and dequeue operations without fixed buffer limits. |
| **Recommendation Array** | Contiguous array of `Recommendation` structs | Holds candidates at distance 2 along with their mutual friend count and mutual friend ID array for sorting and display. |
| **QuickSort (`qsort`)** | C Standard Library comparator-driven quicksort | Ranks recommendation candidates in $O(K \log K)$ time by descending mutual count, tie-breaking alphabetically. |

---

## 6. Algorithm & Pseudocode for BFS Recommendation

### Numbered Steps:
1. **Validation:** Check if the target user `u` exists and is active (`!isDeleted`). If invalid, abort.
2. **Initialization:**
   - Allocate an integer array `dist` of size `userCount` initialized to `-1` for all vertices.
   - Set `dist[u] = 0`.
   - Initialize an empty FIFO queue `Q` and enqueue `u`.
3. **BFS Level Traversal (Bounded to Depth 2):**
   - While `Q` is not empty:
     - Dequeue current vertex `curr`.
     - If `dist[curr] >= 2`, skip exploring its neighbors (depth limit reached).
     - For each neighbor `v` in the adjacency list of `curr`:
       - If `users[v]` is active and `dist[v] == -1`:
         - Set `dist[v] = dist[curr] + 1`.
         - If `dist[v] <= 2`, enqueue `v` into `Q`.
4. **Candidate Identification:**
   - Iterate through all vertices `c` from $0$ to `userCount - 1`.
   - A vertex `c` is a valid recommendation candidate if and only if:
     - `c != u` (not the target user),
     - `!users[c].isDeleted` (active user),
     - `dist[c] == 2` (friend-of-a-friend, not already a direct friend).
5. **Mutual Friends Computation:**
   - For each candidate `c`, intersect the adjacency lists of `u` and `c`.
   - Count the number of common neighbors (`mutualCount`) and record their vertex IDs.
   - If `mutualCount >= 1`, add `c` to the candidate recommendation list.
6. **Ranking:**
   - Sort the candidate list using QuickSort:
     - Primary Key: `mutualCount` in descending order.
     - Secondary Key: `name` in ascending lexicographical order (case-insensitive).
7. **Presentation & Action:**
   - Print the ranked table displaying Candidate Name, City, Mutual Friend Count, and the explicit names of mutual friends.
   - Prompt the user to optionally accept any recommendation by rank number to establish an immediate mutual edge.
8. **Resource Cleanup:**
   - Free the distance array, temporary mutual friend lists, and the recommendations array.

### High-Level Pseudocode:

```text
ALGORITHM RecommendFriends(Graph G, Vertex targetUser)
Input:  Graph G = (V, E), targetUser index u
Output: Sorted list of recommendation records

BEGIN
    IF u is invalid OR G.users[u].isDeleted THEN
        RETURN NULL
    END IF

    // Step 1: Distance array and queue initialization
    dist ← Array of size |V| initialized to -1
    dist[u] ← 0
    Q ← CreateQueue()
    Enqueue(Q, u)

    // Step 2: Bounded BFS Traversal (Depth <= 2)
    WHILE NOT IsEmpty(Q) DO
        curr ← Dequeue(Q)
        IF dist[curr] >= 2 THEN
            CONTINUE
        END IF

        FOR EACH neighbor v IN G.adjLists[curr] DO
            IF NOT G.users[v].isDeleted AND dist[v] == -1 THEN
                dist[v] ← dist[curr] + 1
                IF dist[v] <= 2 THEN
                    Enqueue(Q, v)
                END IF
            END IF
        END FOR
    END WHILE
    FreeQueue(Q)

    // Step 3: Candidate filtering & Mutual Friends calculation
    candidates ← EmptyList()
    FOR candidate ← 0 TO |V| - 1 DO
        IF candidate != u AND NOT G.users[candidate].isDeleted AND dist[candidate] == 2 THEN
            mutualList ← FindCommonNeighbors(G, u, candidate)
            IF Length(mutualList) >= 1 THEN
                rec.candidateId ← candidate
                rec.name ← G.users[candidate].name
                rec.mutualCount ← Length(mutualList)
                rec.mutualFriends ← mutualList
                candidates.Append(rec)
            END IF
        END IF
    END FOR

    // Step 4: Sorting by mutual connections (descending) & name (alphabetical)
    QuickSort(candidates, CompareByMutualsThenName)

    RETURN candidates
END
```

---

## 7. Complete C Source Code
*(Both single-file and multi-file versions are provided in full in the accompanying implementation files).*

### Single-File Version:
- File path: `social_network_friend_recommendation/single_file/main.c`
- Contains all data structures, functions, persistence, and menu logic in a unified, standalone compilation unit.

### Multi-File Modular Version:
- `common.h` & `common.c`: Shared configurations, string sanitization, and input helpers.
- `hashtable.h` & `hashtable.c`: djb2 hash function and separate chaining table.
- `queue.h` & `queue.c`: Linked-list FIFO queue for BFS.
- `graph.h` & `graph.c`: Dynamic adjacency list, user registry, deletion, and BFS recommendation logic.
- `storage.h` & `storage.c`: Serialization and deserialization to text files.
- `main.c`: Menu-driven user interface and controller.
- `Makefile`: Build automation for GCC.

---

## 8. Sample Input and Output (Realistic Execution Session)

```text
======================================================================
   SOCIAL NETWORK FRIEND RECOMMENDATION SYSTEM USING GRAPH & BFS      
        BTech AIML | Data Structures and Algorithms Laboratory        
======================================================================

-------------------------- [ MAIN MENU ] ---------------------------
  1.  Register New User
  2.  Add Friendship (Create Undirected Edge)
  3.  Remove Friendship
  4.  Delete User Profile (Clean up edges)
  5.  View a User's Friend List
  6.  Find Mutual Friends Between Two Users
  7.  Recommend Friends (BFS Depth 2 + Mutual Scoring + Quick Add)
  8.  Search User by Name (O(1) Hash Table Lookup)
  9.  Show Network Statistics & Metrics
  10. Display Entire Network (Graph Adjacency View)
  11. File Storage (Save / Load Network Database)
  12. Load Official Presentation Demo Data
  0.  Exit System
--------------------------------------------------------------------
Choice (0 - 12): 12

[SUCCESS] Demo Data Loaded Successfully!
  Registered 5 Users: Rahul, Amit, Priya, Neha, Karan
  Created 3 Friendships: Rahul <-> Amit, Rahul <-> Neha, Amit <-> Priya
  (Notice: Karan is intentionally isolated for edge-case demonstration)

-------------------------- [ MAIN MENU ] ---------------------------
Choice (0 - 12): 10

======================================================================
                     COMPLETE SOCIAL NETWORK GRAPH                    
======================================================================
[00] Rahul          (Delhi      ) -> [Neha, Amit]
[01] Amit           (Bangalore  ) -> [Priya, Rahul]
[02] Priya          (Mumbai     ) -> [Amit]
[03] Neha           (Delhi      ) -> [Rahul]
[04] Karan          (Pune       ) -> [No friends]
======================================================================

-------------------------- [ MAIN MENU ] ---------------------------
Choice (0 - 12): 7

--- [7. RECOMMEND FRIENDS (BFS DEPTH 2)] ---
Enter Target Username / ID: Rahul

========================================================================================
                   RECOMMENDED FRIENDS FOR: Rahul                                
========================================================================================
 Rank | Candidate Name  | City            | Mutuals | Mutual Friend Names
----------------------------------------------------------------------------------------
 1    | Priya           | Mumbai          | 1       | Amit
========================================================================================

Accept a recommendation? Enter Rank # to add friend (0 to return to menu): Choice (0 - 1): 1
[SUCCESS] Friendship established! Rahul and Priya are now friends.

-------------------------- [ MAIN MENU ] ---------------------------
Choice (0 - 12): 10

======================================================================
                     COMPLETE SOCIAL NETWORK GRAPH                    
======================================================================
[00] Rahul          (Delhi      ) -> [Priya, Neha, Amit]
[01] Amit           (Bangalore  ) -> [Priya, Rahul]
[02] Priya          (Mumbai     ) -> [Rahul, Amit]
[03] Neha           (Delhi      ) -> [Rahul]
[04] Karan          (Pune       ) -> [No friends]
======================================================================

-------------------------- [ MAIN MENU ] ---------------------------
Choice (0 - 12): 8

--- [8. SEARCH USER BY NAME (HASH TABLE)] ---
Enter Username to Search: priya

============================================================
                  USER PROFILE DETAILS                      
============================================================
  User ID       : 2
  Username      : Priya
  City          : Mumbai
  Interests     : Data Science, Cloud
  Total Friends : 2
  Lookup Status : Found via djb2 Hash Table (O(1) average)
============================================================

-------------------------- [ MAIN MENU ] ---------------------------
Choice (0 - 12): 9

============================================================
                 NETWORK STATISTICS & METRICS               
============================================================
  Total Active Users       : 5
  Total Friendships (Edges): 4
  Average Friends / User   : 1.60
  Network Graph Density    : 0.4000 (40.00% connected)
  Most Connected User      : Rahul (ID: 0) with 3 friends
============================================================

-------------------------- [ MAIN MENU ] ---------------------------
Choice (0 - 12): 0

Exiting Social Network System. Freeing all allocated memory...
All resources cleanly released. Program terminated safely.
```

---

## 9. Time and Space Complexity Analysis

Let $V$ be the number of active vertices (users) and $E$ be the number of undirected edges (friendships). Let $d_u$ denote the degree (number of friends) of user $u$.

| Operation | Time Complexity (Best / Avg) | Time Complexity (Worst) | Space Complexity | Theoretical Explanation |
| :--- | :--- | :--- | :--- | :--- |
| **Register User** | $O(1)$ | $O(V)$ | $O(1)$ | djb2 hash insert with duplicate check ($O(1)$ average chain lookup). Dynamic array amortized $O(1)$ expansion. |
| **Search User by Name** | $O(1)$ | $O(V)$ | $O(1)$ | djb2 hash computation + bucket chain traversal. Worst case occurs only under hash collisions. |
| **Add Friendship** | $O(d_u + d_v)$ | $O(V)$ | $O(1)$ | Checks for duplicate edge in adjacency list, then allocates and prepends two `AdjListNode` elements in $O(1)$ time. |
| **Remove Friendship** | $O(d_u + d_v)$ | $O(V)$ | $O(1)$ | Linear scan through the adjacency lists of $u$ and $v$ to unlink and deallocate matching edge nodes. |
| **Delete User Profile** | $O(\sum_{f \in N(u)} d_f)$ | $O(V + E)$ | $O(1)$ | Unlinks user from all direct friends' lists, frees user's own adjacency list, deletes hash table entry, and marks tombstone. |
| **Find Mutual Friends** | $O(d_u \cdot d_v)$ | $O(V^2)$ | $O(\min(d_u, d_v))$ | Iterates through neighbors of $u$ and tests membership in the adjacency list of $v$ via linked traversal. |
| **BFS Recommendation** | $O(V + E + K \log K)$ | $O(V^2 + K \log K)$ | $O(V)$ | BFS bounded to depth 2 visits $O(V+E)$ states. Mutual friend scoring for $K$ candidates followed by quicksort ranking in $O(K \log K)$. |
| **Network Statistics** | $O(V)$ | $O(V)$ | $O(1)$ | Single pass over user array to sum degrees, compute maximum degree, and evaluate density. |
| **Save / Load Network** | $O(V + E)$ | $O(V + E)$ | $O(V)$ | Sequential file serialization and deserialization of active vertices and unique undirected edge pairs. |

---

## 10. Test Cases Table

| Test Case ID | Test Category | Input Data / Actions | Expected Behavior | Actual System Output | Pass/Fail |
| :---: | :--- | :--- | :--- | :--- | :---: |
| **TC-01** | Normal Case | Load Demo Data (Rahul, Amit, Priya, Neha, Karan) | 5 users registered, 3 friendships created, Karan isolated. | `[SUCCESS] Demo Data Loaded Successfully!` | **PASS** |
| **TC-02** | Recommendation | Recommend friends for `Rahul` | Recommends `Priya` (1 mutual: Amit); excludes `Karan` (0 mutuals). | Rank 1: Priya, Mutuals: 1, Mutual Friend: Amit. | **PASS** |
| **TC-03** | Interactive Add | Accept recommendation rank #1 for Rahul | Creates friendship Rahul $\longleftrightarrow$ Priya immediately. | `[SUCCESS] Friendship established! Rahul and Priya are now friends.` | **PASS** |
| **TC-04** | Edge Case | Recommend friends for `Rahul` again | Priya is now a direct friend; no remaining depth-2 candidates. | `[INFO] No friend recommendations found for Rahul.` | **PASS** |
| **TC-05** | Invalid Input | Empty username during registration `""` | Rejects registration, prompts error without crashing. | `[ERROR] Registration failed: Username cannot be empty.` | **PASS** |
| **TC-06** | Duplicate Case | Register user `"rahul"` when `"Rahul"` exists | Case-insensitive rejection via djb2 hash table lookup. | `[ERROR] User 'rahul' already exists (case-insensitive duplicate).` | **PASS** |
| **TC-07** | Invalid Input | Self-friending `Rahul` with `Rahul` | Rejects operation with clear feedback. | `[ERROR] Self-friending is not allowed.` | **PASS** |
| **TC-08** | Edge Case | Recommend friends for isolated user `Karan` | Isolated node has no neighbors; depth-2 BFS returns 0 candidates. | `[INFO] No friend recommendations found for Karan.` | **PASS** |
| **TC-09** | Robustness | Non-numeric input `"abc"` in menu choice | Validates input, prevents buffer overrun, re-prompts user. | `[!] Invalid input. Please enter numeric digits only.` | **PASS** |
| **TC-10** | Deletion | Delete user `Rahul` | Disconnects Rahul from Amit, Priya, Neha; removes from hash table. | Edge cleanup verified; Rahul degree becomes 0 in all neighbor lists. | **PASS** |
| **TC-11** | Persistence | Save to `network.txt` and reload | All active vertices and active edges restored faithfully. | `[SUCCESS] Network successfully loaded from 'network.txt'!` | **PASS** |
| **TC-12** | Memory Clean | Exit program via option `0` | All dynamically allocated memory freed cleanly (Valgrind verified). | `All resources cleanly released. Program terminated safely.` | **PASS** |

---

## 11. Limitations and Future Scope

### Current Limitations:
1. **Unweighted Connections:** Friendships are currently unweighted binary edges. In reality, friendships have varying strengths based on shared interests, interaction frequency, and geographic proximity.
2. **Volatile Local File Format:** File persistence relies on standard flat text parsing (`network.txt`). In large enterprise deployments, ACID-compliant databases (PostgreSQL/SQLite) are required.
3. **In-Memory Traversal Scalability:** The entire graph resides in RAM. For billion-node graphs (like Facebook or LinkedIn), distributed graph processing engines (Apache Giraph or Neo4j) are needed.
4. **Console-Based User Interface:** While clean and robust, a terminal interface lacks graphical node visualization.

### Future Scope:
1. **Weighted Edges (Cosine Similarity on Interests):** Compute Jaccard or Cosine similarity over user interest vectors (e.g., "AI", "Cloud", "Gaming") to weight edges and produce hybrid content-collaborative recommendations.
2. **Degrees of Separation (Dijkstra / Bidirectional BFS):** Implement Dijkstra's or Bidirectional BFS to find the shortest path between any two arbitrary users, proving the "Six Degrees of Separation" theory.
3. **Community Detection (Louvain / Girvan-Newman):** Implement modularity-based clustering to automatically discover student groups, colleges, or interest circles.
4. **Interactive GUI / Web Interface:** Build a modern web front end (Next.js / WebGL / D3.js) connecting to a C REST API backend (using WebSockets or libmicrohttpd) for real-time interactive force-directed graph layouts.

---

## 12. Conclusion
The **Social Network Friend Recommendation System** successfully demonstrates the practical synergy between core computer science data structures: **Graphs**, **Hash Tables**, **Queues**, and **Sorting Algorithms**. By implementing an unweighted graph with dynamic adjacency lists and pairing it with a djb2 hash table, the system achieves $O(1)$ average identity resolution and $O(V + E)$ graph traversal efficiency. The Depth-2 BFS friend recommendation engine mirrors the fundamental algorithmic architecture powering production social networks worldwide. Designed with rigorous input validation, safe string parsing, and zero memory leaks, this project stands as a complete, academic, and industry-aligned demonstration of Data Structures and Algorithms in C11.

---

## 13. Viva Questions with Model Answers

#### Q1: Why did you use an Adjacency List instead of an Adjacency Matrix to represent the graph?
**Answer:** Social networks are typically sparse graphs where $|E| \ll |V|^2$ (users are friends with only a tiny fraction of total users). An Adjacency Matrix consumes $\Theta(V^2)$ memory and requires $O(V)$ time to iterate through neighbors. An Adjacency List consumes only $O(V + E)$ memory and allows neighbor traversal in $O(\text{degree}(u))$ time, which is substantially faster and memory-efficient.

#### Q2: Why is BFS used for friend recommendation instead of DFS?
**Answer:** BFS visits vertices in order of increasing distance from the source vertex (level by level). Since friend recommendations in real-life social networks rely on "friends-of-friends" (vertices at shortest-path distance equal to 2), BFS guarantees discovery of all candidates at depth 2 before exploring any deeper nodes. DFS travels deep down a single path, making it inefficient for bounded neighborhood analysis.

#### Q3: Why is the recommendation bounded to depth 2?
**Answer:** Distance 1 represents users who are already direct friends. Distance 2 represents friends-of-friends who have at least one mutual friend with the target user. At distance 3 or greater, users share zero mutual friends, rendering mutual-friend-based recommendation scores 0 and connection likelihood negligible.

#### Q4: How does the djb2 hash function work, and why is it preferred?
**Answer:** djb2 is an empirical string hash created by Daniel J. Bernstein. It initializes `hash = 5381` (an odd prime) and computes `hash = ((hash << 5) + hash) + c` (equivalent to `hash * 33 + c`) for each character. It exhibits excellent bit dispersion, low collision rates, and is computationally fast since multiplication by 33 is executed via bitwise shift and addition.

#### Q5: How do you handle hash collisions in your implementation?
**Answer:** We implemented **separate chaining**. Each bucket in the hash table points to the head of a singly linked list of `HashNode` structs. When two distinct usernames hash to the exact same index, the new node is prepended to that bucket's linked list.

#### Q6: How does user deletion avoid dangling pointers in the graph?
**Answer:** When user $u$ is deleted, the system iterates through all vertices in $u$'s adjacency list and removes $u$ from each friend's list. Then, $u$'s own adjacency list nodes are freed, its username is deleted from the hash table, and its vertex slot is marked as deleted (tombstoned). This ensures no other vertex holds a pointer or reference to the deleted user.

#### Q7: What is the time complexity of ranking the recommendations?
**Answer:** If $K$ candidates are discovered at depth 2, mutual friend counts are calculated for each candidate. The candidates are then sorted using `qsort` which implements QuickSort with an average time complexity of $O(K \log K)$ and worst-case $O(K^2)$.

#### Q8: How is the system protected against buffer overflows and invalid inputs?
**Answer:** We completely avoided unsafe functions like `gets()` or raw `scanf("%s")`. All text input is captured using `fgets()` with bounded buffers. Integers are parsed from input buffers using `strtol()` with pointer validation to reject non-numeric characters and out-of-bounds menu selections without crashing or entering infinite loops.

#### Q9: How is memory managed to prevent leaks?
**Answer:** Every dynamically allocated block (`malloc`, `calloc`, `realloc`) is systematically tracked and freed. Upon selecting exit (option 0), `graph_free()` walks through every adjacency list node, hash bucket chain, and dynamic array, returning all heap memory to the operating system.

#### Q10: How do you break ties when two candidates have the exact same mutual friend count?
**Answer:** In the `compare_recommendations` comparator function passed to `qsort`, if `recA->mutualCount == recB->mutualCount`, we invoke `str_case_cmp(recA->candidateName, recB->candidateName)` to sort the candidates in alphabetical ascending order.
