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

using namespace std::chrono;

using namespace std;

class Hasher {
private:

    unsigned long long seed;

public:

    Hasher() {
        cout<<"default ctor"<<endl;
        seed = 0;

    }
    Hasher(unsigned long long seed) : seed(seed){


    }

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

public:

    ObliviousDictionary(int hashSize) : hashSize(hashSize){}

    uint64_t getPolynomialValue(uint64_t key);

    void createSets(){
        first = unordered_set<uint64_t, Hasher>(hashSize, Hasher(firstSeed));
        second = unordered_set<uint64_t, Hasher>(hashSize, Hasher(secondSeed));

        tableRealSize = first.bucket_count();
        cout<<"tableRealSize = "<<tableRealSize<<endl;

        while(tableRealSize/1.2 < hashSize){
            first = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(firstSeed));
            second = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(secondSeed));

            tableRealSize = first.bucket_count();
            cout<<"tableRealSize = "<<tableRealSize<<endl;
        }
        hashSize = tableRealSize/1.2;
        cout<<"new hashSize = "<<hashSize<<endl;
    }

};


class ObliviousDictionaryDB : public ObliviousDictionary {

private:
    unordered_map<uint64_t, uint64_t> vals;

    vector<uint64_t> peelingVector;
    int peelingCounter;


public:

    ObliviousDictionaryDB(int size) : ObliviousDictionary(size) {

        auto key = prg.generateKey(128);
        prg.setKey(key);

        firstSeed = prg.getRandom64();
        secondSeed = prg.getRandom64();
        createSets();

        cout<<"after create sets"<<endl;
        cout<<"tableRealSize = "<<tableRealSize<<endl;
        cout<<"hashSize = "<<hashSize<<endl;

        firstEncValues.resize(tableRealSize, 0);
        secondEncValues.resize(tableRealSize, 0);


        keys.resize(hashSize);
        vals.reserve(hashSize);

        for (int i=0; i<hashSize; i++){
            keys[i] = prg.getRandom64() >> 3;
            vals.insert({keys[i],prg.getRandom64()>>3});
        }

        int numKeysToCheck = 10;
        cout<<"keys to check with the other party"<<endl;
        for (int i=0; i<numKeysToCheck; i++){
            cout<<"key = "<<keys[i]<<" val = "<<vals[keys[i]]<<endl;
        }

    }

    void fillTables(){

        for (int i=0; i<hashSize; i++){

//            cout<<"key is "<<keys[i]<<endl;
            auto pair = first.insert(keys[i]);
//            first.insert(keys[i]);
            second.insert(keys[i]);

            if (pair.second == false){
                cout<<"key = "<<keys[i]<<" i = "<<i<<endl;
            }
        }


        cout<<"first set contains "<<first.size()<<endl;
        cout<<"second set contains "<<second.size()<<endl;

    }

    void peeling(){

        peelingVector.resize(hashSize);
        peelingCounter = 0;

        auto t1 = high_resolution_clock::now();
        //Goes on the first hash
        for (int position = 0; position<tableRealSize; position++){
            if (first.bucket_size(position) == 1){
                //Delete the vertex from the graph
                auto key = *first.begin(position);
//                cout<<"remove key "<<key<<endl;
                peelingVector[peelingCounter++] = key;
                first.erase(key);

                //Update the second vertex on the edge
                second.erase(key);
            }
        }
        auto t2 = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(t2-t1).count();

        cout << "time in milliseconds for first loop: " << duration << endl;

        t1 = high_resolution_clock::now();
        //goes on the second hash
        for (int position = 0; position<tableRealSize; position++){
            if (second.bucket_size(position) == 1){
                //Delete the vertex from the graph
                auto key = *second.begin(position);
//                peelingVector.push_back(key);
//                cout<<"remove key "<<key<<endl;
//                second.erase(key);

                int secondBucket = 0;

                while(secondBucket <= position) {

                    peelingVector[peelingCounter++] = key;
//                    cout<<"remove key "<<key<<endl;
                    second.erase(key);

//                    if (secondBucket>0) cout<<"loop in peeling"<<endl;
                    //Update the second vertex on the edge
                    int bucket = first.bucket(key);
                    first.erase(key);
                    if (first.bucket_size(bucket) == 1) {
                        key = *first.begin(bucket);
//                        cout<<"remove key from first "<<key<<endl;
                        peelingVector[peelingCounter++] = key;
                        first.erase(key);

                        //Update the second vertex on the edge
                        secondBucket = second.bucket(key);
                        second.erase(key);
                        if (second.bucket_size(secondBucket) == 1) {
                            key = *second.begin(secondBucket);
//                            peelingVector.push_back(key);
//                            cout<<"remove key "<<key<<endl;
//                            second.erase(key);
                        } else {
                            secondBucket = position + 1;
                        }
                    } else {
                        secondBucket = position + 1;
                    }

                }
            }
        }
        t2 = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(t2-t1).count();

        cout << "time in milliseconds for second loop: " << duration << endl;

        if (hasLoop()){
            cout << "remain loops!!!" << endl;
        }

        cout<<"peelingCounter = "<<peelingCounter<<endl;

    }

