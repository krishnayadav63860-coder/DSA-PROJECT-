/**
 * LinkedIn Front-End Controller & DSA Recommendation Engine Integration
 * Supports 15 realistic members, full LinkedIn profile view, and real-time Graph BFS updates.
 */

document.addEventListener('DOMContentLoaded', () => {
  // 1. Initialize Graph & Canvas
  const graph = new window.SocialGraph();
  graph.loadDemoData(); // Loads 15 members with rich profiles & clustered friendships

  let activeUserId = 0; // Rahul by default
  let viewingProfileId = 0;
  let previousTab = 'feed';
  let recommendationData = graph.recommendFriends(activeUserId);

  const canvasElem = document.getElementById('networkCanvas');
  const networkCanvas = new window.NetworkGraphCanvas(canvasElem, graph, (selectedId) => {
    openUserProfile(selectedId);
  });

  // 2. UI Elements
  const activeUserSelect = document.getElementById('activeUserSelect');
  const sidebarName = document.getElementById('sidebarName');
  const sidebarHeadline = document.getElementById('sidebarHeadline');
  const sidebarAvatar = document.getElementById('sidebarAvatar');
  const sidebarCover = document.getElementById('sidebarCover');
  const sidebarConnectionCount = document.getElementById('sidebarConnectionCount');
  const sidebarDepth2Count = document.getElementById('sidebarDepth2Count');
  const postAvatarMini = document.getElementById('postAvatarMini');
  const networkBadge = document.getElementById('networkBadge');

  const recommendationsGrid = document.getElementById('recommendationsGrid');
  const connectionsList = document.getElementById('connectionsList');
  const distanceTable = document.getElementById('distanceTable');
  const bfsTraceList = document.getElementById('bfsTraceList');

  // Telemetry elements
  const telemetryTarget = document.getElementById('telemetryTarget');
  const telemetryCandidates = document.getElementById('telemetryCandidates');

  // Profile View Elements
  const viewProfile = document.getElementById('viewProfile');
  const viewProfileCover = document.getElementById('viewProfileCover');
  const viewProfileAvatar = document.getElementById('viewProfileAvatar');
  const viewProfileName = document.getElementById('viewProfileName');
  const viewProfileDegree = document.getElementById('viewProfileDegree');
  const viewProfileHeadline = document.getElementById('viewProfileHeadline');
  const viewProfileLocation = document.getElementById('viewProfileLocation');
  const viewProfileConnectionsCount = document.getElementById('viewProfileConnectionsCount');
  const viewProfileMutualBanner = document.getElementById('viewProfileMutualBanner');
  const viewProfileFacepile = document.getElementById('viewProfileFacepile');
  const viewProfileMutualText = document.getElementById('viewProfileMutualText');
  const btnProfileConnect = document.getElementById('btnProfileConnect');
  const viewProfileAbout = document.getElementById('viewProfileAbout');
  const viewProfileExperienceList = document.getElementById('viewProfileExperienceList');
  const viewProfileEducationList = document.getElementById('viewProfileEducationList');
  const viewProfileSkillsList = document.getElementById('viewProfileSkillsList');
  const viewProfileMutualCard = document.getElementById('viewProfileMutualCard');
  const viewProfileMutualList = document.getElementById('viewProfileMutualList');

  // 3. Helper: Toast Notifications
  function showToast(message, type = 'success') {
    const container = document.getElementById('toastContainer');
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `
      <svg width="18" height="18" viewBox="0 0 24 24" fill="currentColor">
        <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-2 15l-5-5 1.41-1.41L10 14.17l7.59-7.59L19 8l-9 9z"/>
      </svg>
      <span>${message}</span>
    `;
    container.appendChild(toast);
    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateY(10px)';
      toast.style.transition = 'all 0.3s ease';
      setTimeout(() => toast.remove(), 300);
    }, 3500);
  }

  // 4. Update Main UI View
  function refreshUI() {
    const user = graph.users[activeUserId];
    recommendationData = graph.recommendFriends(activeUserId);

    // Update Profile Card in Left Sidebar
    sidebarName.textContent = user.name;
    sidebarHeadline.textContent = user.headline;
    sidebarAvatar.src = user.avatar;
    sidebarCover.style.background = user.cover;
    postAvatarMini.src = user.avatar;

    const friendsCount = graph.adjLists[activeUserId].filter(id => !graph.users[id].isDeleted).length;
    sidebarConnectionCount.textContent = friendsCount;
    sidebarDepth2Count.textContent = recommendationData.recommendations.length;
    networkBadge.textContent = recommendationData.recommendations.length;

    // Telemetry
    telemetryTarget.textContent = `${user.name} (ID: ${user.id})`;
    telemetryCandidates.textContent = recommendationData.recommendations.length;

    // Update User Switch Select options (15 members)
    activeUserSelect.innerHTML = '';
    graph.users.forEach(u => {
      if (!u.isDeleted) {
        const opt = document.createElement('option');
        opt.value = u.id;
        opt.textContent = `${u.name} (${u.company}) - ID: ${u.id}`;
        if (u.id === activeUserId) opt.selected = true;
        activeUserSelect.appendChild(opt);
      }
    });

    // Update Canvas
    networkCanvas.updateData(activeUserId, recommendationData);

    // Render Recommendation Cards
    renderRecommendations();

    // Render My Network View
    renderNetworkMetrics();
    renderConnectionsList();

    // Render DSA Inspector View
    renderDSAInspector();

    // If currently viewing a profile, re-render it to update connection status
    if (viewProfile.classList.contains('active')) {
      renderUserProfile(viewingProfileId);
    }
  }

  function switchActiveUser(userId) {
    if (userId >= 0 && userId < graph.users.length && !graph.users[userId].isDeleted) {
      activeUserId = userId;
      refreshUI();
      showToast(`Switched active view to ${graph.users[userId].name}`, 'info');
    }
  }

  activeUserSelect.addEventListener('change', (e) => {
    switchActiveUser(parseInt(e.target.value, 10));
  });

  // Clicking sidebar avatar/name opens own full profile
  sidebarAvatar.addEventListener('click', () => openUserProfile(activeUserId));
  sidebarName.addEventListener('click', () => openUserProfile(activeUserId));

  // 5. Render Recommendations Grid (with Facepiles & Clickable Profiles)
  function renderRecommendations() {
    recommendationsGrid.innerHTML = '';
    const recs = recommendationData.recommendations;

    if (recs.length === 0) {
      recommendationsGrid.innerHTML = `
        <div style="grid-column: 1 / -1; text-align: center; padding: 30px; color: var(--text-secondary);">
          <div style="font-size: 32px; margin-bottom: 8px;">🎉</div>
          <div style="font-weight: 600; font-size: 14px; margin-bottom: 4px;">No immediate recommendations found</div>
          <div style="font-size: 12px;">You are already directly connected to everyone in your 2-hop network, or isolated users have 0 mutual friends!</div>
        </div>
      `;
      return;
    }

    recs.forEach(rec => {
      const cand = rec.candidate;
      const mutualNames = rec.mutualFriends.map(f => f.name).join(', ');

      // Facepile avatars (first 2 mutual friends)
      const facepileHtml = rec.mutualFriends.slice(0, 2).map(f => `
        <img class="facepile-avatar" src="${f.avatar}" alt="${f.name}" title="${f.name}">
      `).join('');

      let mutualDisplayText = `${rec.mutualCount} mutual`;
      if (rec.mutualFriends.length === 1) {
        mutualDisplayText = `1 mutual (${rec.mutualFriends[0].name})`;
      } else if (rec.mutualFriends.length === 2) {
        mutualDisplayText = `2 mutuals (${rec.mutualFriends[0].name}, ${rec.mutualFriends[1].name})`;
      } else {
        mutualDisplayText = `${rec.mutualCount} mutuals (${rec.mutualFriends[0].name} & +${rec.mutualCount - 1} more)`;
      }

      const card = document.createElement('div');
      card.className = 'rec-card';
      card.innerHTML = `
        <div class="rec-banner" style="background: ${cand.cover};" data-profile-id="${cand.id}"></div>
        <div class="rec-avatar-wrapper" data-profile-id="${cand.id}">
          <img class="rec-avatar" src="${cand.avatar}" alt="${cand.name}">
        </div>
        <div class="rec-details">
          <h4 class="rec-name" data-profile-id="${cand.id}">${cand.name}</h4>
          <p class="rec-headline">${cand.headline}</p>
          <div class="rec-mutuals-badge" data-profile-id="${cand.id}" title="Mutual Connections: ${mutualNames}">
            <div class="facepile-avatars">${facepileHtml}</div>
            <span><strong>${mutualDisplayText}</strong></span>
          </div>
        </div>
        <button class="btn-connect" data-candidate-id="${cand.id}">
          <svg width="16" height="16" viewBox="0 0 24 24" fill="currentColor">
            <path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>
          </svg>
          Connect
        </button>
      `;

      // Connect button event
      card.querySelector('.btn-connect').addEventListener('click', (e) => {
        handleConnect(rec.candidateId, e.currentTarget);
      });

      // Clicking banner, avatar, name, or mutual badge opens full profile!
      card.querySelectorAll('[data-profile-id]').forEach(el => {
        el.addEventListener('click', () => {
          openUserProfile(cand.id);
        });
      });

      recommendationsGrid.appendChild(card);
    });
  }

  function handleConnect(candidateId, buttonElem) {
    if (graph.addFriendship(activeUserId, candidateId)) {
      if (buttonElem) {
        buttonElem.classList.add('btn-connected');
        buttonElem.innerHTML = `✓ Connected`;
      }
      const candName = graph.users[candidateId].name;
      showToast(`Connected with ${candName}! Edge added to adjacency list.`, 'success');
      setTimeout(() => {
        refreshUI();
      }, 400);
    }
  }

  // 6. FULL LINKEDIN PROFILE VIEW LOGIC
  function openUserProfile(userId) {
    if (userId < 0 || userId >= graph.users.length || graph.users[userId].isDeleted) return;

    viewingProfileId = userId;

    // Track previous tab for back button
    const currentActiveNav = document.querySelector('.nav-link.active');
    if (currentActiveNav && currentActiveNav.getAttribute('data-tab') !== 'profile') {
      previousTab = currentActiveNav.getAttribute('data-tab') || 'feed';
    }

    // Hide other views, show profile view
    document.querySelectorAll('.view-container').forEach(v => v.classList.remove('active'));
    document.querySelectorAll('.nav-link').forEach(l => l.classList.remove('active'));
    viewProfile.classList.add('active');

    renderUserProfile(userId);

    // Scroll smoothly to top
    window.scrollTo({ top: 0, behavior: 'smooth' });
  }

  function renderUserProfile(userId) {
    const target = graph.users[userId];
    const isSelf = (userId === activeUserId);
    const isFriend = graph.hasFriendship(activeUserId, userId);
    const distMap = recommendationData.distances || [];
    const distance = distMap[userId];

    // Banner & Avatar
    viewProfileCover.style.background = target.cover;
    viewProfileAvatar.src = target.avatar;
    viewProfileName.textContent = target.name;
    viewProfileHeadline.textContent = target.headline;
    viewProfileLocation.textContent = target.city;

    const totalConns = graph.adjLists[userId].filter(id => !graph.users[id].isDeleted).length;
    viewProfileConnectionsCount.textContent = `${totalConns} connections`;

    // Degree Badge (1st, 2nd, 3rd+)
    viewProfileDegree.className = 'degree-badge';
    if (isSelf) {
      viewProfileDegree.textContent = 'You';
      viewProfileDegree.style.display = 'inline-block';
    } else if (isFriend) {
      viewProfileDegree.textContent = '1st';
      viewProfileDegree.classList.add('d1');
    } else if (distance === 2) {
      viewProfileDegree.textContent = '2nd';
      viewProfileDegree.classList.add('d2');
    } else {
      viewProfileDegree.textContent = '3rd+';
    }

    // Mutual Connections bar
    const mutualIds = graph.getMutualFriends(activeUserId, userId);
    if (!isSelf && mutualIds.length > 0) {
      viewProfileMutualBanner.style.display = 'flex';
      viewProfileFacepile.innerHTML = mutualIds.slice(0, 3).map(id => `
        <img class="facepile-avatar" src="${graph.users[id].avatar}" alt="${graph.users[id].name}" title="${graph.users[id].name}">
      `).join('');

      const mNames = mutualIds.map(id => graph.users[id].name);
      if (mutualIds.length === 1) {
        viewProfileMutualText.innerHTML = `<strong>1 mutual connection:</strong> ${mNames[0]}`;
      } else if (mutualIds.length === 2) {
        viewProfileMutualText.innerHTML = `<strong>2 mutual connections:</strong> ${mNames[0]} and ${mNames[1]}`;
      } else {
        viewProfileMutualText.innerHTML = `<strong>${mutualIds.length} mutual connections:</strong> ${mNames[0]}, ${mNames[1]} and ${mutualIds.length - 2} others`;
      }
    } else {
      viewProfileMutualBanner.style.display = 'none';
    }

    // Connect button state
    if (isSelf) {
      btnProfileConnect.style.display = 'none';
    } else {
      btnProfileConnect.style.display = 'flex';
      if (isFriend) {
        btnProfileConnect.className = 'btn-profile-connect btn-profile-connected';
        btnProfileConnect.innerHTML = `
          <svg width="16" height="16" viewBox="0 0 24 24" fill="currentColor">
            <path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z"/>
          </svg>
          <span>Connected</span>
        `;
        btnProfileConnect.onclick = () => {
          showToast(`You and ${target.name} are already 1st-degree connections.`, 'info');
        };
      } else {
        btnProfileConnect.className = 'btn-profile-connect';
        btnProfileConnect.innerHTML = `
          <svg width="16" height="16" viewBox="0 0 24 24" fill="currentColor">
            <path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>
          </svg>
          <span>Connect</span>
        `;
        btnProfileConnect.onclick = () => {
          handleConnect(userId, btnProfileConnect);
          renderUserProfile(userId);
        };
      }
    }

    // About
    viewProfileAbout.textContent = target.about;

    // Experience List
    viewProfileExperienceList.innerHTML = target.experiences.map(exp => `
      <div class="timeline-item">
        <div class="timeline-icon">💼</div>
        <div class="timeline-details">
          <h4>${exp.role}</h4>
          <div class="company">${exp.company}</div>
          <div class="duration">${exp.duration}</div>
          <div class="location">${exp.location}</div>
          <div class="desc">${exp.description}</div>
        </div>
      </div>
    `).join('');

    // Education List
    viewProfileEducationList.innerHTML = target.education.map(edu => `
      <div class="timeline-item">
        <div class="timeline-icon">🎓</div>
        <div class="timeline-details">
          <h4>${edu.school}</h4>
          <div class="company">${edu.degree}</div>
          <div class="duration">${edu.year}</div>
        </div>
      </div>
    `).join('');

    // Skills
    viewProfileSkillsList.innerHTML = target.skills.map((skill, sIdx) => `
      <div class="skill-pill">${skill} • ${18 - sIdx * 2} endorsements</div>
    `).join('');

    // Mutual Connections Card breakdown
    if (!isSelf && mutualIds.length > 0) {
      viewProfileMutualCard.style.display = 'block';
      viewProfileMutualList.innerHTML = mutualIds.map(mId => {
        const m = graph.users[mId];
        return `
          <div style="display: flex; align-items: center; justify-content: space-between; padding: 10px 0; border-bottom: 1px solid var(--border-light);">
            <div style="display: flex; align-items: center; gap: 10px; cursor: pointer;" onclick="window.openUserProfile(${m.id})">
              <img src="${m.avatar}" style="width: 40px; height: 40px; border-radius: 50%; object-fit: cover;">
              <div>
                <div style="font-size: 13px; font-weight: 600; color: #0a66c2;">${m.name}</div>
                <div style="font-size: 11px; color: var(--text-secondary);">${m.headline}</div>
              </div>
            </div>
            <button class="btn-preset" onclick="window.openUserProfile(${m.id})" style="width: auto; padding: 4px 10px; font-size: 11px;">
              View Profile
            </button>
          </div>
        `;
      }).join('');
    } else {
      viewProfileMutualCard.style.display = 'none';
    }
  }

  // Back button from profile view
  document.getElementById('btnBackFromProfile').addEventListener('click', () => {
    viewProfile.classList.remove('active');
    const targetTabLink = document.querySelector(`.nav-link[data-tab="${previousTab}"]`) || document.getElementById('navHome');
    targetTabLink.click();
  });

  // Global exposure for inline onclick handlers
  window.openUserProfile = openUserProfile;

  // 7. Render My Network Dashboard
  function renderNetworkMetrics() {
    const stats = graph.getNetworkStats();
    document.getElementById('metricUsers').textContent = stats.totalUsers;
    document.getElementById('metricFriendships').textContent = stats.totalFriendships;
    document.getElementById('metricAvgFriends').textContent = stats.avgFriends;
    document.getElementById('metricDensity').textContent = `${stats.densityPercent}%`;
  }

  function renderConnectionsList() {
    connectionsList.innerHTML = '';
    const friends = graph.adjLists[activeUserId].filter(id => !graph.users[id].isDeleted);

    if (friends.length === 0) {
      connectionsList.innerHTML = `<p style="font-size: 13px; color: var(--text-secondary);">No 1st-degree connections yet. Accept recommendations or add friendships!</p>`;
      return;
    }

    friends.forEach(fId => {
      const f = graph.users[fId];
      const row = document.createElement('div');
      row.style.cssText = 'display: flex; align-items: center; justify-content: space-between; padding: 10px 0; border-bottom: 1px solid var(--border-light);';
      row.innerHTML = `
        <div style="display: flex; align-items: center; gap: 12px; cursor: pointer;">
          <img src="${f.avatar}" style="width: 44px; height: 44px; border-radius: 50%; object-fit: cover;">
          <div>
            <div style="font-size: 14px; font-weight: 600; color: var(--text-main);">${f.name} <span class="degree-badge d1" style="font-size: 10px; padding: 1px 6px;">1st</span></div>
            <div style="font-size: 11px; color: var(--text-secondary);">${f.headline} • ${f.city}</div>
          </div>
        </div>
        <div style="display: flex; gap: 8px;">
          <button class="btn-preset" data-view-id="${f.id}" style="width: auto; padding: 4px 10px; font-size: 11px;">
            View Profile
          </button>
          <button class="btn-preset" data-remove-id="${f.id}" style="width: auto; padding: 4px 10px; font-size: 11px; color: #cc1016; border-color: #fca5a5;">
            Remove
          </button>
        </div>
      `;

      row.querySelector('[data-view-id]').addEventListener('click', () => {
        openUserProfile(f.id);
      });

      row.querySelector('[data-remove-id]').addEventListener('click', () => {
        if (confirm(`Remove connection with ${f.name}?`)) {
          graph.removeFriendship(activeUserId, f.id);
          showToast(`Friendship removed between ${graph.users[activeUserId].name} and ${f.name}`, 'info');
          refreshUI();
        }
      });

      connectionsList.appendChild(row);
    });
  }

  // 8. Render DSA Inspector
  function renderDSAInspector() {
    const distMap = recommendationData.distances;

    let tableHtml = `
      <tr style="background: #f1f1f1; border-bottom: 2px solid #ddd;">
        <th style="padding: 8px;">User ID</th>
        <th style="padding: 8px;">User Name</th>
        <th style="padding: 8px;">Company</th>
        <th style="padding: 8px;">BFS Shortest Distance</th>
        <th style="padding: 8px;">Status / Role</th>
        <th style="padding: 8px;">Profile</th>
      </tr>
    `;

    graph.users.forEach((u, id) => {
      if (!u.isDeleted) {
        const d = distMap[id];
        let role = '';
        let badgeColor = '';
        if (id === activeUserId) {
          role = '★ Source Target Node (Depth 0)';
          badgeColor = '#0a66c2';
        } else if (d === 1) {
          role = '✓ Direct 1st-Degree Friend (Depth 1)';
          badgeColor = '#057642';
        } else if (d === 2) {
          role = '⚡ Recommended Candidate (Depth 2)';
          badgeColor = '#e07800';
        } else {
          role = '• Isolated / Distance > 2';
          badgeColor = '#888';
        }

        tableHtml += `
          <tr style="border-bottom: 1px solid #eee;">
            <td style="padding: 8px; font-weight: 600;">${id}</td>
            <td style="padding: 8px; font-weight: 600; color: #0a66c2; cursor: pointer;" onclick="window.openUserProfile(${id})">${u.name}</td>
            <td style="padding: 8px; font-size: 11px; color: #555;">${u.company}</td>
            <td style="padding: 8px; font-family: monospace; font-weight: 700; color: ${badgeColor};">${d === -1 ? '∞' : d}</td>
            <td style="padding: 8px; font-weight: 500; color: ${badgeColor};">${role}</td>
            <td style="padding: 8px;"><button class="btn-preset" onclick="window.openUserProfile(${id})" style="width: auto; padding: 2px 8px; font-size: 10px;">View</button></td>
          </tr>
        `;
      }
    });

    distanceTable.innerHTML = tableHtml;

    // Trace list
    bfsTraceList.innerHTML = '';
    recommendationData.bfsTrace.forEach((trace, idx) => {
      const line = document.createElement('div');
      line.style.padding = '4px 0';
      line.innerHTML = `<strong>Step ${idx + 1}:</strong> ${trace.step} | Queue: [${trace.queue.map(id => graph.users[id]?.name).join(', ')}] | Depth: ${trace.depth}`;
      bfsTraceList.appendChild(line);
    });
  }

  // 9. Navigation Tab Switching
  const navLinks = document.querySelectorAll('.nav-link');
  const views = {
    feed: document.getElementById('viewFeed'),
    network: document.getElementById('viewNetwork'),
    dsa: document.getElementById('viewDSA'),
    profile: viewProfile
  };

  navLinks.forEach(link => {
    link.addEventListener('click', (e) => {
      e.preventDefault();
      const tab = link.getAttribute('data-tab');
      if (!tab) return;

      navLinks.forEach(l => l.classList.remove('active'));
      link.classList.add('active');

      Object.keys(views).forEach(k => {
        if (views[k]) views[k].classList.remove('active');
      });
      if (views[tab]) views[tab].classList.add('active');

      if (tab === 'feed') {
        networkCanvas.initCanvasSize();
      }
    });
  });

  document.getElementById('btnViewNetworkTab').addEventListener('click', () => {
    document.getElementById('navNetwork').click();
  });

  document.getElementById('logoBtn').addEventListener('click', () => {
    document.getElementById('navHome').click();
  });

  // 10. Search Bar with djb2 Hashing & Autocomplete
  const searchInput = document.getElementById('globalSearchInput');
  const searchDropdown = document.getElementById('searchResultsDropdown');

  searchInput.addEventListener('input', () => {
    const query = searchInput.value.trim();
    if (!query) {
      searchDropdown.classList.remove('active');
      searchDropdown.innerHTML = '';
      return;
    }

    const exactId = graph.getUserId(query);
    const matches = graph.users.filter(u =>
      !u.isDeleted && u.name.toLowerCase().includes(query.toLowerCase())
    );

    if (matches.length === 0) {
      searchDropdown.innerHTML = `<div style="padding: 10px; font-size: 12px; color: var(--text-secondary);">No member found in djb2 hash table for "${query}"</div>`;
      searchDropdown.classList.add('active');
      return;
    }

    searchDropdown.innerHTML = '';
    matches.forEach(m => {
      const item = document.createElement('div');
      item.className = 'search-result-item';
      item.innerHTML = `
        <img src="${m.avatar}" alt="${m.name}">
        <div>
          <div style="font-size: 13px; font-weight: 600; color: var(--text-main);">${m.name} ${m.id === exactId ? '<span style="font-size: 10px; color: #0a66c2;">(Exact djb2 Match)</span>' : ''}</div>
          <div style="font-size: 11px; color: var(--text-secondary);">${m.headline}</div>
        </div>
      `;
      item.addEventListener('click', () => {
        openUserProfile(m.id);
        searchInput.value = '';
        searchDropdown.classList.remove('active');
      });
      searchDropdown.appendChild(item);
    });
    searchDropdown.classList.add('active');
  });

  document.addEventListener('click', (e) => {
    if (!searchInput.contains(e.target) && !searchDropdown.contains(e.target)) {
      searchDropdown.classList.remove('active');
    }
  });

  // 11. Modals Management
  function openModal(modalId) {
    document.getElementById(modalId).classList.add('active');
  }

  function closeModal(modalId) {
    document.getElementById(modalId).classList.remove('active');
  }

  document.querySelectorAll('[data-close]').forEach(btn => {
    btn.addEventListener('click', (e) => {
      const overlay = e.target.closest('.modal-overlay');
      if (overlay) overlay.classList.remove('active');
    });
  });

  // Register User
  const modalRegister = 'modalRegisterUser';
  document.getElementById('btnOpenRegisterModal').addEventListener('click', () => openModal(modalRegister));
  document.getElementById('btnRegisterUserDirect').addEventListener('click', () => openModal(modalRegister));
  document.getElementById('btnAddNewUser').addEventListener('click', () => openModal(modalRegister));
  document.getElementById('linkNewUser').addEventListener('click', () => openModal(modalRegister));

  document.getElementById('btnSubmitRegister').addEventListener('click', () => {
    const name = document.getElementById('regName').value.trim();
    const headline = document.getElementById('regHeadline').value.trim();
    const city = document.getElementById('regCity').value.trim();
    const interests = document.getElementById('regInterests').value.trim();

    if (!name) {
      alert('Please enter a username.');
      return;
    }

    const newId = graph.registerUser({ name, headline, city, interests });
    if (newId === -2) {
      alert(`User "${name}" already exists (case-insensitive duplicate check in hash table).`);
      return;
    }

    closeModal(modalRegister);
    showToast(`Registered "${name}" successfully (User ID: ${newId})`, 'success');
    document.getElementById('regName').value = '';
    document.getElementById('regHeadline').value = '';
    document.getElementById('regCity').value = '';
    document.getElementById('regInterests').value = '';

    refreshUI();
  });

  // Mutual Friends Explorer Modal
  const modalMutual = 'modalMutualFriends';
  const mutualSelectU = document.getElementById('mutualSelectU');
  const mutualSelectV = document.getElementById('mutualSelectV');
  const mutualResultsBox = document.getElementById('mutualResultsBox');

  function setupMutualModal() {
    mutualSelectU.innerHTML = '';
    mutualSelectV.innerHTML = '';
    graph.users.forEach(u => {
      if (!u.isDeleted) {
        mutualSelectU.innerHTML += `<option value="${u.id}">${u.name} (ID: ${u.id})</option>`;
        mutualSelectV.innerHTML += `<option value="${u.id}">${u.name} (ID: ${u.id})</option>`;
      }
    });
    if (graph.users.length > 2) {
      mutualSelectU.selectedIndex = 0; // Rahul
      mutualSelectV.selectedIndex = 2; // Priya
    }
    mutualResultsBox.innerHTML = '';
    openModal(modalMutual);
  }

  document.getElementById('linkMutualExplorer').addEventListener('click', setupMutualModal);
  document.getElementById('btnFindMutuals').addEventListener('click', setupMutualModal);

  document.getElementById('btnCalculateMutuals').addEventListener('click', () => {
    const u = parseInt(mutualSelectU.value, 10);
    const v = parseInt(mutualSelectV.value, 10);

    if (u === v) {
      mutualResultsBox.innerHTML = `<div style="padding: 10px; background: #fff3cd; border-radius: 4px; font-size: 12px;">Please select two different users.</div>`;
      return;
    }

    const mutualIds = graph.getMutualFriends(u, v);
    const nameU = graph.users[u].name;
    const nameV = graph.users[v].name;

    if (mutualIds.length === 0) {
      mutualResultsBox.innerHTML = `
        <div style="padding: 12px; background: #f8f9fa; border-radius: 4px; font-size: 13px;">
          ❌ <strong>${nameU}</strong> and <strong>${nameV}</strong> share <strong>0</strong> mutual connections.
        </div>
      `;
    } else {
      const list = mutualIds.map(id => `<li style="margin-bottom: 4px; cursor: pointer; color: #0a66c2;" onclick="window.openUserProfile(${id})">${graph.users[id].name} (${graph.users[id].headline})</li>`).join('');
      mutualResultsBox.innerHTML = `
        <div style="padding: 12px; background: #e6f4ea; border-radius: 4px; font-size: 13px; color: #057642;">
          ✅ Found <strong>${mutualIds.length}</strong> mutual connection(s) between <strong>${nameU}</strong> and <strong>${nameV}</strong>:
          <ul style="margin: 8px 0 0 20px;">${list}</ul>
        </div>
      `;
    }
  });

  // djb2 Hash Table Inspector Modal
  document.getElementById('linkHashDeepDive').addEventListener('click', () => {
    const content = document.getElementById('hashTableContent');
    const inspected = graph.userIndex.inspect();

    let html = `
      <p style="font-size: 12px; color: var(--text-secondary); margin-bottom: 12px;">
        djb2 string hash (Capacity = 53 buckets). Collisions are resolved using Separate Chaining:
      </p>
      <div style="display: flex; flex-direction: column; gap: 8px;">
    `;

    inspected.forEach(b => {
      html += `
        <div style="background: #f8f9fa; border: 1px solid var(--border-color); border-radius: 4px; padding: 8px 12px; font-size: 12px;">
          <div style="font-weight: 600; color: #0a66c2; margin-bottom: 4px;">Bucket [${b.index}] &rarr; Chain Length: ${b.chainLength}</div>
          <div style="font-family: monospace; color: #333;">${b.entries.join(' &rarr; ')}</div>
        </div>
      `;
    });

    html += `</div>`;
    content.innerHTML = html;
    openModal('modalHashTable');
  });

  // Export network.txt Modal
  function showFileTextModal() {
    const text = graph.exportToText();
    document.getElementById('fileTextContent').value = text;
    openModal('modalFileText');
  }

  document.getElementById('linkExportTxt').addEventListener('click', showFileTextModal);
  document.getElementById('btnExportTxtRight').addEventListener('click', showFileTextModal);
  document.getElementById('btnTriggerFileModal').addEventListener('click', showFileTextModal);

  document.getElementById('btnCopyFileText').addEventListener('click', () => {
    const textarea = document.getElementById('fileTextContent');
    textarea.select();
    navigator.clipboard.writeText(textarea.value);
    showToast('network.txt content copied to clipboard!', 'success');
  });

  // Reset 15 People Network
  function handleLoadDemo() {
    graph.loadDemoData();
    activeUserId = 0; // Rahul
    refreshUI();
    showToast('Network reset to 15 members with rich recommendations!', 'success');
  }

  document.getElementById('btnQuickDemo').addEventListener('click', handleLoadDemo);
  document.getElementById('btnLoadDemoPreset').addEventListener('click', handleLoadDemo);

  // Canvas Reset Physics
  document.getElementById('btnResetPhysics').addEventListener('click', () => {
    networkCanvas.updateData(activeUserId, recommendationData);
    showToast('Graph re-centered and equilibrium recalculated.', 'info');
  });

  // Viva Accordion Toggle
  document.querySelectorAll('.accordion-header').forEach(hdr => {
    hdr.addEventListener('click', () => {
      const parent = hdr.parentElement;
      parent.classList.toggle('active');
    });
  });

  // Initial UI Render
  refreshUI();
});
