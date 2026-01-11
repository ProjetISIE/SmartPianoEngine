# Smart Piano UDS Protocol Specification

## Version
Protocol Version: 1.0.0

## Overview

Smart Piano utilise un protocole texte simple sur Unix Domain Socket (UDS) pour la communication entre le moteur de jeu (serveur) et l'interface utilisateur (client).

## Transport

- **Type**: Unix Domain Socket (UDS)
- **Chemin par défaut**: `/tmp/smartpiano.sock`
- **Mode**: SOCK_STREAM (orienté connexion)
- **Encodage**: UTF-8

## Format des messages

### Structure générale

Chaque message est composé de lignes de texte au format `clé=valeur`, terminé par une ligne vide (`\n\n`).

```
clé1=valeur1
clé2=valeur2
clé3=valeur3

```

**Règles syntaxiques**:
- Chaque ligne contient une paire `clé=valeur`
- La clé ne peut pas contenir `=` ou `\n`
- La valeur peut contenir des espaces mais pas de `\n`
- Les lignes sont séparées par `\n` (LF uniquement)
- Le message se termine par `\n\n` (double ligne vide)
- Pas d'espaces autour du `=`
- Encodage UTF-8 pour les caractères accentués

### Exemple
```
type=game_config
game_type=note
scale=Do
mode=Majeur

```

## Types de messages

### 1. Client → Serveur

#### 1.1 Configuration de jeu

Démarre une nouvelle session de jeu.

**Message: `game_config`**
```
type=game_config
game_type=<TYPE>
scale=<GAMME>
mode=<MODE>

```

**Champs**:
- `type`: Toujours `"game_config"`
- `game_type`: Type de jeu
  - `"note"` : Jeu de reconnaissance de notes
  - `"chord_simple"` : Jeu d'accords sans renversement
  - `"chord_inversions"` : Jeu d'accords avec renversements
- `scale`: Gamme musicale
  - Valeurs: `"Do"`, `"Re"`, `"Mi"`, `"Fa"`, `"Sol"`, `"La"`, `"Si"`
- `mode`: Mode de la gamme
  - Valeurs: `"Majeur"`, `"Mineur"`

**Exemple**:
```
type=game_config
game_type=note
scale=Do
mode=Majeur

```

#### 1.2 Prêt pour le challenge suivant

Indique que le client est prêt à recevoir le prochain challenge.

**Message: `ready`**
```
type=ready

```

**Champs**:
- `type`: Toujours `"ready"`

#### 1.3 Abandon de partie

Demande l'arrêt de la session en cours.

**Message: `quit`**
```
type=quit

```

**Champs**:
- `type`: Toujours `"quit"`

### 2. Serveur → Client

#### 2.1 Accusé de réception de configuration

Confirme la réception de la configuration.

**Message: `config_ack`**
```
type=config_ack
status=ok

```

**ou en cas d'erreur**:
```
type=config_ack
status=error
error_code=<CODE>
error_message=<MESSAGE>

```

**Champs**:
- `type`: Toujours `"config_ack"`
- `status`: `"ok"` ou `"error"`
- `error_code`: (optionnel) Code d'erreur
  - `"invalid_game_type"` : Type de jeu invalide
  - `"invalid_scale"` : Gamme invalide
  - `"invalid_mode"` : Mode invalide
  - `"midi_not_ready"` : Périphérique MIDI non disponible
- `error_message`: (optionnel) Message d'erreur descriptif

#### 2.2 Challenge note

Demande au joueur de jouer une note spécifique.

**Message: `challenge_note`**
```
type=challenge_note
note=<NOTE>
challenge_id=<ID>

```

**Champs**:
- `type`: Toujours `"challenge_note"`
- `note`: Note à jouer (ex: `"C4"`, `"D#5"`, `"Gb3"`)
- `challenge_id`: Identifiant unique du challenge (entier positif)

**Exemple**:
```
type=challenge_note
note=C4
challenge_id=1

```

#### 2.3 Challenge accord

Demande au joueur de jouer un accord spécifique.

**Message: `challenge_chord`**
```
type=challenge_chord
chord_name=<NOM>
chord_notes=<NOTES>
inversion=<INVERSION>
challenge_id=<ID>

```