    void calcPolynomial(){

        vector<uint64_t> edgesForPolynomial;
        vector<uint64_t> valuesForPolynomial;

        //Assign random values to all vertex in the circles.
        for (int i=0; i<tableRealSize; i++){
            if (first.bucket_size(i) > 1){
                firstEncValues[i] =  prg.getRandom64() >> 3;
//                cout<<"bucket "<<i<<" got random value in first"<<endl;
            }
            if (second.bucket_size(i) > 1) {
                secondEncValues[i] = prg.getRandom64()  >> 3;
//                cout<<"bucket "<<i<<" got random value in second"<<endl;
            }
        }

        //Get all the edges that are in the graph's circles and calc the polynomial values that should be for them.
        for (int i=0; i<tableRealSize; i++){
            if (first.bucket_size(i) > 1){
                for (auto key = first.begin(i); key!= first.end(i); ++key){
                    edgesForPolynomial.push_back(*key);
                    valuesForPolynomial.push_back(firstEncValues[i] ^ secondEncValues[second.bucket(*key)] ^ vals[*key]);

                }
            }
        }


        //TODO interpolate!!!
        cout<<"poly size =  "<<edgesForPolynomial.size()<<endl;
        polynomial.resize(edgesForPolynomial.size());
        Poly::interpolateMersenne(polynomial, (ZpMersenneLongElement*)edgesForPolynomial.data(), (ZpMersenneLongElement*)valuesForPolynomial.data(), edgesForPolynomial.size());


    }


    void unpeeling(){
        cout<<"in unpeeling"<<endl;
        uint64_t firstPosition, secondPosition, poliVal, key;
        while (peelingCounter > 0){
//            cout<<"key = "<<key<<endl;
            key = peelingVector[--peelingCounter];
            firstPosition = first.bucket(key);
            secondPosition = second.bucket(key);

            poliVal = getPolynomialValue(key);
            if (firstEncValues[firstPosition] == 0 && secondEncValues[secondPosition] == 0){
                firstEncValues[firstPosition] = prg.getRandom64()  >> 3;
//                cout<<"set RANDOM value to first in bucket "<<firstPosition<<endl;
            }
            if (firstEncValues[firstPosition] == 0){
                firstEncValues[firstPosition] = vals[key] ^ secondEncValues[secondPosition] ^ poliVal;
//                cout<<"set value to first in bucket "<<firstPosition<<endl;
            } else {
                secondEncValues[secondPosition] = vals[key] ^ firstEncValues[firstPosition] ^ poliVal;
//                cout<<"set value to second in bucket "<<secondPosition<<endl;
            }
        }
        cout<<"peelingCounter = "<<peelingCounter<<endl;
    }

