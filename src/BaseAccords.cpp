#include "BaseAccords.h"

BaseAccords::BaseAccords() {
    initialiserAccords();
}

void BaseAccords::initialiserAccords() {
    // Tonalité : Do majeur
    accords["Do Majeur"] = {
        {"C", {60, 64, 67}},     // Do (C)
        {"Dm", {62, 65, 69}},    // Ré mineur (Dm)
        {"Em", {64, 67, 71}},    // Mi mineur (Em)
        {"F", {65, 69, 72}},     // Fa (F)
        {"G", {67, 71, 74}},     // Sol (G)
        {"Am", {69, 72, 76}},    // La mineur (Am)
        {"B°", {71, 74, 77}}     // Si diminué (B°)
    };

    // Tonalité : Do Mineur
    accords["Do Mineur"] = {
        {"Cm", {60, 63, 67}},    // Do mineur (Cm)
        {"D°", {62, 65, 68}},    // Ré diminué (D°)
        {"Eb", {63, 67, 70}},    // Mi bémol (Eb)
        {"Fm", {65, 68, 72}},    // Fa mineur (Fm)
        {"Gm", {67, 70, 74}},    // Sol mineur (Gm)
        {"Ab", {68, 72, 75}},    // La bémol (Ab)
        {"Bb", {70, 74, 77}}     // Si bémol (Bb)
    };

    // Tonalité : Ré majeur
    accords["Ré Majeur"] = {
        {"D", {62, 66, 69}},     // Ré (D)
        {"Em", {64, 67, 71}},    // Mi mineur (Em)
        {"F#m", {66, 69, 73}},   // Fa# mineur (F#m)
        {"G", {67, 71, 74}},     // Sol (G)
        {"A", {69, 73, 76}},     // La (A)
        {"Bm", {71, 74, 78}},    // Si mineur (Bm)
        {"C#°", {73, 76, 79}}    // Do# diminué (C#°)
    };

    // Tonalité : Ré Mineur
    accords["Ré Mineur"] = {
        {"Dm", {62, 65, 69}},    // Ré mineur (Dm)
        {"E°", {64, 67, 70}},    // Mi diminué (E°)
        {"F", {65, 69, 72}},     // Fa (F)
        {"Gm", {67, 70, 74}},    // Sol mineur (Gm)
        {"Am", {69, 72, 76}},    // La mineur (Am)
        {"Bb", {70, 74, 77}},    // Si bémol (Bb)
        {"C", {72, 76, 79}}      // Do (C)
    };

    // Tonalité : Mi majeur
    accords["Mi Majeur"] = {
        {"E", {64, 68, 71}},     // Mi (E)
        {"F#m", {66, 69, 73}},   // Fa# mineur (F#m)
        {"G#m", {68, 71, 75}},   // Sol# mineur (G#m)
        {"A", {69, 73, 76}},     // La (A)
        {"B", {71, 75, 78}},     // Si (B)
        {"C#m", {73, 76, 80}},   // Do# mineur (C#m)
        {"D#°", {75, 78, 81}}    // Ré# diminué (D#°)
    };

    // Tonalité : Mi Mineur
    accords["Mi Mineur"] = {
        {"Em", {64, 67, 71}},    // Mi mineur (Em)
        {"F#°", {66, 69, 72}},   // Fa# diminué (F#°)
        {"G", {67, 71, 74}},     // Sol (G)
        {"Am", {69, 72, 76}},    // La mineur (Am)
        {"Bm", {71, 74, 78}},    // Si mineur (Bm)
        {"C", {72, 76, 79}},     // Do (C)
        {"D", {74, 78, 81}}      // Ré (D)
    };

    // Tonalité : Fa majeur
    accords["Fa Majeur"] = {
        {"F", {65, 69, 72}},     // Fa (F)
        {"Gm", {67, 70, 74}},    // Sol mineur (Gm)
        {"Am", {69, 72, 76}},    // La mineur (Am)
        {"Bb", {70, 74, 77}},    // Sib (Bb)
        {"C", {72, 76, 79}},     // Do (C)
        {"Dm", {74, 77, 81}},    // Ré mineur (Dm)
        {"E°", {76, 79, 82}}     // Mi diminué (E°)
    };

    // Tonalité : Fa Mineur
    accords["Fa Mineur"] = {
        {"Fm", {65, 68, 72}},    // Fa mineur (Fm)
        {"G°", {67, 70, 73}},    // Sol diminué (G°)
        {"Ab", {68, 72, 75}},    // La bémol (Ab)
        {"Bbm", {70, 73, 77}},   // Si bémol mineur (Bbm)
        {"Cm", {72, 75, 79}},    // Do mineur (Cm)
        {"Db", {73, 77, 80}},    // Ré bémol (Db)
        {"Eb", {75, 79, 82}}     // Mi bémol (Eb)
    };

    // Tonalité : Sol majeur
    accords["Sol Majeur"] = {
        {"G", {67, 71, 74}},     // Sol (G)
        {"Am", {69, 72, 76}},    // La mineur (Am)
        {"Bm", {71, 74, 78}},    // Si mineur (Bm)
        {"C", {72, 76, 79}},     // Do (C)
        {"D", {74, 78, 81}},     // Ré (D)
        {"Em", {76, 79, 83}},    // Mi mineur (Em)
        {"F#°", {78, 81, 84}}    // Fa# diminué (F#°)
    };

    // Tonalité : Sol Mineur
    accords["Sol Mineur"] = {
        {"Gm", {67, 70, 74}},    // Sol mineur (Gm)
        {"A°", {69, 72, 75}},    // La diminué (A°)
        {"Bb", {70, 74, 77}},    // Si bémol (Bb)
        {"Cm", {72, 75, 79}},    // Do mineur (Cm)
        {"Dm", {74, 77, 81}},    // Ré mineur (Dm)
        {"Eb", {75, 79, 82}},    // Mi bémol (Eb)
        {"F", {77, 81, 84}}      // Fa (F)
    };

    // Tonalité : La majeur
    accords["La Majeur"] = {
        {"A", {69, 73, 76}},     // La (A)
        {"Bm", {71, 74, 78}},    // Si mineur (Bm)
        {"C#m", {73, 76, 80}},   // Do# mineur (C#m)
        {"D", {74, 78, 81}},     // Ré (D)
        {"E", {76, 80, 83}},     // Mi (E)
        {"F#m", {78, 81, 85}},   // Fa# mineur (F#m)
        {"G#°", {80, 83, 86}}    // Sol# diminué (G#°)
    };

    // Tonalité : La Mineur
    accords["La Mineur"] = {
        {"Am", {69, 72, 76}},    // La mineur (Am)
        {"B°", {71, 74, 77}},    // Si diminué (B°)
        {"C", {72, 76, 79}},     // Do (C)
        {"Dm", {74, 77, 81}},    // Ré mineur (Dm)
        {"Em", {76, 79, 83}},    // Mi mineur (Em)
        {"F", {77, 81, 84}},     // Fa (F)
        {"G", {79, 83, 86}}      // Sol (G)
    };

    // Tonalité : Si majeur
    accords["Si Majeur"] = {
        {"B", {71, 75, 78}},     // Si (B)
        {"C#m", {73, 76, 80}},   // Do# mineur (C#m)
        {"D#m", {75, 78, 82}},   // Ré# mineur (D#m)
        {"E", {76, 80, 83}},     // Mi (E)
        {"F#", {78, 82, 85}},    // Fa# (F#)
        {"G#m", {80, 83, 87}},   // Sol# mineur (G#m)
        {"A#°", {82, 85, 88}}    // La# diminué (A#é)
    };

    // Tonalité : Si Mineur
    accords["Si Mineur"] = {
        {"Bm", {71, 74, 78}},    // Si mineur (Bm)
        {"C#°", {73, 76, 79}},   // Do# diminué (C#°)
        {"D", {74, 78, 81}},     // Ré (D)
        {"Em", {76, 79, 83}},    // Mi mineur (Em)
        {"F#m", {78, 81, 85}},   // Fa# mineur (F#m)
        {"G", {79, 83, 86}},     // Sol (G)
        {"A", {81, 85, 88}}      // La (A)
    };
}

std::vector<int> BaseAccords::obtenirAccord(const std::string& tonalite, const std::string& degre) const {
    if (accords.find(tonalite) != accords.end() && accords.at(tonalite).find(degre) != accords.at(tonalite).end()) {
        return accords.at(tonalite).at(degre);
    }
    return {}; // Retourne un accord vide si non trouvé
}
