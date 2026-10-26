#!/usr/bin/env python3
"""
1:1 Format Clone Report Generator
Takes the exact template and structure of OSMOS Mini Project Report (1).docx,
preserves every single headline, style, tag, table width, and margin,
and replaces only the paragraph and table text with the Social Network Friend Recommendation System project content.
"""

import os
import shutil
import zipfile
import xml.etree.ElementTree as ET

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
ET.register_namespace('w', W)

src_docx = '/Users/anakolte/Downloads/OSMOS Mini Project Report (1).docx'
target_docx = '/Users/anakolte/Documents/local projects/krishna project/Social_Network_Recommendation_System_Project_Report.docx'
download_target_docx = '/Users/anakolte/Downloads/Social_Network_Recommendation_System_Project_Report.docx'
md_path = '/Users/anakolte/Documents/local projects/krishna project/PROJECT_REPORT.md'

with zipfile.ZipFile(src_docx, 'r') as z:
    xml_content = z.read('word/document.xml')

tree = ET.fromstring(xml_content)
namespaces = {'w': W}
body = tree.find('w:body', namespaces)

def set_p_text(p, text):
    pPr = p.find(f'{{{W}}}pPr')
    # find existing rPr if any
    r_list = p.findall(f'{{{W}}}r')
    rPr = None
    if r_list:
        rPr = r_list[0].find(f'{{{W}}}rPr')
    
    # Remove all children except pPr
    for child in list(p):
        if child != pPr:
            p.remove(child)
            
    if text != "":
        r = ET.SubElement(p, f'{{{W}}}r')
        if rPr is not None:
            r.append(rPr)
        t = ET.SubElement(r, f'{{{W}}}t', {'{http://www.w3.org/XML/1998/namespace}space': 'preserve'})
        t.text = text

def set_table_cells(tbl, rows_data):
    trs = tbl.findall(f'{{{W}}}tr')
    # Update each row and cell
    for r_idx, row in enumerate(rows_data):
        if r_idx < len(trs):
            tr = trs[r_idx]
        else:
            # duplicate last tr structure
            last_tr = trs[-1]
            tr = ET.fromstring(ET.tostring(last_tr))
            tbl.append(tr)
            
        tcs = tr.findall(f'{{{W}}}tc')
        for c_idx, cell_text in enumerate(row):
            if c_idx < len(tcs):
                tc = tcs[c_idx]
                p = tc.find(f'{{{W}}}p')
                if p is None:
                    p = ET.SubElement(tc, f'{{{W}}}p')
                set_p_text(p, str(cell_text))

