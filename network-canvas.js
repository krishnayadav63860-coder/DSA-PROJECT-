/**
 * Interactive Graph Topology Canvas Visualizer
 * Physics-based force-directed layout for Social Network Graph & BFS Traversal
 * Features non-overlapping collision physics, clean labels, and boundary-safe tooltips.
 */

class NetworkGraphCanvas {
  constructor(canvasElement, graphInstance, onSelectUserCallback) {
    this.canvas = canvasElement;
    this.ctx = canvasElement.getContext('2d');
    this.graph = graphInstance;
    this.onSelectUser = onSelectUserCallback;

    this.nodes = [];
    this.activeUserId = 0;
    this.recommendationData = null;

    this.draggedNode = null;
    this.hoveredNode = null;
    this.mouse = { x: 0, y: 0, isDown: false };
    this.dragStartPos = { x: 0, y: 0 };
    this.hasMovedSignificantly = false;

    this.initCanvasSize();
    this.bindEvents();
    this.startSimulation();
  }

  initCanvasSize() {
    const rect = this.canvas.parentElement.getBoundingClientRect();
    this.width = Math.floor(rect.width) || 620;
    this.height = Math.floor(rect.height) || 440;
    const dpr = window.devicePixelRatio || 1;

    this.canvas.width = Math.floor(this.width * dpr);
    this.canvas.height = Math.floor(this.height * dpr);
    this.ctx.resetTransform?.();
    this.ctx.scale(dpr, dpr);
    this.canvas.style.width = `${this.width}px`;
    this.canvas.style.height = `${this.height}px`;
  }

  updateData(activeUserId, recommendationData) {
    this.activeUserId = activeUserId;
    this.recommendationData = recommendationData;

    const activeUsers = this.graph.users.filter(u => !u.isDeleted);
    const existingNodeMap = new Map(this.nodes.map(n => [n.id, n]));
    const distMap = recommendationData?.distances || [];

    const nonTargetUsers = activeUsers.filter(u => u.id !== activeUserId);

    this.nodes = activeUsers.map(user => {
      if (existingNodeMap.has(user.id)) {
        const existing = existingNodeMap.get(user.id);
        existing.user = user;
        return existing;
      }

      // Active user starts at the center
      if (user.id === activeUserId) {
        return {
          id: user.id,
          user,
          x: this.width / 2,
          y: this.height / 2,
          vx: 0,
          vy: 0,
          radius: 26
        };
      }

      // Distribute others radially by BFS distance
      const idx = nonTargetUsers.findIndex(u => u.id === user.id);
      const angle = (idx / Math.max(1, nonTargetUsers.length)) * Math.PI * 2;
      const d = distMap[user.id];

      let ringRadius = Math.min(this.width, this.height) * 0.38;
      if (d === 1) ringRadius = 115;
      else if (d === 2) ringRadius = 175;

      return {
        id: user.id,
        user,
        x: this.width / 2 + Math.cos(angle) * ringRadius + (Math.random() - 0.5) * 15,
        y: this.height / 2 + Math.sin(angle) * ringRadius + (Math.random() - 0.5) * 15,
        vx: 0,
        vy: 0,
        radius: 22
      };
    });
  }

  bindEvents() {
    window.addEventListener('resize', () => this.initCanvasSize());

    const getPos = (e) => {
      const rect = this.canvas.getBoundingClientRect();
      return {
        x: e.clientX - rect.left,
        y: e.clientY - rect.top
      };
    };

    this.canvas.addEventListener('mousedown', (e) => {
      const pos = getPos(e);
      this.mouse.isDown = true;
      this.dragStartPos = { x: pos.x, y: pos.y };
      this.hasMovedSignificantly = false;
      this.draggedNode = this.getNodeAt(pos.x, pos.y);
    });

    window.addEventListener('mousemove', (e) => {
      const pos = getPos(e);
      this.mouse.x = pos.x;
      this.mouse.y = pos.y;

      if (this.mouse.isDown && this.draggedNode) {
        const distMoved = Math.hypot(pos.x - this.dragStartPos.x, pos.y - this.dragStartPos.y);
        if (distMoved > 6) {
          this.hasMovedSignificantly = true;
        }
      }

      if (!this.mouse.isDown) {
        this.hoveredNode = this.getNodeAt(pos.x, pos.y);
        this.canvas.style.cursor = this.hoveredNode ? 'pointer' : 'default';
      }
    });

    window.addEventListener('mouseup', () => {
      if (this.draggedNode && !this.hasMovedSignificantly) {
        if (this.onSelectUser && this.draggedNode.id !== this.activeUserId) {
          this.onSelectUser(this.draggedNode.id);
        }
      }
      this.draggedNode = null;
      this.mouse.isDown = false;
      this.hasMovedSignificantly = false;
    });
  }

