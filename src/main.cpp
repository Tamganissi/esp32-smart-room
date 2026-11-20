#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =========================
// BROCHES
// =========================

#define DHT_PIN       4
#define DHT_TYPE      DHT11

#define LDR_PIN       34
#define PIR_PIN       27
#define OBSTACLE_PIN  35

#define BUTTON_PIN    25
#define BUZZER_PIN    26
#define BUZZER_FREQUENCY 2000

#define LED_RED       14
#define LED_YELLOW    12
#define LED_GREEN     13

#define POT_PIN       33

#define RELAY_PIN     32

// OLED
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_SDA      21
#define OLED_SCL      22

// =========================
// OBJETS
// =========================

DHT dht(DHT_PIN, DHT_TYPE);

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1
);

// =========================
// VARIABLES
// =========================

bool alarmEnabled = true;
bool alarmActive = false;
bool previousIntrusion = false;
unsigned long alarmStartTime = 0;
const unsigned long alarmDuration = 2000;

bool lastButtonState = HIGH;

unsigned long lastButtonTime = 0;

const unsigned long debounceDelay = 250;

float temperature = NAN;
float humidity = NAN;
unsigned long lastDhtReadTime = 0;
const unsigned long dhtReadInterval = 2000;

const int wakeHour = 7;
const int wakeMinute = 0;
unsigned long clockStartMillis = 0;
int clockStartHour = 0;
int clockStartMinute = 0;
int lastWakeDay = -1;
unsigned long lastDisplayPageTime = 0;
bool displayPage = false;
const unsigned long displayPageInterval = 30000;

void readCompileTime()
{
    clockStartHour = (__TIME__[0] - '0') * 10 + (__TIME__[1] - '0');
    clockStartMinute = (__TIME__[3] - '0') * 10 + (__TIME__[4] - '0');
}

void getClockTime(int &hour, int &minute, int &second, int &day)
{
    unsigned long elapsedSeconds = (millis() - clockStartMillis) / 1000;
    unsigned long startSeconds = clockStartHour * 3600UL + clockStartMinute * 60UL;
    unsigned long totalSeconds = startSeconds + elapsedSeconds;

    day = totalSeconds / 86400UL;
    totalSeconds %= 86400UL;
    hour = totalSeconds / 3600UL;
    minute = (totalSeconds % 3600UL) / 60UL;
    second = totalSeconds % 60UL;
}

// =========================
// SETUP
// =========================

void setup()
{
    Serial.begin(115200);
    readCompileTime();
    clockStartMillis = millis();

    // Entrées
    pinMode(LDR_PIN, INPUT);
    pinMode(PIR_PIN, INPUT);
    pinMode(OBSTACLE_PIN, INPUT);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Sorties
    pinMode(BUZZER_PIN, OUTPUT);

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);

    pinMode(RELAY_PIN, OUTPUT);

    // Tout éteindre au démarrage
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(RELAY_PIN, LOW);

    // DHT
    dht.begin();

    // OLED
    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("Erreur OLED !");
        while (true)
        {
            delay(1000);
        }
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("SMART ROOM");
    display.println();
    display.println("Demarrage...");

    display.display();

    delay(2000);
}

// =========================
// LOOP
// =========================

void loop()
{
    int currentHour;
    int currentMinute;
    int currentSecond;
    int currentDay;
    getClockTime(currentHour, currentMinute, currentSecond, currentDay);

    if (currentHour == wakeHour && currentMinute == wakeMinute && currentDay != lastWakeDay)
    {
        alarmActive = true;
        alarmStartTime = millis();
        lastWakeDay = currentDay;
    }

    if (millis() - lastDisplayPageTime >= displayPageInterval)
    {
        displayPage = !displayPage;
        lastDisplayPageTime = millis();
    }

    // -------------------------
    // LECTURE DU BOUTON
    // -------------------------

    bool buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW && lastButtonState == HIGH)
    {
        if (millis() - lastButtonTime > debounceDelay)
        {
            alarmEnabled = !alarmEnabled;

            lastButtonTime = millis();
        }
    }

    lastButtonState = buttonState;


    // -------------------------
    // LECTURE DES CAPTEURS
    // -------------------------

    if (lastDhtReadTime == 0 || millis() - lastDhtReadTime >= dhtReadInterval)
    {
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        lastDhtReadTime = millis();
    }

    int luminosity = digitalRead(LDR_PIN);

    int motion = digitalRead(PIR_PIN);

    int obstacle = digitalRead(OBSTACLE_PIN);

    int potentiometer = analogRead(POT_PIN);


    // -------------------------
    // VERIFICATION DHT
    // -------------------------

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Erreur DHT11 !");
    }


    // -------------------------
    // DETECTION
    // -------------------------

    bool intrusion = false;

    if (motion == HIGH)
    {
        intrusion = true;
    }

    if (obstacle == LOW)
    {
        intrusion = true;
    }


    // -------------------------
    // SYSTEME D'ALARME
    // -------------------------
