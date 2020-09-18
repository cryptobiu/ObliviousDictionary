#! /bin/bash
for i in $(seq "${1}" 1 "${2}");
do
	./obliviousDictionary -partyID 0 -internalIterationsNumber "${3}" -hashSize "${4}" -partiesFile Parties.txt -reportStatistics 1 -version 3Tables -tool poly -batchSize "${5}" -processId "${i}" -tableRatio "${6}" &
	echo "Running $i..."
done


./obliviousDictionary -partyID 0 -internalIterationsNumber 1000 -hashSize 8000 -partiesFile Parties.txt -reportStatistics 1 -version 3Tables -tool poly -batchSize 10000 -processId 169163864935223809488198610721463478119 -tableRatio 1.31
