# Smart Piano UDS Protocol Specification

> Version : 1.0.0

Smart Piano utilise un protocole texte simple sur Unix Domain Socket (UDS) pour
la communication entre le moteur de jeu (serveur) et l'interface utilisateur
(client).

## Transport

- **Type** : Unix Domain Socket (UDS)
- **Chemin par défaut** : `/tmp/smartpiano.sock`
- **Mode** : SOCK_STREAM (orienté connexion)
- **Encodage** : UTF-8

## Format des messages

Chaque message débute par son type, comporte d’éventuelles lignes de texte au
format `clé=valeur`, et se termine par une ligne vide (`\n\n`).

```
type du message
clé1=valeur1
clé2=valeur2
# ligne vide ici <--
```

- Le type du message peut contenir des espaces, mais pas de `\n` (retour ligne)
- Le type du message se termine par `\n` (LF uniquement)
- Les éventuelles lignes subséquentes contiennent une (seule) paire `clé=valeur`
- Clé et valeur ne peuvent pas contenir `=` ou `\n`
- La valeur peut contenir des espaces, mais pas de `\n` (nouvelle ligne)
- Les lignes se terminent par `\n` (LF uniquement)
- Le message se termine par `\n` après une ligne déjà finie, résultant en `\n\n`
- Pas d'espaces autour du `=` séparant clé et valeur
- Encodage UTF-8, support des caractères accentués, spéciaux…

**Exemple** :

```
config
game=note
scale=c
mode=maj
# ligne vide indiquant la fin du message <--
```

Ci-dessous, la fin d’un bloc de code est considérée comme une ligne vide.

## Types de messages

### 1. Client (interface utilisateur) → Serveur (moteur de jeu)

#### 1.1 Configuration de jeu `config`

Commence une nouvelle session de jeu en la configurant.

```
config
game=<TYPE>
scale=<GAMME>
mode=<MODE>
```

**Champs** :

- `game` : Type de jeu
  - `note` : Jeu de reconnaissance de notes
  - `chord` : Jeu d'accords sans renversement
  - `chord inversed` : Jeu d'accords avec renversements
- `scale` : Éventuelle gamme musicale voulue (sinon le serveur choisit)
  - Valeurs : `c`, `d`, `e`, `f`, `g`, `a`, `b`
  - Cela équivaut en français à "Do", "Ré", "Mi", "Fa", "Sol", "La", "Si"
- `mode` : Éventuel mode de la gamme voulu (sinon le serveur choisit)
  - Valeurs : `maj` pour Majeur, `min` pour mineur

**Exemple** :

```
config
game=note
scale=c
mode=maj
```

#### 1.2 Prêt pour le challenge suivant `ready`

Indique que le client est prêt à recevoir le prochain challenge, à poursuivre le
jeu en cours.

```
ready
```

Aucun **champ**, seulement le type valant `ready`, suivi d’une fin de message.

#### 1.3 Abandon de partie `stop`

Demande l'arrêt de la session en cours.

```
stop
```

Aucun **champ**, uniquement le type valant `stop`, suivi d’une fin de message.

### 2. Serveur (moteur de jeu) → Client (interface utilisateur)

#### 2.1 Accusé de réception (de configuration) `ack`

Confirme la réception de la configuration.

```
ack
status=ok
```

**Exemple en cas d'erreur** :

```
ack
status=error
code=<CODE>
message=<MESSAGE>
```

**Champs** :

- `status` : `ok` ou `error`
- `code` : Code d'erreur éventuel
  - `game` : Type de jeu invalide
  - `scale` : Gamme invalide
  - `mode` : Mode invalide
  - `midi` : Périphérique MIDI non disponible
- `message` : Éventuel message d'erreur descriptif

#### 2.2 Challenge note `note`

Demande au joueur de jouer une note spécifique.

```
note
note=<NOTE>
id=<ID>
```

**Champs** :

- `note` : Note à jouer (ex : `c4`, `d#5`, `gb3`)
- `id` : Identifiant unique du challenge (entier positif)

**Exemple** :

```
note
note=c4
id=1
```

#### 2.3 Challenge accord `chord`

Demande au joueur de jouer un accord spécifique.

```
chord
name=<NOM>
notes=<NOTES>
inversion=<INVERSION>
id=<ID>
```

**Champs** :