  getNodeAt(x, y) {
    for (let i = this.nodes.length - 1; i >= 0; i--) {
      const n = this.nodes[i];
      const dist = Math.hypot(n.x - x, n.y - y);
      if (dist <= n.radius + 8) return n;
    }
    return null;
  }

  startSimulation() {
    const step = () => {
      this.simulatePhysics();
      this.draw();
      requestAnimationFrame(step);
    };
    requestAnimationFrame(step);
  }

  simulatePhysics() {
    const kRepel = 7200;
    const kSpring = 0.026;
    const restLen = 125;
    const damping = 0.85;
    const centerForce = 0.005;

    // 1. Repulsion between all node pairs & Collision Separation
    for (let i = 0; i < this.nodes.length; i++) {
      const n1 = this.nodes[i];
      for (let j = i + 1; j < this.nodes.length; j++) {
        const n2 = this.nodes[j];
        const dx = n2.x - n1.x;
        const dy = n2.y - n1.y;
        const dist = Math.hypot(dx, dy) || 1;

        // Force-directed repulsion
        const force = kRepel / (dist * dist);
        const fx = (dx / dist) * force;
        const fy = (dy / dist) * force;
        n1.vx -= fx;
        n1.vy -= fy;
        n2.vx += fx;
        n2.vy += fy;

        // Hard collision clearance: minimum 82px center-to-center distance
        const minDist = 82;
        if (dist < minDist) {
          const overlap = (minDist - dist) * 0.5;
          const nx = dx / dist;
          const ny = dy / dist;
          if (n1 !== this.draggedNode) {
            n1.x -= nx * overlap;
            n1.y -= ny * overlap;
          }
          if (n2 !== this.draggedNode) {
            n2.x += nx * overlap;
            n2.y += ny * overlap;
          }
        }
      }
    }

    // 2. Spring attraction for friendships
    for (const n1 of this.nodes) {
      const friends = this.graph.adjLists[n1.id] || [];
      for (const friendId of friends) {
        const n2 = this.nodes.find(n => n.id === friendId);
        if (n2 && n1.id < n2.id) {
          const dx = n2.x - n1.x;
          const dy = n2.y - n1.y;
          const dist = Math.hypot(dx, dy) || 1;
          const displacement = dist - restLen;
          const force = displacement * kSpring;
          const fx = (dx / dist) * force;
          const fy = (dy / dist) * force;
          n1.vx += fx;
          n1.vy += fy;
          n2.vx -= fx;
          n2.vy -= fy;
        }
      }
    }

    // 3. Center attraction & boundary damping
    const cx = this.width / 2;
    const cy = this.height / 2;
    for (const n of this.nodes) {
      if (n === this.draggedNode) {
        n.x = this.mouse.x;
        n.y = this.mouse.y;
        n.vx = 0;
        n.vy = 0;
        continue;
      }

      n.vx += (cx - n.x) * centerForce;
      n.vy += (cy - n.y) * centerForce;

      n.vx *= damping;
      n.vy *= damping;

      // Clamp max velocity
      const maxSpeed = 7;
      n.vx = Math.max(-maxSpeed, Math.min(maxSpeed, n.vx));
      n.vy = Math.max(-maxSpeed, Math.min(maxSpeed, n.vy));

      n.x += n.vx;
      n.y += n.vy;

      // Keep within canvas padding boundaries
      const pad = 44;
      n.x = Math.max(pad, Math.min(this.width - pad, n.x));
      n.y = Math.max(pad, Math.min(this.height - pad, n.y));
    }
  }

