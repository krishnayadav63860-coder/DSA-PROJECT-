/**
 * Graph, BFS and djb2 Hash Table Data Structures for Friend Recommendation
 * Direct client-side implementation mirroring the C11 DSA Project
 * Curated 15 realistic LinkedIn profiles with clear, legible graph topology.
 */

// djb2 string hash algorithm with case-insensitivity
function hashDjb2(str, capacity) {
  let hash = 5381;
  const lower = str.toLowerCase();
  for (let i = 0; i < lower.length; i++) {
    const c = lower.charCodeAt(i);
    // hash * 33 + c
    hash = (((hash << 5) + hash) + c) >>> 0;
  }
  return hash % capacity;
}

// Separate Chaining Hash Table (Username -> User ID)
class HashTable {
  constructor(capacity = 37) {
    this.capacity = capacity;
    this.buckets = Array.from({ length: capacity }, () => []);
    this.size = 0;
  }

  insert(key, userId) {
    if (!key) return false;
    const cleanKey = key.trim();
    if (this.search(cleanKey) !== -1) {
      return false; // Duplicate key rejected
    }
    const index = hashDjb2(cleanKey, this.capacity);
    this.buckets[index].push({ key: cleanKey, userId });
    this.size++;
    return true;
  }

  search(key) {
    if (!key) return -1;
    const cleanKey = key.trim().toLowerCase();
    const index = hashDjb2(cleanKey, this.capacity);
    const chain = this.buckets[index];
    for (const node of chain) {
      if (node.key.toLowerCase() === cleanKey) {
        return node.userId;
      }
    }
    return -1; // Not found
  }

  delete(key) {
    if (!key) return false;
    const cleanKey = key.trim().toLowerCase();
    const index = hashDjb2(cleanKey, this.capacity);
    const chain = this.buckets[index];
    const idx = chain.findIndex(n => n.key.toLowerCase() === cleanKey);
    if (idx !== -1) {
      chain.splice(idx, 1);
      this.size--;
      return true;
    }
    return false;
  }

  clear() {
    this.buckets = Array.from({ length: this.capacity }, () => []);
    this.size = 0;
  }

  inspect() {
    return this.buckets.map((chain, index) => ({
      index,
      chainLength: chain.length,
      entries: chain.map(e => `${e.key} (ID: ${e.userId})`)
    })).filter(b => b.chainLength > 0);
  }
}

// FIFO Queue for BFS traversal
class Queue {
  constructor() {
    this.items = [];
  }
  enqueue(element) {
    this.items.push(element);
  }
  dequeue() {
    return this.items.length === 0 ? null : this.items.shift();
  }
  isEmpty() {
    return this.items.length === 0;
  }
  size() {
    return this.items.length;
  }
  peek() {
    return this.items.length === 0 ? null : this.items[0];
  }
}

// Dynamic Social Graph with Adjacency List
class SocialGraph {
  constructor() {
    this.users = [];
    this.adjLists = [];
    this.userIndex = new HashTable(37);
  }

  registerUser({ name, headline, city, company, avatar, cover, interests, about, experiences, education, skills }) {
    if (!name || !name.trim()) return -1;
    const cleanName = name.trim();
    if (this.userIndex.search(cleanName) !== -1) {
      return -2; // Duplicate
    }

    const id = this.users.length;
    const user = {
      id,
      name: cleanName,
      headline: headline || "Professional | Network Member",
      city: city || "Bangalore, India",
      company: company || "Independent",
      avatar: avatar || `https://api.dicebear.com/7.x/avataaars/svg?seed=${encodeURIComponent(cleanName)}`,
      cover: cover || "linear-gradient(135deg, #0a66c2 0%, #004182 100%)",
      interests: interests || "Technology, Networking, Algorithms",
      about: about || `Experienced professional in ${interests || 'technology'}. Dedicated to solving complex problems through efficient algorithms, robust system architectures, and team collaboration.`,
      experiences: experiences || [
        { role: headline ? headline.split('|')[0].trim() : "Software Engineer", company: company || "Tech Solutions", duration: "2023 - Present • 3 yrs", location: city || "India", description: "Architecting high-scale distributed backend systems and graph analytics workflows." }
      ],
      education: education || [
        { school: "Indian Institute of Technology", degree: "B.Tech in Computer Science / AIML", year: "2020 - 2024" }
      ],
      skills: skills || ["Data Structures", "Algorithms", "Graph Theory", "Python", "C++", "System Design"],
      isDeleted: false
    };

    this.users.push(user);
    this.adjLists.push([]);
    this.userIndex.insert(cleanName, id);
    return id;
  }

