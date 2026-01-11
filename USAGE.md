# Smart Piano Engine - Guide d'utilisation

## Vue d'ensemble

Smart Piano Engine est le moteur de jeu pour l'apprentissage du piano. Il communique avec une interface utilisateur via Unix Domain Socket en utilisant un protocole texte simple.

## Architecture

Voir [ARCHITECTURE.md](ARCHITECTURE.md) pour les détails complets de l'architecture.

- **Couche Application**: Point d'entrée (`main_new.cpp`)
- **Couche Domaine**: Logique de jeu (GameEngine, modes de jeu, logique musicale)
- **Couche Infrastructure**: Transport UDS, entrée MIDI, logs

## Protocole de communication

Voir [PROTOCOL.md](PROTOCOL.md) pour la spécification complète du protocole.

Format des messages:
```
type_message
cle1=valeur1
cle2=valeur2

```

## Compilation

### Avec Nix (recommandé)

```bash
nix build
# ou pour cross-compilation ARM64:
nix build .#cross
```

### Avec CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Exécution

### Démarrer le moteur

```bash
./engine
# ou depuis le build Nix:
./result/bin/engine
```

Le moteur démarre et écoute sur `/tmp/smartpiano.sock`.

### Tester avec un client simple

Exemple avec `socat`:

```bash
# Dans un autre terminal
socat - UNIX-CONNECT:/tmp/smartpiano.sock
```

Puis envoyer des commandes:

```
config
game=note
scale=c
mode=maj

```

Le moteur répond avec:
```
ack
status=ok

```

Puis envoyer `ready` pour commencer:
```
ready

```

Le moteur envoie un challenge:
```
note
note=c4
id=1

```

Jouer la note sur le clavier MIDI, le moteur répond:
```
result
id=1
correct=c4
duration=1234

```

## Modes de jeu disponibles

1. **note** : Reconnaissance de notes individuelles
   - Le joueur doit jouer la note affichée

2. **chord** : Accords simples (sans renversement)
   - Le joueur doit jouer les 3 notes de l'accord dans n'importe quel ordre

3. **inversed** : Accords avec renversements
   - Le joueur doit jouer l'accord dans le renversement spécifié

## Configuration

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

Si le socket `/tmp/smartpiano.sock` existe déjà:

```bash
rm /tmp/smartpiano.sock
```

### MIDI non détecté

Vérifier que JACK est démarré et que le périphérique MIDI est connecté:

```bash
aconnect -l
```

### Erreur de compilation

S'assurer que les dépendances sont installées:
- RTMidi
- ALSA
- C++23 compiler (GCC 13+ ou Clang 16+)

## Tests

Les tests unitaires peuvent être exécutés avec:

```bash
cd build
ctest
```

## Développement

### Ajouter un nouveau mode de jeu

1. Créer une classe héritant de `IGameMode`
2. Implémenter `start()`, `play()`, `stop()`
3. Enregistrer dans `GameEngine::createGameMode()`

### Ajouter un nouveau transport

1. Créer une classe héritant de `ITransport`
2. Implémenter les méthodes de communication
3. Injecter dans le main

## Licence

Voir [LICENSE](LICENSE)

## Auteurs

Voir [README.md](README.md)
