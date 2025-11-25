/*
* ===========================================
*  WORTUHR (DS1307) – LED-Word-Clock (Arduino)
* ===========================================
*
* Letzte Version: 27.08.2025
*
* ÜBERSICHT
* ----------
* Diese Sketch steuert eine Wortuhr mit RTC DS1307. Die Stundenwörter werden
* abhängig von den Minuten angezeigt (bis <25 aktuelle Stunde, ab ≥25 nächste
* Stunde). Minutenphrasen (FÜNF/ZEHN/VIERTEL/ZWANZIG/VOR/NACH/HALB) und die
* Minutenreste (1–4) werden separat dargestellt. Ein Sekundenbalken (2..13)
* zeigt die verstrichenen Sekunden in 5-Sekunden-Schritten.
*
* FUNKTIONSUMFANG
* ---------------
* - RTC-Einbindung (DS1307) via I2C, Plausibilitätscheck und Autoreset auf
*   Compile-Zeit bei ungültiger Uhrzeit.
* - 12-Stunden-Anzeige (std = now.Hour() % 12).
* - Anzeigeregeln:
*     • min < 25  → aktuelle Stunde
*     • min ≥ 25  → nächste Stunde (Wrap 11→0)
* - Minutenwörter (FÜNF/ZEHN/ZWANZIG/DREI/VIERTEL/VOR/NACH/HALB).
* - Minutenreste 1–4 (separate Pins).
* - Sekundenbalken (Pins 2..13) mit progressiver Anzeige.
* - Winter-/Sommerzeit per Kippschalter an P51 (LOW = Winterzeit → std-1).
*
* HARDWARE
* --------
* - Controller: Arduino mit genügend GPIOs (z. B. Mega).
* - RTC: DS1307 (Wire/I2C).
* - Beschaltung P51: Externes Pull-Down gemäß Hardware-Hinweis.
*
* PINOUT (LEDs)
* -------------
* Minutenwörter: 22..29
*   22=FÜNF, 23=ZEHN, 24=ZWANZIG, 25=DREI, 26=VIERTEL, 27=VOR, 28=NACH, 29=HALB
*
* Stundenwörter (hourpins-Map):
*   12: 40        (ZWÖLF)
*    1: 33        (EINS)
*    2: 34        (ZWEI)
*    3: 35        (DREI)
*    4: 36        (VIER)
*    5: 31+32     (F + UENF) = FÜNF
*    6: 37        (SECHS)
*    7: 39        (SIEBEN)
*    8: 38        (ACHT)
*    9: 42+43     (N + EUN)  = NEUN
*   10: 41+42     (ZEH + N)  = ZEHN
*   11: 30+31     (EL + F)   = ELF
*
* Minutenreste: 44..47
*   44=EINS, 45=ZWEI, 46=DREI, 47=VIER
*
* Sekundenbalken: 2..13
*
* KONFIGURATION
* -------------
* - delayy (ms): Timing/Animation.
* - buttonpin (P51): DST-Schalter. HIGH = Sommerzeit, LOW = Winterzeit (std-1).
*   → Externes Pull-Down einsetzen (gemäß Kommentar/Hardware).
*
* SELBSTTEST
* ----------
* Beim Start: Lauflicht 22..50, anschließend 2..13.
*
* CHANGELOG
* ---------
* 27.08.2025
*   • Bugfix: „ELF“ bei :00–:04 wird korrekt angezeigt (Minuten-Reset berührt 30/31 nicht).
*   • Bugfix: „ZEHN“ vollständig (41+42) in hourpins.
*   • Bugfix: Minutenreste korrekt auf 44..47 (statt 47..50).
*   • Robustheit: LED-Reset nur auf Ausgänge (22..50, 2..13).
*   • DST-Kippschalter standardmäßig aktiv (Pin 51).
*
* 31.03.2024
*   • Vorversion.
*
* TODO
* ----
* - Poti zur ±30-Minuten-Feinkorrektur integrieren (ADC, Glättung).
* - Optional: Entprellung/Glättung des DST-Schalters.
* - Optional: PWM-Helligkeit/Nachtmodus.
* - Optional: DS3231 (höhere Genauigkeit) statt DS1307.
*
* KNOWN ISSUES
* ------------
* - Keine bekannten Anzeigefehler bzgl. „ELF“; bitte im Feldbetrieb verifizieren.
* - DS1307 kann merkliche Gangabweichung haben; regelmäßige Synchronisation empfohlen.
*/


#include <Wire.h>  // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>
RtcDS1307<TwoWire> rtc(Wire);

#define delayy                150
#define buttonpin             51
#define potipin               A8

