#! /bin/bash
for i in $(seq "${1}" 1 "${2}");
do
	./obliviousDictionary -partyID 0 -internalIterationsNumber "${3}" -hashSize "${4}" -partiesFile Parties.txt -reportStatistics 1 -version 3Tables -tool poly -batchSize "${5}" -processId "${i}" -tableRatio "${6}" &
	echo "Running $i..."
done