    void checkOutput(){

        uint64_t firstPosition, secondPosition, key, val, poliVal;

        for (int i=0; i<hashSize; i++){
            key = keys[i];
            val = vals[key];

            poliVal = getPolynomialValue(key);
            firstPosition = first.bucket(key);
            secondPosition = second.bucket(key);

            if ((firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ poliVal) == val) {
                if (i%100000 == 0)
                    cout<<"good value!!! val = "<<val<<endl;
            } else {//if (!hasLoop()){
                cout<<"invalid value :( val = "<<val<<" wrong val = "<<(firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ poliVal)<<endl;
                cout<<"firstEncValues["<<firstPosition<<"] = "<<firstEncValues[firstPosition]<<endl;
                cout<<"secondEncValues["<<secondPosition<<"] = "<<secondEncValues[secondPosition]<<endl;
                cout<<"poliVal = "<<poliVal<<endl;
            }

        }
    }

    bool hasLoop(){
        for (int position = 0; position<tableRealSize; position++) {
            if (first.bucket_size(position) > 1) {
                return true;
            }
        }
        return false;
    }


    void sendData(shared_ptr<ProtocolPartyData> otherParty){
//TODO should be deleted!!
        otherParty->getChannel()->write((byte*)&firstSeed, 8);
        otherParty->getChannel()->write((byte*)&secondSeed, 8);
        int polySize = polynomial.size();
        cout<<"firstSeed = "<<firstSeed<<endl;
        cout<<"secondSeed = "<<secondSeed<<endl;
        cout<<"polySize = "<<polySize<<endl;
        otherParty->getChannel()->write((byte*)&polySize, 4);
        otherParty->getChannel()->write((byte*)keys.data(), keys.size()*8);
//TODO until here

        otherParty->getChannel()->write((byte*)firstEncValues.data(), firstEncValues.size()*8);
        otherParty->getChannel()->write((byte*)secondEncValues.data(), secondEncValues.size()*8);
        otherParty->getChannel()->write((byte*)polynomial.data(), polynomial.size()*8);
    }
};

class ObliviousDictionaryQuery : public ObliviousDictionary {
public:

    ObliviousDictionaryQuery(int hashSize) : ObliviousDictionary(hashSize){

        auto key = prg.generateKey(128);
        prg.setKey(key);
    }

    void readData(shared_ptr<ProtocolPartyData> otherParty){

//TODO should be deleted!!
        otherParty->getChannel()->read((byte*)&firstSeed, 8);
        otherParty->getChannel()->read((byte*)&secondSeed, 8);

        createSets();


        int size;
        otherParty->getChannel()->read((byte*)&size, 4);
        cout<<"firstSeed = "<<firstSeed<<endl;
        cout<<"secondSeed = "<<secondSeed<<endl;
        cout<<"polySize = "<<size<<endl;
        vector<uint64_t> tmpKeys(hashSize);
        otherParty->getChannel()->read((byte*)tmpKeys.data(), tmpKeys.size()*8);

        int numKeysToCheck = 10;
        keys.resize(numKeysToCheck);
        for (int i=0; i<numKeysToCheck; i++){
            keys[i] = tmpKeys[i];//prg.getRandom32() % hashSize;
        }

        polynomial.resize(size);


        firstEncValues.resize(tableRealSize, 0);
        secondEncValues.resize(tableRealSize, 0);
//TODO until here

        otherParty->getChannel()->read((byte*)firstEncValues.data(), firstEncValues.size()*8);
        otherParty->getChannel()->read((byte*)secondEncValues.data(), secondEncValues.size()*8);
        otherParty->getChannel()->read((byte*)polynomial.data(), polynomial.size()*8);
    }

    void calcRealValues(){

        cout<<"vals:"<<endl;
        uint64_t firstPosition, secondPosition, val, poliVal;
        int size = keys.size();
        vector<uint64_t> vals(size);
        for (int i=0; i<size; i++){
            poliVal = getPolynomialValue(keys[i]);
            firstPosition = first.bucket(keys[i]);
            secondPosition = second.bucket(keys[i]);

            vals[i] = firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ poliVal;
            cout<<"key = "<<keys[i]<<" val = "<<vals[i]<<endl;
        }
    }
};
#endif //BENNYPROJECT_OBLIVIOUSDICTIONARY_H
