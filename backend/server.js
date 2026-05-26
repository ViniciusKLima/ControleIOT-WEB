const express = require("express");

const admin = require("firebase-admin");

// ========================================
// FIREBASE
// ========================================

const serviceAccount =
require("./chave-firebase.json");

admin.initializeApp({
  credential:
    admin.credential.cert(serviceAccount),

  databaseURL:
    "https://controleiot-web-default-rtdb.firebaseio.com"
});

const db = admin.database();

// ========================================
// EXPRESS
// ========================================

const app = express();

app.get("/", (req, res) => {

  res.send("Backend ESP32 ONLINE");

});

// ========================================
// MONITOR ESP32
// ========================================

setInterval(async () => {

  try {

    // pega lastSeen
    const snapshot =
      await db.ref("lastSeen").get();

    const lastSeen =
      snapshot.val() || 0;

    // tempo atual
    const now =
      Math.floor(Date.now() / 1000);

    // diferença
    const diff = now - lastSeen;

    // online/offline
    const online = diff <= 8;

    // atualiza status
    await db.ref("espOnline")
      .set(online);

    console.log(
      "ESP:",
      online ? "ONLINE" : "OFFLINE"
    );

  }

  catch (error) {

    console.log(error);

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