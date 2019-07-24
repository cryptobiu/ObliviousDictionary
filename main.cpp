#include <iostream>

#include "obliviousDictionary.h"

int main() {

    void* buffer = new unsigned char[10];
    memset(buffer, 0, 10);
    unsigned long long  seed = 10;   /* or any other value */
    unsigned long long  hash = XXH64(buffer, 10, seed);
    cout<<"hash = "<<hash<<endl;
    hash = XXH64(buffer, 10, seed);
    cout<<"hash = "<<hash<<endl;

    obliviousDictionary dic(10);
    dic.fillTables();
    dic.peeling();
    dic.calcPolinomial();
    dic.unpeeling();
    dic.checkOutput();
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