const int word_fuenf          = 22;
const int word_zehn           = 23;
const int word_zwanzig        = 24;
const int word_drei           = 25;
const int word_viertel        = 26;
const int word_vor            = 27;
const int word_nach           = 28;
const int word_halb           = 29;

const int word_el             = 30;
const int word_f              = 31;
const int word_uenf           = 32;
const int word_eins           = 33;
const int word_zwei           = 34;
const int word_drei1          = 35;
const int word_vier1          = 36;
const int word_sechs          = 37;
const int word_acht           = 38;
const int word_sieben         = 39;
const int word_zwoelf         = 40;
const int word_zeh            = 41;
const int word_n              = 42;
const int word_eun            = 43;

const int word_minute_eins    = 44;
const int word_minute_zwei    = 45;
const int word_minute_drei    = 46;
const int word_minute_vier    = 47;

// HOUR-PINS: -1 als Platzhalter für "kein Pin"
// Fix: "ZEHN" vollständig => Index 10: {41, 42}
// Fix: "ELF" nutzt {30,31} und wird nicht mehr durch Minuten-Reset gelöscht

const int hourpins[12][2] = {
  { 40, -1 },   // 12 (ZWÖLF)
  { 33, -1 },   // 1  (EINS)
  { 34, -1 },   // 2  (ZWEI)
  { 35, -1 },   // 3  (DREI)
  { 36, -1 },   // 4  (VIER)
  { 31, 32 },   // 5  (F + UENF) = FÜNF
  { 37, -1 },   // 6  (SECHS)
  { 39, -1 },   // 7  (SIEBEN)
  { 38, -1 },   // 8  (ACHT)
  { 42, 43 },   // 9  (N + EUN) = NEUN
  { 41, 42 },   // 10 (ZEH + N) = ZEHN  <-- FIX
  { 30, 31 }    // 11 (EL + F)  = ELF
};

int winterzeit = 0;

void setup() {
  Serial.begin(9600);
  pinMode(buttonpin, INPUT); // Externer Pull-Down laut Kommentar/Hardware

  // Ausgänge setzen
  for (int i = 22; i <= 50; i++) {
    pinMode(i, OUTPUT);
  }
  for (int i = 2; i <= 13; i++) {
    pinMode(i, OUTPUT);
  }

  // TEST-Lauflicht
  for (int i = 22; i <= 50; i++) {
    digitalWrite(i, HIGH);
    delay(delayy * 2);
  }
  for (int i = 2; i <= 13; i++) {
    digitalWrite(i, HIGH);
    delay(delayy);
  }
  delay(delayy * 3);
  for (int i = 22; i <= 50; i++) {
    digitalWrite(i, LOW);
  }
  for (int i = 2; i <= 13; i++) {
    digitalWrite(i, LOW);
  }
  delay(delayy);

  rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  //rtc.SetDateTime(compiled);

  if (!rtc.IsDateTimeValid()) {
    if (rtc.LastError() != 0) {
      Serial.println(rtc.LastError());
    } else {
      rtc.SetDateTime(compiled);
    }
  }

  if (!rtc.GetIsRunning()) {
    rtc.SetIsRunning(true);
  }

  RtcDateTime now = rtc.GetDateTime();
  if (now < compiled) {
    rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");
  } else if (now == compiled) {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);
}

