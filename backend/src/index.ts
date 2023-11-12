import * as WebSocket from "ws";
import { createServer } from "http";

const server = createServer();
const wss = new WebSocket.Server({ server });

wss.on("connection", (ws) => {
  // Handle messages from the client
  ws.on("message", (message) => {
    console.log(message);

    // console.log(`Received: ${message}`);
    // Broadcast the message to all connected clients
    wss.clients.forEach(async (client) => {
      if (ws === client || client.readyState !== WebSocket.OPEN) return;
      client.send(message);
    });
  });

  // Handle client disconnection
  ws.on("close", () => {
    console.log("Client disconnected");
  });
});

const PORT = 3010;
server.listen(PORT, () => {
  console.log(`WebSocket server listening on port ${PORT}`);
});
