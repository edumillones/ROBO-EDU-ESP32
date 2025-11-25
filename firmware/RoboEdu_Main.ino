#include <WiFi.h>
#include <WebServer.h>

// -----------------------------------------------------------------------------
// PROYECTO: ROBO-EDU "LOGIC LAB"
// OBJETIVO: PLATAFORMA EDUCATIVA STEM (ODS 4: EDUCACIÓN DE CALIDAD)
// HARDWARE: ESP32 + DRV8833 + SENSORES
// -----------------------------------------------------------------------------

// --- 1. CONFIGURACIÓN DE RED ---
const char* ssid = "TU_WIFI";      // <--- CAMBIAR
const char* password = "TU_CLAVE"; // <--- CAMBIAR

WebServer server(80);

// --- 2. MODOS EDUCATIVOS ---
enum EduMode {
  IDLE,           // En espera
  AUTO_LOGIC,     // Lógica Difusa (El robot "piensa")
  MODE_FOLLOW,    // Seguimiento (Antes Ataque)
  MODE_ZONE,      // Respetar límites (Antes Defensa)
  MODE_DEMO       // Demostración de movimientos
};

EduMode currentMode = IDLE;

// --- 3. HARDWARE MAPPING (DRV8833) ---
#define MOT_A1 25
#define MOT_A2 27
#define MOT_B1 4
#define MOT_B2 13

// SENSORES (IR PISO y ULTRASONIDOS)
#define FLOOR_L 32
#define FLOOR_R 33

const int Echo1 = 18; const int Trigger1 = 19; // Izq
const int Echo2 = 16; const int Trigger2 = 17; // Centro
const int Echo3 = 14; const int Trigger3 = 15; // Der
#define LED_SYS 2 

// CONFIG PWM
const int pwmFreq = 5000;
const int pwmRes = 8;
const int ch_A1=0, ch_A2=1, ch_B1=2, ch_B2=3;

