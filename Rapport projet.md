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

<br>Grégoire Chaumont: Code C<br> 
- Conception des structures `Usine` et `AVL`.
- Implémentation du parsing CSV.

<br>Kassar Fares: Script Shell & Gnuplot<br>
- Création du script `myScript.sh`.
- Intégration de Gnuplot pour la génération des graphiques.

<br>Carré Tomas: Algorithme de Fuites & Makefile<br>
- Conception de la structure de graphe pour la distribution.
- Implémentation du parcours récursif pour le calcul des pertes.
- Rédaction du `Makefile`.
<br>

* **Semaine 1 :** Analyse du sujet, mise en place du Git et du squelette du projet.
* **Semaine 2 :** Création de l'AVL et de la lecture du CSV et premiers tests.
* **Semaine 3 :** Développement du code C et du script shell.
* **Semaine 4 :** Débogage, voir si ça marche, nettoyage du code et rédaction du rapport.
<br>
Voici les principales difficultés que nous avons surmontées durant le projet :<br>
<br>
Au début on avait des problèmes de compilations mais ce n'était que des bouts de code mal écris.
Après on pensait avoir réussi mais rien de s'affichait, on avait réussi à compiler le code entier sans messages d'erreur et à générer les fichiers mais ils étaient vide, il n'y avait pas d'histogrammes et ce n'était que des fichiers textes. On s'était après rendu compte que le script shell ne trouvait pas de fichier à lire et donc qu'il n'arrivait pas à récupérer le fichier générer par le code C, il faillait donc remodifier un peu le code C.
Puis il y'a eu quelques problèmes avec les fuites qui ont facilement été réglés.
<br><br>

À la date du rendu, voici l'état du projet :
* ✅ **Fonctionnalité Histo :** Opérationnelle à 100% (Max, Src, Real). Les graphiques sont générés correctement.
* ✅ **Fonctionnalité Leaks :** Opérationnelle. Le calcul des fuites se fait bien sur l'ensemble du réseau aval.
* ❌ **Bonus :** L'histogramme empilé (3 couleurs) mentionné dans les bonus  n'a pas été implémenté par manque de temps.


En conclusion, ce projet nous a permis de comprendre l'importance des arbres (AVL) dans le traitement de grands volumes de données.
Nous avons aussi appris que l'utilisation de Shell et C en même temps est très pratique et puissante pour ce genre de tâches. 
Nous avons également appris à travailler en équipe avec Git, en gérant les différents et en se mettant d'accord.
