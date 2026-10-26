const http = require('http');
const fs = require('fs');
const path = require('path');

const DEFAULT_PORT = parseInt(process.env.PORT, 10) || 4000;
const PUBLIC_DIR = path.resolve(__dirname);

const MIME_TYPES = {
  '.html': 'text/html; charset=UTF-8',
  '.css': 'text/css; charset=UTF-8',
  '.js': 'application/javascript; charset=UTF-8',
  '.json': 'application/json; charset=UTF-8',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.jpeg': 'image/jpeg',
  '.gif': 'image/gif',
  '.svg': 'image/svg+xml',
  '.ico': 'image/x-icon',
  '.txt': 'text/plain; charset=UTF-8'
};

function createServer(port) {
  const server = http.createServer((req, res) => {
    try {
      const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
      let pathname = decodeURIComponent(parsedUrl.pathname);

      if (pathname === '/') {
        pathname = '/index.html';
      }

      // Safe path resolution
      const safePath = path.resolve(PUBLIC_DIR, '.' + pathname);

      // Verify path is within PUBLIC_DIR
      if (!safePath.startsWith(PUBLIC_DIR)) {
        res.writeHead(403, { 'Content-Type': 'text/plain; charset=UTF-8' });
        res.end('403 Forbidden');
        return;
      }

      fs.stat(safePath, (err, stats) => {
        if (err || !stats.isFile()) {
          res.writeHead(404, { 'Content-Type': 'text/html; charset=UTF-8' });
          res.end(`<h1>404 Not Found</h1><p>File <code>${pathname}</code> not found.</p>`);
          return;
        }

        const ext = path.extname(safePath).toLowerCase();
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';

        res.writeHead(200, {
          'Content-Type': contentType,
          'Cache-Control': 'no-cache, no-store, must-revalidate'
        });

        const stream = fs.createReadStream(safePath);
        stream.pipe(res);
      });
    } catch (e) {
      res.writeHead(500, { 'Content-Type': 'text/plain; charset=UTF-8' });
      res.end('500 Internal Server Error');
    }
  });

  server.on('error', (err) => {
    if (err.code === 'EADDRINUSE') {
      console.warn(`[!] Port ${port} is occupied, trying port ${port + 1}...`);
      createServer(port + 1);
    } else {
      console.error('[ERROR] Server failed:', err);
    }
  });

  server.listen(port, () => {
    console.log('\n================================================================');
    console.log('   LINKEDIN CLONE + GRAPH BFS FRIEND RECOMMENDATION SYSTEM      ');
    console.log('       Course: BTech AIML | Data Structures and Algorithms      ');
    console.log('================================================================');
    console.log(`\n  >> Web Application running at: http://localhost:${port}`);
    console.log(`  >> Press Ctrl+C in terminal to stop server.\n`);
    console.log('================================================================\n');
  });
}

createServer(DEFAULT_PORT);
