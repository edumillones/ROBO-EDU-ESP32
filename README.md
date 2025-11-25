# 🤖 ROBO-EDU: Plataforma Educativa STEM con ESP32

![Estado](https://img.shields.io/badge/Estado-Prototipo-green)
![ODS](https://img.shields.io/badge/ODS-4_Educación_Calidad-blue)
![Hardware](https://img.shields.io/badge/Hardware-ESP32_%7C_DRV8833-orange)

## 📖 Descripción
**Robo-Edu** es un robot móvil de código abierto diseñado para democratizar la enseñanza de la robótica y la lógica de programación en escuelas rurales. Utiliza un **Sistema Experto basado en Lógica Difusa** para demostrar comportamientos inteligentes (precaución, curiosidad, seguridad) de manera visual y comprensible para niños.

## 🧠 Características Técnicas
- **Cerebro:** ESP32 (WiFi + Bluetooth).
- **Actuadores:** Driver DRV8833 (Control PWM eficiente).
- **Inteligencia:** Motor de inferencia difusa (Fuzzy Logic) para navegación suave.
- **Interfaz:** Web App "Gamificada" servida directamente desde el microcontrolador.

## 📂 Estructura del Repositorio
- `/firmware`: Código fuente en C++ para Arduino IDE.
- `/hardware`:
    - `/source`: Archivos editables en Autodesk Inventor (.ipt).
    - `/stl`: Archivos listos para impresión 3D.
- `/docs`: Diagramas de conexión y esquemáticos.

## 🚀 Instalación y Uso
1. Clonar el repositorio.
2. Abrir `/firmware` en Arduino IDE.
3. Instalar las librerías necesarias (ESP32 Board Manager).
4. Cargar el código y conectar a la red WiFi generada.
5. Acceder a la IP mostrada en el Monitor Serie.

## 🛠️ Tecnologías
- **Software:** C++, HTML5, CSS3 (Cyberpunk/Edu UI).
- **Modelado:** Autodesk Inventor.
- **Fabricación:** Impresión 3D (PLA).

---
*Desarrollado por Eduardo Millones | Ing. Mecatrónica UPN*
