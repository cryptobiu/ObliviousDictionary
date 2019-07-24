//
// Created by moriya on 7/17/19.
//

#ifndef BENNYPROJECT_OBLIVIOUSDICTIONARY_H
#define BENNYPROJECT_OBLIVIOUSDICTIONARY_H
#include <unordered_set>
#include <unordered_map>
#include <libscapi/include/primitives/Prg.hpp>
#include "xxhash.h"

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

    size_t operator() (int const key) const
    {
        unsigned long long const hash = XXH64(&key, 4, seed);
        return hash;
    }

};

class obliviousDictionary {
private:
    int hashSize;
    PrgFromOpenSSLAES prg;
    vector<uint32_t> keys;
    unordered_map<uint32_t, uint32_t> vals;
    vector<uint32_t> firstHashValues;
    vector<uint32_t> secondHashValues;

    vector<uint32_t> firstEncValues;
    vector<uint32_t> secondEncValues;

    unordered_set<uint32_t> first;
    unordered_set<uint32_t, Hasher> second;

    vector<uint32_t> peelingVector;
    int tableRealSize;

public:
    obliviousDictionary(int hashSize) : hashSize(hashSize){
        first.reserve(hashSize);
        second = unordered_set<uint32_t, Hasher>(hashSize, Hasher(10));
        keys.resize(hashSize);
        vals.reserve(hashSize);
        firstHashValues.resize(hashSize);
        secondHashValues.resize(hashSize);

        tableRealSize = first.bucket_count();
        cout<<"tableRealSize = "<<tableRealSize<<endl;

        firstEncValues.resize(tableRealSize);
        secondEncValues.resize(tableRealSize);

        memset(firstEncValues.data(), 0, tableRealSize*4);
        memset(secondEncValues.data(), 0, tableRealSize*4);



        auto key = prg.generateKey(128);
        prg.setKey(key);
    }

    void fillTables(){
        uint32_t key;

        for (int i=0; i<hashSize; i++){
            keys[i] = prg.getRandom32();
            vals.insert({keys[i],prg.getRandom32()});

            cout<<"key is "<<keys[i]<<endl;
            first.insert(keys[i]);
            second.insert(keys[i]);


            firstHashValues[i] = first.bucket(keys[i]);
            secondHashValues[i] = second.bucket(keys[i]);

            cout<<"bucket1 is "<<firstHashValues[i]<<endl;
            cout<<"bucket2 is "<<secondHashValues[i]<<endl;

        }
    }

    void peeling(){

        //Goes on the first hash
        for (int position = 0; position<tableRealSize; position++){
            if (first.bucket_size(position) == 1){
                //Delete the vertex from the graph
                auto key = *first.begin(position);
                cout<<"remove key "<<key<<endl;
                peelingVector.push_back(key);
                first.erase(key);

                //Update the second vertex on the edge
                second.erase(key);
            }
        }

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

                    peelingVector.push_back(key);
                    cout<<"remove key "<<key<<endl;
                    second.erase(key);

                    if (secondBucket>0) cout<<"loop in peeling"<<endl;
                    //Update the second vertex on the edge
                    int bucket = first.bucket(key);
                    first.erase(key);
                    if (first.bucket_size(bucket) == 1) {
                        key = *first.begin(bucket);
                        cout<<"remove key from first "<<key<<endl;
                        peelingVector.push_back(key);
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

        if (hasLoop()){
            cout << "remain loops!!!" << endl;
        }

    }

    void calcPolinomial(){

        vector<uint32_t> edgesForPolinomial;
        vector<uint32_t> valuesForPolinomial;

        //Assign random values to all vertex in the circles.
        for (int i=0; i<tableRealSize; i++){
            if (first.bucket_size(i) > 1){
                firstEncValues[i] =  prg.getRandom32();
                cout<<"bucket "<<i<<" got random value in first"<<endl;
            }
            if (second.bucket_size(i) > 1) {
                secondEncValues[i] = prg.getRandom32();
                cout<<"bucket "<<i<<" got random value in second"<<endl;
            }
        }

        //Get all the edges that are in the graph's circles and calc the polinomial values that should be for them.
        for (int i=0; i<tableRealSize; i++){
            if (first.bucket_size(i) > 1){
                for (auto key = first.begin(i); key!= first.end(i); ++key){
                    edgesForPolinomial.push_back(*key);
                    valuesForPolinomial.push_back(firstEncValues[i] ^ secondEncValues[second.bucket(*key)] ^ vals[*key]);

                }
            }
        }


        //TODO interpolate!!!


    }

    uint32_t getPolinomialValue(uint32_t key){
        return 0;
    }


    void unpeeling(){
        cout<<"in unpeeling"<<endl;
        uint32_t firstPosition, secondPosition, poliVal;
        while (peelingVector.size()>0){
            uint32_t key = peelingVector.back();
            cout<<"key = "<<key<<endl;
            peelingVector.pop_back();
            firstPosition = first.bucket(key);
            secondPosition = second.bucket(key);

            poliVal = getPolinomialValue(key);
            if (firstEncValues[firstPosition] == 0 && secondEncValues[secondPosition] == 0){
                firstEncValues[firstPosition] = prg.getRandom32();
                cout<<"set RANDOM value to first in bucket "<<firstPosition<<endl;
            }
            if (firstEncValues[firstPosition] == 0){
                firstEncValues[firstPosition] = vals[key] ^ secondEncValues[secondPosition] ^ poliVal;
                cout<<"set value to first in bucket "<<firstPosition<<endl;
            } else {
                secondEncValues[secondPosition] = vals[key] ^ firstEncValues[firstPosition] ^ poliVal;
                cout<<"set value to second in bucket "<<secondPosition<<endl;
            }
        }
    }

    void checkOutput(){

        uint32_t firstPosition, secondPosition, key, val, poliVal;

        for (int i=0; i<hashSize; i++){
            key = keys[i];
            val = vals[key];

            poliVal = getPolinomialValue(key);
            firstPosition = first.bucket(key);
            secondPosition = second.bucket(key);

            if ((firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ poliVal) == val) {
                cout<<"good value!!! val = "<<val<<endl;
            } else if (!hasLoop()){
                cout<<"invalid value :( val = "<<val<<" wrong val = "<<(firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ poliVal)<<endl;
                cout<<"firstEncValues[firstPosition] = "<<firstEncValues[firstPosition]<<endl;
                cout<<"secondEncValues[secondPosition] = "<<secondEncValues[secondPosition]<<endl;
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



};


#endif //BENNYPROJECT_OBLIVIOUSDICTIONARY_H
