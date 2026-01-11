# Architecture de Smart Piano Engine

Smart Piano est une application d'apprentissage du piano utilisant un clavier
MIDI, composée d’une partie "règles du jeu" et d’une partie "affichage"
communicant par sockets Unix. Ceci est l’architecture du moteur de jeu (partie
"règles du jeu"), qui suit les principes SOLID et propose une séparation claire
des responsabilités.

## Principes architecturaux

1. **Séparation des préoccupations** : Chaque composant a une responsabilité
   unique et bien définie
2. **Inversion de dépendance** : Les modules de haut niveau ne dépendent pas des
   modules de bas niveau
3. **Interface claire** : Communication via des interfaces bien définies
4. **Testabilité** : Chaque composant peut être testé indépendamment
5. **Extensibilité** : Facile d'ajouter de nouveaux modes de jeu ou
   fonctionnalités

## Architecture en couches

```
┌──────────────────────────────────────────┐
│      Couche applicative (main.cpp)       │
└──────────────────────────────────────────┘
                      │
                      ▼
┌───────────────────────────────────────────────────────┐
│            Couche "logique métier"                    │
│ ┌───────────────────────────────────────────────────┐ │
│ │ Moteur de jeu (Orchestration)                     │ │
│ └───────────────────────────────────────────────────┘ │
│ ┌───────────────────────────────────────────────────┐ │
│ │ Modes de jeux (Notes, Accords, Accords Renversés) │ │
│ └───────────────────────────────────────────────────┘ │
│ ┌───────────────────────────────────────────────────┐ │
│ │ Logique musicale (Gamme, Note, Accord, Justesse)  │ │
│ └───────────────────────────────────────────────────┘ │
└───────────────────────────────────────────────────────┘
                      │
                      ▼
┌──────────────────────────────────────────┐
│         Couche d’infrastructure          │
│ ┌───────────┐ ┌─────────────┐ ┌────────┐ │
│ │  IPC/UDS  │ │ Entrée MIDI │ │ Logger │ │
│ │ Transport │ │ (Sortie)    │ │        │ │
│ └───────────┘ └─────────────┘ └────────┘ │
└──────────────────────────────────────────┘
```

## Composants principaux

### Couche applicative

**Responsabilité** : Point d'entrée de l'application, initialisation et gestion
du cycle de vie.

- `main.cpp` : Configure l'application, instancie les composants et lance le
  moteur de jeu

### Couche logique métier

#### Moteur de jeu (GameEngine)

**Responsabilité** : Orchestrateur principal qui coordonne les différents
composants.

```cpp
class GameEngine {
public:
  GameEngine(ITransport& transport, IMidiInput& midi);
  void run();
  void stop();
  
private:
  void handleClientConnection();
  void processGameSession(const GameConfig& config);
  ITransport& transport_;
  IMidiInput& midi_;
  std::unique_ptr<IGameMode> currentGame_;
};
```

#### Interface mode de jeu (IGameMode)

**Responsabilité** : Définit le contrat pour tous les modes de jeu.

```cpp
class IGameMode {
public:
  virtual ~IGameMode() = default;
  virtual void start() = 0;
  virtual GameResult play() = 0;
  virtual void stop() = 0;
};
```

#### Modes de jeu concrets

- `NoteGame` : Jeu de reconnaissance de notes individuelles
- `ChordGameSimple` : Jeu d'accords sans renversement
- `ChordGameInversions` : Jeu d'accords avec renversements

#### Logique musicale

- `Scale` : Représente une gamme musicale (Do majeur, La mineur, etc.)
- `Note` : Représente une note musicale (C4, D#5, etc.)
- `Chord` : Représente un accord (Do majeur, Ré mineur, etc.)
- `NoteGenerator` : Génère des notes/accords aléatoires selon une gamme
- `NoteValidator` : Valide si les notes jouées correspondent à l'attendu

### Couche d’infrastructure

#### Interface de transport (ITransport)

**Responsabilité** : Abstraction de la communication client-serveur.

```cpp
class ITransport {
public:
  virtual ~ITransport() = default;
  virtual bool start(const std::string& endpoint) = 0;
  virtual void waitForClient() = 0;
  virtual void send(const Message& msg) = 0;
  virtual Message receive() = 0;
  virtual void stop() = 0;
};
```

#### Implémentation du transport (UdsTransport)

**Responsabilité** : Implémentation UDS (Unix Domain Socket) du transport.

#### Interface d’entrée MIDI (IMidiInput)

**Responsabilité** : Abstraction de l'entrée MIDI.

```cpp
class IMidiInput {
public:
  virtual ~IMidiInput() = default;
  virtual bool initialize() = 0;
  virtual std::vector<Note> readNotes() = 0;
  virtual void close() = 0;
};
```

#### Implémentation d’entrée MIDI (RtMidiInput)

**Responsabilité** : Implémentation RTMidi de l'entrée MIDI.

## Flux de données

### Démarrage, mise en place

1. `main()` crée les composants
2. `GameEngine.run()` lance le transport UDS
3. `GameEngine` attend une connexion client

### Session de jeu

1. `Client` se connecte
2. `Client` envoie `GameConfig` (type de jeu, gamme, mode)
3. `GameEngine` crée le `GameMode` approprié
4. `GameMode` génère un challenge (note ou accord)
5. `GameMode` envoie le challenge au client via Transport
6. `GameMode` attend l'input MIDI
7. `GameMode` valide l'input
8. `GameMode` envoie le résultat au client
9. Répéter 4-8 jusqu'à fin du jeu
10. `GameMode` envoie le score final
11. `GameEngine` attend une nouvelle configuration ou déconnexion

## Avantages de cette architecture

1. **Testabilité** : Chaque composant peut-être "simulé" via son interface
2. **Flexibilité** : Facile d'ajouter de nouveaux modes de jeu ou transports
3. **Maintenance** : Séparation claire des responsabilités
4. **Modularité** : Les composants du domaine musical sont réutilisables
5. **Performance** : Pas de couplage inutile, chaque composant est optimisé

## Niveaux d’erreurs

1. **Erreurs fatales** : Log + arrêt de l'application (ex : MIDI non disponible)
   car impossible de continuer
2. **Erreurs de session** : Log + fin de la session en cours (ex : client
   déconnecté) ; le moteur attend une nouvelle connexion client
3. **Erreurs de jeu** : Log + retour d'erreur au client (ex : note invalide)

## Configuration

```cpp
struct GameConfig {
  std::string gameType; // "note", "chord_simple", "chord_inversions"
  std::string scale;    // "Do", "Re", "Mi", etc.
  std::string mode;     // "Majeur", "Mineur"
  int maxChallenges;    // Nombre de défis
};
```

## Extensibilité

Il est facile de créer un mode de jeu consistant en une classe (dans
`GameEngine`) héritant de `IGameMode` et implémentant les méthodes `start()`,
`play()`, `stop()`.

Idem pour un transport consistant en une classe héritant de `ITransport` (dans
`GameEngine` aussi) et implémentant les méthodes de communication.
