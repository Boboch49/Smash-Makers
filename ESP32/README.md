# ESP32-S3 — Smash-Makers V15

**Créateur : Cédric Boboch · Projet : Smash-Makers V15**

Ce dossier est destiné **uniquement à la carte Waveshare ESP32-S3 RGB Matrix** (affichage et audio). La source du contrôleur SKR Mini E3 V3 et son firmware ne sont **pas** publiés ici.

## Organisation

- `source/` : projet PlatformIO ESP32-S3 avec `platformio.ini` et `src/`, incluant les ressources intégrées aux sources.
- `firmware/` : fichiers de flashage **ESP32-S3 uniquement** provenant de l'archive V15, pas du contrôleur SKR.

## Matériel

- Waveshare ESP32-S3 RGB Matrix, référence 34422, N32R16.
- Deux panneaux HUB75 P2.5 de 64 × 32 pixels, chaînés horizontalement pour une résolution 128 × 32.
- Audio via le codec ES8311, ressources multimédias supplémentaires sur microSD.
- Communication UART avec le contrôleur mécanique (protocole de synchronisation V15, version 9).

## Compilation

Ouvrir `source/` dans Visual Studio Code + PlatformIO, puis choisir l'environnement `waveshare_game`. Le fichier `platformio.ini` définit la version de la plateforme ESP32, le Framework Arduino, la mémoire PSRAM et les bibliothèques HUB75/Adafruit.

`platformio run -d source -e waveshare_game`

Pour les variantes de diagnostic, voir les environnements secondaires dans `source/platformio.ini`.

## Installation (ESP32-S3 seulement)

**Attention :** un firmware incorrect ou une configuration flash incompatible peut rendre la carte temporairement inutilisable. Vérifier la référence du matériel, le port USB et les options du système avant toute opération.

La version `flash_complete.bin` est une image flash complète ESP32-S3. Avec `esptool` compatible, placer l'image à l'adresse **0x0** sur l'ESP32-S3, selon les spécifications matérielles. Ne jamais utiliser ces binaires sur la SKR.

Les images séparées (`bootloader.bin`, `partitions.bin`, `boot_app0.bin`, `firmware.bin`) nécessitent des adresses de flash cohérentes avec la table de partitions et les réglages PlatformIO : **ne pas deviner leurs adresses**.

Avant de flasher : débrancher la liaison UART vers la carte mécanique et sauvegarder les contenus nécessaires. La partie jeu intégrée utilise la **version 9 du protocole** : elle doit correspondre au logiciel déjà présent côté contrôleur mécanique, qui n'est pas fourni dans ce dépôt.

Le fonctionnement complet des modes Hockey et Strange requiert aussi des fichiers sur microSD, **non tous diffusés ici**, notamment parce que les droits de certains médias restent à confirmer. Sans la microSD correspondante, certaines séquences ne fonctionneront pas.

## Limites

Les fichiers sont repris de l'archive V15. Aucun reflash matériel ni nouvelle compilation n'a été exécuté pour cette publication. Les références SKR visibles dans le **code ESP32** sont uniquement les noms de connexion UART et de protocole ; **aucun code source ni binaire SKR** n'est compris dans le présent dossier.

© 2026 Cédric Boboch — parties originales Smash-Makers. Les bibliothèques et médias tiers restent soumis à leurs licences respectives.
