#include <WiFi.h>

#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include <time.h>

// ======================================================
// WIFI
// ======================================================

#define WIFI_SSID "testeesp"
#define WIFI_PASSWORD "vinicius"

// ======================================================
// FIREBASE
// ======================================================

#define API_KEY "AIzaSyAR9zltVe8E45Sk3WcVnmXZ42BZRoqycHc"

#define DATABASE_URL "https://controleiot-web-default-rtdb.firebaseio.com"

// ======================================================
// RELAYS
// ======================================================

#define RELAY1 25
#define RELAY2 26

// ======================================================
// RGB STATUS
// RGB ÂNODO COMUM
// LOW  = ACENDE
// HIGH = APAGA
// ======================================================

#define RGB_RED 16
#define RGB_GREEN 17

// ======================================================
// LED TIMER
// INVERTIDO
// LOW  = ACENDE
// HIGH = APAGA
// ======================================================

#define TIMER_LED 4

// ======================================================
// FIREBASE OBJETOS
// ======================================================

FirebaseData fbdo;

FirebaseAuth auth;

FirebaseConfig config;

// ======================================================
// TIMERS
// ======================================================

unsigned long timer1Start = 0;
unsigned long timer2Start = 0;

int timer1Duration = 0;
int timer2Duration = 0;

// ======================================================
// CONTROLE DE TEMPO
// ======================================================

unsigned long lastFirebaseCheck = 0;

unsigned long lastHeartbeat = 0;

// ======================================================
// FUNÇÃO WIFI
// ======================================================

void connectWiFi() {

  Serial.println("");
  Serial.print("Conectando WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

  }

  Serial.println("");
  Serial.println("WiFi conectado");

  Serial.print("IP: ");

  Serial.println(WiFi.localIP());

}

// ======================================================
// FUNÇÃO FIREBASE
// ======================================================

void connectFirebase() {

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  // login anônimo
  if (Firebase.signUp(&config, &auth, "", "")) {

    Serial.println("Firebase signup OK");

  }

  else {

    Serial.printf(
      "Signup erro: %s\n",
      config.signer.signupError.message.c_str()
    );

  }

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

  Serial.print("Conectando Firebase");

  while (!Firebase.ready()) {

    delay(500);

    Serial.print(".");

  }

  Serial.println("");
  Serial.println("Firebase READY");

}

// ======================================================
// FUNÇÃO HEARTBEAT
// ENVIA SINAL DE VIDA
// ======================================================

void updateHeartbeat() {

  if (millis() - lastHeartbeat >= 2000) {

    lastHeartbeat = millis();

    int currentTime =
      (int)time(nullptr);

    bool success =
      Firebase.RTDB.setInt(
        &fbdo,
        "/lastSeen",
        currentTime
      );

    if (success) {

      Serial.println("Heartbeat enviado");

    }

    else {

      Serial.println("Erro heartbeat");

    }

  }

}

// ======================================================
// FUNÇÃO LÂMPADA 1
// ======================================================

void updateLamp1() {

  if (Firebase.RTDB.getBool(&fbdo, "/lamp1")) {

    bool lamp1 = fbdo.boolData();

    // relé invertido
    digitalWrite(RELAY1, lamp1 ? LOW : HIGH);

    // se desligou a lâmpada
    // remove timer também
    if (!lamp1) {

      timer1Duration = 0;

      Firebase.RTDB.setInt(
        &fbdo,
        "/lamp1Timer",
        0
      );

    }

  }

}

// ======================================================
// FUNÇÃO LÂMPADA 2
// ======================================================

void updateLamp2() {

  if (Firebase.RTDB.getBool(&fbdo, "/lamp2")) {

    bool lamp2 = fbdo.boolData();

    // relé invertido
    digitalWrite(RELAY2, lamp2 ? LOW : HIGH);

    // se desligou a lâmpada
    // remove timer também
    if (!lamp2) {

      timer2Duration = 0;

      Firebase.RTDB.setInt(
        &fbdo,
        "/lamp2Timer",
        0
      );

    }

  }

}

// ======================================================
// FUNÇÃO TIMER 1
// ======================================================