// --- 4. INTERFAZ WEB EDUCATIVA (HTML/CSS) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>ROBO-EDU LAB</title>
    <style>
        :root {
            --primary: #4A90E2; /* Azul Educativo */
            --success: #7ED321; /* Verde Lógica */
            --warning: #F5A623; /* Naranja Acción */
            --danger: #D0021B;  /* Rojo Stop */
            --bg: #F0F4F8;
            --card: #FFFFFF;
        }
        body {
            background-color: var(--bg);
            color: #333;
            font-family: 'Verdana', sans-serif;
            text-align: center;
            margin: 0; padding: 20px;
            display: flex; flex-direction: column; align-items: center;
        }
        h1 { color: var(--primary); margin-bottom: 5px; }
        p { color: #666; font-size: 0.9rem; margin-top: 0; }
        
        .status-card {
            background: var(--card);
            padding: 15px;
            border-radius: 15px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            width: 90%; max-width: 400px;
            margin-bottom: 20px;
            border-left: 5px solid var(--success);
        }
        #status-text { font-weight: bold; font-size: 1.2rem; color: var(--primary); }

        .grid-container {
            display: grid; grid-template-columns: 1fr 1fr; gap: 15px;
            width: 100%; max-width: 400px;
        }

        button {
            border: none; border-radius: 12px;
            padding: 20px; font-size: 1rem; font-weight: bold;
            cursor: pointer; transition: transform 0.1s;
            color: white; box-shadow: 0 4px 0 rgba(0,0,0,0.2);
        }
        button:active { transform: translateY(4px); box-shadow: none; }

        .btn-logic { background-color: var(--success); grid-column: span 2; }
        .btn-action { background-color: var(--primary); }
        .btn-zone { background-color: var(--warning); }
        .btn-stop { background-color: var(--danger); grid-column: span 2; margin-top: 10px;}
        
        .footer { margin-top: 30px; font-size: 0.8rem; color: #999; }
    </style>
</head>
<body>
    <h1>ROBO-EDU LAB</h1>
    <p>Aprendizaje de Lógica y Robótica</p>

    <div class="status-card">
        <small>ESTADO DEL SISTEMA:</small><br>
        <span id="status-text">LISTO PARA APRENDER</span>
    </div>

    <div class="grid-container">
        <button class="btn-logic" onclick="cmd('auto')">▶ EJECUTAR LÓGICA DIFUSA</button>
        <button class="btn-action" onclick="cmd('follow')">MODO "SÍGUEME"</button>
        <button class="btn-zone" onclick="cmd('zone')">TEST "ZONA SEGURA"</button>
        <button class="btn-action" onclick="cmd('demo')">DEMO MOTORES</button>
        
        <button class="btn-stop" onclick="cmd('stop')">⏹ DETENER PROGRAMA</button>
    </div>

    <div class="footer">
        PROYECTO DE RESPONSABILIDAD SOCIAL<br>
        Ingeniería Mecatrónica - UPN
    </div>

    <script>
        function cmd(mode) {
            // Feedback visual educativo
            const txt = document.getElementById('status-text');
            const msgs = {
                'auto': 'PENSANDO (Fuzzy Logic)...',
                'follow': 'EJECUTANDO: if(obj < 20)',
                'zone': 'VALIDANDO LÍMITES...',
                'demo': 'TEST DE HARDWARE',
                'stop': 'PROGRAMA DETENIDO'
            };
            txt.innerText = msgs[mode] || mode.toUpperCase();
            
            // Envío al servidor
            fetch('/api?mode=' + mode);
        }
    </script>
</body>
</html>
)rawliteral";

// --- DECLARACIONES ---
void setMotorA(int v); void setMotorB(int v);
void stopMotors(); void move(int l, int r);
long getDist(int t, int e);
void runEducationalFuzzy(); 
void runFollowMode();

void setup() {
  Serial.begin(115200);
  
  // Configuración de Pines
  pinMode(FLOOR_L, INPUT); pinMode(FLOOR_R, INPUT);
  pinMode(Trigger1, OUTPUT); pinMode(Echo1, INPUT);
  pinMode(Trigger2, OUTPUT); pinMode(Echo2, INPUT);
  pinMode(Trigger3, OUTPUT); pinMode(Echo3, INPUT);
  pinMode(LED_SYS, OUTPUT);

  // PWM DRV8833
  ledcAttachChannel(MOT_A1, pwmFreq, pwmRes, ch_A1);
  ledcAttachChannel(MOT_A2, pwmFreq, pwmRes, ch_A2);
  ledcAttachChannel(MOT_B1, pwmFreq, pwmRes, ch_B1);
  ledcAttachChannel(MOT_B2, pwmFreq, pwmRes, ch_B2);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("\n--- ROBO-EDU CONECTADO ---");
  Serial.println(WiFi.localIP());

  // API
  server.on("/", [](){ server.send(200, "text/html", index_html); });
  server.on("/api", [](){
    String m = server.arg("mode");
    if(m=="stop") currentMode=IDLE;
    else if(m=="auto") currentMode=AUTO_LOGIC;
    else if(m=="follow") currentMode=MODE_FOLLOW;
    else if(m=="zone") currentMode=MODE_ZONE;
    else if(m=="demo") currentMode=MODE_DEMO;
    server.send(200, "text/plain", "OK");
  });
  server.begin();
}

void loop() {
  server.handleClient();

  // 1. SEGURIDAD (LECCIÓN: LÓGICA BOOLEANA)
  // Siempre verificamos si nos salimos de la zona segura (mesa/pista)
  if (currentMode != IDLE) {
    if (!digitalRead(FLOOR_L) || !digitalRead(FLOOR_R)) {
      move(-200, -200); delay(400); // Retroceder
      move(200, -200);  delay(300); // Girar para volver a zona segura
      // No retornamos, solo corregimos y seguimos la lógica
    }
  }

  // 2. CEREBRO DEL ROBOT
  switch (currentMode) {
    case IDLE: stopMotors(); break;
    
    case AUTO_LOGIC: 
      runEducationalFuzzy(); // Tu sistema experto IA
      break;
      
    case MODE_FOLLOW: // Antes era Ataque, ahora sigue la mano
      runFollowMode();
      break;

    case MODE_ZONE: // Modo pacífico, solo evita caerse
      move(100, 100); // Avanza lento hasta encontrar borde (arriba ya lo gestiona)
      break;
      
    case MODE_DEMO:
      move(150, 150); delay(500);
      move(-150, -150); delay(500);
      stopMotors(); currentMode = IDLE;
      break;
  }
}

// =============================================================================
// LÓGICA DIFUSA EDUCATIVA (SISTEMA EXPERTO)
// =============================================================================
// Explica esto como: "El robot decide su velocidad basándose en qué tan
// seguro está de ver un objeto".

float fuzzyGrade(float val, float low, float peak, float high) {
  if (val <= low || val >= high) return 0.0;
  if (val == peak) return 1.0;
  if (val < peak) return (val - low) / (peak - low);
  return (high - val) / (high - peak);
}

void runEducationalFuzzy() {
  long dist = getDist(Trigger2, Echo2); 
  if (dist > 60) dist = 60; 

  // FUSIFICACIÓN (Variables Lingüísticas)
  float isClose  = fuzzyGrade(dist, -1, 0, 25);   // "Muy cerca"
  float isMedium = fuzzyGrade(dist, 15, 30, 45);  // "Distancia Media"
  float isFar    = fuzzyGrade(dist, 35, 60, 80);  // "Lejos"

  // REGLAS (Base de Conocimiento)
  // 1. Si está CERCA, detenerse suavemente (Precaución)
  // 2. Si está MEDIO, velocidad de aprendizaje (Observación)
  // 3. Si está LEJOS, explorar (Curiosidad)
  
  float ruleCaution = isClose;
  float ruleObserve = isMedium;
  float ruleExplore = isFar;

  // DESFUSIFICACIÓN (Cálculo de velocidad PWM)
  // Cerca -> 0 (Stop), Medio -> 120 (Lento), Lejos -> 180 (Normal)
  float num = (ruleCaution * 0) + (ruleObserve * 120) + (ruleExplore * 180);
  float den = ruleCaution + ruleObserve + ruleExplore;
  
  int speed = (den > 0) ? (int)(num / den) : 100;

  // ACTUACIÓN
  if (dist < 10) {
    stopMotors(); // Parada de seguridad absoluta
  } else {
    move(speed, speed);
  }
  delay(50);
}

void runFollowMode() {
  // Lógica Proporcional simple para seguir la mano
  long d = getDist(Trigger2, Echo2);
  if (d > 5 && d < 20) {
    move(150, 150); // Acercarse
  } else if (d < 5) {
    move(-150, -150); // Alejarse (Espacio personal)
  } else {
    stopMotors(); // Esperar instrucción (mano)
  }
}

// =============================================================================
// DRIVERS (DRV8833) & SENSORES
// =============================================================================
void setMotorA(int v) {
  if(v>0){ ledcWrite(MOT_A1, v); ledcWrite(MOT_A2, 0); }
  else   { ledcWrite(MOT_A1, 0); ledcWrite(MOT_A2, abs(v)); }
}
void setMotorB(int v) {
  if(v>0){ ledcWrite(MOT_B1, v); ledcWrite(MOT_B2, 0); }
  else   { ledcWrite(MOT_B1, 0); ledcWrite(MOT_B2, abs(v)); }
}
void move(int l, int r) { setMotorA(l); setMotorB(r); }
void stopMotors() { move(0,0); }

long getDist(int t, int e) {
  digitalWrite(t, LOW); delayMicroseconds(2);
  digitalWrite(t, HIGH); delayMicroseconds(10);
  digitalWrite(t, LOW);
  long d = pulseIn(e, HIGH, 4000);
  return (d==0)? 99 : d/58.2;
}