- `name` : Nom (affiché) de l'accord (ex : `"Do majeur"`, `"Re mineur"`)
- `notes` : Notes de l'accord séparées par des espaces (ex : `"c4 e4 g4"`)
- `inversion` : Éventuel numéro de renversement
  - `0` : Position fondamentale, pas de renversement
  - `1` : Premier renversement, tierce à la basse, tonique à l’octave
  - `2` : Deuxième renversement, quinte à la basse, tonique et tierce à l’octave
- `id` : Identifiant unique du challenge

**Exemple (position fondamentale)** :

```
chord
name=Do majeur
notes=c4 e4 g4
inversion=0
id=5
```

**Exemple (premier renversement)** :

```
chord
name=Do majeur 1
notes=e4 g4 c5
inversion=1
id=6
```

#### 2.4 Résultat du challenge `result`

Informe le joueur du résultat de sa tentative.

```
result
id=<ID>
result=<RESULT>
notes=<NOTES>
```

**Champs** :

- `id` : Identifiant du challenge correspondant
- `result` : Résultat
  - `correct` : Réponse correcte
  - `incorrect` : Réponse incorrecte
  - `partial` : Réponse partiellement correcte (optionnel pour accords)
- `notes` : Notes jouées par l'utilisateur (séparées par des espaces)

**Exemple (correct)** :

```
result
id=1
result=correct
notes=c4
```

**Exemple (incorrect)** :

```
result
id=2
result=incorrect
notes=d4
```

#### 2.5 Fin de partie `over`

Indique la fin de la session de jeu avec le score.

```
over
score=<SCORE>
duration=<DURATION>
total=<TOTAL>
correct=<CORRECT>
```

**Champs** :

- `score` : Score final (temps total en secondes)
- `duration` : Durée totale de la partie en secondes
- `total` : Nombre total de challenges
- `correct` : Nombre de challenges réussis

**Exemple** :

```
over
score=45
duration=45
total=10
correct=10
```

#### 2.6 Erreur `error`

Signale une erreur générale.

```
error
code=<CODE>
message=<MESSAGE>
```

**Champs** :

- `code` : Code d'erreur
  - `protocol` : Erreur de protocole (message mal formé)
  - `state` : État invalide (ex : ready sans config)
  - `midi` : Erreur MIDI
  - `internal` : Erreur interne du serveur
- `message` : Message d'erreur descriptif

**Exemple** :

```
error
code=protocol
message=Message mal formé: champ 'id' manquant
```

## Diagramme de séquence

### Session complète

```
Client                        Serveur
  |                              |
  |--- game_config ------------->|
  |                              |
  |<-- config_ack (ok) ----------|
  |                              |
  |--- ready ------------------->|
  |                              |
  |<-- challenge_note -----------|
  |                              |
  | [Joueur joue une note]       |
  |                              |
  |<-- challenge_result ---------|
  |                              |
  |--- ready ------------------->|
  |                              |
  |<-- challenge_note -----------|
  |                              |
  | [Répéter pour N challenges]  |
  |                              |
  |<-- game_over ----------------|
  |                              |
  |--- quit -------------------->|
  |                              |
```

### Gestion d'erreur

```
Client                  Serveur
  |                        |
  |--- game_config ------->|
  |    (invalide)          |
  |                        |
  |<-- config_ack (error) -|
  |                        |
  |--- game_config ------->|
  |   (valide)             |
  |                        |
  |<-- config_ack (ok) ----|
  |                        |
```

## États de la connexion

1. **DISCONNECTED** : Aucune connexion établie
2. **CONNECTED** : Client connecté, mais pas de configuration
3. **CONFIGURED** : Configuration reçue et validée
4. **PLAYING** : Session de jeu en cours
5. **WAITING** : En attente du prochain challenge (après un `ready`)
6. **FINISHED** : Partie terminée

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

1. Tous les champs obligatoires (non "éventuels") doivent être présents
2. Les valeurs des champs doivent être dans les ensembles autorisés
3. Le message doit se terminer par `\n\n`
4. Timeout de réception après 1 seconde

Le serveur doit aussi s’assurer que le challenge `id` est unique et croissant.

## Gestion des erreurs de protocole

En cas d'erreur de protocole (message mal formé, champ manquant, valeur
invalide), le serveur doit :

1. Journaliser (log) l'erreur
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

- Le serveur DOIT refuser les connexions de clients avec un protocole
  incompatible
- Le client DOIT vérifier la compatibilité du protocole avant d'envoyer des
  commandes
- Version du protocole : Inclure dans les logs mais pas dans les messages
  (v1.0.0)