void updateTimer1() {

  if (Firebase.RTDB.getInt(&fbdo, "/lamp1Timer")) {

    int value = fbdo.intData();

    // iniciou timer
    if (value > 0 && timer1Duration != value) {

      timer1Duration = value;

      timer1Start = millis();

      Serial.print("Timer1 iniciado: ");

      Serial.println(value);

    }

    // timer desligado
    if (value == 0) {

      timer1Duration = 0;

    }

  }

}

// ======================================================
// FUNÇÃO TIMER 2
// ======================================================

void updateTimer2() {

  if (Firebase.RTDB.getInt(&fbdo, "/lamp2Timer")) {

    int value = fbdo.intData();

    // iniciou timer
    if (value > 0 && timer2Duration != value) {

      timer2Duration = value;

      timer2Start = millis();

      Serial.print("Timer2 iniciado: ");

      Serial.println(value);

    }

    // timer desligado
    if (value == 0) {

      timer2Duration = 0;

    }

  }

}

// ======================================================
// VERIFICA TIMER 1
// ======================================================

void checkTimer1() {

  if (timer1Duration > 0) {

    if (millis() - timer1Start >= timer1Duration * 1000) {

      Serial.println("Timer1 FINALIZADO");

      Firebase.RTDB.setBool(
        &fbdo,
        "/lamp1",
        false
      );

      Firebase.RTDB.setInt(
        &fbdo,
        "/lamp1Timer",
        0
      );

      timer1Duration = 0;

    }

  }

}

// ======================================================
// VERIFICA TIMER 2
// ======================================================

void checkTimer2() {

  if (timer2Duration > 0) {

    if (millis() - timer2Start >= timer2Duration * 1000) {

      Serial.println("Timer2 FINALIZADO");

      Firebase.RTDB.setBool(
        &fbdo,
        "/lamp2",
        false
      );

      Firebase.RTDB.setInt(
        &fbdo,
        "/lamp2Timer",
        0
      );

      timer2Duration = 0;

    }

  }

}

// ======================================================
// LED TIMER
// ======================================================

void updateTimerLED() {

  bool anyTimerActive =
    (timer1Duration > 0) ||
    (timer2Duration > 0);

  // led invertido
  digitalWrite(
    TIMER_LED,
    anyTimerActive ? HIGH : LOW
  );

}

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  // ==================================================
  // RELAYS
  // ==================================================

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  // relés desligados
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  // ==================================================
  // RGB STATUS
  // ==================================================

  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);

  // vermelho inicial
  digitalWrite(RGB_RED, LOW);
  digitalWrite(RGB_GREEN, HIGH);

  // ==================================================
  // LED TIMER
  // ==================================================

  pinMode(TIMER_LED, OUTPUT);

  // apagado
  digitalWrite(TIMER_LED, LOW);

  // ==================================================
  // WIFI
  // ==================================================

  connectWiFi();

  // verde conectado
  digitalWrite(RGB_RED, HIGH);
  digitalWrite(RGB_GREEN, LOW);

  // ==================================================
  // SINCRONIZA HORÁRIO
  // ==================================================

  configTime(0, 0, "pool.ntp.org");

  Serial.print("Sincronizando horário");

  time_t now = time(nullptr);

  while (now < 100000) {

    delay(500);

    Serial.print(".");

    now = time(nullptr);

  }

  Serial.println("");
  Serial.println("Horário sincronizado");

  // ==================================================
  // FIREBASE
  // ==================================================

  connectFirebase();

}

// ======================================================
// LOOP
// ======================================================

void loop() {

  // ==================================================
  // VERIFICA WIFI
  // ==================================================

  if (WiFi.status() != WL_CONNECTED) {

    // vermelho
    digitalWrite(RGB_RED, LOW);
    digitalWrite(RGB_GREEN, HIGH);

    return;

  }

  // verde
  digitalWrite(RGB_RED, HIGH);
  digitalWrite(RGB_GREEN, LOW);

  // ==================================================
  // HEARTBEAT
  // ==================================================

  updateHeartbeat();

  // ==================================================
  // LEITURA FIREBASE
  // ==================================================

  if (millis() - lastFirebaseCheck >= 300) {

    lastFirebaseCheck = millis();

    updateLamp1();

    updateLamp2();

    updateTimer1();

    updateTimer2();

  }

  // ==================================================
  // VERIFICA TIMERS
  // ==================================================

  checkTimer1();

  checkTimer2();

  // ==================================================
  // LED TIMER
  // ==================================================

  updateTimerLED();

}