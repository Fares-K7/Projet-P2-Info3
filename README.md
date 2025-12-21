# Projet-P2-Info3
# nettoyage et compilation
make cleanfile 
make clean && make

# Pour l'autorisation

chmod u+x script.sh

# Histogrammes

./script.sh c-wildwater_v3.dat histo max

./script.sh c-wildwater_v3.dat histo src

./script.sh c-wildwater_v3.dat histo real

# Afficher les fuites d'une usine (Plant #TF300295S ici)

leaks : ./script.sh c-wildwater_v3.dat leaks "Plant #TF300295S"

# Pour cloner le github

cd ~

mkdir test_github

cd test_github

git clone https://github.com/Fares-K7/Projet-P2-Info3.git

cd Projet-P2-Info3