// =========================
// GESTION DE L'ALARME
// =========================

// Détection d'une intrusion
if (alarmEnabled && !alarmActive && intrusion && !previousIntrusion)
{
    alarmActive = true;
    alarmStartTime = millis();
}

previousIntrusion = intrusion;


// =========================
// ALARME ACTIVE
// =========================

if (alarmActive)
{
    // Tant que 10 secondes ne sont pas passées
    if (millis() - alarmStartTime < alarmDuration)
    {
        // LED ROUGE
        digitalWrite(LED_RED, HIGH);

        // LED JAUNE OFF
        digitalWrite(LED_YELLOW, LOW);

        // LED VERTE OFF
        digitalWrite(LED_GREEN, LOW);

        // Buzzer passif : produire un son audible
        tone(BUZZER_PIN, BUZZER_FREQUENCY);

        // RELAIS
        digitalWrite(RELAY_PIN, HIGH);
    }
    else
    {b
        // Les 10 secondes sont terminées
        alarmActive = false;

        digitalWrite(LED_RED, LOW);
        noTone(BUZZER_PIN);
        digitalWrite(RELAY_PIN, LOW);

        // Retour au vert
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_YELLOW, LOW);
    }
}


// =========================
// SI ALARME DESACTIVEE
// =========================

if (!alarmEnabled)
{
    alarmActive = false;

    digitalWrite(LED_RED, LOW);
    noTone(BUZZER_PIN);
    digitalWrite(RELAY_PIN, LOW);

    // JAUNE = système désactivé
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, LOW);
}
else if (!alarmActive)
{
    digitalWrite(LED_RED, LOW);
    noTone(BUZZER_PIN);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
}
    // -------------------------
    // OLED
    // -------------------------

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0, 0);
    display.print("SMART ROOM ");
    if (currentHour < 10) display.print('0');
    display.print(currentHour);
    display.print(':');
    if (currentMinute < 10) display.print('0');
    display.print(currentMinute);
    display.print(':');
    if (currentSecond < 10) display.print('0');
    display.println(currentSecond);

    display.setCursor(0, 12);
    if (!displayPage)
    {
        display.print("Temp: ");
        if (isnan(temperature)) display.println("ERR");
        else { display.print(temperature, 1); display.println(" C"); }
        display.print("Hum : ");
        if (isnan(humidity)) display.println("ERR");
        else { display.print(humidity, 0); display.println(" %"); }
        display.print("Reveil: 07:00");
    }
    else
    {
        display.print("LDR : ");
        display.println(luminosity == LOW ? "LUM" : "SOMBRE");
        display.print("PIR:");
        display.print(motion == HIGH ? "MOTION" : "NONE");
        display.print(" Obs:");
        display.println(obstacle == LOW ? "YES" : "NO");
        display.print("Pot:");
        display.print(potentiometer);
        display.print(" Alarm:");
        display.println(alarmActive ? "ON" : "OFF");
    }
    display.display();


    // -------------------------
    // MONITEUR SERIE
    // -------------------------

    Serial.println("======================");

    Serial.print("Temperature : ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidite    : ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Luminosite  : ");
    Serial.println(luminosity);

    Serial.print("PIR         : ");
    Serial.println(motion);

    Serial.print("Obstacle    : ");
    Serial.println(obstacle);

    Serial.print("Potentiometre : ");
    Serial.println(potentiometer);

    Serial.print("Alarme      : ");

    if (alarmActive)
        Serial.println("ON");
    else
        Serial.println("OFF");

    delay(500);
}