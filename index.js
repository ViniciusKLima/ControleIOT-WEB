// Firebase
import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js";

import {
  getDatabase,
  ref,
  set,
  onValue,
  get
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";

// =========================
// CONFIG FIREBASE
// =========================

const firebaseConfig = {
  apiKey: "AIzaSyAR9zltVe8E45Sk3WcVnmXZ42BZRoqycHc",
  authDomain: "controleiot-web.firebaseapp.com",
  databaseURL: "https://controleiot-web-default-rtdb.firebaseio.com",
  projectId: "controleiot-web",
  storageBucket: "controleiot-web.firebasestorage.app",
  messagingSenderId: "730183038031",
  appId: "1:730183038031:web:c6a8b0d818de96bb84a63a"
};

// =========================
// FIREBASE
// =========================

const app = initializeApp(firebaseConfig);

const db = getDatabase(app);

// =========================
// ELEMENTOS
// =========================

const lampSections = document.querySelectorAll(".lamp");

const statusESP = document.querySelector(".status-esp32 span");

// =========================
// STATUS ESP32
// =========================

onValue(ref(db, "espOnline"), (snapshot) => {

  const online = snapshot.val();

  if (online) {

    statusESP.textContent = "CONECTADO";

    statusESP.style.color = "#00ff88";

  } else {

    statusESP.textContent = "DESCONECTADO";

    statusESP.style.color = "#ff3b3b";

  }

});

// =========================
// LÂMPADAS
// =========================

lampSections.forEach((section, index) => {

  const lampId = index + 1;

  const toggle = section.querySelector(".toggle");

  const timerButtons = section.querySelectorAll(".btn-timer");

  // =========================
  // CLICK TOGGLE
  // =========================

  toggle.addEventListener("click", async () => {

    const lampRef = ref(db, `lamp${lampId}`);

    const snapshot = await get(lampRef);

    const currentState = snapshot.val();

    const newState = !currentState;

    // Atualiza lâmpada
    await set(lampRef, newState);

    // Se desligar -> remove timer
    if (!newState) {

      await set(ref(db, `lamp${lampId}Timer`), 0);

    }

  });

  // =========================
  // SINCRONIZA TOGGLE
  // =========================

  onValue(ref(db, `lamp${lampId}`), (snapshot) => {

    const state = snapshot.val();

    if (state) {

      toggle.classList.add("toggle-ativo");

    } else {

      toggle.classList.remove("toggle-ativo");

      // remove leds timer
      section.querySelectorAll(".led-timer").forEach((led) => {

        led.classList.remove("ativo");

      });

    }

  });

  // =========================
  // BOTÕES TIMER
  // =========================

  timerButtons.forEach((button) => {

    button.addEventListener("click", async () => {

      const led = button.querySelector(".led-timer");

      const alreadyActive = led.classList.contains("ativo");

      // remove todos
      section.querySelectorAll(".led-timer").forEach((l) => {

        l.classList.remove("ativo");

      });

      // desativa se clicar no mesmo
      if (alreadyActive) {

        await set(ref(db, `lamp${lampId}Timer`), 0);

        return;

      }

      // ativa visual
      led.classList.add("ativo");

      // texto botão
      const text = button.querySelector("p").textContent;

      let seconds = 0;

      if (text === "10s") seconds = 10;

      if (text === "30s") seconds = 30;

      if (text === "1min") seconds = 60;

      // envia timer
      await set(ref(db, `lamp${lampId}Timer`), seconds);

    });

  });

  // =========================
  // SINCRONIZA TIMER VISUAL
  // =========================

  onValue(ref(db, `lamp${lampId}Timer`), (snapshot) => {

    const value = snapshot.val();

    // remove todos
    section.querySelectorAll(".led-timer").forEach((led) => {

      led.classList.remove("ativo");

    });

    // ativa correspondente

    if (value === 10) {

      timerButtons[0]
        .querySelector(".led-timer")
        .classList.add("ativo");

    }

    if (value === 30) {

      timerButtons[1]
        .querySelector(".led-timer")
        .classList.add("ativo");

    }

    if (value === 60) {

      timerButtons[2]
        .querySelector(".led-timer")
        .classList.add("ativo");

    }

  });

});