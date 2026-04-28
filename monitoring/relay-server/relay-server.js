// relay-server.js
const net = require("net");
const WebSocket = require("ws");

// =============================
// 설정
// =============================
const TCP_HOST = "127.0.0.1";
const TCP_PORT = 9000;
const WS_PORT = 8080;

let wss = null;
let tcpClient = null;
let buffer = Buffer.alloc(0);

// =============================
// main
function main() {
  wss = startWebSocketServer(WS_PORT);
  setupHeartbeat(wss);
  connectTCP();
}

// =============================
// WebSocket 서버
function startWebSocketServer(port) {
  const server = new WebSocket.Server({ port });
  console.log(`[WS] Server started on port ${port}`);

  server.on("connection", (ws) => {
    console.log("[WS] Client connected");
    ws.isAlive = true;

    ws.on("pong", () => (ws.isAlive = true));
    ws.on("close", () => console.log("[WS] Client disconnected"));
  });

  return server;
}

// =============================
// Heartbeat
function setupHeartbeat(server) {
  setInterval(() => {
    server.clients.forEach((ws) => {
      if (!ws.isAlive) return ws.terminate();
      ws.isAlive = false;
      ws.ping();
    });
  }, 30000);
}

// =============================
// TCP 연결
function connectTCP() {
  tcpClient = new net.Socket();

  // 🔥 reconnect 시 버퍼 초기화
  buffer = Buffer.alloc(0);

  tcpClient.connect(TCP_PORT, TCP_HOST, () => {
    console.log(`[TCP] Connected to ${TCP_HOST}:${TCP_PORT}`);
  });

  // 🔥 절대 중첩 등록 금지
  tcpClient.on("data", (data) => {
    console.log("📦 RAW chunk size:", data.length);
    console.log(data); // Buffer 그대로 확인

    buffer = Buffer.concat([buffer, data]);
    handleTCPBuffer();
  });

  tcpClient.on("close", () => {
    console.log("[TCP] Connection closed. Reconnecting...");
    setTimeout(connectTCP, 1000);
  });

  tcpClient.on("error", (err) => {
    console.error("[TCP] Error:", err.message);
  });
}

// =============================
// Length-prefix 처리
function handleTCPBuffer() {
  while (buffer.length >= 4) {
    // 1️⃣ 길이 읽기
    const msgLen = buffer.readUInt32BE(0);

    console.log("📏 msgLen:", msgLen);
    console.log("📦 buffer length:", buffer.length);

    // 2️⃣ 데이터 부족하면 대기
    if (buffer.length < 4 + msgLen) break;

    // 3️⃣ JSON 추출
    const jsonBuf = buffer.slice(4, 4 + msgLen);

    // console.log("🧩 JSON RAW:");
    // console.log(jsonBuf.toString().slice(0, 200)); // 너무 길어서 일부만

    buffer = buffer.slice(4 + msgLen);

    try {
      const msg = JSON.parse(jsonBuf.toString());

      console.log("✅ PARSED FULL JSON ↓↓↓");
      console.log(JSON.stringify(msg, null, 2)); // 🔥 핵심

      broadcast(msg);
    } catch (err) {
      console.error("❌ JSON parse failed:", err.message);
    }

    // try {
    //   const msg = JSON.parse(jsonBuf.toString());

    //   console.log("✅ PARSED OK");
    //   console.log("timestamp:", msg.timestamp);
    //   console.log("sensor[0]:", msg.sensors[0]);

    //   broadcast(msg);
    // } catch (err) {
    //   console.error("❌ JSON parse failed:", err.message);
    // }
  }
}

// =============================
// WebSocket broadcast
function broadcast(data) {
  const message = JSON.stringify(data);

  wss.clients.forEach((ws) => {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(message);
    }
  });
}

// =============================
main();
