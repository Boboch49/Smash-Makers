# 🎮 Smash-Makers

**Un jeu d'arcade électromécanique imaginé par un grand enfant… qui a grandi sans perdre l'envie de jouer.** ❤️

**Projet V15 — 18 novembre 2026** · *Prototype en développement*

Smash-Makers mêle la passion du jeu d'arcade, la mécanique motorisée, l'impression 3D, l'électronique et les animations LED. Deux raquettes mobiles, une balle guidée, des moteurs pas à pas et deux matrices RGB transforment une idée de gamin en machine bien réelle.

> 🔒 **Ce dépôt est une vitrine du projet : aucun code source, aucun firmware, aucun fichier de flashage.**
>
> 🧪 **État du projet :** les validations logicielles documentées ne remplacent pas les essais physiques de la V15 complète.

## ❤️ L'histoire du grand enfant derrière la machine

Il paraît qu'en grandissant, on range ses jouets et on devient sérieux…

Eh bien, moi, j'ai dû rater cette étape ! 😁

Je suis adulte, avec les responsabilités qui vont avec. Mais au fond, je suis toujours ce gamin qui s'émerveillait devant les bornes d'arcade, qui voulait comprendre comment les choses fonctionnaient et qui rêvait de construire ses propres machines.

La différence, c'est qu'aujourd'hui, mes jouets ont changé : à la place des petites voitures, je m'amuse avec des moteurs, des courroies, des imprimantes 3D, des cartes électroniques et des matrices LED.

**Je ne me contente plus de jouer aux jeux des autres… je crée les miens !**

Un projet, ce sont des heures à imaginer, dessiner, assembler, tester, parfois recommencer. Et puis une animation s'allume, un mécanisme fonctionne enfin, et le sourire du gamin revient.

**Grandir, oui. Arrêter de rêver et de jouer ? Jamais !** ❤️

➡️ [Lire mon histoire complète](documentation/LE_GRAND_ENFANT_DERRIERE_SMASH_MAKERS.md)

## 🕹️ Trois univers de jeu

### 1. Smash Makers — Pong à sets

![Smash Makers V15](media/animations/01_smash_makers_v15.gif)

Le classique revisité en version mécanique :
- Parties à **11 points**, avec **2 points d'écart**.
- Victoire en **2 sets gagnants**.
- Jeu à un ou deux joueurs.

### 2. Smash Hockey — Match chronométré

![Smash Hockey V15](media/animations/02_smash_hockey_v15.gif)

Une ambiance hockey avec **3 minutes de temps de jeu actif**, engagements, tirs, parades, buts, scores et animations des personnages.

### 3. Strange Makers — Ambiance fantastique

Une introduction audiovisuelle d'environ **53,48 secondes**, une horloge et un événement qui provoque une **inversion des commandes pendant 8 secondes**.

![Strange Makers, horloge V15](media/animations/03_strange_makers_horloge_v15.gif)