**Champs**:
- `type`: Toujours `"challenge_chord"`
- `chord_name`: Nom de l'accord (ex: `"Do majeur"`, `"Re mineur"`)
- `chord_notes`: Notes de l'accord séparées par des espaces (ex: `"C4 E4 G4"`)
- `inversion`: Numéro de renversement
  - `"0"` : Position fondamentale
  - `"1"` : Premier renversement
  - `"2"` : Deuxième renversement
  - `"-1"` : Renversement non spécifié (accord simple)
- `challenge_id`: Identifiant unique du challenge

**Exemple (position fondamentale)**:
```
type=challenge_chord
chord_name=Do majeur
chord_notes=C4 E4 G4
inversion=0
challenge_id=5

```

**Exemple (premier renversement)**:
```
type=challenge_chord
chord_name=Do majeur 1
chord_notes=E4 G4 C5
inversion=1
challenge_id=6

```

#### 2.4 Résultat du challenge

Informe le joueur du résultat de sa tentative.

**Message: `challenge_result`**
```
type=challenge_result
challenge_id=<ID>
result=<RESULT>
notes_played=<NOTES>

```

**Champs**:
- `type`: Toujours `"challenge_result"`
- `challenge_id`: Identifiant du challenge correspondant
- `result`: Résultat
  - `"correct"` : Réponse correcte
  - `"incorrect"` : Réponse incorrecte
  - `"partial"` : Réponse partiellement correcte (optionnel pour accords)
- `notes_played`: Notes jouées par l'utilisateur (séparées par des espaces)

**Exemple (correct)**:
```
type=challenge_result
challenge_id=1
result=correct
notes_played=C4

```

**Exemple (incorrect)**:
```
type=challenge_result
challenge_id=2
result=incorrect
notes_played=D4

```

#### 2.5 Fin de partie

Indique la fin de la session de jeu avec le score.

**Message: `game_over`**
```
type=game_over
score=<SCORE>
duration=<DURATION>
total_challenges=<TOTAL>
correct_challenges=<CORRECT>

```

**Champs**:
- `type`: Toujours `"game_over"`
- `score`: Score final (temps total en secondes)
- `duration`: Durée totale de la partie en secondes
- `total_challenges`: Nombre total de challenges
- `correct_challenges`: Nombre de challenges réussis

**Exemple**:
```
type=game_over
score=45
duration=45
total_challenges=10
correct_challenges=10

```

#### 2.6 Erreur

Signale une erreur générale.

**Message: `error`**
```
type=error
error_code=<CODE>
error_message=<MESSAGE>

```

**Champs**:
- `type`: Toujours `"error"`
- `error_code`: Code d'erreur
  - `"protocol_error"` : Erreur de protocole (message mal formé)
  - `"invalid_state"` : État invalide (ex: ready sans config)
  - `"midi_error"` : Erreur MIDI
  - `"internal_error"` : Erreur interne du serveur
- `error_message`: Message d'erreur descriptif

**Exemple**:
```
type=error
error_code=protocol_error
error_message=Message mal formé: champ 'type' manquant

```

## Diagramme de séquence

### Session complète

```
Client                          Serveur
  |                                |
  |--- game_config ---------------->|
  |                                |
  |<-- config_ack (ok) ------------|
  |                                |
  |--- ready ---------------------->|
  |                                |
  |<-- challenge_note -------------|
  |                                |
  | [Joueur joue une note]         |
  |                                |
  |<-- challenge_result -----------|
  |                                |
  |--- ready ---------------------->|
  |                                |
  |<-- challenge_note -------------|
  |                                |
  | [Répéter pour N challenges]    |
  |                                |
  |<-- game_over ------------------|
  |                                |
  |--- quit ----------------------->|
  |                                |
```

### Gestion d'erreur

```
Client                          Serveur
  |                                |
  |--- game_config ---------------->|
  |   (invalide)                   |
  |                                |
  |<-- config_ack (error) ---------|
  |                                |
  |--- game_config ---------------->|
  |   (valide)                     |
  |                                |
  |<-- config_ack (ok) ------------|
  |                                |
```

## États de la connexion

