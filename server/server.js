const http = require("node:http");

const port = Number(process.env.PORT) || 3000;

const server = http.createServer((request, response) => {
  if (request.method === "GET" && request.url === "/health") {
	response.writeHead(200, { "Content-Type": "application/json" });
	response.end(JSON.stringify({ status: "ok" }));
	return;
  }

  response.writeHead(404, { "Content-Type": "application/json" });
  response.end(JSON.stringify({ error: "Not found" }));
});

server.listen(port, "0.0.0.0", () => {
  console.log(`Server listening on port ${port}`);
});