  draw() {
    this.ctx.clearRect(0, 0, this.width, this.height);

    // Subtle background grid
    this.drawGrid();

    // Direct friendship edges
    this.drawEdges();

    // Recommendation dashed paths
    this.drawRecommendationPaths();

    // Node circles & labels
    this.drawNodes();

    // Tooltip overlay (drawn on top)
    if (this.hoveredNode && !this.draggedNode) {
      this.drawTooltip(this.hoveredNode);
    }
  }

  drawGrid() {
    this.ctx.strokeStyle = 'rgba(224, 223, 220, 0.4)';
    this.ctx.lineWidth = 1;
    const step = 40;
    this.ctx.beginPath();
    for (let x = 0; x < this.width; x += step) {
      this.ctx.moveTo(x, 0);
      this.ctx.lineTo(x, this.height);
    }
    for (let y = 0; y < this.height; y += step) {
      this.ctx.moveTo(0, y);
      this.ctx.lineTo(this.width, y);
    }
    this.ctx.stroke();
  }

  drawEdges() {
    this.ctx.lineWidth = 2.5;
    for (const n1 of this.nodes) {
      const friends = this.graph.adjLists[n1.id] || [];
      for (const friendId of friends) {
        const n2 = this.nodes.find(n => n.id === friendId);
        if (n2 && n1.id < n2.id) {
          const isTargetEdge = (n1.id === this.activeUserId || n2.id === this.activeUserId);
          this.ctx.strokeStyle = isTargetEdge ? 'rgba(5, 118, 66, 0.75)' : 'rgba(180, 180, 180, 0.5)';
          this.ctx.beginPath();
          this.ctx.moveTo(n1.x, n1.y);
          this.ctx.lineTo(n2.x, n2.y);
          this.ctx.stroke();
        }
      }
    }
  }

  drawRecommendationPaths() {
    if (!this.recommendationData || !this.recommendationData.recommendations) return;

    const targetNode = this.nodes.find(n => n.id === this.activeUserId);
    if (!targetNode) return;

    this.ctx.setLineDash([4, 4]);
    this.ctx.lineWidth = 2;

    for (const rec of this.recommendationData.recommendations) {
      const candNode = this.nodes.find(n => n.id === rec.candidateId);
      if (!candNode) continue;

      this.ctx.strokeStyle = 'rgba(10, 102, 194, 0.7)';
      this.ctx.beginPath();
      this.ctx.moveTo(targetNode.x, targetNode.y);
      this.ctx.lineTo(candNode.x, candNode.y);
      this.ctx.stroke();
    }
    this.ctx.setLineDash([]);
  }

