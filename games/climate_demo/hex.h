/*
 * hex.h — TV Kumanda NEC Komut Byte Degerleri
 *
 * Bu degerleri TV kumandasini ESP32'ye tutarak Serial Monitor'dan
 * okudunuz. Protocole: NEC. Command byte'lari asagidadir.
 *
 * Kullanim:
 *   - ac_automation : Learn modunda bu butonlara basarak profillere kaydedilir.
 *   - climate_demo  : Bu byte'lar TV_CMDS[] dizisine eslestirilerek profil gosterimi yapilir.
 *
 * Profil Atamasi (climate_demo.ino ile eslesiyor):
 *   P1 (COOL  22C) ← 0x5F
 *   P2 (SLEEP 26C) ← 0x4E
 *   P3 (ECO   28C) ← 0x53
 *   P4 (HEAT  20C) ← 0x50
 *   P5 (TURBO 18C) ← 0x12
 *   P6 (AUTO  24C) ← 0x4F
 *   DRY  22C  ← 0x4C
 *   FAN       ← 0x0E
 *   COOL 20C  ← 0x4B
 *   HEAT 22C  ← 0x48
 *   NIGHT     ← 0x0A
 *   ECO+      ← 0x06
 *   OFF       ← 0x44
 *   DEHUM     ← 0x47
 */

#pragma once

// Ana profiller (RainMaker + LilyGO)
#define TV_CMD_P1     0x5F   // COOL  22C
#define TV_CMD_P2     0x4E   // SLEEP 26C + 8h timer
#define TV_CMD_P3     0x53   // ECO   28C
#define TV_CMD_P4     0x50   // HEAT  20C
#define TV_CMD_P5     0x12   // TURBO 18C
#define TV_CMD_P6     0x4F   // AUTO  24C

// Ek profiller (sadece LilyGO TV remote direkt)
#define TV_CMD_DRY    0x4C   // DRY   22C
#define TV_CMD_FAN    0x0E   // FAN ONLY
#define TV_CMD_COOL2  0x4B   // COOL  20C (Guclu)
#define TV_CMD_HEAT2  0x48   // HEAT  22C (Hafif)
#define TV_CMD_NIGHT  0x0A   // NIGHT 28C + 6h timer
#define TV_CMD_ECOPL  0x06   // ECO+  27C
#define TV_CMD_OFF    0x44   // KAPALI
#define TV_CMD_DEHUM  0x47   // DEHUM 22C

#define TV_CMD_TOTAL  14
