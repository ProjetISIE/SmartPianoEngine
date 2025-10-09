# Piano Trainer (moteur)

Piano Trainer est une application aidant à progresser au piano en s'entrainer
intelligemment à jouer des **notes** et **accords** sur un **clavier MIDI**
connecté.

L'application propose plusieurs **modes de jeu** et évalue la performance du
joueur en temps réel pour lui proposer les exercices les plus propices à le
faire progresser.

Ce projet, développé dans un cadre scolaire (Polytech Tours), vise à être une
plateforme ayant un intérêt **pédagogique** à la fois sur le piano et sur le
développement (`C++`…).

## Matériel

Smart Piano a été conçu pour fonctionner avec :

- **Raspberry Pi 4** sous **Raspberry Pi OS** avec une carte
  Micro SD de **32 Go minimum**.
- **Un écran tactile** connecte à la Raspberry Pi via HDMI.
- **Un clavier MIDI** standard (ex : `SWISSONIC EasyKeys49`).

De plus, la compilation peut requérir :

- **Connexion internet** pour télécharger les dépendances.
- **Accès au terminal** pour l'installation et la configuration.

Il est néanmoins possible que l'application fonctionne sur d'autres
systèmes d’exploitations et architectures.

## Utilisation

### Écran d'accueil

Lors du lancement, l'utilisateur peut :

- **Sélectionner un type de jeu** :
  - **Jeu de note** : Joue des notes une par une.
  - **Jeu d'accords SR** : Joue des accords sans renversement.
  - **Jeu d'accords AR** : Joue des accords avec renversement.
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

## Dépannage et résolution des problèmes

| **Problème**                    | **Solution**                                                                                    |
| ------------------------------- | ----------------------------------------------------------------------------------------------- |
| **Aucune note n'est détectée**  | Vérifiez que le **clavier MIDI est bien branché** et reconnu avec `aconnect -l`.                |
| **Pas de son**                  | Vérifiez que l'option **"Activer le son"** est cochée et que les fichiers `.wav` sont présents. |
| **Connexion au MDJ impossible** | Assurez-vous que le **Moteur de Jeu (MDJ)** est bien lancé avec `./PianoTrainerMDJV1`.          |
| **L'application plante**        | **Relancez l'application** et, si nécessaire, **redémarrez la Raspberry Pi**.                   |

## Communications front-end <-> back-end

L'application s’architecture autour d’une communication client-serveur
HTTP entre l'*IHM* et le **moteur de jeu** (MDJ).

**Le serveur tourne sur le port 8080** et échange des messages encodés en
**JSON** entre l'IHM et le MDJ :

- **Paramètres du jeu** envoyés par l'IHM.
- **Notes et accords** générés par le MDJ.
- **Validation des entrées MIDI**.
- **Envoi du score final**.

## Contribution

Ce projet utilise [Nix](https://nixos.org) pour télécharger les
(bonnes versions des) dépendances, configurer l’environnement, et permettant
in-fine d’effectuer des compilations (croisées) reproductibles.
Il est défini dans [`flake.nix`](./flake.nix) et s’active avec
la commande `nix flake develop` (`nix` doit être installé) ou plus simplement via
[`direnv`](https://direnv.net) (qui doit aussi être installé séparément).

### Compilation

Commencer donc par activer l’environnement pour l’IHM avec `nix flake develop`,
puis générer les Makefiles avec la commande `qmake`, et finalement compiler
l’IHM avec la commande `make`.

Réitérer désormais les mêmes étapes pour le MDJ (Moteur de Jeu), à commencer
pas changer de répertoire de travail avec `cd engine`. Ensuite, activer
l’environnement pour le MDJ avec `nix flake develop`, générer les Makefiles
avec la commande `qmake`, et finalement compiler le MDJ avec la commande `make`.

Bravo, il est maintenant possible de lancer l’IHM (qui devrait lancer le MDJ
automatiquement) avec `./IHM` (après être revenu dans le répertoire `cd ..`).

## Auteurs

Fankam Jisele, Fauré Guilhem

> L’auteur original est Mahut Vivien
