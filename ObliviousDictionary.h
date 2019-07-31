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

    void createSets();

};


class ObliviousDictionaryDB : public ObliviousDictionary{
protected:
    unordered_map<uint64_t, uint64_t> vals;

    vector<uint64_t> peelingVector;
    int peelingCounter;

public:

    ObliviousDictionaryDB(int hashSize) : ObliviousDictionary(hashSize){}

    virtual void fillTables() = 0;

    virtual void peeling() = 0;

    virtual void unpeeling() = 0;

    virtual void generateExternalToolValues() = 0;

    virtual void checkOutput() = 0;

    virtual void sendData(shared_ptr<ProtocolPartyData> otherParty) = 0;
};

class ObliviousDictionaryQuery : public ObliviousDictionary{
public:

    ObliviousDictionaryQuery(int hashSize) : ObliviousDictionary(hashSize){}

    virtual void readData(shared_ptr<ProtocolPartyData> otherParty) = 0;

    virtual void calcRealValues() = 0;

    virtual void output() = 0;
};



class ObliviousDictionaryDB2Tables : public ObliviousDictionaryDB {

public:

    ObliviousDictionaryDB2Tables(int size);

    void fillTables();

    void peeling();

    void generateExternalToolValues();

    void calcPolynomial();

    void unpeeling();

    void checkOutput();

    bool hasLoop();

    void sendData(shared_ptr<ProtocolPartyData> otherParty);
};

class ObliviousDictionaryQuery2Tables : public ObliviousDictionaryQuery {
public:

    ObliviousDictionaryQuery2Tables(int hashSize);

    void readData(shared_ptr<ProtocolPartyData> otherParty);

    void calcRealValues();

    void output();
};
#endif //BENNYPROJECT_OBLIVIOUSDICTIONARY_H
