#include <iostream>

#include "ProtocolParty.h"
#include <chrono>

using namespace std::chrono;

int main(int argc, char* argv[]) {

    CmdParser parser;
    auto parameters = parser.parseArguments("", argc, argv);
    int partyID = stoi(parser.getValueByKey(parameters, "partyID"));

//    PrgFromOpenSSLAES prg;
//    auto key = prg.generateKey(128);
//    for (int i=0; i<16; i++){
//        cout<<(int)key.getEncoded()[i]<<" ";
//    }
//    cout<<endl;

//    SecretKey key({144, 41, 147, 169, 38, 73 ,94, 215, 215, 60, 171 ,19, 249, 232 ,54 ,72}, "");
//    prg.setKey(key);

//    vector<uint64_t> keys;
//    if (partyID == 0) {
//
//        keys.resize(hashSize);
//
//        for (int i = 0; i < hashSize; i++) {
//            keys[i] = prg.getRandom64() >> 24;
//        }
//
//    } else {
//    }

    if (partyID == 0) {
        DBParty dic(argc, argv);
        dic.run();
    } else {
        QueryParty dic(argc, argv);
        dic.run();
    }

//    auto start = high_resolution_clock::now();
//    auto t1 = high_resolution_clock::now();
//
//    dic.fillTables();
//    auto t2 = high_resolution_clock::now();
//
//    auto duration = duration_cast<milliseconds>(t2-t1).count();
//    cout << "fillTables took in milliseconds: " << duration << endl;
//
//    t1 = high_resolution_clock::now();
//    dic.peeling();
//
//    t2 = high_resolution_clock::now();
//
//    duration = duration_cast<milliseconds>(t2-t1).count();
//    cout << "peeling took in milliseconds: " << duration << endl;
//
//    t1 = high_resolution_clock::now();
//    dic.calcPolynomial();
//    t2 = high_resolution_clock::now();
//
//    duration = duration_cast<milliseconds>(t2-t1).count();
//    cout << "calcPolinomial took in milliseconds: " << duration << endl;
//
//    t1 = high_resolution_clock::now();
//    dic.unpeeling();
//
//    t2 = high_resolution_clock::now();
//
//    duration = duration_cast<milliseconds>(t2-t1).count();
//    cout << "unpeeling took in milliseconds: " << duration << endl;
//
//    auto end = high_resolution_clock::now();
//
//    duration = duration_cast<milliseconds>(end-start).count();
//    cout << "all protocol took in milliseconds: " << duration << endl;
//
//    dic.checkOutput();
//    unordered_set<int> set1(1000);
//    cout<<"number of  buckets in set1 = "<<set1.bucket_count()<<endl;
//    unordered_set<int, Hasher> set2(1000, Hasher(10));

//    for (int i=0; i<1200; i++) {
//        set1.insert(i);
//
//    }
////    int bucket = set1.bucket(i);
////    cout<<"set1 put 2080 in bucket number "<<bucket<<endl;
////
//    set1.insert(2081);
//    auto bucket = set1.bucket(2081);
//    cout<<"set1 put 2081 in bucket number "<<bucket<<endl;
//    cout<<"number of  buckets in set1 = "<<set1.bucket_count()<<endl;
//
//    set2.insert(2081);
//    bucket = set2.bucket(2081);
//    cout<<"set2 put 2081 in bucket number "<<bucket<<endl;
//    cout<<"number of  buckets in set2 = "<<set2.bucket_count()<<endl;
//    bucket = set2.bucket(2081);
//    cout<<"set2 put 2081 in bucket number "<<bucket<<endl;
//    bucket = set2.bucket(2081);
//    cout<<"set2 put 2081 in bucket number "<<bucket<<endl;
//    bucket = set2.bucket(2081);
//    cout<<"set2 put 2081 in bucket number "<<bucket<<endl;


    return 0;
}
