const express = require("express");
const cors = require("cors");

// ========================================
// FIREBASE
// ========================================

const admin = require("firebase-admin");

const serviceAccount = JSON.parse(
  process.env.FIREBASE_SERVICE_ACCOUNT
);

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),

  databaseURL:
    "https://controleiot-web-default-rtdb.firebaseio.com"
});

const db = admin.database();

// ========================================
// EXPRESS
// ========================================

const app = express();

app.use(cors());

app.use(express.json());

// ========================================
// ROTA TESTE
// ========================================

app.get("/", (req, res) => {

  res.send("Backend ESP32 ONLINE");

});

// ========================================
// STATUS ESP32
// ========================================

app.get("/status", async (req, res) => {

  try {

    const snapshot =
      await db.ref("espOnline").get();

    const online =
      snapshot.val() || false;

    res.json({
      online
    });

  }

  catch (error) {

    res.status(500).json({
      error: error.message
    });

  }

});

// ========================================
// MONITOR ESP32
// ========================================

setInterval(async () => {

  try {

    // ================================
    // PEGA LASTSEEN
    // ================================

    const snapshot =
      await db.ref("lastSeen").get();

    const lastSeen =
      snapshot.val() || 0;

    // ================================
    // HORÁRIO ATUAL
    // ================================

    const now =
      Math.floor(Date.now() / 1000);

    // ================================
    // DIFERENÇA
    // ================================

    const diff = now - lastSeen;

    // ================================
    // DEFINE ONLINE/OFFLINE
    // ================================

    const online = diff <= 8;

    // ================================
    // ATUALIZA FIREBASE
    // ================================

    await db.ref("espOnline")
      .set(online);

    console.log(
      "ESP:",
      online ? "ONLINE" : "OFFLINE"
    );

  }

  catch (error) {

    console.log(
      "Erro monitor ESP:",
      error
    );

  }

}, 2000);

// ========================================
// START SERVER
// ========================================

const PORT =
  process.env.PORT || 3000;

app.listen(PORT, () => {

  console.log(
    `Servidor rodando na porta ${PORT}`
  );

});