[Aperçu vidéo de l'horloge sans musique](media/videos/Strange_Makers_Horloge_V15_sans_audio.mp4). Cet extrait ne montre pas l'introduction complète.

Les trois univers disposent d'animations matricielles distinctes dans la V15. Les trois GIF ci-dessus sont disponibles, mais **l'introduction longue de Strange Makers reste volontairement non publiée** (droits sur les personnages et la musique à vérifier).

> 🎞️ Les GIF affichés sont des aperçus des ressources V15, pas des captures filmées d'une matrice physique.

## ⚙️ Mécanique RD17

![Animation RD17](media/animations/04_mecanique_rd17.gif)

![Cadre RD17](media/images/rd17_cadre.png)

[Regarder la rotation mécanique RD17 — vidéo Web (muette)](media/videos/Smash_Makers_RD17_Cadre_360_web.mp4)

### Galerie mécanique RD17

*Vues issues du dossier V15. Certaines pièces corrigées le 16 novembre n'ont pas encore été réintégrées dans toutes les illustrations ; les fichiers déclarés validés restent la référence.*

![Ponts vissés RD17](media/images/rd17_ponts_vis_v15.jpg)

![Raquette éclatée](media/images/rd17_raquette_eclatee.jpg)

![Balle éclatée](media/images/rd17_balle_eclatee.jpg)

![Ensemble du cadre 860 × 780](media/images/rd17_cadre_860_ensemble.jpg)

### Fichiers de documentation RD17

- [Notice et historique RD17](mecanique/RD17/LIRE_RD17.txt)
- [Nomenclature des pièces](mecanique/RD17/Nomenclature_RD17.csv)
- [Visserie](mecanique/RD17/Visserie_reperee_RD17.csv)
- [Liste de découpe aluminium](mecanique/RD17/Liste_coupe_cadre_860x780.csv)
- [Pièces validées au 16 novembre 2026](mecanique/RD17/PIECES_VALIDEES_2026-11-16.md)
- [Modifications et attributions](mecanique/RD17/ATTRIBUTION.md)
- [Notice de licence des éléments mécaniques dérivés](mecanique/RD17/LICENCE_ORIGINALE.txt)

La structure comprend notamment un cadre en profilés aluminium, des courroies GT2, des guidages, des chariots, des supports de moteurs et des pièces imprimables.

Le modèle mécanique **RD17** est la référence actuelle du projet. Les documents techniques, attributions et notices de licence sont conservés dans le dossier mécanique.

## 🌈 Matrices LED et électronique

- **Deux matrices LED HUB75 P2.5 de 64 × 32**, disposées horizontalement : affichage **128 × 32 pixels**.
- **BTT SKR Mini E3 V3** pour la commande mécanique.
- **Waveshare ESP32-S3 RGB Matrix** pour les animations et l'audio.
- Communication entre les deux cartes par liaison série **UART**.
- **Carte fille EXP1 R5 CMS**, avec documentation du choix d'alimentation JP1 et de la correction D2.

![Carte fille EXP1 R5](media/images/carte_fille_exp1_r5.png)

## 📚 Documentation et partage

- [Histoire du projet](documentation/LE_GRAND_ENFANT_DERRIERE_SMASH_MAKERS.md)
- [État du projet et matériel](documentation/PRESENTATION_V15.md)
- [Règles de partage et fichiers exclus](documentation/CONTENU_ET_EXCLUSIONS.md)
- [Droits et précautions](documentation/DROITS_ET_PRECAUTIONS.md)

Les **trois GIF de démonstration**, des vidéos mécaniques et de l'horloge, les aperçus électroniques, les fichiers de fabrication R5 et les documents RD17 sont en ligne. Les gros plans STEP/STL complets et plusieurs longues vidéos restent à transférer après revue des droits, des exigences GPL et des limites de dépôt.

## 📐 Fichiers de la carte fille R5

Les premiers fichiers de fabrication sont maintenant en ligne :
- [Dossier de la carte EXP1 R5](electronique/carte-fille-exp1-r5/)
- [Archive KiCad/Gerber CMS](electronique/carte-fille-exp1-r5/CARTE_EXP1_R5_KICAD_GERBER_CMS.zip)

Les plans mécaniques STEP/STL volumineux et les autres médias ne sont **pas encore entièrement téléversés**. Aucun firmware ni code source n'est inclus.

## 🔒 Code et firmware privés

Ne sont **pas** distribués : programmes Arduino/C/C++/Python, scripts, projets de compilation, binaires `.bin`, `.hex`, `.elf`, `.uf2`, images de flash, clés ou fichiers sensibles.

Les ressources graphiques et sonores ne sont pas des firmwares, mais leur publication doit également respecter les droits d'auteur.

## ⚖️ Crédits et droits

Projet personnel **Smash-Makers**. Tous droits réservés pour les créations originales non placées sous une licence explicite. Les composants, plans dérivés, musiques, personnages ou documents tiers conservent leurs licences et droits respectifs. Ce dépôt ne donne pas d'autorisation générale de redistribution d'œuvres tierces.

---

*« Imaginé avec un cœur d'enfant, construit avec des mains d'adulte, développé avec une passion sans limites. »* ❤️