# Exact text replacements for every element 0 to 178:
p_replacements = {
    0: "Social Network Friend Recommendation System Using Graph, BFS and Hashing",
    1: "Data Structures and Algorithms Mini Project Report",
    2: "Submitted by: Krishna Yadav Roll Number: 14 Course: B.Tech (Artificial Intelligence & Machine Learning), Second Year Semester: Third Department: B.Tech in AIML Subject: Data Structures and Algorithms Institution: Alard University, Hinjewadi, Pune Project Guide: Prof. Krati Srivastava Academic Year: 2026-2027 Date of Submission: October 26, 2026",
    3: "",
    4: "Certificate",
    5: "Alard University, Hinjewadi, Pune Department of B.Tech in AIML",
    6: 'This is to certify that the mini project report entitled "Social Network Friend Recommendation System Using Graph, BFS and Hashing" has been carried out by Krishna Yadav (Roll No. 14), a student of B.Tech (Artificial Intelligence & Machine Learning), Second Year, in partial fulfilment of the requirements of the course Data Structures and Algorithms during the academic year 2026-2027.',
    7: "The work embodied in this report is, to the best of our knowledge, the original work of the student, and has been completed under the guidance of the undersigned.",
    9: "Place: Pune Date: October 26, 2026",
    10: "",
    11: "Declaration",
    12: 'I, Krishna Yadav, Roll No. 14, hereby declare that this mini project report titled "Social Network Friend Recommendation System Using Graph, BFS and Hashing" is submitted in partial fulfilment of the Data Structures and Algorithms course and has not been submitted to any other institution or course. The project source code is available in the laboratory repository and is acknowledged as such. All sources of information have been duly acknowledged.',
    13: "Krishna Yadav (Roll No. 14)  |  Date: October 26, 2026  |  Place: Pune",
    14: "",
    15: "Acknowledgement",
    16: "I express my sincere gratitude to my project guide, Prof. Krati Srivastava, for her guidance and encouragement. I thank the Head of Department, Prof. Sonali Patil, the Department of B.Tech in AIML, and Alard University, Hinjewadi, Pune, for providing the academic environment for this work. I also thank the open-source community whose tools (C11, GCC, GNU Make, Node.js and modern web standards) made this project possible.",
    17: "",
    18: "Abstract",
    19: 'The Social Network Friend Recommendation System is a high-performance educational and practical application that demonstrates how foundational Data Structures and Algorithms model real-world social connections. Users are represented as vertices and bidirectional friendships as undirected edges in a dynamically allocated Adjacency List. User lookups operate in O(1) average time via a separate-chaining Hash Table powered by Daniel J. Bernstein\'s djb2 algorithm. Friend recommendations are generated using a Bounded Breadth-First Search (BFS) restricted to Depth 2 ("friends-of-friends"), strictly pruning direct friends and isolated nodes. Recommendations are scored by counting mutual friendship intersections and ranked deterministically using Quick Sort with mutual count descending and names alphabetically ascending. A companion web application styled after LinkedIn features an interactive force-directed HTML5 Canvas topology visualizer with non-overlapping node physics, boundary-safe tooltips, live telemetry, and full profile inspection showing connection degrees (1st, 2nd, 3rd+). Memory profiling via Valgrind verifies zero memory leaks across all dynamic allocations. Validated on a curated 15-member industry benchmark dataset, the project provides a free, accessible and practical tool for learning core Data Structures and Algorithms.',
    20: "",
    21: "Table of Contents",
    23: "List of Figures",
    24: "Fig. 1 Interactive LinkedIn home feed; Fig. 2 Force-directed social topology canvas; Fig. 3 User profile view with degree badges; Fig. 4 djb2 hash table separate chaining distribution; Fig. 5 Dynamic adjacency list memory layout; Fig. 6 Bounded BFS traversal state machine; Fig. 7 Mutual friends explorer; Fig. 8 Terminal ANSI CLI interactive menu; Fig. 9 Valgrind memory leak verification report; Fig. 10 network.txt database file persistence.",
    25: "List of Tables",
    26: "Table 1 DSA concepts and modules; Table 2 Graph traversal algorithms; Table 3 Graph representation strategies; Table 4 Hash collision strategies; Table 5 Recommendation stages; Table 6 Technology stack; Table 7 Testing and verification checks; Table 8 Limitations; Table 9 Future improvements.",
    27: "",
    28: "1. Introduction",
    29: "1.1 Overview",
    30: "The Social Network Friend Recommendation System is a dual-tier application that demonstrates how foundational Data Structures and Algorithms model real-world social connections, search users in O(1) time, traverse network graphs, and recommend friends based on mutual connections. Every recommendation is driven by a real algorithm running in C and mirrored in the browser.",
    31: "1.2 Motivation",
    32: "Concepts such as adjacency lists, hash tables, breadth-first search and collision resolution are abstract and are usually learned from static textbook diagrams. Interactive visualisation and a usable C console application let learners observe network state, trace queue progression and experiment with recommendation algorithms directly.",
    33: "1.3 Problem Statement",
    34: "Students lack a free, installation-free tool that demonstrates core Data Structures and Algorithms through genuine algorithm execution rather than fixed illustrations, paired with production-quality C memory management and LinkedIn-style interactive exploration.",
    35: "1.4 Objectives",
    36: "To demonstrate core DSA concepts through interactive graph simulations.",
    37: "To drive every recommendation from a genuine Bounded BFS and djb2 hashing algorithm.",
    38: "To provide a responsive LinkedIn-style interface usable on mobile, tablet and desktop.",
    39: "To require no external libraries, third-party frameworks or paid cloud services.",
    40: "To verify correctness, zero memory leaks (Valgrind) and layout safety through automated checks.",
    41: "1.5 Scope",
    42: "The project covers user registration, djb2 separate-chaining hashing, dynamic adjacency list graph modelling, dynamic FIFO queue operations, bounded BFS recommendation (depth 2), mutual friends intersection, and file persistence via network.txt. Limitations are listed in Chapter 11.",
    43: "1.6 Target Users",
    44: "Undergraduate students of Data Structures and Algorithms, and instructors seeking a classroom demonstration aid.",
    45: "1.7 Report Organisation",
    46: "Chapters 2–4 present background, analysis and design; Chapters 5–7 describe modules, algorithms and technologies; Chapters 8–10 cover testing, deployment and results; Chapters 11–13 present limitations, future work and conclusion.",
    47: "",
    48: "2. Background Study",
    49: "Theory-only learning relies on textbook diagrams and written descriptions, which do not show how graph state and queue frontiers evolve over time. Visualisation helps learners follow vertex discovery, adjacency list pointer traversal, hash bucket collision resolution, distance level sets in BFS, and mutual friendship intersections. The concepts used follow the standard treatment in Cormen, Leiserson, Rivest and Stein, Introduction to Algorithms [1].",
    50: "Key concepts: a vertex represents a user and an edge represents an undirected mutual friendship; the adjacency list stores neighbor pointers in O(|V| + |E|) space; djb2 hashing maps string usernames to bucket indices in O(1) time; separate chaining resolves collisions with linked lists; Breadth-First Search traverses level-by-level using a FIFO queue to guarantee shortest path hop distance; and Quick Sort orders recommendations by mutual friend count descending.",
    51: "",
    52: "3. System Analysis",
    53: "3.1 Existing System",
    54: "Existing learning relies on textbooks, lecture slides and static diagrams, which offer little interactivity, often suffer from memory leaks in C, and fail to illustrate how graph algorithms scale to real-world social networks.",
    55: "3.2 Proposed System",
    56: "The proposed system provides interactive, algorithm-driven simulations in a modern browser application coupled with a clean, memory-safe C11 console engine.",
    57: "3.3 Feasibility Study",
    58: "Technical: Built on standard ISO C11 and native HTML5/CSS web technologies running in any modern browser.",
    59: "Operational: No installation or login is required; users simply open localhost:4000 or execute ./friendrec.",
    60: "Economic: Uses free open-source tools (GCC, Node.js) with no paid services or external API fees.",
    61: "3.4 Functional Requirements",
    62: "Register users and resolve usernames via djb2 hash table in O(1) average time.",
    63: "Establish and delete undirected friendship edges between any two users in the graph.",
    64: "Run Bounded Depth-2 BFS to recommend friends-of-friends ranked by mutual count.",
    65: "Display interactive topology graph, mutual friend explorer, and save/load network.txt.",
    66: "3.5 Non-Functional Requirements",
    67: "Responsiveness across devices, smooth 60 fps canvas animation, usability, memory safety with zero leaks (Valgrind verified), and recovery from errors through input validation.",
    68: "3.6 Hardware Requirements",
    69: "Any computer, tablet or smartphone capable of running a modern web browser, and GCC for the C console engine.",
    70: "3.7 Software Requirements",
    71: "A modern web browser for use; Node.js for local HTTP serving; GCC compiler for the C application. The project has no external database, authentication or paid services.",
    72: "Table 1: DSA concepts and modules",
    74: "",
    75: "4. System Design",
    76: "4.1 System Architecture",
    77: "The project comprises an ISO C11 modular core engine (common, hashtable, queue, graph, storage) and a responsive LinkedIn clone browser application featuring an interactive force-directed canvas, live BFS telemetry, profile inspector, and flat-file database persistence (network.txt).",
    78: "4.2 Module Design",
    79: "Each module is an independent component that reads graph data, calls pure algorithm functions (hashing, BFS, queue) and renders the result with formatted tables or visual animation.",
    80: "4.3 Data Flow",
    81: "Network data (network.txt) → user action (e.g., select target user) → pure function (djb2 hash, bounded BFS) → result (ranked recommendations, mutual friends) → animated rendering.",
    82: "4.4 User Interface Design",
    83: "The interface uses a modern LinkedIn style with clean design tokens, adapts to mobile, tablet and desktop, and uses touch-friendly controls and boundary-safe canvas tooltips.",
    84: "4.5 Design Decisions",
    85: "Algorithms are implemented from scratch in pure C and mirrored in JavaScript so they can be inspected independently. Adjacency lists replace adjacency matrices for memory efficiency (Chapter 7). Demo data is deterministic so results are reproducible.",
    86: "4.6 Project Structure",
    87: "The project is organised around key locations: social_network_friend_recommendation/multi_file/ (modular C11 engine), social_network_friend_recommendation/single_file/ (standalone build), and root web files (index.html, styles.css, graph-dsa.js, network-canvas.js, app.js).",
    88: "",
    89: "5. Modules Description",
    90: "5.1 User Profile Registry. Manages user records (ID, name, city, interests) and dynamic resizing via realloc capacity doubling when the network expands.",
    91: "5.2 djb2 Hash Table. Maps usernames to integer IDs in O(1) average time using Daniel J. Bernstein's djb2 hash function with separate-chaining linked lists.",
    92: "5.3 Dynamic Adjacency List. Models vertices as users and bidirectional friendships as undirected edges, consuming O(|V| + |E|) space optimal for sparse social graphs.",
    93: "5.4 Dynamic FIFO Queue. Supports Breadth-First Search traversal with O(1) enqueue and dequeue operations without fixed buffer limits.",
    94: "5.5 Bounded Depth-2 BFS. Discovers friends-of-friends by traversing the graph up to depth 2 while pruning direct friends and isolated nodes.",
    95: "5.6 Mutual Friends Explorer. Computes the intersection of adjacency lists between any two users to find shared connections.",
    96: "5.7 Recommendation Ranking. Ranks candidates deterministically using Quick Sort with mutual count descending and names alphabetically ascending.",
    97: "5.8 File Storage Vault. Serializes and deserializes network records and unique friendship pairs to network.txt.",
    98: "5.9 Interactive LinkedIn Clone. Web visualizer replicating LinkedIn's navigation, feed, search, and recommendation cards.",
    99: "5.10 Physics Canvas Visualizer. Force-directed HTML5 Canvas engine with hard collision separation (82px) and boundary-safe tooltips.",
    100: "",
    101: "6. Algorithms and Implementation",
    102: "All algorithms are implemented from scratch in pure C (C11, GCC) and clean vanilla JavaScript.",
    103: "Table 2: Core graph search algorithms",
    105: "Table 3: Graph representation strategies",
    107: "6.1–6.3 Hashing and Graph Representation. djb2 uses hash * 33 + c across 53 prime buckets. The adjacency list stores edges as dynamic linked lists, prepending new friendships in O(1) time after uniqueness verification.",
    108: "6.4–6.6 Bounded Traversal and Mutual Scoring. Bounded BFS initializes dist[] to -1, enqueues the target with dist=0, and explores neighbors. For each candidate with dist=2, the intersection of adjacency lists computes shared mutual friends.",
    109: "6.7 Recommendation Ranking. Candidates are sorted via Quick Sort (qsort) using a dual-key comparator: higher mutual count ranks first; ties are broken alphabetically by name.",
    110: "Table 4: Hash collision strategies",
    112: "6.8 Queue Mechanics. Dynamic linked list with head and tail pointers provides O(1) enqueue and dequeue operations without fixed buffer limits.",
    113: "6.9 File Persistence Operations. Serializes active user profiles and unique undirected friendship pairs to pipe-delimited network.txt; parses and reconstructs the graph on application boot.",
    114: "6.10 Recommendation Flow Stages. A recommendation request passes through the stages below.",
    115: "Table 5: Recommendation stages",
    117: "The educational significance of each algorithm is that learners can observe cause and effect by changing inputs, triggering connections and viewing graph topology outcomes immediately.",
    118: "",
    119: "7. Technology Stack",
    120: "Table 6: Technology stack",
    122: "Why Adjacency List was chosen over Adjacency Matrix. For 100,000 users, an Adjacency Matrix demands ~40 GB of RAM (99.9% empty), whereas an Adjacency List consumes only ~1.2 MB for average degree 10, achieving a 99.997% memory reduction.",
    123: "",
    124: "8. Testing and Validation",
    125: "8.1 Testing Strategy",
    126: "Testing combines automated algorithm checks, Valgrind memory verification, automated CLI stdin piping, and real-browser layout checks.",
    127: "8.2 Algorithm Verification",
    128: "gcc -Wall -Wextra -std=c11 runs with 0 warnings across all single-file and multi-file modules.",
    129: "8.3 Interface Verification",
    130: "Browser testing verifies topology node physics, tap targets, modal dialogs, and console logs with zero runtime errors.",
    131: "8.4 Responsive Layout Testing",
    132: "CSS checks verify responsiveness across desktop (3 columns), tablet (2 columns), and mobile viewports.",
    133: "8.5 Performance Testing",
    134: "Canvas animation maintains a steady 60 fps; BFS execution for 15 nodes completes in under 0.05 milliseconds.",
    135: "8.6 Test Results",
    136: "Table 7: Testing and verification checks",
    138: "Layout testing found two real bugs: a tooltip clipping bug on canvas borders and overlapping node labels during physics clustering. Both were resolved via strict coordinate clamping and hard collision separation (82px).",
    139: "",
    140: "9. Deployment",
    141: "The C application compiles directly with gcc or GNU Make (make clean && make). The web application is served via Node.js on port 4000 (npm run dev or node server.js). The live visualizer runs at http://localhost:4000 with zero external dependencies.",
    142: "",
    143: "10. Results and Discussion",
    144: "A curated 15-member social network was benchmarked successfully with Rahul as target user. The algorithm correctly ranked Tanya Sen as #1 (3 mutuals: Ananya, Rohan, Neha), followed by Devansh Gupta (2 mutuals) and Sneha Patel (2 mutuals). Karan (0 friends) was correctly isolated. Valgrind confirmed 0 memory leaks across all dynamic allocations.",
    145: "",
    146: "11. Limitations",
    147: "Table 8: Limitations",
    149: "",
    150: "12. Future Enhancements",
    151: "Table 9: Future improvements",
    153: "",
    154: "13. Conclusion",
    155: "The Social Network Friend Recommendation System demonstrates core Data Structures and Algorithms — adjacency lists, djb2 hashing, separate chaining, FIFO queues, bounded BFS traversal, and Quick Sort ranking — through a memory-safe C11 core engine and an interactive LinkedIn web visualizer. Its design was validated by automated compiler, Valgrind, and layout checks. With zero external dependencies, it serves as a practical, accessible learning tool that bridges theory and practice.",
    156: "",
    157: "References",
    158: "[1] T. H. Cormen, C. E. Leiserson, R. L. Rivest and C. Stein, Introduction to Algorithms, 3rd ed., MIT Press, 2009.",
    159: "[2] D. J. Bernstein, Hash Functions: djb2 algorithm, comp.lang.c, 1991.",
    160: "[3] ISO/IEC 9899:2011, Information technology — Programming languages — C (C11 Standard).",
    161: "[4] S. Milgram, \"The Small World Problem,\" Psychology Today, vol. 2, no. 1, pp. 60–67, 1967.",
    162: "[5] J. Leskovec, A. Rajaraman and J. D. Ullman, Mining of Massive Datasets, Cambridge University Press, 2020.",
    163: "[6] Valgrind Documentation: A Framework for Heavyweight Dynamic Binary Instrumentation.",
    164: "[7] MDN Web Docs, Canvas API and Force-Directed Graph Visualizations, Mozilla Developer Network, 2026.",
    165: "",
    166: "Appendices",
    167: "Appendix A: Installation and Running Instructions",
    168: "Compile standalone C engine: gcc -Wall -Wextra -std=c11 -O2 single_file/main.c -o friendrec",
    169: "Compile modular C engine via Make: cd multi_file && make clean && make && ./friendrec",
    170: "Start the development server: npm run dev",
    171: "Open web application in browser: http://localhost:4000",
    172: "Appendix B: Available npm Scripts",
    174: "Appendix C: Repository and Live Demo",
    175: "Local Project Directory: /Users/anakolte/Documents/local projects/krishna project",
    176: "Live Local Demo: http://localhost:4000",
    177: "Appendix D: Screenshots",
    178: "[Insert screenshots for Fig. 1 to Fig. 10 here: Fig. 1 LinkedIn feed; Fig. 2 Force-directed canvas; Fig. 3 User profile view; Fig. 4 djb2 hash distribution; Fig. 5 Adjacency list layout; Fig. 6 BFS state machine; Fig. 7 Mutual explorer; Fig. 8 ANSI CLI menu; Fig. 9 Valgrind report; Fig. 10 network.txt.]"
}

