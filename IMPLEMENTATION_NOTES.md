# Implementation Notes & Design Decisions

## Vue d'ensemble

Ce document recense les décisions de conception prises lors de l'implémentation de Smart Piano Engine, ainsi que les ambiguïtés rencontrées et les choix effectués.

## Ambiguïtés et clarifications nécessaires

### 1. Comportement MIDI - Accumulation des notes

**Ambiguïté**: Le protocole ne spécifie pas comment gérer l'accumulation des notes MIDI pour former un accord.

**Décision actuelle**: 
- Les notes sont accumulées dès réception
- Pour les accords, toutes les notes reçues sont considérées comme faisant partie de la tentative
- Pas de timeout entre les notes d'un accord

**À clarifier**:
- Faut-il un délai maximum entre les notes d'un accord?
- Comment gérer les "Note Off" (relâchement de touche)?
- Faut-il attendre un signal explicite (ex: pédale) pour valider l'accord?

**Suggestion**: Ajouter un paramètre de configuration `chord_timeout_ms` dans le message `config`.

### 2. Validation des accords - Ordre et octaves

**Ambiguïté**: Pour le mode `chord` (sans renversement), le protocole dit "dans n'importe quel ordre". Qu'en est-il des octaves?

**Décision actuelle**:
- Mode `chord`: Les notes doivent correspondre exactement (même octave)
- L'ordre n'a pas d'importance
- Seules les 3 notes attendues sont validées

**À clarifier**:
- Faut-il accepter les notes dans n'importe quelle octave?
- Faut-il accepter des doublures (ex: jouer C4 C5 E4 G4 pour C majeur)?

**Suggestion**: Ajouter dans PROTOCOL.md une section sur la validation des accords.

### 3. Score calculation

**Ambiguïté**: Le champ `score` dans le message `over` est défini comme "interprétation non définie par le protocole".

**Décision actuelle**:
- Score = durée totale en millisecondes
- Plus rapide = meilleur score

**À clarifier**:
- Faut-il un score plus sophistiqué (ex: points par note correcte)?
- Faut-il pénaliser les erreurs?
- Faut-il un système de bonus pour les parfaits?

**Suggestion**: Définir une formule de score standard dans PROTOCOL.md v1.1.0.

### 4. Gestion des erreurs MIDI en cours de jeu

**Ambiguïté**: Que faire si MIDI se déconnecte pendant une partie?

**Décision actuelle**:
- La partie continue
- Si `readNotes()` bloque indéfiniment, le moteur reste bloqué

**À clarifier**:
- Faut-il un timeout sur `readNotes()`?
- Faut-il envoyer un message `error` au client?
- Faut-il terminer la partie automatiquement?

**Suggestion**: Ajouter un timeout configurable et un message `error` avec code `midi`.

### 5. Renversements - Notation

**Ambiguïté**: Le nom de l'accord avec renversement (ex: "Do majeur 1") n'est pas formellement défini.

**Décision actuelle**:
- Format: `<accord> <numéro renversement>`
- Ex: "Do majeur 1" = premier renversement
- "Do majeur 2" = deuxième renversement

**À clarifier**:
- Notation française standard?
- Faut-il utiliser les chiffres romains (ex: "Do majeur I")?

**Suggestion**: Définir formellement dans PROTOCOL.md.

### 6. Gammes et modes - Completude

**Ambiguïté**: Seules les gammes majeures et mineures naturelles sont implémentées.

**Décision actuelle**:
- 7 gammes supportées: C, D, E, F, G, A, B
- 2 modes: majeur, mineur (naturel)

**À clarifier**:
- Faut-il supporter d'autres modes (harmonique, mélodique)?
- Faut-il supporter d'autres gammes (chromatique, pentatonique)?

**Suggestion**: Extension future dans PROTOCOL.md v2.0.0.

### 7. Messages non sollicités du client

**Ambiguïté**: Que faire si le client envoie un message inattendu (ex: `quit` pendant un challenge)?

**Décision actuelle**:
- Le moteur ignore le message et attend la réponse attendue
- Risque de désynchronisation

**À clarifier**:
- Faut-il accepter `quit` à tout moment?
- Faut-il envoyer un message `error` pour les messages inattendus?

**Suggestion**: Définir une machine à états stricte dans PROTOCOL.md avec transitions autorisées.

## Décisions de conception

### 1. Architecture en couches

**Décision**: Séparation stricte en 3 couches (Application/Domain/Infrastructure).

**Justification**:
- Testabilité maximale
- Facilite le remplacement de composants
- Suit les principes SOLID

**Trade-off**: Plus de fichiers et d'indirections, mais meilleure maintenabilité.

### 2. Interfaces vs classes abstraites

**Décision**: Utiliser des interfaces pures (`ITransport`, `IMidiInput`, `IGameMode`).

**Justification**:
- Flexibilité maximale
- Pas de couplage avec l'implémentation
- Facilite les mocks pour tests

### 3. Gestion de la mémoire

