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
#include <queue>

class Tools;

using namespace std::chrono;

using namespace std;

class ObliviousDictionary {
protected:
    int hashSize;
    int tableRealSize;

    PrgFromOpenSSLAES* prg;
    vector<uint64_t> keys;

    Tools* tool;

public:

    ObliviousDictionary(int hashSize) : hashSize(hashSize){
        prg = new PrgFromOpenSSLAES(hashSize*100);
    }
//    virtual ~ObliviousDictionary(){
//        delete tool;
//    }
};


class ObliviousDictionaryDB : public ObliviousDictionary{
protected:
    unordered_map<uint64_t, uint64_t> vals;

    vector<uint64_t> peelingVector;
    int peelingCounter;
    int reportStatistics=0;
    ofstream statisticsFile;
    ofstream groupedStatisticsFile;
    int processId;
    int batchSize;
    float tableRatio;
    int hashOriginalSize;
    vector<int> circleVec;


public:

    ObliviousDictionaryDB(int hashSize) : ObliviousDictionary(hashSize) {};

    virtual ~ObliviousDictionaryDB() {
        if (reportStatistics == 1) {


            statisticsFile.close();
            groupedStatisticsFile.close();

        }
    };
    virtual void createSets() = 0;

    virtual void fillTables() = 0;

    virtual void peeling() = 0;

    virtual void unpeeling() = 0;

    virtual void generateExternalToolValues() = 0;

    virtual void checkOutput() = 0;

    virtual void sendData(shared_ptr<ProtocolPartyData> otherParty) = 0;

    void setReportStatstics(int flag){
        cout << "in report statistics" << endl;
        reportStatistics = flag;
        if (reportStatistics == 1) {

            cout<<"statistics file created"<<endl;
            statisticsFile.open("results/ungroup-" + to_string(processId) + "-m-" + to_string(hashOriginalSize) +
            "-batch-"+ to_string(batchSize)+ "-ratio" + to_string(tableRatio)+  "-.csv");
            groupedStatisticsFile.open("results/group-" + to_string(processId) + "-m-" + to_string(hashOriginalSize) + "-runs-" +
                                       "-batch-"+ to_string(batchSize)+ "-ratio-" + to_string(tableRatio) + "-.csv");
            groupedStatisticsFile<<"2core > 0.5logm, 1logm, 2logm, 3logm, 4logm, 5logm\n";
            // groupedStatisticsFile<<"greater than 5Log , greater than 3 Log , greater than 2 Log, greater than Log, greater than 0.5 Log, \n";

            circleVec.resize(batchSize);
        }};


    virtual void updateIteration(int iteration)=0;
    virtual void init();
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

    virtual void init();

    void updateIteration(int iteration){}
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

    void peelSecondSet(int position, uint64_t key, bool deleteFromThree, vector<uint64_t> * keysToDeleteFromThree, uint64_t originalKey);
    void peelThirdSet(int position, vector<uint64_t> * keysToDeleteFromThree);
public:

    ObliviousDictionaryDB3Tables(int size, string toolType,int batchSize, int processId, float tableRatio);

    void createSets();

    void fillTables();

    void peeling();

    void peeling2();

    void generateExternalToolValues();

    void calcPolynomial();

    void unpeeling();

    void checkOutput();

    bool hasLoop();

    void sendData(shared_ptr<ProtocolPartyData> otherParty);


    void handleQueue(queue<int> &queueMain, unordered_set<uint64_t, Hasher> &main,
                queue<int> &queueOther1, unordered_set<uint64_t, Hasher> &other1,
                queue<int> &queueOther2,unordered_set<uint64_t, Hasher> &other2);

    void updateIteration(int iteration);

    virtual void init();
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
