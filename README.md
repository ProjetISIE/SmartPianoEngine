---
lang: fr
---

# Smart Piano (moteur de jeu)

<!--toc:start-->

- [Smart Piano (moteur de jeu)](#smart-piano-moteur-de-jeu)
  - [Matériel](#matériel)
  - [Dépannage et résolution des problèmes](#dépannage-et-résolution-des-problèmes)
  - [Contribution et conventions de code](#contribution-et-conventions-de-code)
    - [Outils](#outils)
    - [Documentation du code](#documentation-du-code)
    - [Formatage du code (sauts de ligne, espaces)](#formatage-du-code-sauts-de-ligne-espaces)
    - [Nommage des symboles (casse, tirets)](#nommage-des-symboles-casse-tirets)
    - [Organisation](#organisation)
    - [Autre](#autre)
  - [Auteurs](#auteurs)

<!--toc:end-->

Smart Piano est une application aidant à progresser au piano en s'entrainant à
en jouer d’une manière optimisant l’apprentissage grâce aux exercices
intelligents de l’application.

L’utilisateur interagit via un **clavier MIDI** connecté au dispositif Smart
Piano.

L'application propose plusieurs **modes de jeu** (notes, accords…) et évalue la
performance en temps réel pour proposer les exercices les plus propices à faire
progresser.

Ce projet, développé dans le cadre du cursus de Polytech Tours, vise à être une
plateforme ayant un intérêt **pédagogique** à la fois sur le piano et sur le
développement (`C++`).

## Matériel

Smart Piano a été conçu pour fonctionner avec :

- **Raspberry Pi 4**
  - Sous **Raspberry Pi OS**
  - MicroSD de **32 Go minimum**
- **Écran tactile** connecté à la Raspberry Pi (via HDMI)
- **Clavier MIDI** standard (ex. SWISSONIC EasyKeys49)

De plus, la compilation peut requérir :

- **Connexion internet** pour télécharger les dépendances
- **Accès au terminal** pour l'installation et la configuration

Il est néanmoins possible que l'application fonctionne sur d'autres systèmes
d’exploitations ou architectures, sans garantie.

## Dépannage et résolution des problèmes

| **Problème**                    | **Solution**                                                                    |
| ------------------------------- | ------------------------------------------------------------------------------- |
| **Aucune note n'est détectée**  | Vérifier que le **clavier MIDI est bien branché** et reconnu avec `aconnect -l` |
| **Connexion au MDJ impossible** | S’assurer que le **Moteur de Jeu (MDJ)** est bien lancé : `./PianoTrainerMDJV1` |
| **L'application plante**        | **Relancer l'application**, voire **redémarrer la Raspberry Pi**                |

## Architecture

Voir [ARCHITECTURE.md](ARCHITECTURE.md) pour les détails complets de
l'architecture.

- **Couche Application** : Point d'entrée (`main.cpp`)
- **Couche Domaine** : Logique de jeu (GameEngine, modes de jeu, logique
  musicale)
- **Couche Infrastructure** : Transport UDS, entrée MIDI, logs

Voir [PROTOCOL.md](PROTOCOL.md) pour la spécification complète du protocole.

## Contribution et conventions de code

Ce projet utilise [Nix](https://nixos.org) pour télécharger les (bonnes versions
des) dépendances, configurer l’environnement, et permettre in-fine d’effectuer
des compilations (croisées) reproductibles. L’environnement Nix est défini dans
[`flake.nix`](./flake.nix) et s’active avec la commande `nix flake develop`
(`nix` doit être installé) ou plus simplement via [`direnv`](https://direnv.net)
(qui doit aussi être installé séparément).

Pour compiler le projet, il est possible (pour tester) d’utiliser
[CMake](https://cmake.org) (`cmake` puis `cmake --build`) directement depuis un
environnement Nix activé, mais la solution préconisée (car reproductible) est
d’utiliser `nix build` ; ou `nix build .#cross` pour compiler en ciblant
l’architecture de la Raspberry Pi 4 (ARM64).

Le serveur peut être lancé avec `./result/bin/engine`, ou `./build/engine` si
compilé avec CMake.

Le moteur démarre et écoute sur `/tmp/smartpiano.sock`.

### Tester avec un client simple

Exemple avec `socat` :

```bash
# Dans un autre terminal
socat - UNIX-CONNECT:/tmp/smartpiano.sock
```

Puis envoyer des commandes :

```
config
game=note
scale=c
mode=maj
```

Le moteur répond (normalement) avec :

```
ack
status=ok
```

Puis envoyer `ready` pour commencer :

```
ready
```

Le moteur envoie un « challenge » :

```
note
note=c4
id=1
```

Jouer la note sur le clavier MIDI, le moteur répond :

```
result
id=1
correct=c4
duration=1234
```

### Ajouter un nouveau mode de jeu

1. Créer une classe héritant de `IGameMode`
2. Implémenter `start()`, `play()`, `stop()`
3. Enregistrer dans `GameEngine::createGameMode()`

### Ajouter un nouveau transport

1. Créer une classe héritant de `ITransport`
2. Implémenter les méthodes de communication
3. Injecter dans le `main.cpp`

### Outils

| Fonction                     | Outil                                         |
| ---------------------------- | --------------------------------------------- |
| Compilation C++              | [Clang](https://clang.llvm.org)               |
| Système de build             | [CMake](https://cmake.org) (+ Ninja)          |
| Dépendances et environnement | [Nix](https://nixos.org)                      |
| Versionnage et collaboration | [Git](https://git-scm.com) avec GitHub        |
| Tests unitaires              | [doctest](https://github.com/doctest/doctest) |
| Assistance langage C++       | [clangd](https://clangd.llvm.org) (LSP)       |
| Documentation depuis le code | [Doxygen](https://www.doxygen.nl)             |
| Formatage du C++             | [clang-format](https://clangd.llvm.org)       |
| Contrôle qualité C++         | [clang-tidy](https://clangd.llvm.org)         |
| Débogage C++                 | [lldb](https://lldb.llvm.org)                 |

### Documentation du code

Documentation de toutes les méthodes / fonctions en français, suivant la syntaxe
[Doxygen](https://www.doxygen.nl/manual), comportant au moins un (court) premier
paragraphe expliquant rapidement la raison d’être de la fonction, ainsi qu’une
ligne `@param` par paramètre, et `@return` si son type de retour n’est pas
`void`.

```c++
/**
 * Ne fais rien, mis à part être une fonction d’exemple
 * @param firstArg Le premier argument
 * @param secondArg Le second argument
 */
void myFunc(uint32_t firstArg, uint16_t secondArg);
```

### Formatage du code (sauts de ligne, espaces)

Formatage automatique avec clang-format selon la convention de code de LLVM
(l’organisation derrière Clang) avec quelques ajustements pour rendre le code
plus compact (définis dans le fichier [`.clang-format`](./.clang-format)).

```yaml
BasedOnStyle: LLVM # Se baser sur le style « officiel » de Clang
IndentWidth: 4 # Indenter fortement pour décourager trop de sous-imbrication
---
Language: Cpp # Règles spécifiques au C++ (le projet pourrait utiliser plusieurs langages)
AllowShortBlocksOnASingleLine: Empty # Code plus compact, ex. {}
AllowShortCaseExpressionOnASingleLine: true # Code plus compact, ex. switch(a) { case 1: … }
AllowShortCaseLabelsOnASingleLine: true # Code plus compact, ex. case 1: …
AllowShortIfStatementsOnASingleLine: AllIfsAndElse # Code plus compact, ex. if (a) { … } else { … }
AllowShortLoopsOnASingleLine: true # Code plus compact, ex. for (…) { … }
DerivePointerAlignment: false # Tout le temps…
PointerAlignment: Left # afficher le marquer de pointeur * collé au type
```

### Nommage des symboles (casse, tirets)

Avertissements de non-respect de la convention par clang-tidy selon les règles
de nommage définies dans le fichier [`.clang-tidy`](./.clang-tidy).

```yaml
CheckOptions.readability-identifier-naming:
  EnumConstantCase: UPPER_CASE
  ConstexprVariableCase: UPPER_CASE
  GlobalConstantCase: UPPER_CASE
  ClassCase: CamelCase
  StructCase: CamelCase
  EnumCase: CamelCase
  FunctionCase: camelBack
  GlobalFunctionCase: camelBack
  VariableCase: camelBack
  GlobalVariableCase: camelBack
  ParameterCase: camelBack
  NamespaceCase: lower_case
```

| Symbole            | Convention                    |
| ------------------ | ----------------------------- |
| Enum constante     | `UPPER_CASE` (`MY_ENUM`)      |
| Constexpr          | `UPPER_CASE` (`MY_CONSTEXPR`) |
| Constante globale  | `UPPER_CASE` (`MY_CONST`)     |
| Classe             | `CamelCase` (`MyClass`)       |
| Struct             | `CamelCase` (`MyStruct`)      |
| Enum               | `CamelCase` (`MyEnum`)        |
| Fonction           | `camelBack` (`MyMethod`)      |
| Fonction globale   | `camelBack` (`MyFunc`)        |
| Variable / Objet   | `camelBack` (`myVar`)         |
| Variable globale   | `camelBack` (`myGlobalVar`)   |
| Paramètre          | `camelBack` (`myParam`)       |
| Espace de nommage  | `snake_case` (`my_namespace`) |
| Définition de type | `snake_case` suivi de `_t`    |

### Organisation

| Fichier      | Contenu                                              |
| ------------ | ---------------------------------------------------- |
| `src/main.c` | Initialisation, boucle principale, fin et nettoyages |
| `src/*`      | À compléter…                                         |

### Autre

Utilisation des entiers de taille strictement définie `uint8_t`, `uint16_t`,
`uint32_t` et `uint64_t` de la bibliothèque `<stdint.h>` au lieu des `char`,
`short`, `int` et `long` variant selon la plateforme ou l’environnement.

Vérification automatique de nombreuses règles de qualité de code par clang-tidy
(définies dans [`.clang-tidy`](./.clang-tidy)).

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
```

Norme utilisée du langage C++ la plus récente (stable),
[C++23](https://en.wikipedia.org/wiki/C%2B%2B23).

## Auteurs

Fankam Jisele, Fauré Guilhem

> L’auteur original est Mahut Vivien

## Configuration

### Modes de jeu disponibles

1. Notes `note` : Reconnaissance de notes individuelles
   - Le joueur doit jouer la note affichée

2. Accords `chord` : Accords simples (sans renversement)
   - Le joueur doit jouer les 3 notes de l'accord dans n'importe quel ordre

3. Accords renversés `inversed` : Accords avec renversements
   - Le joueur doit jouer l'accord, mais il est renversé

### Gammes supportées

- `c` : Do
- `d` : Ré
- `e` : Mi
- `f` : Fa
- `g` : Sol
- `a` : La
- `b` : Si

### Modes supportés

- `maj` : Majeur
- `min` : Mineur

## Logs

Le moteur génère deux fichiers de logs:

- `smartpiano.log` : Logs normaux
- `smartpiano.err.log` : Logs d'erreurs

## Dépannage

### Le socket existe déjà

Si le socket `/tmp/smartpiano.sock` existe déjà, le supprimer :

```bash
rm -f /tmp/smartpiano.sock
```

### MIDI non détecté

Vérifier que JACK est démarré et que le périphérique MIDI est connecté :

```bash
aconnect -l
```

## Tests

Les tests unitaires peuvent être exécutés avec :

```bash
cd build
ctest
```

Ils sont automatiquement exécutés lors des builds avec Nix.

## Licence

Voir [LICENSE](LICENSE)