**Décision**: 
- RAII strict
- `std::unique_ptr` pour ownership
- Références pour dependencies injectées

**Justification**:
- Pas de fuites mémoire
- Propriété claire des objets
- Exception-safe

### 4. Gestion des erreurs

**Décision**: 
- Exceptions pour les erreurs fatales
- Codes de retour booléens pour les erreurs récupérables
- Messages `error` via protocole pour les erreurs client

**Justification**:
- Sépare les erreurs internes des erreurs protocolaires
- Facilite le debug
- Permet au client de réagir

### 5. Threading

**Décision**:
- Un thread détaché pour la réception MIDI
- Le reste est mono-thread

**Justification**:
- MIDI nécessite un traitement asynchrone
- Évite la complexité du multi-threading
- Synchronisation minimale (mutex sur les notes)

### 6. Génération aléatoire

**Décision**: `std::mt19937` avec `std::random_device` pour le seed.

**Justification**:
- Qualité aléatoire suffisante pour un jeu
- Portable
- Standard C++

### 7. Protocole - Format texte

**Décision**: Format `type\nkey=value\n\n` ultra-simple.

**Justification**:
- Facile à débugger (lisible)
- Facile à implémenter côté client
- Pas besoin de bibliothèque de parsing
- Extension facile

**Trade-off**: Plus verbeux que du binaire, mais négligeable pour ce cas d'usage.

## Limitations actuelles

### 1. Pas de reconnexion automatique

Si le client se déconnecte, le moteur attend une nouvelle connexion mais ne garde pas l'état.

**Solution future**: Ajouter un système de sessions avec ID.

### 2. Un seul client à la fois

Le moteur ne supporte qu'un seul client connecté.

**Solution future**: Thread pool pour gérer plusieurs clients simultanément.

### 3. Validation simpliste des accords

La validation des accords ne vérifie que la présence des notes, pas leur relation harmonique.

**Solution future**: Analyse harmonique plus sophistiquée.

### 4. Gammes hard-codées

Les définitions de gammes sont en dur dans le code.

**Solution future**: Fichier de configuration ou base de données de gammes.

### 5. Pas de persistance

Les scores ne sont pas sauvegardés.

**Solution future**: Base de données SQLite pour historique.

## Extensions recommandées

### Court terme (v1.1.0)

1. **Timeout MIDI**: Ajouter un timeout sur `readNotes()` (5-10 secondes)
2. **Validation robuste**: Améliorer la validation des messages
3. **Tests unitaires**: Ajouter des tests pour tous les composants
4. **Metrics**: Ajouter des statistiques (temps moyen, taux de réussite)

### Moyen terme (v1.2.0)

1. **Mode practice**: Mode entraînement sans limite de temps
2. **Difficultés**: Niveaux facile/moyen/difficile
3. **Progression**: Système de niveaux et déblocages
4. **Audio feedback**: Sons pour les bonnes/mauvaises réponses

### Long terme (v2.0.0)

1. **Multi-client**: Support de plusieurs clients simultanés
2. **Modes avancés**: Gammes, arpèges, rythmes
3. **IA adaptative**: Difficulté qui s'adapte au joueur
4. **Statistiques avancées**: Analyse détaillée des performances
5. **TCP transport**: En plus de UDS pour clients distants

## Conformité aux standards

### C++23

Utilisation de:
- `std::print` (C++23)
- Structured bindings
- `if constexpr`
- Concepts (potentiel futur)

### POSIX

Utilisation de:
- Unix Domain Sockets (POSIX)
- Signals (SIGINT, SIGTERM)

**Limitation**: Code non portable sous Windows sans WSL.

## Performance

### Mesures actuelles

Non mesurées précisément, mais:
- Latence MIDI: <10ms (dépend de JACK)
- Latence réseau UDS: <1ms
- CPU: Négligeable (<1%)
- Mémoire: ~2-3 MB

### Optimisations possibles

1. **Pool de messages**: Réutiliser les objets Message
2. **Buffer circulaire**: Pour les notes MIDI
3. **Zero-copy**: Pour le transport UDS

**Note**: Non prioritaire, les performances actuelles sont largement suffisantes.

## Sécurité

### Considérations

1. **Injection**: Le parsing texte est sûr (pas d'eval)
2. **Buffer overflow**: Utilisation de std::string (safe)
3. **DoS**: Pas de protection contre un client malveillant

### Améliorations futures

1. **Rate limiting**: Limiter le nombre de messages par seconde
2. **Validation stricte**: Taille maximale des messages
3. **Authentication**: Système de token pour clients autorisés

## Conclusion

L'implémentation actuelle est **fonctionnelle et conforme** aux spécifications ARCHITECTURE.md et PROTOCOL.md v1.0.0.

Les ambiguïtés identifiées nécessitent des clarifications pour les versions futures, mais ne bloquent pas l'utilisation actuelle.

L'architecture choisie permet une **extension facile** pour adresser toutes les limitations mentionnées.