1. **DISCONNECTED**: Aucune connexion établie
2. **CONNECTED**: Client connecté mais pas de configuration
3. **CONFIGURED**: Configuration reçue et validée
4. **PLAYING**: Session de jeu en cours
5. **WAITING**: En attente du prochain challenge (après un `ready`)
6. **FINISHED**: Partie terminée

## Transitions d'états

```
DISCONNECTED --[client connect]--> CONNECTED
CONNECTED --[game_config + ok]--> CONFIGURED
CONFIGURED --[ready]--> PLAYING
PLAYING --[challenge]--> WAITING
WAITING --[notes played]--> PLAYING
PLAYING --[last challenge done]--> FINISHED
FINISHED --[game_config]--> CONFIGURED
ANY --[quit]--> DISCONNECTED
ANY --[error]--> DISCONNECTED
```

## Règles de validation

### Messages Client

1. Tous les champs obligatoires doivent être présents
2. Les valeurs des champs doivent être dans les ensembles autorisés
3. Le message doit se terminer par `\n\n`
4. Longueur maximale d'un message : 4096 octets
5. Timeout de réception : 30 secondes

### Messages Serveur

1. Tous les champs obligatoires doivent être présents
2. Le `challenge_id` doit être unique et croissant
3. Le message doit se terminer par `\n\n`
4. Longueur maximale d'un message : 4096 octets

## Gestion des erreurs de protocole

En cas d'erreur de protocole (message mal formé, champ manquant, valeur invalide), le serveur doit :

1. Logger l'erreur
2. Envoyer un message `error` avec le code et message appropriés
3. Fermer la connexion si l'erreur est critique
4. Continuer la session si l'erreur est récupérable

## Exemples d'échanges complets

### Exemple 1 : Jeu de notes réussi

```
→ type=game_config
→ game_type=note
→ scale=Do
→ mode=Majeur
→

← type=config_ack
← status=ok
←

→ type=ready
→

← type=challenge_note
← note=C4
← challenge_id=1
←

[Joueur joue C4]

← type=challenge_result
← challenge_id=1
← result=correct
← notes_played=C4
←

→ type=ready
→

← type=challenge_note
← note=E4
← challenge_id=2
←

[Joueur joue E4]

← type=challenge_result
← challenge_id=2
← result=correct
← notes_played=E4
←

[... après 10 challenges ...]

← type=game_over
← score=32
← duration=32
← total_challenges=10
← correct_challenges=10
←

→ type=quit
→
```

### Exemple 2 : Jeu d'accords avec erreur

```
→ type=game_config
→ game_type=chord_simple
→ scale=Do
→ mode=Majeur
→

← type=config_ack
← status=ok
←

→ type=ready
→

← type=challenge_chord
← chord_name=Do majeur
← chord_notes=C4 E4 G4
← inversion=-1
← challenge_id=1
←

[Joueur joue C4 E4 F4 - erreur!]

← type=challenge_result
← challenge_id=1
← result=incorrect
← notes_played=C4 E4 F4
←

← type=challenge_chord
← chord_name=Do majeur
← chord_notes=C4 E4 G4
← inversion=-1
← challenge_id=1
←

[Joueur joue C4 E4 G4 - correct!]

← type=challenge_result
← challenge_id=1
← result=correct
← notes_played=C4 E4 G4
←

[Suite du jeu...]
```

## Extensions futures

### Version 2.0 (propositions)

- Support de défis complexes (gammes, arpèges)
- Statistiques en temps réel
- Mode multijoueur
- Compression optionnelle des messages
- Authentification
- Chiffrement TLS sur UDS

## Notes d'implémentation

### Côté serveur

1. Parser robuste avec gestion d'erreurs
2. Validation stricte des messages reçus
3. Timeout pour éviter les connexions zombies
4. Log de tous les échanges pour debug
5. Gestion propre de la déconnexion

### Côté client

1. Reconnexion automatique en cas de déconnexion
2. Buffer de réception pour gérer les messages partiels
3. Validation des messages serveur
4. Interface asynchrone (non bloquante)
5. Gestion des timeouts

## Compatibilité

- Le serveur DOIT refuser les connexions de clients avec un protocole incompatible
- Le client DOIT vérifier la compatibilité du protocole avant d'envoyer des commandes
- Version du protocole : Inclure dans les logs mais pas dans les messages (v1.0.0)