  drawNodes() {
    const distMap = this.recommendationData?.distances || [];

    for (const n of this.nodes) {
      const isTarget = n.id === this.activeUserId;
      const isFriend = this.graph.hasFriendship(this.activeUserId, n.id);
      const isRec = distMap[n.id] === 2;
      const isHovered = this.hoveredNode === n;

      let radius = n.radius;
      if (isTarget) radius = 27;
      if (isHovered) radius += 3;

      // Glow halo
      if (isTarget) {
        this.ctx.beginPath();
        this.ctx.arc(n.x, n.y, radius + 8, 0, Math.PI * 2);
        this.ctx.fillStyle = 'rgba(10, 102, 194, 0.22)';
        this.ctx.fill();
      } else if (isRec) {
        this.ctx.beginPath();
        this.ctx.arc(n.x, n.y, radius + 6, 0, Math.PI * 2);
        this.ctx.fillStyle = 'rgba(224, 120, 0, 0.2)';
        this.ctx.fill();
      }

      // Base circle
      this.ctx.beginPath();
      this.ctx.arc(n.x, n.y, radius, 0, Math.PI * 2);
      this.ctx.fillStyle = '#ffffff';
      this.ctx.fill();

      // Border styling according to BFS level
      let strokeColor = '#b0b0b0';
      let strokeWidth = 3;

      if (isTarget) {
        strokeColor = '#0a66c2';
        strokeWidth = 4;
      } else if (isFriend) {
        strokeColor = '#057642';
        strokeWidth = 3.5;
      } else if (isRec) {
        strokeColor = '#e07800';
        strokeWidth = 3.5;
      }

      this.ctx.lineWidth = strokeWidth;
      this.ctx.strokeStyle = strokeColor;
      this.ctx.stroke();

      // Initials inside circle
      this.ctx.save();
      this.ctx.beginPath();
      this.ctx.arc(n.x, n.y, radius - 2, 0, Math.PI * 2);
      this.ctx.clip();
      this.ctx.fillStyle = '#f3f2ef';
      this.ctx.fill();

      this.ctx.fillStyle = '#191919';
      this.ctx.font = `bold ${isTarget ? 13 : 11}px -apple-system, sans-serif`;
      this.ctx.textAlign = 'center';
      this.ctx.textBaseline = 'middle';
      const initial = n.user.name.charAt(0).toUpperCase();
      this.ctx.fillText(initial, n.x, n.y);
      this.ctx.restore();

      // Name label below node
      this.ctx.font = `bold 11px -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif`;
      this.ctx.textAlign = 'center';
      this.ctx.fillStyle = isTarget ? '#0a66c2' : '#2d3748';
      this.ctx.fillText(n.user.name, n.x, n.y + radius + 13);

      // BFS Distance tag badge
      let badgeText = "";
      let badgeColor = "#718096";
      if (isTarget) {
        badgeText = "You (d:0)";
        badgeColor = "#0a66c2";
      } else if (isFriend) {
        badgeText = "Friend (d:1)";
        badgeColor = "#057642";
      } else if (isRec) {
        badgeText = "Rec (d:2)";
        badgeColor = "#d97706";
      } else {
        badgeText = "d:∞";
        badgeColor = "#a0aec0";
      }

      this.ctx.font = `9px -apple-system, sans-serif`;
      this.ctx.fillStyle = badgeColor;
      this.ctx.fillText(badgeText, n.x, n.y + radius + 24);
    }
  }

  wrapText(text, maxChars = 34) {
    if (!text) return [];
    const words = text.split(' ');
    const lines = [];
    let current = words[0] || '';

    for (let i = 1; i < words.length; i++) {
      const word = words[i];
      if ((current + ' ' + word).length <= maxChars) {
        current += ' ' + word;
      } else {
        lines.push(current);
        current = word;
      }
    }
    if (current) lines.push(current);
    return lines;
  }

