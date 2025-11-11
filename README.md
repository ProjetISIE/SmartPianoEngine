---
lang: fr
---

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

## Dépannage et résolution des problèmes

| **Problème**                    | **Solution**                                                                                    |
| ------------------------------- | ----------------------------------------------------------------------------------------------- |
| **Aucune note n'est détectée**  | Vérifiez que le **clavier MIDI est bien branché** et reconnu avec `aconnect -l`.                |
| **Pas de son**                  | Vérifiez que l'option **"Activer le son"** est cochée et que les fichiers `.wav` sont présents. |
| **Connexion au MDJ impossible** | Assurez-vous que le **Moteur de Jeu (MDJ)** est bien lancé avec `./PianoTrainerMDJV1`.          |
| **L'application plante**        | **Relancez l'application** et, si nécessaire, **redémarrez la Raspberry Pi**.                   |

## Contribution et conventions de code

Ce projet utilise [Nix](https://nixos.org) pour télécharger les
(bonnes versions des) dépendances, configurer l’environnement, et permettant
in-fine d’effectuer des compilations (croisées) reproductibles.
Il est défini dans [`flake.nix`](./flake.nix) et s’active avec
la commande `nix flake develop` (`nix` doit être installé) ou plus simplement via
[`direnv`](https://direnv.net) (qui doit aussi être installé séparément).

### Outils

| Fonction                                | Outil                                     |
| --------------------------------------- | ----------------------------------------- |
| Compilation C++                         | [Clang](https://clang.llvm.org)           |
| Assistance langage C++                  | [clangd](https://clangd.llvm.org) (LSP)   |
| Formatage de code                       | [clangd](https://clangd.llvm.org) (LSP)   |
| Débogage C++                            | [lldb](https://lldb.llvm.org)             |
| Système de build                        | [CMake](https://cmake.org) (utilise Make) |
| Versionnage et collaboration            | [Git](https://git-scm.com) avec GitHub    |
| Gestion de dépendances et environnement | [Nix](https://nixos.org)                  |

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

### Organisation

| Fichier                 | Contenu                                              |
| ----------------------- | ---------------------------------------------------- |
| `src/main.c`            | Initialisation, boucle principale, fin et nettoyages |

### Nommage des symboles

> Définis dans [`.clang-tidy`](./.clang-tidy)

| Symbole                                     | Convention                   |
| ------------------------------------------- | ---------------------------- |
| Variable (`uint32_t`)                       | `snake_case` (`my_var = …`)  |
| Tableau (`example_tab [] = …`)              | `snake_case` suivi de `_tab` |
| Pointeur (`* example_ptr = …`)              | `snake_case` suivi de `_ptr` |
| Macro (constante) (`#DEFINE`)               | `UPPER_CASE` (`MY_CONST …`)  |
| Fonction (`void my_func(uint32_t arg) {…}`) | `snake_case`                 |
| Types (`typedef` `struct`/`enum` `{…} …`)   | `snake_case` suivi de `_t`   |

```yaml
Checks: >
  bugprone-*,
  cert-*,
  clang-analyzer-*,
  clang-diagnostic-*,
  concurrency-*,
  modernize-*,
  performance-*,
  readability-*,
CheckOptions:
  readability-identifier-naming:
    EnumConstantCase:      UPPER_CASE
    ConstexprVariableCase: UPPER_CASE
    GlobalConstantCase:    UPPER_CASE
    ClassCase:             CamelCase
    StructCase:            CamelCase
    EnumCase:              CamelCase
    FunctionCase:          camelBack
    GlobalFunctionCase:    camelBack
    VariableCase:          camelBack
    GlobalVariableCase:    camelBack
    ParameterCase:         camelBack
    NamespaceCase:         lower_case
```

### Formatage du code (sauts de ligne, indentation…)

Formatage automatique avec clang-format. Style de code défini dans le fichier
[`.clang-format`](./.clang-format).

```yml
BasedOnStyle: LLVM # Se baser sur le style « officiel » de Clang
IndentWidth: 4     # Indenter fortement pour décourager trop de sous-niveaux
---
Language: Cpp # Règles spécifiques au C++
AllowShortBlocksOnASingleLine: Empty # Code plus compact, ex. {}
AllowShortCaseExpressionOnASingleLine: true # Code plus compact, ex. switch(a) { case 1: … }
AllowShortCaseLabelsOnASingleLine: true # Code plus compact, ex. case 1: …
AllowShortIfStatementsOnASingleLine: AllIfsAndElse # Code plus compact, ex. if (a) { … } else { … }
AllowShortLoopsOnASingleLine: true # Code plus compact, ex. for (…) { … }
DerivePointerAlignment: false # Tout le temps…
PointerAlignment: Left # afficher le marquer de pointeur "*" collé au type
```

### Documentation du code

Documentation de toutes les méthodes/fonctions en français, suivant la syntaxe
[Doxygen](https://www.doxygen.nl/manual), comportant au moins la mention
`@brief`, ainsi que `@param` si la fonction prend un ou plusieurs paramètres, et
`@return` si son type de retour n’est pas `void`.

```c
/**
 * @brief Ne fais rien, mis à part être un fonction d’exemple
 * @param arg1 Le premier argument
 * @param arg2 Le second argument
 */
void my_func(uint32_t arg1, uint16_t arg2);
```

### Autre

Utilisation des entiers de taille définie `uint8_t`, `uint16_t`, `uint32_t` et
`uint64_t` de la bibliothèque `<stdint.h>` au lieu des `char`, `short`, `int` et
`long` dépendants de la plateforme.

Norme utilisée du langage C++ la plus récente,
[C++23](https://en.wikipedia.org/wiki/C23_(C_standard_revision)).

## Auteurs

Fankam Jisele, Fauré Guilhem

> L’auteur original est Mahut Vivien
