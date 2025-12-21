# Projet-P2-Info3

make cleanfile 
make clean && make

./script.sh c-wildwater_v0.dat histo max

#pour l'autorisation:

chmod u+x script.sh

#histogrammes :

./script.sh c-wildwater_v3.dat histo max
./script.sh c-wildwater_v3.dat histo src
./script.sh c-wildwater_v3.dat histo real
leaks : ./script.sh c-wildwater_v3.dat leaks "Plant #TF300295S"

#pour cloner le github

cd ~

mkdir test_github

cd test_github

git clone https://github.com/Fares-K7/Projet-P2-Info3.git
cd Projet-P2-Info3
