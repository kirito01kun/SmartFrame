const express = require("express");
const app = express();
const cors = require("cors");
app.use(express.json());
app.use(cors());


let armed = false;
let logs = [
  { timestamp: new Date().toISOString(), type: "boot", sensor: "system" }
];

// Status endpoint
app.get("/status", (req, res) => {
  res.json({ ok: true });
});

// Logs endpoint
app.get("/logs", (req, res) => {
  res.json({ logs, armed });
});

// Arm endpoint
app.post("/arm", (req, res) => {
  armed = true;
  logs.push({ timestamp: new Date().toISOString(), type: "armed", sensor: "system" });
  res.json({ ok: true });
});

// Disarm endpoint
app.post("/disarm", (req, res) => {
  armed = false;
  logs.push({ timestamp: new Date().toISOString(), type: "disarmed", sensor: "system" });
  res.json({ ok: true });
});

// Add a random motion event every 10s
setInterval(() => {
  if (armed) {
    logs.push({ timestamp: new Date().toISOString(), type: "motion", sensor: "pir1" });
    if (logs.length > 50) logs.shift();
  }
}, 10000);

const PORT = 8081;
app.listen(PORT, () => {
  console.log(`Mock ESP running at http://localhost:${PORT}`);
});
