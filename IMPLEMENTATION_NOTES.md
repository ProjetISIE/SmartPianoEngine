# Notes d’implémentation Smart Piano

Ce document recense les décisions de conception prises lors de l'implémentation
de Smart Piano Engine, les choix effectués face aux ambiguïtés rencontrées.

## Comportement MIDI - Accumulation des notes

Comment gérer l'accumulation des notes MIDI pour former un accord ?

La première note pressée est ajoutée à l’accord et déclenche un compte à rebours
de `chord_timeout` ms. Toute note pressée avant la fin de ce délai est ajoutée à
l’accord et remet le compteur à 0. Lorsque le délai expire, les notes accumulées
forment l’accord.

`chord_timeout` est pour l’instant une durée de 100 ms définie en `constexpr`.

## Validation des accords - Ordre et octaves

Pour l’instant, Smart Piano ne se préoccupe pas de l’ordre dans lequel sont
jouées les notes.

Les notes choisies par le moteur de jeu sont associées à leur octave, et le
joueur doit les jouer à cette octave, pas à une autre.

## Calcul du score

Il n’y a plus de notion de score pour l’instant.

## Gestion des erreurs MIDI en cours de jeu

Que faire si MIDI se déconnecte pendant une partie ?

Envoyer un message `error` au client pour lui indiquer de reconnecter son
contrôleur, mais continuer la partie en cours une fois la connexion rétablie.

Le protocole a été mis à jour pour refléter ce comportement.

## Renversements - Notation

Protocole mis à jour pour définir le nom des accords renversés.

## Gammes et modes - Completude

Pour l’instant, seules les gammes majeures et mineures naturelles sont
supportées.

## Messages non sollicités du client

Que faire si le client envoie un message inattendu (ex: `quit` pendant un
challenge) ?

Lorsque le client envoie un message inattendu, le moteur répond par un message
d’erreur `state` avec une description de l’erreur. En revanche, il reste à son
état actuel et continue d’attendre le bon message.

Un `quit` durant un challenge n’est pas inattendu (protocole mis à jour), il
réinitialise la configuration.

## Limitations actuelles

### Pas de reconnexion automatique

Si le client se déconnecte, le moteur attend une nouvelle connexion, mais ne
garde pas l'état.

Pas un problème, car pour l’instant le jeu est sans état.

### Un seul client à la fois

Le moteur ne supporte qu'un seul client connecté. Pour l’instant aucune volonté
d’en supporter plusieurs.

### Validation simple des accords

La validation des accords ne vérifie que la présence des notes, pas leur
relation harmonique.

Ceci n’est pas nécessaire, car les accords proposés doivent être uniquement des
accords majeurs ou mineurs corrects, éventuellement renversés.

### Gammes hard-codées

Pour l’instant, les définitions de gammes sont en dur dans le code.

### Pas de persistance

Aucune donnée n’est sauvegardée pour l’instant.
