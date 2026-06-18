---
lang: fr
---

# Smart Piano (moteur de jeu)

<!--toc:start-->

- [Modes de Jeu](#modes-de-jeu)
  - [Gammes Supportées](#gammes-supportées)
  - [Modes Supportés](#modes-supportés)
- [Retour Audio](#retour-audio)
- [Outillage](#outillage)
- [Compilation & Exécution](#compilation-exécution)
  - [Test Manuel](#test-manuel)
  - [Tests Automatiques](#tests-automatiques)
- [Conventions de Code](#conventions-de-code)
  - [Documentation Doxygen](#documentation-doxygen)
  - [Nommage des Symboles (casse, tirets)](#nommage-des-symboles-casse-tirets)
  - [Formatage du Code (sauts de ligne, espaces)](#formatage-du-code-sauts-de-ligne-espaces)
  - [Autres](#autres)
- [Journalisation](#journalisation)
- [Auteurs & Licence](#auteurs-licence)
- [Contribution](#contribution)
  - [Ajout d’un Mode de Jeu](#ajout-dun-mode-de-jeu)
  - [Ajout d’un Transport](#ajout-dun-transport)
- [Architecture](#architecture)
  - [Composants Musicaux](#composants-musicaux)

<!--toc:end-->

Smart Piano est un système aidant à progresser au piano en s'entrainant à en
jouer d’une manière optimisant l’apprentissage grâce à des exercices
intelligents.

Ce dépôt est la partie moteur de jeu, déterminant les challenges et les règles
du jeu, mais ne communiquent que via un [protocole](PROTOCOL.md) textuel simple.
Une [interface utilisateur] a été développée sur ce protocole pour rendre
l’utilisation agréable.

L’utilisateur interagit via un **clavier MIDI** connecté au dispositif Smart
Piano, sur lequel fonctionne l’application.

L'application propose plusieurs **modes de jeu** (notes, accords…) et évalue la
performance en temps réel pour proposer les exercices les plus propices à faire
progresser.

Ce projet, développé dans le cadre du cursus de [Polytech Tours], vise à être
une plateforme ayant un intérêt **pédagogique** à la fois sur le piano et sur le
développement (`C++`).

## Modes de Jeu

1. Notes `note` : Reconnaissance de notes individuelles
   - Le joueur doit jouer la note affichée

2. Accords `chord` : Accords simples (sans renversement)
   - Le joueur doit jouer les 3 notes de l'accord dans n'importe quel ordre

3. Accords renversés `inversed` : Accords avec renversements
   - Le joueur doit jouer l'accord, mais il est renversé

### Gammes Supportées

- `c` : Do
- `d` : Ré
- `e` : Mi
- `f` : Fa
- `g` : Sol
- `a` : La
- `b` : Si

### Modes Supportés

- `maj` : Majeur
- `min` : Mineur

## Retour Audio

**Smart Piano** ne produit pas de son en lui-même, il traite simplement les
données _MIDI_ (les notes jouées) en fonction d’exercices. Pour entendre un son
de piano, il est nécessaire de router aussi les données _MIDI_ vers un
synthétiseur tel que [FluidSynth] tournant en parallèle.

## Outillage

| Fonction                         | Outil                    |
| -------------------------------- | ------------------------ |
| Compilation C++                  | [Clang]                  |
| Système de build                 | [CMake] (+ Ninja)        |
| Dépendances et environnement     | [Nix]                    |
| Versionnage et collaboration     | [Git] hébergé sur GitHub |
| Tests automatisés                | [doctest]                |
| Couverture de code               | [llvm-cov]               |
| Assistance langage C++           | [clangd] (LSP)           |
| Documentation depuis le code     | [Doxygen]                |
| Formatage du C++                 | [clang-format]           |
| Contrôle qualité C++             | [clang-tidy]             |
| Débogage C++                     | [lldb]                   |
| Test manuel communication socket | [socat]                  |
| Édition du code                  | [VS Code], [Helix]…      |

## Compilation & Exécution

Ce projet utilise [Nix] pour télécharger les (bonnes versions des) dépendances,
configurer l’environnement, et permettre in-fine d’effectuer des compilations
(croisées) reproductibles. L’environnement [Nix] est défini dans
[`flake.nix`](./flake.nix) et s’active avec la commande `nix flake develop`
(`nix` doit être installé) ou plus simplement via [`direnv`] (qui doit aussi
être installé séparément).

Pour compiler le projet, il est possible (pour tester) d’utiliser [CMake]
(`cmake --build build`) directement depuis un environnement [Nix] activé, mais
la solution préconisée (car reproductible) est d’utiliser `nix build` ; ou
`nix build .#cross` pour compiler en ciblant l’architecture de la Raspberry Pi 4
(ARM64).

Le serveur peut être lancé avec `./result/bin/main`, ou `./build/main` si
compilé avec [CMake] (ou automatiquement après un build avec
`cmake --build build --target run`). Le moteur démarre et écoute sur
`/tmp/smartpiano.sock`.

> Pour accélérer les opérations impliquant `cmake`, indiquer le nombre `N` de
> threads correspondant au nombre de cœurs de processeur avec `-jN` (ex.
> `cmake --build build -j4`) ou `--jobs N` pour `nix` (ex.
> `nix build .#cross --jobs 4`)

### Test Manuel

Il est possible de tester le moteur de jeu manuellement avec un client UDS tel
que `socat`.

```bash
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

### Tests Automatiques

Les tests unitaires et tests d’intégration peuvent être exécutés manuellement
avec la commande `cmake --build build --target tests`.

Ils sont automatiquement exécutés lors des builds avec [Nix].

De plus, il est possible de générer un rapport de couverture de code avec
`cmake --build build --target coverage`, puis d’en visualiser un résumé avec
`llvm-cov report build/src/main -instr-profile=build/coverage.profdata -ignore-filename-regex="test/.*"`.

Sur la branche principale `main`, tous les tests automatiques (unitaires,
intégration) doivent passer parfaitement, avec une couverture de `100%` des
fonctions et d’au moins `90%` des lignes de code. Il faut s’assurer qu’une
branche répond à ces critères avant de la fusionner dans `main`.

L’objectif est de couvrir `100%` des lignes de codes, mais certains cas peuvent
être trop difficiles à simuler en tests automatiques, auquel cas, ils doivent
être commentés comme tel.

## Conventions de Code

Norme utilisée du langage C++ la plus récente (stable), `C++23`. Utilisation de
ses fonctionnalités modernes et respect des meilleures pratiques. Par exemple,
`std::println()` est préféré à `std::cout <<` ou `std::cerr <<`.

### Documentation Doxygen

Documentation des attributs, méthodes et fonctions en _français_ (correct),
suivant la syntaxe [Doxygen], comportant au moins un (concis) premier paragraphe
expliquant rapidement la raison d’être de la fonction, ainsi qu’une ligne
`@param` par paramètre, et `@return` si son type de retour n’est pas `void`.

```c++
const std::string message; ///< Message intéressant

/**
 * Ne fais rien, mis à part être une fonction d’exemple
 * @param firstArg Le premier argument
 * @param secondArg Le second argument
 */
void myFunc(uint32_t firstArg, uint16_t secondArg);
```

Ne pas commenter les éléments évidents tels que les getters/setters simples ou
constructeurs/destructeurs simples.

### Nommage des Symboles (casse, tirets)

Avertissements de non-respect de la convention par [clang-tidy] selon les règles
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

| Symbole           | Convention                    |
| ----------------- | ----------------------------- |
| Enum constante    | `UPPER_CASE` (`MY_ENUM`)      |
| Constexpr         | `UPPER_CASE` (`MY_CONSTEXPR`) |
| Constante globale | `UPPER_CASE` (`MY_CONST`)     |
| Classe            | `CamelCase` (`MyClass`)       |
| Struct            | `CamelCase` (`MyStruct`)      |
| Enum              | `CamelCase` (`MyEnum`)        |
| Fonction          | `camelBack` (`MyMethod`)      |
| Fonction globale  | `camelBack` (`MyFunc`)        |
| Variable / Objet  | `camelBack` (`myVar`)         |
| Variable globale  | `camelBack` (`myGlobalVar`)   |
| Paramètre         | `camelBack` (`myParam`)       |
| Espace de nommage | `snake_case` (`my_namespace`) |
| Type C (à éviter) | `snake_case` suivi de `_t`    |

### Formatage du Code (sauts de ligne, espaces)

Formatage automatique avec [clang-format] selon la convention de code de LLVM
(l’organisation derrière [Clang]) avec quelques ajustements pour rendre le code
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

### Autres

Utilisation des entiers de taille strictement définie `uint8_t`, `uint16_t`,
`uint32_t` et `uint64_t` de la bibliothèque `<stdint.h>` au lieu des `char`,
`short`, `int` et `long` variant selon la plateforme ou l’environnement.

- Structure des classes en quatre blocs
  1. Attributs privés
  2. Éventuels attributs publics
  3. Éventuelles méthodes privées
  4. Méthodes publiques
- Utilisation de `this->` pour identifier attributs
  - Pas de préfixe ou postfixe tel que `_`
- Pas d’attributs initialisés avec valeurs littérales dans le constructeur
  - Initialiser à la déclaration, ex. `type name{value};`
- Méthodes de moins de trois lignes dans le `.hpp` uniquement
  - Ex. getters/setters simples, constructeurs/destructeurs simples
- Commentaires et strings concis
  - Pas de verbe ni déterminants si compréhensible sans
  - Pas de points finaux
  - Pas d’espace avant les `:`
- Pas de blocs `{}` pour une seule instruction
- Limiter les lignes vides au sein d’une même méthode
  - Préférer la séparation en blocs logiques avec des commentaires
- Limiter l’imbrication des blocs
  - Préférer les retours anticipés (`early return`)
  - Préférer les ET `&&` aux imbrications `if` multiples
  - Privilégier les `switch case` lorsque possible
- Toujours expliciter strictement le comportement des attributs et des méthodes
  - `const`, `noexcept`, `override`, `final`, `nodiscard`, …

Vérification automatique de nombreuses règles de qualité de code par
[clang-tidy] (définies dans [`.clang-tidy`](./.clang-tidy)).

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

## Journalisation

Le moteur génère deux fichiers de journaux (« logs ») qu’il est possible
d’utiliser pour vérifier le bon fonctionnement de Smart Piano ou comprendre
l’origine des erreurs.

- `smartpiano.log` : Logs normaux (tout ce qu’il se passe)
- `smartpiano.err.log` : Logs d'erreurs

## Auteurs & Licence

- Fankam Jisele
- Fauré Guilhem

> Initiateur du projet : Mahut Vivien

L’entièreté de Smart Piano est sous GNU GPL v3, voir [LICENSE](LICENSE).

## Contribution

Respecter les instructions des sections précédentes pour contribuer, en
particulier celles des [Conventions de Code](#conventions-de-code).

### Ajout d’un Mode de Jeu

Pour ajouter un nouveau mode de jeu, suivez ces étapes. Prenons l'exemple d'un
jeu de rythme (`rhythm`) :

1. **Créer la classe du mode de jeu** Créez les fichiers d'en-tête et de source
   (ex. `include/RhythmGame.hpp` et `src/RhythmGame.cpp`). La classe doit
   hériter de l'interface `IGameMode`.

   ```cpp
   #pragma once
   #include "IGameMode.hpp"
   #include "ITransport.hpp"
   #include "IMidiInput.hpp"

   class RhythmGame : public IGameMode {
     public:
       RhythmGame(ITransport& transport, IMidiInput& midi, GameConfig config);
       void start() override;
       GameResult play() override;
       void stop() override;
       
     private:
       ITransport& transport;
       IMidiInput& midi;
       GameConfig config;
       bool running{false};
   };
   ```

2. **Implémenter les méthodes de l'interface** Dans le fichier source (`.cpp`),
   implémentez la logique :
   - `start()` : initialise l'état du jeu.
   - `play()` : contient la boucle principale de la partie. Génère les défis
     (ex. jouer en rythme), écoute le piano via `IMidiInput`, envoie l'état au
     client via `ITransport`, et retourne un `GameResult`.
   - `stop()` : permet l'interruption prématurée du jeu (ex. via un appui de
     touche d'abandon, ou la déconnexion du client).

3. **Déclarer le mode au client** Dans `src/GameEngine.cpp` (méthode
   `handleClientConnection()`), ajoutez votre mode dans la liste diffusée au
   démarrage pour que l'interface le propose :
   ```cpp
   std::vector<GameDesc> games = {
       {"note", "Jeu de notes", 7},
       {"chord", "Jeu d'accords", 14},
       {"inversed", "Jeu d'accords renversés", 14},
       {"rhythm", "Jeu de rythme", 7} // Nouveau mode !
   };
   ```

4. **L'enregistrer dans la fabrique** Toujours dans `src/GameEngine.cpp`
   (méthode `createGameMode()`), ajoutez la condition pour instancier votre jeu
   :
   ```cpp
   std::unique_ptr<IGameMode>
   GameEngine::createGameMode(const GameConfig& config) {
       if (config.gameType == "note")
           return std::make_unique<NoteGame>(/*...*/);
       // ...
       else if (config.gameType == "rhythm")
           return std::make_unique<RhythmGame>(this->transport, this->midi, config);
       
       Logger::err("[GameEngine] Mode de jeu inconnu: {}", config.gameType);
       return nullptr;
   }
   ```

5. **Ajouter aux sources compilées** N'oubliez pas d'ajouter
   `src/RhythmGame.cpp` à la liste des fichiers source compilés dans le fichier
   `CMakeLists.txt` (`add_executable(main ...)` et/ou pour les tests unitaires).

### Ajout d’un Transport

1. Créer une classe héritant de `ITransport`
2. Implémenter les méthodes de communication
3. Injecter dans le `main.cpp`

## Architecture

Voir [PROTOCOL](PROTOCOL.md) pour la spécification complète du protocole de
communication entre le moteur de jeu et l'interface utilisateur.

L'architecture suit le principe **d'inversion de dépendances** avec des
interfaces abstraites (`IGameMode`, `ITransport`, `IMidiInput`) permettant de
découpler les composants et faciliter les tests et l'extensibilité.

[`main`](src/main.cpp): Point d'entrée de l'application. Configure la
journalisation, instancie le transport (UDS) et l'entrée MIDI (RtMidi), crée le
moteur de jeu puis lance la boucle d'événements principale.

[`GameEngine`](include/GameEngine.hpp): **Orchestrateur central** coordonne le
transport, l'entrée MIDI et les modes de jeu. Gère le cycle complet d'une
session : connexion client, réception configuration, création du mode approprié,
exécution de la partie.

[`IGameMode`](include/IGameMode.hpp) Interface définissant le contrat pour tous
les modes de jeu (`start()`, `play()`, `stop()`).

- [`NoteGame`](include/NoteGame.hpp) Mode reconnaissance de notes individuelles
- [`ChordGame`](include/ChordGame.hpp) Mode accords simples ou renversés

[`ITransport`](include/ITransport.hpp) Interface définissant la communication
bidirectionnelle client-serveur.

- [`UdsTransport`](include/UdsTransport.hpp) Implémentation via Unix Domain
  Socket avec sérialisation/parsing de messages selon le protocole défini
- [`Message`](include/Message.hpp) Structure immuable représentant un message du
  protocole (type + champs clé-valeur)

[`IMidiInput`](include/IMidiInput.hpp) Interface pour la lecture MIDI.

- [`RtMidiInput`](include/RtMidiInput.hpp) Implémentation utilisant la
  bibliothèque RtMidi avec traitement asynchrone et conversion MIDI vers `Note`

### Composants Musicaux

[`Note`](include/Note.hpp) Classe immuable représentant une note musicale en
notation standard (lettre a-g + altération optionnelle + octave 0-8).

[`ChordRepository`](include/ChordRepository.hpp) Référentiel statique contenant
tous les accords musicaux mappés par tonalité et degré avec leurs notes MIDI
correspondantes (ex: Do majeur I = C4, E4, G4).

[`ChallengeFactory`](include/ChallengeFactory.hpp) Générateur de notes et
d'accords (simples ou avec renversements) selon une gamme et un mode musical
donné. Intègre un **modèle de Markov** couplé à un système de **répétition
espacée** en mémoire vive pour cibler et faire retravailler dynamiquement les
enchaînements difficiles lors d'une session.

[`AnswerValidator`](include/AnswerValidator.hpp) Validateur spécialisé qui
compare notes et accords joués vs attendus, avec support des renversements
d'accords.

[boitier-support VESA]: https://makerworld.com/en/models/2940514-raspberry-pi-4-vesa-case
[CMake]: https://cmake.org
[Clang]: https://clang.llvm.org
[clangd]: https://clangd.llvm.org
[clang-format]: https://clangd.llvm.org
[clang-tidy]: https://clangd.llvm.org
[C++23]: https://en.wikipedia.org/wiki/C%2B%2B23
[direnv]: https://direnv.net
[`direnv`]: https://direnv.net
[doctest]: https://github.com/doctest/doctest
[Doxygen]: https://www.doxygen.nl
[Doxygen Docs]: https://www.doxygen.nl/manual
[FluidSynth]: https://www.fluidsynth.org
[Git]: https://git-scm.com
[Helix]: https://helix-editor.com
[interface utilisateur]: https://github.com/ProjetISIE/SmartPianoLightUI
[Joy-It RB-LCD-10-2]: https://joy-it.net/en/products/RB-LCD-10-2
[lcov]: https://github.com/linux-test-project/lcov
[lldb]: https://lldb.llvm.org
[llvm-cov]: https://llvm.org/docs/CommandGuide/llvm-cov.html
[Nix]: https://nixos.org
[NixGL]: https://github.com/nix-community/nixGL
[Polytech Tours]: https://polytech.univ-tours.fr
[Raspberry Pi 4]: https://www.raspberrypi.com/products/raspberry-pi-4-model-b
[Raylib]: https://www.raylib.com
[RB-LCD-10-2]: https://joy-it.net/en/products/RB-LCD-10-2
[socat]: http://www.dest-unreach.org/socat
[tio]: https://github.com/tio/tio
[VS Code]: https://code.visualstudio.com
