//
// Created by moriya on 7/17/19.
//

#ifndef BENNYPROJECT_OBLIVIOUSDICTIONARY_H
#define BENNYPROJECT_OBLIVIOUSDICTIONARY_H
#include <unordered_set>
#include <unordered_map>
#include <libscapi/include/primitives/Prg.hpp>
#include <libscapi/include/comm/MPCCommunication.hpp>
#include "xxhash.h"
#include "Poly.h"
#include <chrono>
#include <math.h>       /* log2 */

using namespace std::chrono;

using namespace std;

class Hasher {
private:

    unsigned long long seed;

public:

    Hasher() {
       seed = 0;
    }

    Hasher(unsigned long long seed) : seed(seed){    }

    Hasher(const Hasher & hash){
        seed = hash.seed;
    }

    size_t operator() (uint64_t const key) const
    {
        unsigned long long const hash = XXH64(&key, 8, seed);
        return hash;
    }

};

class ObliviousDictionary {
protected:
    int hashSize;
    int tableRealSize;

    uint64_t firstSeed, secondSeed;

    PrgFromOpenSSLAES prg;
    vector<uint64_t> keys;

    unordered_set<uint64_t, Hasher> first;
    unordered_set<uint64_t, Hasher> second;

    vector<uint64_t> firstEncValues;
    vector<uint64_t> secondEncValues;

    vector<ZpMersenneLongElement> polynomial;
    int polySize;

public:

    ObliviousDictionary(int hashSize) : hashSize(hashSize){}

    uint64_t getPolynomialValue(uint64_t key);

    void createSets();

};


class ObliviousDictionaryDB : public ObliviousDictionary {

private:
    unordered_map<uint64_t, uint64_t> vals;

    vector<uint64_t> peelingVector;
    int peelingCounter;

public:

    ObliviousDictionaryDB(int size);

    void fillTables();

    void peeling();

    void calcPolynomial();

    void unpeeling();

    void checkOutput();

    bool hasLoop();

    void sendData(shared_ptr<ProtocolPartyData> otherParty);
};

class ObliviousDictionaryQuery : public ObliviousDictionary {
public:

    ObliviousDictionaryQuery(int hashSize);

    void readData(shared_ptr<ProtocolPartyData> otherParty);

    void calcRealValues();
};
#endif //BENNYPROJECT_OBLIVIOUSDICTIONARY_H
