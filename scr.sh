#RUN ME: bash scr.sh
#
#!/bin/bash
POOP="CS 1541 Testing Script"
echo $POOP
read -p "/afs/cs.pitt.edu/courses/1541/" DIR
echo "Input: $DIR"
gcc -o CPU+cache CPU+cache.c
#Experiment 1 - Cache and Block sizes
echo $'Experiment 1 - Cache and Block Sizes\n' > results.txt
echo $'1\n4\n4\n1\n4\n4\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_1"
echo $'1\n4\n16\n1\n4\n16\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_2"
echo $'1\n4\n64\n1\n4\n64\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_3"
echo $'1\n4\n256\n1\n4\n256\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_4"
echo $'16\n4\n4\n16\n4\n4\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_5"
echo $'16\n4\n16\n16\n4\n16\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_6"
echo $'16\n4\n64\n16\n4\n64\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_7"
echo $'16\n4\n256\n16\n4\n256\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_8"
echo $'128\n4\n4\n128\n4\n4\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_9"
echo $'128\n4\n16\n128\n4\n16\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_10"
echo $'128\n4\n64\n128\n4\n64\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1_11"
echo $'128\n4\n256\n128\n4\n256\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX1"
#Experiment 3 - Associativity
echo $'\nExperiment 3 - Associativity\n' >> results.txt
echo $'32\n1\n32\n32\n1\n32\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX3_1"
echo $'32\n4\n32\n32\n4\n32\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX3_2"
echo $'32\n8\n32\n32\n8\n32\n20\n' > cache_config.txt
./CPU+cache /afs/cs.pitt.edu/courses/1541/$DIR 0 >> results.txt
echo "DONE: EX3"
echo "Script Complete!"
