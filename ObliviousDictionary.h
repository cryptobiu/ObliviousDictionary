//
// Created by moriya on 7/17/19.
//

#ifndef BENNYPROJECT_OBLIVIOUSDICTIONARY_H
#define BENNYPROJECT_OBLIVIOUSDICTIONARY_H

#include <unordered_set>
#include <unordered_map>
#include <libscapi/include/primitives/Prg.hpp>
#include <libscapi/include/comm/MPCCommunication.hpp>
#include "Poly.h"
#include "Hasher.h"

#include <chrono>
class Tools;

using namespace std::chrono;

using namespace std;

class ObliviousDictionary {
protected:
    int hashSize;
    int tableRealSize;

    PrgFromOpenSSLAES prg;
    vector<uint64_t> keys;

    Tools* tool;

public:

    ObliviousDictionary(int hashSize) : hashSize(hashSize){}
//    virtual ~ObliviousDictionary(){
//        delete tool;
//    }
};


class ObliviousDictionaryDB : public ObliviousDictionary{
protected:
    unordered_map<uint64_t, uint64_t> vals;

    vector<uint64_t> peelingVector;
    int peelingCounter;

public:

    ObliviousDictionaryDB(int hashSize) : ObliviousDictionary(hashSize){}

    virtual void createSets() = 0;

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

    virtual void createSets() = 0;

    virtual void readData(shared_ptr<ProtocolPartyData> otherParty) = 0;

    virtual void calcRealValues() = 0;

    virtual void output() = 0;
};



class ObliviousDictionaryDB2Tables : public ObliviousDictionaryDB {
private:
    uint64_t firstSeed, secondSeed;

    unordered_set<uint64_t, Hasher> first;
    unordered_set<uint64_t, Hasher> second;

    vector<uint64_t> firstEncValues;
    vector<uint64_t> secondEncValues;
public:

    ObliviousDictionaryDB2Tables(int size, string toolType);

    void createSets();

    void fillTables();

    void peeling();

    void generateExternalToolValues();

    void unpeeling();

    void checkOutput();

    bool hasLoop();

    void sendData(shared_ptr<ProtocolPartyData> otherParty);
};

class ObliviousDictionaryQuery2Tables : public ObliviousDictionaryQuery {
private:
    uint64_t firstSeed, secondSeed;

    unordered_set<uint64_t, Hasher> first;
    unordered_set<uint64_t, Hasher> second;

    vector<uint64_t> firstEncValues;
    vector<uint64_t> secondEncValues;

public:

    ObliviousDictionaryQuery2Tables(int hashSize, string toolType);

    void createSets();

    void readData(shared_ptr<ProtocolPartyData> otherParty);

    void calcRealValues();

    void output();
};


class ObliviousDictionaryDB3Tables : public ObliviousDictionaryDB {
private:
    uint64_t firstSeed, secondSeed, thirdSeed;
    unordered_set<uint64_t, Hasher> first;
    unordered_set<uint64_t, Hasher> second;
    unordered_set<uint64_t, Hasher> third;

    vector<uint64_t> firstEncValues;
    vector<uint64_t> secondEncValues;
    vector<uint64_t> thirdEncValues;
public:

    ObliviousDictionaryDB3Tables(int size, string toolType);

    void createSets();

    void fillTables();

    void peeling();

    void generateExternalToolValues();

    void calcPolynomial();

    void unpeeling();

    void checkOutput();

    bool hasLoop();

    void sendData(shared_ptr<ProtocolPartyData> otherParty);
};

class ObliviousDictionaryQuery3Tables : public ObliviousDictionaryQuery {
private:
    uint64_t firstSeed, secondSeed, thirdSeed;

    unordered_set<uint64_t, Hasher> first;
    unordered_set<uint64_t, Hasher> second;
    unordered_set<uint64_t, Hasher> third;

    vector<uint64_t> firstEncValues;
    vector<uint64_t> secondEncValues;
    vector<uint64_t> thirdEncValues;

public:

    ObliviousDictionaryQuery3Tables(int hashSize, string toolType);

    void createSets();

    void readData(shared_ptr<ProtocolPartyData> otherParty);

    void calcRealValues();

    void output();
};
#endif //BENNYPROJECT_OBLIVIOUSDICTIONARY_H
