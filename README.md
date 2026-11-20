# ESP32 Smart Room — surveillance de pièce et alarme

> Système embarqué sur **ESP32** qui surveille une pièce (température, humidité, luminosité, présence, obstacle), affiche les mesures sur un **écran OLED** et déclenche une **alarme** (buzzer, LED, relais) en cas d'intrusion. Il intègre aussi un réveil programmé.

**Compétences mobilisées :** C++ embarqué (Arduino / **PlatformIO**) · bus I²C (OLED SSD1306) · capteurs numériques et analogiques (DHT11, LDR, PIR, capteur d'obstacle, potentiomètre) · machine à états non bloquante (`millis()`) · anti-rebond du bouton · pilotage d'actionneurs (buzzer, LED, relais).

## Fonctionnement
- **Mesures** : température et humidité (DHT11, lues toutes les 2 s), luminosité, présence (PIR), obstacle, potentiomètre.
- **Détection d'intrusion** : mouvement **ou** obstacle → alarme pendant 2 s (buzzer à 2 kHz + LED rouge).
- **Bouton** : active ou désactive l'alarme, avec anti-rebond de 250 ms.
- **LED d'état** : vert = normal, jaune / rouge = alerte.
- **Réveil** : alarme quotidienne à 7 h 00 (horloge initialisée à l'heure de compilation).
- **Écran OLED** : deux pages qui alternent toutes les 30 s.
- Aucune attente bloquante : tout est géré avec `millis()`.

## Câblage (ESP32 DevKit V1)
| Élément | GPIO |
|---|---|
| DHT11 | 4 |
| LDR (photorésistance) | 34 |
| PIR (présence) | 27 |
| Capteur d'obstacle | 35 |
| Bouton | 25 |
| Buzzer passif | 26 |
| LED rouge / jaune / verte | 14 / 12 / 13 |
| Potentiomètre | 33 |
| Relais | 32 |
| OLED SDA / SCL | 21 / 22 |

## Compiler et téléverser
```bash
pio run -t upload
pio device monitor     # 115200 bauds
```
Les bibliothèques (Adafruit GFX, SSD1306, DHT, Unified Sensor) sont installées automatiquement depuis `platformio.ini`.

---
**Fidèle Nissi Djoyum Tamga** — Master EEA, Université de Rennes.