  drawTooltip(node) {
    const isTarget = node.id === this.activeUserId;
    const isFriend = this.graph.hasFriendship(this.activeUserId, node.id);
    const rec = this.recommendationData?.recommendations?.find(r => r.candidateId === node.id);

    // Degree badge text
    let degreeBadge = "3rd+";
    let degreeColor = "#94a3b8";
    if (isTarget) {
      degreeBadge = "You";
      degreeColor = "#38bdf8";
    } else if (isFriend) {
      degreeBadge = "1st";
      degreeColor = "#34d399";
    } else if (rec) {
      degreeBadge = "2nd • Rec";
      degreeColor = "#fbbf24";
    }

    // Wrapped headline (prevents tooltip horizontal overflow)
    const headlineLines = this.wrapText(node.user.headline || "", 30).slice(0, 2);

    // Recommendation / Status note
    let statusNote = "";
    if (isTarget) {
      statusNote = "★ Active Profile (You)";
    } else if (isFriend) {
      statusNote = "✓ 1st-Degree Connection";
    } else if (rec) {
      const mutuals = rec.mutualCount;
      const names = rec.mutualFriendNames ? rec.mutualFriendNames.slice(0, 2).join(', ') : '';
      statusNote = `★ Recommended (${mutuals} mutual${mutuals > 1 ? 's' : ''}: ${names})`;
    } else {
      statusNote = "• No mutual connection";
    }
    const statusLines = this.wrapText(statusNote, 32).slice(0, 2);

    // Measure required box dimensions
    this.ctx.font = 'bold 12px -apple-system, sans-serif';
    const nameWidth = this.ctx.measureText(node.user.name).width + 60;

    this.ctx.font = '11px -apple-system, sans-serif';
    const hWidths = headlineLines.map(l => this.ctx.measureText(l).width);
    const sWidths = statusLines.map(l => this.ctx.measureText(l).width);
    const maxContentWidth = Math.max(nameWidth, ...hWidths, ...sWidths, 180);

    const pad = 12;
    const boxW = Math.min(270, maxContentWidth + pad * 2);
    const totalLines = 1 + headlineLines.length + 1 + statusLines.length + 1; // name + headline + meta + status + hint
    const boxH = totalLines * 16 + pad * 2 + 6;

    // Smart, non-overlapping positioning
    let bx = node.x + node.radius + 14;
    let by = node.y - boxH / 2;

    // If placing on right overflows canvas, try left
    if (bx + boxW > this.width - 12) {
      bx = node.x - node.radius - 14 - boxW;
    }

    // If placing on left also overflows, center horizontally and place above/below
    if (bx < 12) {
      bx = node.x - boxW / 2;
      if (node.y - node.radius - 16 - boxH >= 12) {
        by = node.y - node.radius - 16 - boxH;
      } else {
        by = node.y + node.radius + 34;
      }
    }

    // STRICT BOUNDARY CLAMPING: Tooltip will NEVER clip or have negative coordinates
    bx = Math.max(12, Math.min(this.width - boxW - 12, bx));
    by = Math.max(12, Math.min(this.height - boxH - 12, by));

    // Draw Tooltip Container with rounded corners & shadow
    this.ctx.save();
    this.ctx.shadowColor = 'rgba(0, 0, 0, 0.45)';
    this.ctx.shadowBlur = 14;
    this.ctx.shadowOffsetY = 4;

    this.ctx.fillStyle = 'rgba(15, 23, 42, 0.95)';
    this.ctx.beginPath();
    this.ctx.roundRect(bx, by, boxW, boxH, 8);
    this.ctx.fill();

    // Subtle crisp border
    this.ctx.shadowColor = 'transparent';
    this.ctx.lineWidth = 1;
    this.ctx.strokeStyle = 'rgba(255, 255, 255, 0.14)';
    this.ctx.stroke();

    // Render Text Lines inside Tooltip
    this.ctx.textAlign = 'left';
    this.ctx.textBaseline = 'top';
    let lineY = by + pad;

    // Line 1: Name + Degree Pill
    this.ctx.fillStyle = '#ffffff';
    this.ctx.font = 'bold 12px -apple-system, sans-serif';
    this.ctx.fillText(node.user.name, bx + pad, lineY);

    this.ctx.fillStyle = degreeColor;
    this.ctx.font = 'bold 10px -apple-system, sans-serif';
    this.ctx.fillText(`[${degreeBadge}]`, bx + pad + this.ctx.measureText(node.user.name).width + 8, lineY + 1);
    lineY += 18;

    // Line 2+: Headline Lines
    this.ctx.fillStyle = '#cbd5e1';
    this.ctx.font = '11px -apple-system, sans-serif';
    headlineLines.forEach(line => {
      this.ctx.fillText(line, bx + pad, lineY);
      lineY += 15;
    });

    // Line: Location & Connections
    this.ctx.fillStyle = '#94a3b8';
    this.ctx.font = '10px -apple-system, sans-serif';
    const connCount = this.graph.adjLists[node.id]?.length || 0;
    this.ctx.fillText(`📍 ${node.user.city} • 🔗 ${connCount} friends`, bx + pad, lineY);
    lineY += 16;

    // Line: Recommendation / Mutual Connections Highlight
    this.ctx.fillStyle = rec ? '#fbbf24' : (isFriend ? '#34d399' : '#38bdf8');
    this.ctx.font = 'bold 10.5px -apple-system, sans-serif';
    statusLines.forEach(line => {
      this.ctx.fillText(line, bx + pad, lineY);
      lineY += 15;
    });

    // Action Hint
    this.ctx.fillStyle = '#64748b';
    this.ctx.font = 'italic 9.5px -apple-system, sans-serif';
    this.ctx.fillText('💡 Click node to view full profile', bx + pad, lineY);

    this.ctx.restore();
  }
}

window.NetworkGraphCanvas = NetworkGraphCanvas;
