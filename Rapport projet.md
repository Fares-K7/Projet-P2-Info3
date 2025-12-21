2025-2026 – Pré-ING2
CY Tech
MEF-D

Gregoire Chaumont
Kassar Fares
Carré Tomas

Ce projet, intitulé "C-WildWater", vise à développer une solution logicielle performante pour analyser un réseau de distribution d'eau fictif mais réaliste. 
L'objectif était de traiter un fichier de données volumineux  pour extraire des statistiques sur les usines de traitement et calculer les pertes d'eau.

L'application combine un script Shell pour l'interface utilisateur et l'automatisation, et un programme en langage C pour le traitement intensif des données et la gestion de la mémoire.

Nous avons séparé le travail de manière simple et efficace.

Grégoire Chaumont: Code C 

<br>- Conception des structures `Usine` et `AVL`.<br>
<br>- Implémentation du parsing CSV.<br>

<br>Kassar Fares: Script Shell & Gnuplot<br>

<br>- Création du script `myScript.sh` (gestion des arguments, aide).<br>

<br>- Intégration de Gnuplot pour la génération des graphiques.<br>

<br>- Compilation automatique et gestion des erreurs. |
| **Étudiant C** | **Algorithme de Fuites (Leaks) & Makefile**<br>

<br>- Conception de la structure de graphe pour la distribution.<br>

<br>- Implémentation du parcours récursif pour le calcul des pertes.<br>

<br>- Rédaction du `Makefile` et documentation. |

2.2 Planning de réalisation 

* **Semaine 1 :** Analyse du sujet, mise en place du Git et du squelette du projet (`Makefile`, `main.c`, `myScript.sh`).
* **Semaine 2 :** Implémentation de l'AVL et de la lecture du CSV. Premiers tests sur le module `histo`.
* **Semaine 3 :** Développement du module `leaks` (gestion des listes d'adjacence). Script Gnuplot.
* **Semaine 4 :** Débogage, gestion des fuites mémoires (Valgrind), nettoyage du code et rédaction du rapport.

---

## 3. Choix Techniques

* **Structure de données (Histo) :** Nous avons choisi un **AVL** (Arbre Binaire de Recherche Équilibré) pour stocker les usines. Cela permet une insertion et une recherche en , indispensable vu le nombre d'entrées.
* 
**Lecture du CSV :** Pour limiter l'empreinte mémoire, nous ne chargeons pas tout le fichier. Nous lisons ligne par ligne (`fgets`) et mettons à jour l'AVL à la volée.


* **Tri des données :** Le tri inverse demandé pour Gnuplot est effectué directement lors du parcours de l'AVL (Droite -> Racine -> Gauche) lors de l'écriture du fichier de sortie.

---

## 4. Problèmes Rencontrés et Solutions

Voici les principales difficultés que nous avons surmontées durant le projet :

### 4.1 La structure du fichier CSV

* **Problème :** Les lignes du fichier CSV sont mélangées. Une usine peut être définie à la ligne 100, mais recevoir de l'eau d'une source définie à la ligne 5000.


* **Solution :** Nous avons utilisé une approche "Lazy creation". Si on rencontre un lien "Source -> Usine" avant la définition de l'usine, on crée l'usine dans l'AVL avec des valeurs par défaut. Ses infos (capacité) sont mises à jour plus tard quand on rencontre sa ligne de définition.

### 4.2 Gestion des caractères de fin de ligne

* **Problème :** Lors de la lecture avec `fgets` et le découpage avec `strtok`, les identifiants en fin de ligne récupéraient le caractère invisible `\n` ou `\r`, ce qui faussait les comparaisons de chaînes (`strcmp`).
* **Solution :** Création d'une fonction utilitaire `trimNewline()` qui nettoie systématiquement les buffers de lecture.

### 4.3 Optimisation Gnuplot

* **Problème :** Gnuplot ne gérait pas correctement les barres horizontales avec nos premières données.
* **Solution :** Ajustement du script Gnuplot pour utiliser le style `boxxyerror` ou `histograms` et rotation des étiquettes (labels) pour la lisibilité.

---

5. Limitations Fonctionnelles 

À la date du rendu, voici l'état du projet :

* ✅ **Fonctionnalité Histo :** Opérationnelle à 100% (Max, Src, Real). Les graphiques sont générés correctement.
* ✅ **Fonctionnalité Leaks :** Opérationnelle. Le calcul des fuites se fait bien sur l'ensemble du réseau aval.
* ⚠️ **Performance Leaks :** Sur des très grands réseaux avals, le temps de traitement peut prendre quelques secondes du fait de la recherche des enfants dans le fichier (si non optimisé par un second AVL comme suggéré).
* ❌ **Bonus :** L'histogramme empilé (3 couleurs) mentionné dans les bonus  n'a pas été implémenté par manque de temps.



---

6. Exemples de résultats 

*(Insérez ici les images générées par votre programme)*

**Figure 1 :** Histogramme des capacités (Mode `histo max`)
*(Insérer image ici)*

**Figure 2 :** Extrait du terminal lors d'un calcul de fuite (`leaks`)

```text
$ ./myScript.sh leaks "Facility complex #RH400057F"
Compilation... OK.
Calcul des fuites pour l'usine : Facility complex #RH400057F
[...]
Resultat : 45.2 M.m3 de pertes.
Durée : 2 secondes.

```

---

## 7. Conclusion

Ce projet nous a permis de comprendre l'importance des structures de données adaptées (AVL) pour traiter de grands volumes de données. La séparation Shell/C est une architecture puissante qui allie flexibilité et performance. Nous avons également appris à travailler en équipe avec Git, en gérant les conflits et en synchronisant nos avancées.
