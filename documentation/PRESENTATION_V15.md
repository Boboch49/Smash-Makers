# Smash-Makers — Présentation V15

**Version : 18 novembre 2026 — prototype en développement.**

**Création, conception et développement : Cédric Boboch.**

Smash-Makers est un projet de table de jeu électromécanique avec trois univers :
- **Smash Makers** : Pong, 11 points avec deux points d'écart, deux sets gagnants, un ou deux joueurs.
- **Smash Hockey** : match de trois minutes de temps de jeu actif, buts, parades, affichage du score et animations de personnages.
- **Strange Makers** : introduction audiovisuelle d'environ 53,48 secondes et événement d'horloge inversant temporairement les commandes pendant huit secondes.

## Principe mécanique

Une table de jeu électromécanique animée par des moteurs avec des raquettes mobiles. **Cédric Boboch précise avoir repris et redessiné de A à Z la mécanique de la version actuelle.** Les plans techniques correspondants ne sont pas encore remis en ligne.

## Affichage

Deux matrices LED RGB HUB75 P2.5 de 64 × 32 pixels côte à côte forment un affichage de **128 × 32 pixels**.

## Électronique

- Carte **BTT SKR Mini E3 V3** : mouvements et logique de jeu.
- Carte **Waveshare ESP32-S3 RGB Matrix** : animations et son.
- Synchronisation série **UART**.
- Carte fille **EXP1 R5 CMS**, avec sélection d'alimentation JP1 et correction documentée autour de D2.

**Logiciel partagé :** [sources et firmware ESP32-S3](../ESP32/README.md), y compris la [distribution V15 en ZIP](../ESP32/ESP32_V15_SOURCE_ET_FIRMWARE_SANS_SKR.zip). **Le programme SKR Mini E3 V3 reste privé.**

## État de validation

Les archives V15 contiennent des rapports de compilation et de tests logiciels positifs. Cela ne constitue pas une validation complète du fonctionnement réel de l'ensemble mécanique et électronique. Des essais physiques restent nécessaires.

## Contenu public

La documentation, les présentations et les plans doivent être partagés après contrôle des droits de diffusion. Les fichiers mécaniques détaillés restent à remettre en ligne. Le code source et les firmwares **ESP32-S3** sont diffusés ; les sources, configurations et firmwares du contrôleur **SKR** restent exclus.
