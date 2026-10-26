# Social Network Friend Recommendation System Using Graph, BFS and Hashing

**BTech AIML | Data Structures and Algorithms (DSA) Final Project**  
**Language:** C (C11 standard, GCC / Clang / MinGW / MSVC)

---

## 📁 Project Structure

```
social_network_friend_recommendation/
│
├── single_file/
│   ├── main.c              # Complete standalone single-file version
│   ├── friendrec           # Compiled executable (macOS / Linux)
│   └── network.txt         # Pre-configured demo network database
│
├── multi_file/
│   ├── common.h / common.c # String trimming, portable case-cmp, input validators
│   ├── hashtable.h / .c    # djb2 hash table with separate chaining (O(1) avg)
│   ├── queue.h / queue.c   # Linked list FIFO queue for BFS
│   ├── graph.h / graph.c   # Dynamic adjacency list, deletion, depth-2 BFS recommendation
│   ├── storage.h / .c      # Text file persistence (save / load network)
│   ├── main.c              # Menu controller & user interface
│   ├── Makefile            # Automated build script for GCC
│   └── network.txt         # Pre-configured demo network database
│
├── PROJECT_REPORT.md       # Complete academic report with all 13 required sections
├── README.md               # Quick setup and compilation guide
└── network.txt             # Project-level database file
```

---

## 🚀 Quick Compilation & Execution

### 1. Single-File Version (Fastest for Viva / Submission)
```bash
cd social_network_friend_recommendation/single_file
gcc -Wall -Wextra -std=c11 main.c -o friendrec
./friendrec
```

### 2. Multi-File Version (Modular Engineering)
```bash
cd social_network_friend_recommendation/multi_file
make
./friendrec
```
Or manually without `make`:
```bash
gcc -Wall -Wextra -std=c11 -O2 main.c common.c hashtable.c queue.c graph.c storage.c -o friendrec
./friendrec
```

### 3. Windows (MinGW / Code::Blocks / PowerShell)
In Windows Command Prompt or PowerShell with MinGW installed:
```cmd
gcc -Wall -Wextra -std=c11 main.c -o friendrec.exe
friendrec.exe
```
Or open `single_file/main.c` in **Code::Blocks** / **Dev-C++** / **VS Code** and click **Build and Run (F9)**.

---

## 🎯 Verification with Official Demo Data
1. Run `./friendrec`.
2. Choose **Option 12** to load the official demo dataset:
   - Users: **Rahul, Amit, Priya, Neha, Karan**
   - Friendships: **Rahul $\leftrightarrow$ Amit**, **Rahul $\leftrightarrow$ Neha**, **Amit $\leftrightarrow$ Priya**
3. Choose **Option 7** and enter `Rahul`:
   - System outputs **Priya** with **1 mutual friend (Amit)**.
   - **Karan** is isolated with 0 mutual friends and is correctly omitted.
4. Accept recommendation #1 to automatically connect Rahul and Priya!
5. Choose **Option 10** to view the updated graph.
6. Choose **Option 11** $\rightarrow$ `1` to persist the updated social graph to `network.txt`.