void loop() {
  if (!rtc.IsDateTimeValid()) {
    // Blink-Muster bei Fehler
    for (int j = 0; j <= 3; j++) {
      for (int i = 22; i <= 50; i++) digitalWrite(i, HIGH);
      delay(500);
      for (int i = 22; i <= 50; i++) digitalWrite(i, LOW);
      delay(500);
    }
    if (rtc.LastError() != 0) {
      Serial.print("RTC communications error = ");
      Serial.println(rtc.LastError());
    } else {
      Serial.println("RTC lost confidence in the DateTime!");
    }
  }

  RtcDateTime now = rtc.GetDateTime();

  // Reset aller LEDs (nur Ausgänge, keine RX/TX)
  for (int i = 22; i <= 50; i++) digitalWrite(i, LOW);
  for (int i = 2;  i <= 13; i++) digitalWrite(i, LOW);

  // Zeitwerte
  int std = (int)now.Hour();
  int min = (int)now.Minute();
  int sek = (int)now.Second();

  // 24h -> 12h
  if (std >= 12) {
    std -= 12;
  }

  // 1-0 Schalter mit statischem Kippschalter (externes Pull-Down laut Kommentar)
  // HIGH = Sommerzeit (winterzeit=0), LOW = Winterzeit (winterzeit=1)
  if (digitalRead(buttonpin) == HIGH) {
    winterzeit = 0;
  } else {
    winterzeit = 1;
  }

  // Winterzeit: Stunde -1 (mit Wrap von 0->11)
  if (winterzeit) {
    if (std == 0) {
      std = 11;
    } else {
      std -= 1;
    }
  }

  // --- Poti-Feinabgleich: -15 .. +15 Minuten ---
  int deltaMin = map(analogRead(potipin), 0, 1023, -15, 15);
  
  // (Optional) kleine Totzone gegen Flackern
  if (abs(deltaMin) <= 1) deltaMin = 0;
  
  // Offset anwenden
  min += deltaMin;
  
  // Minuten/H-Stunden normalisieren (einmal reicht, da |delta| ≤ 15)
  if (min < 0) {
    min += 60;
    std = (std == 0) ? 11 : (std - 1);
  } else if (min >= 60) {
    min -= 60;
    std = (std == 11) ? 0 : (std + 1);
  }


  // Stundenanzeige: bis <25 aktuelle, sonst nächste Stunde
  if (min < 25) {
    for (int i = 0; i < 2; i++) {
      if (hourpins[std][i] != -1) {
        digitalWrite(hourpins[std][i], HIGH);
      }
    }
  } else {
    int nextStd = (std + 1) % 12; // 11->0 sicher
    for (int i = 0; i < 2; i++) {
      if (hourpins[nextStd][i] != -1) {
        digitalWrite(hourpins[nextStd][i], HIGH);
      }
    }
  }

  // Minutenwörter
  if (min < 5) {
    // Nur die MINUTEN-Wörter ausschalten – NICHT 30/31 (EL/F)!
    digitalWrite(word_fuenf,    LOW);
    digitalWrite(word_zehn,     LOW);
    digitalWrite(word_zwanzig,  LOW);
    digitalWrite(word_drei,     LOW);
    digitalWrite(word_viertel,  LOW);
    digitalWrite(word_vor,      LOW);
    digitalWrite(word_nach,     LOW);
    digitalWrite(word_halb,     LOW);
  } else if (min < 10) {
    digitalWrite(word_fuenf, HIGH);
    digitalWrite(word_nach, HIGH);
  } else if (min < 15) {
    digitalWrite(word_zehn, HIGH);
    digitalWrite(word_nach, HIGH);
  } else if (min < 20) {
    digitalWrite(word_viertel, HIGH);
    digitalWrite(word_nach, HIGH);
  } else if (min < 25) {
    digitalWrite(word_zwanzig, HIGH);
    digitalWrite(word_nach, HIGH);
  } else if (min < 30) {
    digitalWrite(word_fuenf, HIGH);
    digitalWrite(word_vor, HIGH);
    digitalWrite(word_halb, HIGH);
  } else if (min < 35) {
    digitalWrite(word_halb, HIGH);
  } else if (min < 40) {
    digitalWrite(word_fuenf, HIGH);
    digitalWrite(word_nach, HIGH);
    digitalWrite(word_halb, HIGH);
  } else if (min < 45) {
    digitalWrite(word_zwanzig, HIGH);
    digitalWrite(word_vor, HIGH);
  } else if (min < 50) {
    digitalWrite(word_drei, HIGH);
    digitalWrite(word_viertel, HIGH);
  } else if (min < 55) {
    digitalWrite(word_zehn, HIGH);
    digitalWrite(word_vor, HIGH);
  } else {
    digitalWrite(word_fuenf, HIGH);
    digitalWrite(word_vor, HIGH);
  }

  // Minutenreste (1–4) über 44..47
  for (int i = 44; i <= 47; i++) digitalWrite(i, LOW);  // <-- FIX: 44..47 statt 47..50
  switch (min % 5) {
    case 1: digitalWrite(word_minute_eins, HIGH);  break;
    case 2: digitalWrite(word_minute_zwei, HIGH);  break;
    case 3: digitalWrite(word_minute_drei, HIGH);  break;
    case 4: digitalWrite(word_minute_vier, HIGH);  break;
    default:
      for (int i = 44; i <= 47; i++) digitalWrite(i, LOW); // <-- FIX: 44..47
  }

  // Sekundenbalken (Pins 2..13)
  int sekundo = now.Second() / 5;
  sekundo += 2;
  for (int i = 2; i <= 13; i++) {
    if (i < sekundo) {
      digitalWrite(i, HIGH);
    }
    if (now.Second() > 58) {
      digitalWrite(13, HIGH);
    }
  }

  delay(900);  // 900 Millisekunden
}