  hasFriendship(u, v) {
    if (u < 0 || u >= this.users.length || v < 0 || v >= this.users.length) return false;
    if (this.users[u].isDeleted || this.users[v].isDeleted) return false;
    return this.adjLists[u].includes(v);
  }

  addFriendship(u, v) {
    if (u === v) return false;
    if (u < 0 || u >= this.users.length || v < 0 || v >= this.users.length) return false;
    if (this.users[u].isDeleted || this.users[v].isDeleted) return false;
    if (this.hasFriendship(u, v)) return false;

    // Undirected edge
    this.adjLists[u].push(v);
    this.adjLists[v].push(u);
    return true;
  }

  removeFriendship(u, v) {
    if (u < 0 || u >= this.users.length || v < 0 || v >= this.users.length) return false;
    const idxU = this.adjLists[u].indexOf(v);
    const idxV = this.adjLists[v].indexOf(u);
    if (idxU !== -1) this.adjLists[u].splice(idxU, 1);
    if (idxV !== -1) this.adjLists[v].splice(idxV, 1);
    return idxU !== -1 && idxV !== -1;
  }

  deleteUser(userId) {
    if (userId < 0 || userId >= this.users.length || this.users[userId].isDeleted) return false;

    for (const friendId of [...this.adjLists[userId]]) {
      this.removeFriendship(userId, friendId);
    }
    this.adjLists[userId] = [];
    this.userIndex.delete(this.users[userId].name);
    this.users[userId].isDeleted = true;
    return true;
  }

  getUserId(name) {
    return this.userIndex.search(name);
  }

  getMutualFriends(u, v) {
    if (u < 0 || u >= this.users.length || v < 0 || v >= this.users.length) return [];
    if (this.users[u].isDeleted || this.users[v].isDeleted) return [];

    const neighborsU = this.adjLists[u].filter(id => !this.users[id].isDeleted);
    const neighborsV = new Set(this.adjLists[v].filter(id => !this.users[id].isDeleted));
    return neighborsU.filter(id => neighborsV.has(id));
  }

  /**
   * Breadth-First Search (BFS) Friend Recommendation Engine:
   * Traverses social graph up to Depth 2 from target user.
   * Finds friends-of-friends with mutual connections.
   * Sorts descending by mutualCount, tie-breaking alphabetically.
   */
  recommendFriends(userId) {
    if (userId < 0 || userId >= this.users.length || this.users[userId].isDeleted) {
      return { recommendations: [], distances: [], bfsTrace: [] };
    }

    const n = this.users.length;
    const dist = Array(n).fill(-1);
    const bfsTrace = [];
    const q = new Queue();

    dist[userId] = 0;
    q.enqueue(userId);
    bfsTrace.push({ step: `Queue initialized with target user (${this.users[userId].name})`, queue: [userId], current: userId, depth: 0 });

    while (!q.isEmpty()) {
      const u = q.dequeue();
      if (dist[u] >= 2) continue;

      for (const v of this.adjLists[u]) {
        if (!this.users[v].isDeleted && dist[v] === -1) {
          dist[v] = dist[u] + 1;
          if (dist[v] <= 2) {
            q.enqueue(v);
            bfsTrace.push({
              step: `Visited ${this.users[v].name} via ${this.users[u].name} (Distance: ${dist[v]})`,
              queue: [...q.items],
              current: v,
              depth: dist[v]
            });
          }
        }
      }
    }

    const recommendations = [];
    for (let candidate = 0; candidate < n; candidate++) {
      if (candidate !== userId && !this.users[candidate].isDeleted && dist[candidate] === 2) {
        const mutualIds = this.getMutualFriends(userId, candidate);
        if (mutualIds.length >= 1) {
          recommendations.push({
            candidateId: candidate,
            candidate: this.users[candidate],
            mutualCount: mutualIds.length,
            mutualFriends: mutualIds.map(id => this.users[id]),
            distance: 2
          });
        }
      }
    }

    // Sort: mutualCount descending, then candidate name ascending
    recommendations.sort((a, b) => {
      if (b.mutualCount !== a.mutualCount) {
        return b.mutualCount - a.mutualCount;
      }
      return a.candidate.name.localeCompare(b.candidate.name, undefined, { sensitivity: 'base' });
    });

    return {
      recommendations,
      distances: dist,
      bfsTrace
    };
  }

