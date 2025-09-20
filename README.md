# Piano Trainer UI

Piano Trainer est une application interactive permettant aux utilisateurs
d'apprendre et de s'entrainer à jouer des **notes et des accords** sur un
**clavier MIDI** connecté à une **Raspberry Pi 4** et un **écran tactile**.

L'application propose plusieurs **modes de jeu** et évalue la performance du
joueur en temps réel.

## Installation

### Prérequis

Avant d'installer l'application, assurez-vous d'avoir :

- **Raspberry Pi 4** sous **Raspberry Pi OS** avec une carte Micro SD de **32 Go
  minimum**.
- **Un écran tactile** connecte à la Raspberry Pi via HDMI.
- **Un clavier MIDI** compatible (ex : `SWISSONIC EasyKeys49`).
- **Connexion Internet** pour télécharger les dépendances.
- **Accès au terminal** (SSH ou interface locale).

### Étapes d'installation

1. Mettre à jour le système

   `sudo apt update && sudo apt upgrade -y`

2. Installer les dépendances Qt

   `sudo apt install -y qtbase5-dev qttools5-dev qttools5-dev-tools qt5-qmake libqt5network5 libqt5network5-dev libqt5gui5 libqt5widgets5`

3. Installer les outils de compilation

   `sudo apt install -y build-essential cmake git`

4. Installer RtMidi

   `git clone https://github.com/thestk/rtmidi.git cd rtmidi mkdir build && cd build cmake .. make -j$(nproc) sudo make install`

5. Installer ALSA et outils audio

   `sudo apt install -y alsa-utils libasound2-dev`

6. Vérifier l'installation

   `qmake --version cmake --version g++ --version aplay --version aconnect -l`

## Lancer l'application

### Cloner le projet depuis le dépôt GitHub

    git clone <URL_DU_REPO>
    cd PianoTrainer

### Compiler l'application

    qmake && make

### Exécuter l'application

    ./IHM => l'IHM lance le MDJ automatiquement en arrière plan.

## Utilisation de l'application

### Écran d'accueil

Lors du lancement, l'utilisateur peut :

- **Sélectionner un type de jeu** :
  - **Jeu de note** : Joue des notes une par une.
  - **Jeu d'accords SR** : Joue des accords sans renversement.
  - **Jeu d'accords AR** : Joue des accords avec renversement.
- **Choisir une gamme** (Do, Ré, Mi, etc.).
- **Définir le mode** (Majeur ou Mineur).
- **Activer/désactiver le son**.
- **Lancer le jeu** en cliquant sur "Lancer le jeu".

### Écran du jeu

- Affiche la **note ou l'accord** à jouer.
- Compare la **note jouée** avec celle attendue.
- Enregistre le **temps total** pour terminer le jeu.

### Écran des résultats

- **Affiche le score final** (temps total du jeu).
- Permets de **rejouer** ou de **retourner à l'accueil**.

## Communication réseau

L'application utilise un **serveur local** pour gérer les interactions entre
l'IHM et le **Moteur de Jeu (MDJ)**.

**Le serveur tourne sur le port 8080** et échange des **messages JSON** entre
l'IHM et le MDJ :

- **Paramètres du jeu** envoyés par l'IHM.
- **Notes et accords** générés par le MDJ.
- **Validation des entrées MIDI**.
- **Envoi du score final**.

## Dépannage et résolution de problèmes

| **Problème**                    | **Solution**                                                                                    |
| ------------------------------- | ----------------------------------------------------------------------------------------------- |
| **Aucune note n'est détectée**  | Vérifiez que le **clavier MIDI est bien branché** et reconnu avec `aconnect -l`.                |
| **Pas de son**                  | Vérifiez que l'option **"Activer le son"** est cochée et que les fichiers `.wav` sont présents. |
| **Connexion au MDJ impossible** | Assurez-vous que le **Moteur de Jeu (MDJ)** est bien lancé avec `./PianoTrainerMDJV1`.          |
| **L'application plante**        | **Relancez l'application** et, si nécessaire, **redémarrez la Raspberry Pi**.                   |

## Auteurs

- **Auteurs** : Fankam Jisele, Fauré Guilhem
- **Auteur original** : _Mahut Vivien_
- **Documentation utilisateur et technique** : _Voir
  `docs/DocumentationUtilisateur.pdf`_

## Remarque

Ce projet a été développé dans le cadre d'un projet scolaire d'ingénierie et
vise à être une **plateforme pédagogique d'apprentissage du piano** sur
**clavier MIDI**.

## Licence

**Open Source** : Vous pouvez utiliser et modifier ce projet en respectant la
licence du dépôt.

**Merci d'utiliser Piano Trainer !**