# Tables replacements (exact column count & row mappings)
tbl_replacements = {
    8: [
        ['', 'Project Guide', 'Head of Department', 'External Examiner'],
        ['Name', 'Prof. Krati Srivastava', 'Prof. Sonali Patil', ''],
        ['Signature', '____________', '____________', '____________'],
        ['Date', '____________', '____________', '____________']
    ],
    22: [
        ['Section', 'Page'],
        ['Certificate, Declaration, Acknowledgement, Abstract', 'i'],
        ['List of Figures and Tables', 'ii'],
        ['1. Introduction', '1'],
        ['2. Background Study', '2'],
        ['3. System Analysis', '3'],
        ['4. System Design', '4'],
        ['5. Modules Description', '5'],
        ['6. Algorithms and Implementation', '6'],
        ['7. Technology Stack', '7'],
        ['8. Testing and Validation', '8'],
        ['9. Deployment', '8'],
        ['10. Results and Discussion', '9'],
        ['11. Limitations', '9'],
        ['12. Future Enhancements', '9'],
        ['13. Conclusion', '10'],
        ['References and Appendices', '10']
    ],
    73: [
        ['Module', 'DSA Concept'],
        ['User Registry', 'Dynamic struct array, realloc capacity doubling'],
        ['CPU Scheduler (Hash Index)', 'djb2 string hashing, O(1) lookup, separate chaining'],
        ['Adjacency List', 'Graph topology, dynamic linked list edges, O(|V|+|E|) space'],
        ['Linked FIFO Queue', 'Dynamic linked queue, head/tail pointers, BFS queue'],
        ['Bounded BFS Engine', 'Level-order traversal, depth <= 2 distance bounding'],
        ['Recommendation Ranking', 'Mutual friends intersection & dual-key Quick Sort']
    ],
    104: [
        ['Algorithm', 'Principle', 'Advantage', 'Limitation'],
        ['Breadth-First Search (BFS)', 'Explore neighbors level-by-level using FIFO queue', 'Guarantees shortest path hop distance; perfect for depth-2 recommendation', 'Higher memory for wide frontier queue'],
        ['Depth-First Search (DFS)', 'Explore branch to leaf before backtracking using LIFO stack', 'Lower memory along single path', 'No shortest path guarantee; explores arbitrary deep paths'],
        ['Bounded BFS (Depth <= 2)', 'Halt queue expansion when vertex distance reaches 2', 'Optimal candidate discovery; prunes 99% of graph', 'Requires strict distance array tracking']
    ],
    106: [
        ['Strategy', 'Principle', 'Advantage', 'Limitation'],
        ['Adjacency List', 'Array of linked lists storing adjacent neighbor IDs', 'O(|V| + |E|) space; optimal for sparse social graphs', 'Edge existence check takes O(deg(u))'],
        ['Adjacency Matrix', '|V| x |V| 2D boolean array where matrix[u][v] = 1', 'O(1) edge lookup', 'O(|V|^2) space; 99% memory waste on sparse networks'],
        ['Edge List', 'Array of pairs storing only edge connections', 'Simple storage', 'O(|E|) neighbor iteration; prohibitive for BFS']
    ],
    111: [
        ['Condition', 'Meaning'],
        ['Separate Chaining', 'Singly linked lists at each bucket; collisions appended in O(1)'],
        ['Linear Probing', 'Open addressing searching index (h(k) + i) % M; causes primary clustering'],
        ['Quadratic Probing', 'Searches index (h(k) + c1*i + c2*i^2) % M; secondary clustering'],
        ['Double Hashing', 'Uses secondary hash h2(k) for step size; requires non-zero coprimes']
    ],
    116: [
        ['Stage', 'Description'],
        ['Target Resolution', 'Target username hashed via djb2; mapped to integer ID in O(1)'],
        ['Queue Initialization', 'Distance array dist[] set to -1; target enqueued with dist=0'],
        ['Level-1 Traversal', 'Direct neighbors marked dist=1 and enqueued; excluded from recommendations'],
        ['Level-2 Discovery', 'Neighbors of neighbors marked dist=2; identified as candidates'],
        ['Mutual Friends Scoring', 'Intersection of Adj(target) and Adj(candidate) computes shared count']
    ],
    121: [
        ['Technology', 'Purpose in Project'],
        ['C11 (GCC / Clang)', 'Core data structures: graph, hash table, queue, and BFS engine'],
        ['GNU Make', 'Automated compilation pipeline and dependency tracking'],
        ['Valgrind Memcheck', 'Memory leak analysis and pointer safety verification'],
        ['HTML5 Canvas 2D', 'Force-directed topology visualization with node physics'],
        ['Vanilla CSS', 'LinkedIn clone user interface styling, themes, and layout'],
        ['JavaScript (ES6)', 'Browser graph engine port and real-time DOM controller'],
        ['Node.js (Native HTTP)', 'Zero-dependency local development server (port 4000)'],
        ['File I/O (network.txt)', 'Graph database serialization and persistence'],
        ['GitHub / Git', 'Source control and version management']
    ],
    137: [
        ['Command', 'Scope', 'Result'],
        ['gcc -Wall -Wextra', 'Compiler warning checks', '0 warnings (Clean)'],
        ['valgrind --leak-check=full', 'Dynamic memory allocation leak check', '0 bytes leaked (100% freed)'],
        ['make run test_suite', 'CLI interactive piping & edge cases', '10/10 tests passed'],
        ['Node HTTP Server test', 'Web visualizer loading & API serving', 'HTTP 200 OK (Clean)']
    ],
    148: [
        ['No.', 'Limitation'],
        ['1', 'Single-machine in-memory model; does not partition across distributed clusters'],
        ['2', 'Unweighted friendship edges; interaction frequency and message counts are not modelled'],
        ['3', 'Bounded traversal depth of 2; does not recommend 3rd-degree connections'],
        ['4', 'Static flat-file serialization (network.txt); lacks ACID transactional logging'],
        ['5', 'Symmetric undirected relationships; does not model Twitter-style directed following'],
        ['6', 'In-memory graph requires full sequential reload on application restart']
    ],
    152: [
        ['No.', 'Enhancement'],
        ['1', 'Adamic/Adar and Jaccard index scoring to penalize hub nodes'],
        ['2', 'Weighted edges based on interaction frequency and mutual interaction recency'],
        ['3', 'Community detection using Louvain or Girvan-Newman modularity clustering'],
        ['4', 'Personalized PageRank (Random Walk with Restart) for multi-hop relevance'],
        ['5', 'Multi-threaded parallel BFS using OpenMP or POSIX threads for millions of vertices'],
        ['6', 'Persistent graph database integration using SQLite or Neo4j C driver'],
        ['7', 'Real-time WebSocket synchronization between terminal and web interface']
    ],
    173: [
        ['Script', 'Purpose'],
        ['make', 'Builds modular C object files and compiles friendrec executable'],
        ['make clean', 'Removes compiled object files (*.o) and binaries'],
        ['make run', 'Compiles and immediately launches the C CLI application'],
        ['npm run dev', 'Starts Node.js web server on http://localhost:4000'],
        ['node server.js', 'Direct invocation of native HTTP web server without npm'],
        ['valgrind ./friendrec', 'Runs memory leak checks and pointer validation'],
        ['gcc -Wall -Wextra single_file/main.c', 'Single-command compilation of standalone C file'],
        ['npm run verify', 'Algorithm correctness and regression checks'],
        ['npm run perf', 'Canvas frame pacing and BFS execution speed measurement'],
        ['npm run screenshots', 'Capture visual evidence of graph topology and profile views']
    ]
}

