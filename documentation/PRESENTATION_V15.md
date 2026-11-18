# Smash-Makers — Présentation V15

**Version : 18 novembre 2026 — prototype en développement.**

Smash-Makers est un projet de table de jeu électromécanique avec trois univers :
- **Smash Makers** : Pong, 11 points avec deux points d'écart, deux sets gagnants, un ou deux joueurs.
- **Smash Hockey** : match de trois minutes de temps de jeu actif, buts, parades, affichage du score et animations de personnages.
- **Strange Makers** : introduction audiovisuelle d'environ 53,48 secondes et événement d'horloge inversant temporairement les commandes pendant huit secondes.

## Principe mécanique

Une table de jeu électromécanique animée par des moteurs avec des raquettes mobiles. Le présent dépôt ne diffuse pas les modèles de conception mécanique tiers ni leurs adaptations.

## Affichage

Deux matrices LED RGB HUB75 P2.5 de 64 × 32 pixels côte à côte forment un affichage de **128 × 32 pixels**.

## Électronique

- Carte **BTT SKR Mini E3 V3** : mouvements et logique de jeu.
- Carte **Waveshare ESP32-S3 RGB Matrix** : animations et son.
- Synchronisation série **UART**.
- Carte fille **EXP1 R5 CMS**, avec sélection d'alimentation JP1 et correction documentée autour de D2.

## État de validation

Les archives V15 contiennent des rapports de compilation et de tests logiciels positifs. Cela ne constitue pas une validation complète du fonctionnement réel de l'ensemble mécanique et électronique. Des essais physiques restent nécessaires.

## Contenu public

La documentation et les présentations doivent être partagées après contrôle des droits de diffusion. Les plans mécaniques tiers et leurs adaptations ne sont pas inclus. Aucun code source ni firmware n'est destiné à la publication.