  getNetworkStats() {
    const active = this.users.filter(u => !u.isDeleted);
    let totalEdges = 0;
    let maxDegree = -1;
    let mostConnected = null;

    active.forEach(u => {
      const degree = this.adjLists[u.id].filter(id => !this.users[id].isDeleted).length;
      totalEdges += degree;
      if (degree > maxDegree) {
        maxDegree = degree;
        mostConnected = u;
      }
    });

    const totalFriendships = Math.floor(totalEdges / 2);
    const avgFriends = active.length > 0 ? (2 * totalFriendships / active.length).toFixed(2) : "0.00";
    const density = active.length > 1
      ? ((2 * totalFriendships) / (active.length * (active.length - 1))).toFixed(4)
      : "0.0000";

    return {
      totalUsers: active.length,
      totalFriendships,
      avgFriends,
      density,
      densityPercent: (parseFloat(density) * 100).toFixed(1),
      mostConnectedUser: mostConnected ? `${mostConnected.name} (${maxDegree} connections)` : "None"
    };
  }

  /**
   * Load Curated 15 Demo People with Authentic Profiles & Clean Topology
   */
  loadDemoData() {
    this.users = [];
    this.adjLists = [];
    this.userIndex.clear();

    const people15 = [
      {
        name: "Rahul",
        headline: "AI & Machine Learning Researcher | B.Tech AIML @ DTU",
        city: "Delhi, India",
        company: "Google AI Student Lab",
        avatar: "https://images.unsplash.com/photo-1534528741775-53994a69daeb?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #0a66c2 0%, #004182 100%)",
        interests: "Artificial Intelligence, Graph Neural Networks, Data Structures & Algorithms",
        about: "Pre-final year B.Tech AIML student researching social network topologies, BFS recommendation heuristics, and Graph Convolutional Networks. Recipient of Dean's Academic Excellence Award 2025.",
        experiences: [
          { role: "AI Research Fellow", company: "Google AI Student Lab", duration: "May 2024 - Present • 1 yr", location: "Delhi, India", description: "Developing topological graph traversal algorithms for low-latency recommendation engines." },
          { role: "DSA Teaching Assistant", company: "DTU Department of CSE", duration: "Jan 2024 - May 2024 • 5 mos", location: "Delhi, India", description: "Mentored 120+ undergraduates in dynamic memory management, BFS/DFS, and hashing in C." }
        ],
        education: [
          { school: "Delhi Technological University (DTU)", degree: "B.Tech in Artificial Intelligence & Machine Learning", year: "2022 - 2026 • CGPA: 9.4/10" }
        ],
        skills: ["Graph Theory", "BFS / DFS Traversal", "C11 & GCC", "Python & PyTorch", "djb2 Hashing", "Algorithms Design"]
      },
      {
        name: "Amit",
        headline: "Full-Stack Software Engineer @ Tech Mahindra | Ex-Amazon Intern",
        city: "Bangalore, India",
        company: "Tech Mahindra",
        avatar: "https://images.unsplash.com/photo-1507003211169-0a1dd7228f2d?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #0077b5 0%, #00a0dc 100%)",
        interests: "Distributed Systems, Next.js, Cloud Native Architectures",
        about: "Passionate engineer scaling real-time distributed web systems. Love graph-driven APIs and microservices architectures.",
        experiences: [
          { role: "Software Development Engineer", company: "Tech Mahindra", duration: "2023 - Present • 2 yrs", location: "Bangalore, India", description: "Building scalable distributed services handling 5M+ daily requests." }
        ],
        education: [
          { school: "IIIT Bangalore", degree: "B.Tech in Computer Science and Engineering", year: "2019 - 2023" }
        ],
        skills: ["Next.js", "C++", "Java", "Distributed Systems", "PostgreSQL", "Docker"]
      },
      {
        name: "Priya",
        headline: "Data Science Specialist @ Flipkart Data Labs | MLOps Enthusiast",
        city: "Mumbai, India",
        company: "Flipkart",
        avatar: "https://images.unsplash.com/photo-1494790108377-be9c29b29330?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #833ab4 0%, #fd1d1d 50%, #fcb045 100%)",
        interests: "Predictive Analytics, Collaborative Filtering, BigQuery ML",
        about: "Leading recommendation and personal ranking pipelines at Flipkart. Working at the intersection of Big Data and user engagement.",
        experiences: [
          { role: "Senior Data Scientist", company: "Flipkart", duration: "2022 - Present • 3 yrs", location: "Mumbai, India", description: "Trained hybrid collaborative filtering models driving a 14% lift in click-through rates." }
        ],
        education: [
          { school: "IIT Bombay", degree: "M.Tech in Data Science & Artificial Intelligence", year: "2020 - 2022" }
        ],
        skills: ["Machine Learning", "Graph Analytics", "Python", "Apache Spark", "TensorFlow"]
      },
      {
        name: "Neha",
        headline: "Lead Product Designer & Design Systems Advocate @ Figma Community",
        city: "Delhi, India",
        company: "Figma Community",
        avatar: "https://images.unsplash.com/photo-1517841905240-472988babdf9?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #11998e 0%, #38ef7d 100%)",
        interests: "Human-Computer Interaction, Design Systems, UX Metrics",
        about: "Crafting intuitive digital experiences that balance user delight with clean engineering ergonomics.",
        experiences: [
          { role: "Lead Product Designer", company: "Independent Design Lab", duration: "2021 - Present • 4 yrs", location: "Delhi, India", description: "Created universal design systems adopted by 50,000+ developers globally." }
        ],
        education: [
          { school: "National Institute of Design (NID)", degree: "Master of Design (M.Des) in Interaction Design", year: "2018 - 2021" }
        ],
        skills: ["Figma", "UI/UX Architecture", "Wireframing", "User Research", "Interaction Design"]
      },
      {
        name: "Karan",
        headline: "Cybersecurity Analyst & Penetration Tester | CEH & OSCP Certified",
        city: "Pune, India",
        company: "CyberSec Global",
        avatar: "https://images.unsplash.com/photo-1500648767791-00dcc994a43e?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #232526 0%, #414345 100%)",
        interests: "Ethical Hacking, Network Security, Cryptography",
        about: "Securing modern cloud networks and enterprise infrastructures against advanced persistent threats. Intentionally unlinked for disconnected vertex demonstration.",
        experiences: [
          { role: "Information Security Analyst", company: "CyberSec Global", duration: "2022 - Present • 3 yrs", location: "Pune, India", description: "Conducted 40+ zero-day vulnerability audits for Fortune 500 financial clients." }
        ],
        education: [
          { school: "COEP Technological University", degree: "B.Tech in Computer Engineering", year: "2018 - 2022" }
        ],
        skills: ["Penetration Testing", "Network Security", "Cryptography", "Linux Hardening", "Wireshark"]
      },
      {
        name: "Rohan Verma",
        headline: "Staff Software Engineer @ Uber | Core Platform Architecture",
        city: "Hyderabad, India",
        company: "Uber",
        avatar: "https://images.unsplash.com/photo-1519085360753-af0119f7cbe7?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #000000 0%, #434343 100%)",
        interests: "High Throughput Systems, Kafka, Go, Distributed Graph Engines",
        about: "Architecting Uber's real-time routing graph dispatchers. Experienced in p99 latency optimization.",
        experiences: [
          { role: "Staff Software Engineer", company: "Uber", duration: "2021 - Present • 4 yrs", location: "Hyderabad, India", description: "Scaled driver-rider matching graph algorithms handling 100k events/sec." }
        ],
        education: [{ school: "IIT Kharagpur", degree: "B.Tech in Computer Science", year: "2015 - 2019" }],
        skills: ["Go", "Distributed Systems", "Kafka", "Graph Traversal", "Kubernetes"]
      },
      {
        name: "Ananya Iyer",
        headline: "AI Research Scientist @ Microsoft Research India | NLP & Graph Models",
        city: "Bangalore, India",
        company: "Microsoft Research",
        avatar: "https://images.unsplash.com/photo-1573496359142-b8d87734a5a2?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #008272 0%, #00bcf2 100%)",
        interests: "Graph Neural Networks, Large Language Models, Semantic Parsing",
        about: "Published 12+ papers in NeurIPS, ICML, and ACL on knowledge graphs and reasoning.",
        experiences: [
          { role: "Research Scientist", company: "Microsoft Research", duration: "2022 - Present • 3 yrs", location: "Bangalore, India", description: "Authoring hybrid graph-transformer models for enterprise knowledge extraction." }
        ],
        education: [{ school: "IISc Bangalore", degree: "Ph.D. in Artificial Intelligence", year: "2018 - 2022" }],
        skills: ["PyTorch", "Graph Neural Networks", "Knowledge Graphs", "Python", "Algorithms"]
      },
      {
        name: "Vikramaditya Rao",
        headline: "VP of Engineering @ Zomato | Scaling Consumer Hyperlocal Logistics",
        city: "Gurugram, India",
        company: "Zomato",
        avatar: "https://images.unsplash.com/photo-1472099645785-5658abf4ff4e?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #cb202d 0%, #8b0000 100%)",
        interests: "Engineering Leadership, Hyperlocal Routing, Microservices",
        about: "Passionate engineering leader leading a 150+ developer organization across logistics and payments.",
        experiences: [
          { role: "Vice President of Engineering", company: "Zomato", duration: "2020 - Present • 5 yrs", location: "Gurugram, India", description: "Leading engineering teams powering 3M+ daily meals delivered." }
        ],
        education: [{ school: "BITS Pilani", degree: "B.E. in Computer Science", year: "2010 - 2014" }],
        skills: ["Engineering Leadership", "System Architecture", "Logistics Tech", "Microservices"]
      },
      {
        name: "Sneha Patel",
        headline: "Principal Frontend Architect @ Swiggy | Web Performance & Design Systems",
        city: "Bangalore, India",
        company: "Swiggy",
        avatar: "https://images.unsplash.com/photo-1580489944761-15a19d654956?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #fc8019 0%, #e05e00 100%)",
        interests: "React, WebAssembly, Microfrontends, Performance",
        about: "Obsessed with Sub-100ms web experiences and scalable component design systems.",
        experiences: [
          { role: "Principal Frontend Engineer", company: "Swiggy", duration: "2021 - Present • 4 yrs", location: "Bangalore, India", description: "Architected Swiggy Web with sub-second LCP scores across all devices." }
        ],
        education: [{ school: "NIT Trichy", degree: "B.Tech in Computer Science", year: "2015 - 2019" }],
        skills: ["React", "TypeScript", "Performance Tuning", "HTML5 Canvas", "Next.js"]
      },
      {
        name: "Kabir Mehta",
        headline: "Founder & CEO @ NeuralGraph AI | Forbes 30 Under 30",
        city: "Mumbai, India",
        company: "NeuralGraph AI",
        avatar: "https://images.unsplash.com/photo-1506794778202-cad84cf45f1d?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #1f4037 0%, #99f2c8 100%)",
        interests: "Startup Ecosystem, Graph Databases, Enterprise AI",
        about: "Founder building next-generation graph intelligence pipelines for Fortune 500 supply chains.",
        experiences: [
          { role: "Founder & Chief Executive Officer", company: "NeuralGraph AI", duration: "2022 - Present • 3 yrs", location: "Mumbai, India", description: "Raised $6M Series A for enterprise property graph search engines." }
        ],
        education: [{ school: "Stanford University", degree: "M.S. in Computer Science", year: "2019 - 2021" }],
        skills: ["Venture Capital", "Graph Databases", "Product Strategy", "AI Strategy"]
      },
      {
        name: "Pooja Nair",
        headline: "Senior Machine Learning Engineer @ NVIDIA | GPU Accelerated Graphs",
        city: "Pune, India",
        company: "NVIDIA",
        avatar: "https://images.unsplash.com/photo-1544005313-94ddf0286df2?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #76b900 0%, #1a4d00 100%)",
        interests: "CUDA, cuGraph, High Performance Computing, Parallel BFS",
        about: "Speeding up graph traversal algorithms on Tensor Core architectures by 50x.",
        experiences: [{ role: "Senior ML Engineer", company: "NVIDIA", duration: "2022 - Present", location: "Pune, India", description: "Accelerating BFS and PageRank on multi-GPU nodes." }],
        education: [{ school: "IIT Madras", degree: "B.Tech in Electrical & CSE", year: "2017 - 2021" }],
        skills: ["CUDA", "C++17", "Parallel Algorithms", "NVIDIA RAPIDS", "cuGraph"]
      },
      {
        name: "Arjun Kapoor",
        headline: "Distributed Systems Lead @ Google Cloud | Spanner & Bigtable",
        city: "Hyderabad, India",
        company: "Google",
        avatar: "https://images.unsplash.com/photo-1522075469751-3a6694fb2f61?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #4285f4 0%, #0d47a1 100%)",
        interests: "Consensus Algorithms, Raft, Paxos, Globally Distributed Databases",
        about: "Working on planetary scale consistency and fault-tolerant storage primitives at Google Cloud.",
        experiences: [{ role: "Staff Systems Engineer", company: "Google Cloud", duration: "2020 - Present", location: "Hyderabad, India", description: "Core contributor to multi-region consensus engines." }],
        education: [{ school: "IIT Delhi", degree: "B.Tech in Computer Science", year: "2014 - 2018" }],
        skills: ["Distributed Systems", "C++", "Paxos", "Google Cloud", "Distributed Hash Tables"]
      },
      {
        name: "Tanya Sen",
        headline: "Staff Product Manager @ Atlassian | Jira Cloud Ecosystem",
        city: "Bangalore, India",
        company: "Atlassian",
        avatar: "https://images.unsplash.com/photo-1567532939604-b6b5b0db2604?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #0052cc 0%, #2684ff 100%)",
        interests: "Product Discovery, Agile Workflows, User Analytics",
        about: "Defining product strategy for collaborative enterprise workflows reaching 250k+ organizations.",
        experiences: [{ role: "Staff PM", company: "Atlassian", duration: "2021 - Present", location: "Bangalore, India", description: "Leading enterprise developer platform features." }],
        education: [{ school: "IIM Ahmedabad", degree: "MBA", year: "2017 - 2019" }],
        skills: ["Product Management", "Data Analytics", "Strategy", "User Growth"]
      },
      {
        name: "Devansh Gupta",
        headline: "Solutions Architect @ Amazon Web Services (AWS) | Cloud Migration",
        city: "Delhi, India",
        company: "AWS",
        avatar: "https://images.unsplash.com/photo-1501196354995-cbb51c65aaea?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #ff9900 0%, #232f3e 100%)",
        interests: "Serverless, Well-Architected Framework, Terraform",
        about: "Advising enterprise CTOs on migrating mission-critical workloads to cloud infrastructure.",
        experiences: [{ role: "Solutions Architect", company: "AWS", duration: "2022 - Present", location: "Delhi, India", description: "Architected 50+ enterprise cloud adoptions." }],
        education: [{ school: "Thapar Institute", degree: "B.E. in Computer Science", year: "2016 - 2020" }],
        skills: ["AWS", "Cloud Architecture", "Terraform", "Security", "DevOps"]
      },
      {
        name: "Riya Choudhury",
        headline: "Computer Vision Engineer @ Adobe Sensei | Generative Media",
        city: "Noida, India",
        company: "Adobe",
        avatar: "https://images.unsplash.com/photo-1524504388940-b1c1722653e1?w=300&auto=format&fit=crop&q=80",
        cover: "linear-gradient(135deg, #ff0000 0%, #7b0000 100%)",
        interests: "Diffusion Models, Neural Rendering, OpenCV, C++",
        about: "Bringing AI-powered image synthesis and vector segmentation to Adobe Creative Cloud.",
        experiences: [{ role: "Computer Vision Engineer", company: "Adobe", duration: "2023 - Present", location: "Noida, India", description: "Optimizing neural inpainting models for mobile." }],
        education: [{ school: "DTU", degree: "B.Tech in Information Technology", year: "2019 - 2023" }],
        skills: ["Computer Vision", "PyTorch", "C++", "Diffusion Models", "OpenCV"]
      }
    ];

    // Register all 15 members
    people15.forEach(p => {
      this.registerUser(p);
    });

    // Curated Friendships Topology (15 Members):
    // Rahul (ID 0) Direct Connections (Depth 1):
    // Amit (1), Neha (3), Rohan (5), Ananya (6)
    this.addFriendship(0, 1);
    this.addFriendship(0, 3);
    this.addFriendship(0, 5);
    this.addFriendship(0, 6);

    // Friendships connecting Depth 1 to Depth 2 (Generating rich mutual connections for Rahul):
    // Tanya (12) connects to Rohan(5), Ananya(6), Neha(3) -> (3 mutuals with Rahul: Rohan, Ananya, Neha)
    this.addFriendship(12, 5);
    this.addFriendship(12, 6);
    this.addFriendship(12, 3);

    // Sneha (8) connects to Amit(1), Rohan(5) -> (2 mutuals with Rahul: Amit, Rohan)
    this.addFriendship(8, 1);
    this.addFriendship(8, 5);

    // Devansh (13) connects to Amit(1), Ananya(6) -> (2 mutuals with Rahul: Amit, Ananya)
    this.addFriendship(13, 1);
    this.addFriendship(13, 6);

    // Priya (2) connects to Amit(1) -> (1 mutual with Rahul: Amit)
    this.addFriendship(2, 1);

    // Riya (14) connects to Ananya(6) -> (1 mutual with Rahul: Ananya)
    this.addFriendship(14, 6);

    // Additional cross-connections between other members:
    this.addFriendship(1, 3);   // Amit - Neha
    this.addFriendship(5, 7);   // Rohan - Vikramaditya
    this.addFriendship(7, 8);   // Vikramaditya - Sneha
    this.addFriendship(8, 12);  // Sneha - Tanya
    this.addFriendship(9, 10);  // Kabir - Pooja
    this.addFriendship(10, 11); // Pooja - Arjun
    this.addFriendship(11, 9);  // Arjun - Kabir
    this.addFriendship(9, 2);   // Kabir - Priya
    this.addFriendship(13, 11); // Devansh - Arjun

    // Karan (4) is intentionally isolated (0 friendships) for edge-case demonstration!

    return { totalUsers: this.users.length };
  }

  exportToText() {
    const active = this.users.filter(u => !u.isDeleted);
    let output = `# Social Network Friend Recommendation System Database\n`;
    output += `# Generated by LinkedIn Clone Front-End Visualizer (15 Members)\n`;
    output += `USERS_COUNT: ${active.length}\n`;
    active.forEach(u => {
      output += `${u.name}|${u.city}|${u.interests}\n`;
    });

    const uniquePairs = [];
    active.forEach(u => {
      this.adjLists[u.id].forEach(friendId => {
        if (friendId > u.id && !this.users[friendId].isDeleted) {
          uniquePairs.push(`${u.name}|${this.users[friendId].name}`);
        }
      });
    });

    output += `FRIENDSHIPS_COUNT: ${uniquePairs.length}\n`;
    uniquePairs.forEach(pair => {
      output += `${pair}\n`;
    });

    return output;
  }
}

// Universal module exposure
if (typeof window !== 'undefined') {
  window.SocialGraph = SocialGraph;
  window.HashTable = HashTable;
}
if (typeof module !== 'undefined' && module.exports) {
  module.exports = { SocialGraph, HashTable };
}