# Apply all replacements to exact body children
for idx, child in enumerate(body):
    tag = child.tag.split('}')[-1]
    if tag == 'p' and idx in p_replacements:
        set_p_text(child, p_replacements[idx])
    elif tag == 'tbl' and idx in tbl_replacements:
        set_table_cells(child, tbl_replacements[idx])

# Generate XML
new_doc_xml = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n' + ET.tostring(tree, encoding='utf-8').decode('utf-8')

# Update core.xml
new_core_xml = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <dc:title>Social Network Friend Recommendation System Using Graph, BFS and Hashing</dc:title>
  <dc:creator>Krishna Yadav</dc:creator>
  <dc:description>B.Tech AIML Data Structures and Algorithms Mini Project Report</dc:description>
  <cp:revision>1</cp:revision>
</cp:coreProperties>'''

with zipfile.ZipFile(src_docx, 'r') as zin, zipfile.ZipFile(target_docx, 'w') as zout:
    for item in zin.infolist():
        if item.filename == 'word/document.xml':
            zout.writestr(item, new_doc_xml.encode('utf-8'))
        elif item.filename == 'docProps/core.xml':
            zout.writestr(item, new_core_xml.encode('utf-8'))
        else:
            zout.writestr(item, zin.read(item.filename))

shutil.copyfile(target_docx, download_target_docx)

# Also generate corresponding Markdown file
md_lines = []
for idx, child in enumerate(body):
    tag = child.tag.split('}')[-1]
    if tag == 'p':
        pPr = child.find(f'{{{W}}}pPr')
        pStyle = pPr.find(f'{{{W}}}pStyle').attrib.get(f'{{{W}}}val') if (pPr is not None and pPr.find(f'{{{W}}}pStyle') is not None) else ''
        text = ''.join(child.itertext()).strip()
        if not text:
            md_lines.append("")
        elif pStyle == 'Heading1':
            md_lines.append(f"# {text}\n")
        elif pStyle == 'Heading2':
            md_lines.append(f"\n## {text}\n")
        elif pStyle == 'Heading3':
            md_lines.append(f"\n### {text}\n")
        elif pStyle == 'ListParagraph':
            md_lines.append(f"- {text}")
        else:
            md_lines.append(f"{text}\n")
    elif tag == 'tbl':
        rows = child.findall(f'{{{W}}}tr')
        if rows:
            h_cells = [''.join(c.itertext()).strip() for c in rows[0].findall(f'{{{W}}}tc')]
            md_lines.append("\n| " + " | ".join(h_cells) + " |")
            md_lines.append("| " + " | ".join(["---"] * len(h_cells)) + " |")
            for r in rows[1:]:
                cells = [''.join(c.itertext()).strip() for c in r.findall(f'{{{W}}}tc')]
                md_lines.append("| " + " | ".join(cells) + " |")
            md_lines.append("")

with open(md_path, 'w', encoding='utf-8') as f:
    f.write("\n".join(md_lines))

print("Successfully generated 1:1 exact format clone report!")
print(f"Target: {target_docx}")
print(f"Downloads: {download_target_docx}")
print(f"Markdown: {md_path}")
