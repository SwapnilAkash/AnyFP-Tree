rm *.o
rm *.exe
rm main
rm FI.txt
g++ -c data.cpp
g++ -c anyfptree.cpp
g++ -c main.cpp
g++ -o main main.o data.o anyfptree.o
./main >> "FI.txt"