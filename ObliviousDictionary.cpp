//
// Created by moriya on 7/17/19.
//

#include "ObliviousDictionary.h"
#include "Tools.h"
#include <queue>

#define print_timings false
#define print false

void ObliviousDictionaryDB::init() {
    vals.clear();
    keys.clear();
    peelingVector.clear();
    peelingCounter = 0;

    keys.resize(hashSize);
    vals.reserve(hashSize);

    for (int i=0; i < hashSize; i++){
        keys[i] = prg->getRandom64() >> 3;
        vals.insert({keys[i], prg->getRandom64() >> 3});
    }

    if(print) {
        int numKeysToCheck = 10;
        cout << "keys to check with the other party" << endl;
        for (int i = 0; i < numKeysToCheck; i++) {
            cout << "key = " << keys[i] << " val = " << vals[keys[i]] << endl;
        }
    }
}


void ObliviousDictionaryDB2Tables::init() {

    firstSeed = prg->getRandom64();
    secondSeed = prg->getRandom64();

    ObliviousDictionaryDB::init();

    first.clear();
    second.clear();
}
ObliviousDictionaryDB2Tables::ObliviousDictionaryDB2Tables(int size, string toolType) : ObliviousDictionaryDB(size) {

    auto key = prg->generateKey(128);
    prg->setKey(key);

    firstSeed = 123456;//prg->getRandom64();
    secondSeed = 987654;//prg->getRandom64();

    auto start = high_resolution_clock::now();
    createSets();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end-start).count();

    if(print_timings)
        cout << "time in milliseconds for create sets: " << duration << endl;

    start = high_resolution_clock::now();
    first.insert(1);
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end-start).count();
    if(print_timings)
        cout << "time in milliseconds for insert first element: " << duration << endl;

    start = high_resolution_clock::now();
    first.erase(1);
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end-start).count();
    if(print_timings)
        cout << "time in milliseconds for erase first element: " << duration << endl;

    if(print) {
        cout << "after create sets" << endl;
        cout << "tableRealSize = " << tableRealSize << endl;
        cout << "hashSize = " << hashSize << endl;
    }

    firstEncValues.resize(tableRealSize, 0);
    secondEncValues.resize(tableRealSize, 0);


//    keys.resize(hashSize);
//    vals.reserve(hashSize);
//
//    for (int i=0; i<hashSize; i++){
//        keys[i] = prg->getRandom64() >> 3;
//        vals.insert({keys[i],prg->getRandom64()>>3});
//    }
//
//    int numKeysToCheck = 10;
//    cout<<"keys to check with the other party"<<endl;
//    for (int i=0; i<numKeysToCheck; i++){
//        cout<<"key = "<<keys[i]<<" val = "<<vals[keys[i]]<<endl;
//    }

    if (toolType.compare("poly") == 0){
        tool = new PolynomialTool(hashSize);
    }


}

void ObliviousDictionaryDB2Tables::createSets(){
    first = unordered_set<uint64_t, Hasher>(hashSize, Hasher(firstSeed));
    second = unordered_set<uint64_t, Hasher>(hashSize, Hasher(secondSeed));

    tableRealSize = first.bucket_count();
    if(print)
        cout<<"tableRealSize = "<<tableRealSize<<endl;

    while(tableRealSize/1.2 < hashSize){
        first = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(firstSeed));
        second = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(secondSeed));

        tableRealSize = first.bucket_count();
        if(print)
            cout<<"tableRealSize = "<<tableRealSize<<endl;
    }

    hashSize = tableRealSize/1.2;
    if(print)
        cout<<"new hashSize = "<<hashSize<<endl;
}

void ObliviousDictionaryDB2Tables::fillTables(){

    for (int i=0; i<hashSize; i++){

//            cout<<"key is "<<keys[i]<<endl;
//        auto pair = first.insert(keys[i]);
        first.insert(keys[i]);
        second.insert(keys[i]);

//        if (pair.second == false){
//            cout<<"key = "<<keys[i]<<" i = "<<i<<endl;
//        }
    }


    if(print) {
        cout << "first set contains " << first.size() << endl;
        cout << "second set contains " << second.size() << endl;
    }
}

void ObliviousDictionaryDB2Tables::peeling(){

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

    if(print_timings)
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

    if(print_timings)
        cout << "time in milliseconds for second loop: " << duration << endl;

    if (hasLoop()){
        cout << "remain loops!!!" << endl;
    }

    if(print)
        cout<<"peelingCounter = "<<peelingCounter<<endl;

}

void ObliviousDictionaryDB2Tables::generateExternalToolValues(){
    int polySize = 5*log2(hashSize);
    vector<uint64_t> edgesForPolynomial(polySize);
    vector<uint64_t> valuesForPolynomial(polySize);

    //Assign random values to all vertex in the circles.
    for (int i=0; i<tableRealSize; i++){
        if (first.bucket_size(i) > 1){
            firstEncValues[i] =  prg->getRandom64() >> 3;
//                cout<<"bucket "<<i<<" got random value in first"<<endl;
        }
        if (second.bucket_size(i) > 1) {
            secondEncValues[i] = prg->getRandom64()  >> 3;
//                cout<<"bucket "<<i<<" got random value in second"<<endl;
        }
    }

    //Get all the edges that are in the graph's circles and calc the polynomial values that should be for them.
    int polyCounter = 0;
    for (int i=0; i<tableRealSize; i++){
        if (first.bucket_size(i) > 1){
            for (auto key = first.begin(i); key!= first.end(i); ++key){
                edgesForPolynomial[polyCounter] = *key;
                valuesForPolynomial[polyCounter] = firstEncValues[i] ^ secondEncValues[second.bucket(*key)] ^ vals[*key];
                polyCounter++;
            }
        }
    }

    if(print)
        cout<<"circles size =  "<<polyCounter<<endl;

    if(reportStatistics==1) {

        statisticsFile << polyCounter << ", \n";
    }


if (polyCounter < polySize){
        for (int i=polyCounter; i<polySize; i++){
            edgesForPolynomial[polyCounter] = peelingVector[i];
            valuesForPolynomial[polyCounter] = prg->getRandom64() >> 3;
            polyCounter++;
        }
    } else if (polyCounter > polySize){
        cout<<"error!!! circles size is bigger than polynomial size"<<endl;
    }


    tool->generateToolValues(edgesForPolynomial, valuesForPolynomial);
}




void ObliviousDictionaryDB2Tables::unpeeling(){
    if(print)
        cout<<"in unpeeling"<<endl;

    uint64_t firstPosition, secondPosition, poliVal, key;
//    vector<uint64_t> polyVals(peelingCounter);
//    Poly::evalMersenneMultipoint(polyVals, polynomial, peelingVector);

    while (peelingCounter > 0){
//            cout<<"key = "<<key<<endl;
        key = peelingVector[--peelingCounter];
        firstPosition = first.bucket(key);
        secondPosition = second.bucket(key);

        poliVal = tool->getValue(key);
//        Poly::evalMersenne((ZpMersenneLongElement*)&poliVal, polynomial, (ZpMersenneLongElement*)&key);
//        poliVal = polyVals[peelingCounter];
        if (firstEncValues[firstPosition] == 0 && secondEncValues[secondPosition] == 0){
            firstEncValues[firstPosition] = prg->getRandom64()  >> 3;
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
    if(print)
        cout<<"peelingCounter = "<<peelingCounter<<endl;
}

void ObliviousDictionaryDB2Tables::checkOutput(){

    uint64_t firstPosition, secondPosition, key, val, poliVal;

    for (int i=0; i<hashSize; i++){
        key = keys[i];
        val = vals[key];

        poliVal = tool->getValue(key);
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

bool ObliviousDictionaryDB2Tables::hasLoop(){
    for (int position = 0; position<tableRealSize; position++) {
        if (first.bucket_size(position) > 1) {
            return true;
        }
    }
    return false;
}


void ObliviousDictionaryDB2Tables::sendData(shared_ptr<ProtocolPartyData> otherParty){
//TODO should be deleted!!
//    otherParty->getChannel()->write((byte*)&firstSeed, 8);
//    otherParty->getChannel()->write((byte*)&secondSeed, 8);
    if(print) {
        cout << "firstSeed = " << firstSeed << endl;
        cout << "secondSeed = " << secondSeed << endl;
        cout << "key size in bytes: " << keys.size() * 8 << endl;
    }
    otherParty->getChannel()->write((byte*)keys.data(), keys.size()*8);
//TODO until here

    otherParty->getChannel()->write((byte*)firstEncValues.data(), firstEncValues.size()*8);
    otherParty->getChannel()->write((byte*)secondEncValues.data(), secondEncValues.size()*8);
    int polySize = tool->getSendableDataSize();
    if(print)
        cout<<"polySize = "<<polySize/8<<endl;
    otherParty->getChannel()->write(tool->getSendableData(), polySize);
}

ObliviousDictionaryQuery2Tables::ObliviousDictionaryQuery2Tables(int hashSize, string toolType) : ObliviousDictionaryQuery(hashSize){

    auto key = prg->generateKey(128);
    prg->setKey(key);

    if (toolType.compare("poly") == 0){
        tool = new PolynomialTool(hashSize);
    }
}

void ObliviousDictionaryQuery2Tables::createSets(){
    first = unordered_set<uint64_t, Hasher>(hashSize, Hasher(firstSeed));
    second = unordered_set<uint64_t, Hasher>(hashSize, Hasher(secondSeed));

    tableRealSize = first.bucket_count();
    if(print)
        cout<<"tableRealSize = "<<tableRealSize<<endl;

    while(tableRealSize/1.2 < hashSize){
        first = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(firstSeed));
        second = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(secondSeed));

        tableRealSize = first.bucket_count();
        if(print)
            cout<<"tableRealSize = "<<tableRealSize<<endl;
    }

   hashSize = tableRealSize/1.2;
    if(print)
        cout<<"new hashSize = "<<hashSize<<endl;

    tool->setHashSize(hashSize);
}

void ObliviousDictionaryQuery2Tables::readData(shared_ptr<ProtocolPartyData> otherParty){

//TODO should be deleted!!
//    otherParty->getChannel()->read((byte*)&firstSeed, 8);
//    otherParty->getChannel()->read((byte*)&secondSeed, 8);
    firstSeed = 123456;//prg->getRandom64();
    secondSeed = 987654;

    if(print) {
        cout << "firstSeed = " << firstSeed << endl;
        cout << "secondSeed = " << secondSeed << endl;
    }
    createSets();

    vector<uint64_t> tmpKeys(hashSize);
    otherParty->getChannel()->read((byte*)tmpKeys.data(), tmpKeys.size()*8);

    int numKeysToCheck = 10;
    keys.resize(numKeysToCheck);
    for (int i=0; i<numKeysToCheck; i++){
        keys[i] = tmpKeys[i];//prg->getRandom32() % hashSize;
    }
    int polySize = 5*log2(hashSize)*8;
    if(print)
        cout<<"polySize = "<<polySize/8<<endl;
    byte* polynomial = new byte[polySize];

    firstEncValues.resize(tableRealSize, 0);
    secondEncValues.resize(tableRealSize, 0);
//TODO until here

    otherParty->getChannel()->read((byte*)firstEncValues.data(), firstEncValues.size()*8);
    otherParty->getChannel()->read((byte*)secondEncValues.data(), secondEncValues.size()*8);

    otherParty->getChannel()->read(polynomial, polySize);
    tool->setSendableData(polynomial);

    delete polynomial;
}

void ObliviousDictionaryQuery2Tables::calcRealValues(){

    if(print)
        cout<<"vals:"<<endl;

    uint64_t firstPosition, secondPosition, poliVal;
    int size = keys.size();
    vector<uint64_t> vals(size);
    for (int i=0; i<size; i++){
        poliVal = tool->getValue(keys[i]);
        firstPosition = first.bucket(keys[i]);
        secondPosition = second.bucket(keys[i]);

        vals[i] = firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ poliVal;
        if(print)
            cout<<"key = "<<keys[i]<<"val = "<<vals[i]<<endl;
//        cout<<"firstEncValues["<<firstPosition<<"] = "<<firstEncValues[firstPosition]<<endl;
//        cout<<"secondEncValues["<<secondPosition<<"] = "<<secondEncValues[secondPosition]<<endl;
//        cout<<"poliVal = "<<poliVal<<endl;
    }

}

void ObliviousDictionaryQuery2Tables::output(){

}


ObliviousDictionaryDB3Tables::ObliviousDictionaryDB3Tables(int size, string toolType,int batchSize, std::string processId, float tableRatio) : ObliviousDictionaryDB(size) {

    this->batchSize = batchSize;
    this->processId = processId;
    this->tableRatio = tableRatio;
    this->hashOriginalSize = size;


    circleVec.resize(batchSize);
    auto key = prg->generateKey(128);
    prg->setKey(key);

    firstSeed = prg->getRandom64();
    secondSeed = prg->getRandom64();
    thirdSeed = prg->getRandom64();
    createSets();

    if(print) {
        cout << "after create sets" << endl;
        cout << "tableRealSize = " << tableRealSize << endl;
        cout << "hashSize = " << hashSize << endl;
    }

    firstEncValues.resize(tableRealSize, 0);
    secondEncValues.resize(tableRealSize, 0);
    thirdEncValues.resize(tableRealSize, 0);




    if (toolType.compare("poly") == 0){
        tool = new PolynomialTool(hashSize);
    }


}

void ObliviousDictionaryDB3Tables::init() {

    firstSeed = prg->getRandom64();
    secondSeed = prg->getRandom64();
    thirdSeed = prg->getRandom64();

    ObliviousDictionaryDB::init();

    first.clear();
    second.clear();
    third.clear();
}


void ObliviousDictionaryDB3Tables::createSets(){
    first = unordered_set<uint64_t, Hasher>(hashSize*tableRatio/3.0, Hasher(firstSeed));
    second = unordered_set<uint64_t, Hasher>(hashSize*tableRatio/3.0, Hasher(secondSeed));
    third = unordered_set<uint64_t, Hasher>(hashSize*tableRatio/3.0, Hasher(thirdSeed));



    tableRealSize = first.bucket_count();
    if(print)
        cout<<"tableRealSize = "<<tableRealSize<<endl;

    while(tableRealSize*3.0/tableRatio < hashSize){
        first = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(firstSeed));
        second = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(secondSeed));
        third = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(thirdSeed));

        tableRealSize = first.bucket_count();
        if(print)
            cout<<"tableRealSize = "<<tableRealSize<<endl;
    }

    first.max_load_factor(3.0/tableRatio + 1);
    second.max_load_factor(3.0/tableRatio +1);
    third.max_load_factor(3.0/tableRatio + 1);

    hashSize = tableRealSize*3.0/tableRatio;

    if(print)
        cout<<"new hashSize = "<<hashSize<<endl;
}

void ObliviousDictionaryDB3Tables::fillTables(){

    for (int i=0; i<hashSize; i++){

//            cout<<"key is "<<keys[i]<<endl;
//        auto pair = first.insert(keys[i]);
        first.insert(keys[i]);
        second.insert(keys[i]);
        third.insert(keys[i]);

//        cout<<"first bucket = "<<first.bucket(keys[i])<<" second bucket = "<<second.bucket(keys[i])<<" third bucket = "<<third.bucket(keys[i])<<endl;

//        if (pair.second == false){
//            cout<<"key = "<<keys[i]<<" i = "<<i<<endl;
//        }
    }


    if(print) {
        cout << "first set contains " << first.size() << endl;
        cout << "second set contains " << second.size() << endl;
        cout << "third set contains " << third.size() << endl;
    }

}

void ObliviousDictionaryDB3Tables::peeling2() {

    peelingVector.resize(hashSize);
    peelingCounter = 0;
    int counterInLoop = 1;

    cout << "in peeling" << endl;
//    cout<<"first loop"<<endl;
    while (counterInLoop > 0){
        counterInLoop = 0;
//        auto t1 = high_resolution_clock::now();
        //Goes on the first hash
        for (int position = 0; position < tableRealSize; position++) {
            if (first.bucket_size(position) == 1) {
                //Delete the vertex from the graph
                auto key = *first.begin(position);
//                cout << "remove key " << key << endl;
                counterInLoop++;
                peelingVector[peelingCounter++] = key;
                first.erase(key);

                //Update the second vertex on the edge
                second.erase(key);
                third.erase(key);
            }
        }
//        auto t2 = high_resolution_clock::now();
//        auto duration = duration_cast<milliseconds>(t2 - t1).count();

//        cout << "time in milliseconds for first loop: " << duration << endl;

//        t1 = high_resolution_clock::now();
        //Goes on the first hash
        for (int position = 0; position < tableRealSize; position++) {
            if (second.bucket_size(position) == 1) {
                //Delete the vertex from the graph
                auto key = *second.begin(position);
//                cout << "remove key " << key << endl;
                counterInLoop++;
                peelingVector[peelingCounter++] = key;
                second.erase(key);

                //Update the second vertex on the edge
                first.erase(key);
                third.erase(key);
            }
        }
//        t2 = high_resolution_clock::now();
//        duration = duration_cast<milliseconds>(t2 - t1).count();

//        cout << "time in milliseconds for second loop: " << duration << endl;

//        t1 = high_resolution_clock::now();
        //Goes on the first hash
        for (int position = 0; position < tableRealSize; position++) {
            if (third.bucket_size(position) == 1) {
                //Delete the vertex from the graph
                auto key = *third.begin(position);
//                cout << "remove key " << key << endl;
                counterInLoop++;
                peelingVector[peelingCounter++] = key;
                third.erase(key);

                //Update the second vertex on the edge
                second.erase(key);
                first.erase(key);
            }
        }
//        t2 = high_resolution_clock::now();
//        duration = duration_cast<milliseconds>(t2 - t1).count();

//        cout << "time in milliseconds for third loop: " << duration << endl;
//
//        cout<<" counterInLoop = "<< counterInLoop<<endl;
    }

/*
//    cout<<"second loop"<<endl;
auto     t1 = high_resolution_clock::now();
    //goes on the second hash
    for (int position = 0; position<tableRealSize; position++){
        if (second.bucket_size(position) == 1){
            //Delete the vertex from the graph
            auto key = *second.begin(position);
//                peelingVector.push_back(key);
//                cout<<"remove key "<<key<<endl;
//                second.erase(key);

            peelSecondSet(position, key, true, nullptr, key);

        }
    }
 auto   t2 = high_resolution_clock::now();
  auto  duration = duration_cast<milliseconds>(t2-t1).count();

    cout << "time in milliseconds for second loop: " << duration << endl;

//    cout<<"third loop"<<endl;

    t1 = high_resolution_clock::now();
    //goes on the second hash
    for (int position = 0; position<tableRealSize; position++){
        vector<uint64_t> keysToDeleteFromThree;
        peelThirdSet(position, &keysToDeleteFromThree);
//
//        cout<<"keysToDeleteFromThree size = "<<keysToDeleteFromThree.size()<<endl;

        for (int i=0; i<keysToDeleteFromThree.size(); i++){
            auto key = keysToDeleteFromThree[i];
            int bucket = third.bucket(key);
            third.erase(key);
//            cout<<"remove key from third "<<key<<endl;
            peelThirdSet(bucket, &keysToDeleteFromThree);
//            cout<<"keysToDeleteFromThree size = "<<keysToDeleteFromThree.size()<<endl;
        }

    }
    t2 = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(t2-t1).count();

    cout << "time in milliseconds for third loop: " << duration << endl;

    if (hasLoop()){
        cout << "remain loops!!!" << endl;
    }
*/
    if(print) {
        cout << "peelingCounter = " << peelingCounter << endl;
        cout << "circle size  = " << (hashSize - peelingCounter) << endl;
    }
//    cout<<"peeling vector:"<<endl;
//    for (int i=0; i<peelingCounter; i++){
//        cout<<peelingVector[i]<<" ";
//    }
//    cout<<endl;

}

void ObliviousDictionaryDB3Tables::peelThirdSet(int position, vector<uint64_t> * keysToDeleteFromThree){
    if (third.bucket_size(position) == 1){
        //Delete the vertex from the graph
        auto key = *third.begin(position);
        peelingVector[peelingCounter++] = key;
//        cout<<"remove key from third "<<key<<endl;
        third.erase(key);


        //Dealing with set number 2
        int bucket = second.bucket(key);
        second.erase(key);
//        cout<<"remove key from second "<<key<<endl;
        if (second.bucket_size(bucket) == 1) {
            auto secondKey = *second.begin(bucket);
//cout<<"go to peelSecondSet. bucket = "<<bucket<<" key = "<<secondKey<<endl;
            peelSecondSet(second.bucket_count(), secondKey, false, keysToDeleteFromThree, key);
        }


        //Dealing with set number 1
        bucket = first.bucket(key);
        first.erase(key);
//        cout<<"remove key from first "<<key<<endl;
        if (first.bucket_size(bucket) == 1) {
            auto secondKey = *first.begin(bucket);
            peelingVector[peelingCounter++] = secondKey;
            keysToDeleteFromThree->push_back(secondKey);
//            cout<<"put "<<secondKey<<" in keysToDeleteFromThree"<<endl;
            first.erase(secondKey);
//            cout<<"remove key from first "<<secondKey<<endl;

            bucket = second.bucket(secondKey);
            second.erase(secondKey);
            if (second.bucket_size(bucket) == 1) {
                secondKey = *second.begin(bucket);
                peelSecondSet(second.bucket_count(), secondKey, false, keysToDeleteFromThree, key);
            }
        }
    }
}

void ObliviousDictionaryDB3Tables::peelSecondSet(int position, uint64_t key, bool deleteFromThree, vector<uint64_t> * keysToDeleteFromThree, uint64_t originalKey) {

    int secondBucket = 0;

    while (secondBucket <= position) {

        peelingVector[peelingCounter++] = key;

//                    cout<<"remove key from second "<<key<<endl;
        second.erase(key);
        if (deleteFromThree) {
            third.erase(key);
//            cout<<"remove key from third "<<key<<endl;
        } else {
            keysToDeleteFromThree->push_back(key);
//            cout<<"put "<<key<<" in keysToDeleteFromThree"<<endl;
        }

//                    if (secondBucket>0) cout<<"loop in peeling"<<endl;
        //Update the second vertex on the edge
        int bucket = first.bucket(key);
        first.erase(key);
//        cout<<"remove key from first "<<key<<endl;
        if (first.bucket_size(bucket) == 1) {
            key = *first.begin(bucket);


            if (key != originalKey) {
//                cout << "remove key from first " << key << endl;
                peelingVector[peelingCounter++] = key;
                first.erase(key);
                if (deleteFromThree) {
                    third.erase(key);
//                    cout<<"remove key from third "<<key<<endl;
                } else {
                    keysToDeleteFromThree->push_back(key);
//                    cout<<"put "<<key<<" in keysToDeleteFromThree"<<endl;
                }

                //Update the second vertex on the edge
                secondBucket = second.bucket(key);
                second.erase(key);
//                cout << "remove key from second " << key << endl;
                if (second.bucket_size(secondBucket) == 1) {
                    key = *second.begin(secondBucket);
                } else {
                    secondBucket = position + 1;
                }
            } else {
//                cout<<"key will be deleted in the future"<<endl;
                secondBucket = position + 1;
            }
        } else {
            secondBucket = position + 1;
        }

    }

}


void ObliviousDictionaryDB3Tables::peeling() {

    peelingVector.resize(hashSize);
    peelingCounter = 0;
    int counterInLoop = 1;

    queue<int> queueFirst;
    queue<int> queueSecond;
    queue<int> queueThird;

    // cout << "in peeling" << endl;
//    cout<<"first loop"<<endl;

    auto start = high_resolution_clock::now();

     //Goes on the first hash
    for (int position = 0; position < tableRealSize; position++) {
        if (first.bucket_size(position) == 1) {
            //Delete the vertex from the graph
            auto key = *first.begin(position);
//                cout << "remove key " << key << endl;
            counterInLoop++;
            peelingVector[peelingCounter++] = key;
            first.erase(key);

            //Update the second vertex on the edge
            second.erase(key);
            third.erase(key);
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end-start).count();
    if(print_timings)
        cout << "time in milliseconds for first peel: " << duration << endl;

    start = high_resolution_clock::now();
    int bucketInd;
    //Goes on the second has
    for (int position = 0; position < tableRealSize; position++) {
        if (second.bucket_size(position) == 1) {
            //Delete the vertex from the graph
            auto key = *second.begin(position);
            second.erase(key);
//                cout << "remove key " << key << endl;
            counterInLoop++;
            peelingVector[peelingCounter++] = key;

            bucketInd = first.bucket(key);
            first.erase(key);

            if(first.bucket_size(bucketInd)==1)
                queueFirst.push(bucketInd);

            //Update the second vertex on the edge
            third.erase(key);
        }
    }

    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end-start).count();
    if(print_timings)
        cout << "time in milliseconds for second peel: " << duration << endl;

    start = high_resolution_clock::now();
    for (int position = 0; position < tableRealSize; position++) {
        if (third.bucket_size(position) == 1) {
            //Delete the vertex from the graph
            auto key = *third.begin(position);
            third.erase(key);
//                cout << "remove key " << key << endl;
            counterInLoop++;
            peelingVector[peelingCounter++] = key;

            bucketInd = first.bucket(key);
            first.erase(key);

            if(first.bucket_size(bucketInd)==1)
                queueFirst.push(bucketInd);

            bucketInd = second.bucket(key);
            second.erase(key);

            if(second.bucket_size(bucketInd)==1)
                queueSecond.push(bucketInd);

        }
    }

    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end-start).count();
    if(print_timings)
        cout << "time in milliseconds for third peel: " << duration << endl;

    if(print) {
        cout << "peelingCounter : " << peelingCounter << endl;
        cout << "hashSize : " << hashSize << endl;

        cout << "queueFirst.size() : " << queueFirst.size() << endl;
        cout << "queueSecond.size() : " << queueSecond.size() << endl;
        cout << "queueThird.size() : " << queueThird.size() << endl;
    }

    start = high_resolution_clock::now();
    //handle the queues one by one
    while(queueFirst.size()!=0 ||
            queueSecond.size()!=0 ||
            queueThird.size()!=0){

        handleQueue(queueFirst, first,
                    queueSecond, second,
                       queueThird,third);

        handleQueue(queueSecond, second,
                    queueFirst, first,
                    queueThird,third);


        handleQueue(queueThird,third,
                queueFirst, first,
                    queueSecond, second);

    }

    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end-start).count();
    if(print_timings)
        cout << "time in milliseconds for peel queues: " << duration << endl;


    if(print) {
        cout << "peelingCounter : " << peelingCounter << endl;
        cout << "hashSize : " << hashSize << endl;
    }

//    if(reportStatistics==1) {
//
//        statisticsFile << "tesintg" << ", \n";
//    }

}

void ObliviousDictionaryDB3Tables::handleQueue(queue<int> &queueMain, unordered_set<uint64_t, Hasher> &main,
                                                queue<int> &queueOther1, unordered_set<uint64_t, Hasher> &other1,
                                                queue<int> &queueOther2,unordered_set<uint64_t, Hasher> &other2) {

    int bucketInd;
    for(int i=0; i < queueMain.size(); i++){

        int pos = queueMain.front();
        queueMain.pop();
        if(main.bucket_size(pos) == 1) {
            auto key = *main.begin(pos);
            main.erase(key);
//                cout << "remove key " << key << endl;
            peelingVector[peelingCounter++] = key;

            bucketInd = other1.bucket(key);
            other1.erase(key);

            if (other1.bucket_size(bucketInd) == 1)
                queueOther1.push(bucketInd);

            bucketInd = other2.bucket(key);
            other2.erase(key);

            if (other2.bucket_size(bucketInd) == 1)
                queueOther2.push(bucketInd);

        }
    }
}


void ObliviousDictionaryDB3Tables::updateIteration(int iteration){


    circleVec[iteration%batchSize] = hashSize - peelingCounter;

    //write to file
    if(iteration%batchSize==batchSize-1){


        int greater0 = 0;
        int greaterHalfLog = 0;
        int greater1Log = 0;
        int greater2Log = 0;
        int greater3Log = 0;
        int greater4Log = 0;
        int greater5Log = 0;

        statisticsFile<<"* finished *" << batchSize << "* runs" << endl;

        for(int i=0; i<batchSize; i++){
            auto logm = log2(hashSize);

            if (circleVec[i]>0) {
                greater0 ++;
                // statisticsFile<<circleVec[i] << "\n";
            }

            // if(circleVec[i]<5*log2(hashSize)){
            //     //do nothing
            // }
            if(circleVec[i]>0){
            // cout << "logm = " << logm << ", 2core size = " << circleVec[i] << endl;
            
            if(circleVec[i]>5*logm) {
                // cout << "greater than 5logm" << endl;
                greater5Log++;
                }
            if(circleVec[i]>4*logm){
                // cout << "greater than 4logm" << endl;
                greater4Log++;
                }
            if(circleVec[i]>3*logm){
                // cout << "greater than 3logm" << endl;
                greater3Log++;
            }
            if(circleVec[i]>2*logm){
                // cout << "greater than 2logm" << endl;
                greater2Log++;
            }
            if(circleVec[i]>1*logm){
                // cout << "greater than 1logm" << endl;
                greater1Log++;
            }
            if(circleVec[i]>0.5*logm){
                // cout << "greater than 0.5logm" << endl;
                greaterHalfLog++;
            }
            }

        }

        statisticsFile<<flush;

        groupedStatisticsFile << greaterHalfLog << ", " << greater1Log << ", " << greater2Log << ", " << greater3Log << ", " << greater4Log << ", " << greater5Log << ", " << greater0 << endl;
        // groupedStatisticsFile.flush;
        // groupedStatisticsFile<<greater5Log << "," << greater3Log << ","<< greater2Log<< ","<<greater1Log<<","<<greaterHalfLog<<endl;

    }
}
void ObliviousDictionaryDB3Tables::generateExternalToolValues(){
//    cout<<"in generate"<<endl;
    int polySize = 5*log2(hashSize);
    if(print)
        cout<<"polySize = "<<polySize<<endl;
    vector<uint64_t> edgesForPolynomial(polySize);
    vector<uint64_t> valuesForPolynomial(polySize);
//cout<<"before for"<<endl;
    //Assign random values to all vertex in the circles.
    for (int i=0; i<tableRealSize; i++){
        if (first.bucket_size(i) > 1){
            firstEncValues[i] =  prg->getRandom64() >> 3;
//                cout<<"bucket "<<i<<" got random value in first"<<endl;
        }
        if (second.bucket_size(i) > 1) {
            secondEncValues[i] = prg->getRandom64()  >> 3;
//                cout<<"bucket "<<i<<" got random value in second"<<endl;
        }
        if (third.bucket_size(i) > 1) {
            thirdEncValues[i] = prg->getRandom64()  >> 3;
//                cout<<"bucket "<<i<<" got random value in second"<<endl;
        }
    }
    if(print)
        cout<<"after for"<<endl;
    //Get all the edges that are in the graph's circles and calc the polynomial values that should be for them.
    int polyCounter = 0;
    for (int i=0; i<tableRealSize; i++){
        if (first.bucket_size(i) > 1){
//            cout<<"i = "<<i<<endl;
//            cout<<"polyCounter = "<<polyCounter<<endl;
            for (auto key = first.begin(i); key!= first.end(i); ++key){
//cout<<"1"<<endl;
                edgesForPolynomial[polyCounter] = *key;
//                cout<<"2"<<endl;
//                cout<<"search for val in index "<<second.bucket(*key)<< " and "<< third.bucket(*key)<<endl;
                valuesForPolynomial[polyCounter] = firstEncValues[i] ^ secondEncValues[second.bucket(*key)] ^ thirdEncValues[third.bucket(*key)];
//                cout<<"3"<<endl;
                valuesForPolynomial[polyCounter] ^= vals[*key];

                polyCounter++;
            }
        }
    }

    if(print)
        cout<<"circles size =  "<<polyCounter<<endl;

    if (polyCounter < polySize){
        for (int i=polyCounter; i<polySize; i++){
            edgesForPolynomial[polyCounter] = peelingVector[i];
            valuesForPolynomial[polyCounter] = prg->getRandom64() >> 3;
            polyCounter++;
        }
    } else if (polyCounter > polySize){
        cout<<"error!!! circles size is bigger than polynomial size"<<endl;
    }


    tool->generateToolValues(edgesForPolynomial, valuesForPolynomial);
}




void ObliviousDictionaryDB3Tables::unpeeling(){
    if(print)
        cout<<"in unpeeling"<<endl;
    uint64_t firstPosition, secondPosition, thirdPosition, polyVal, key;
//    vector<uint64_t> polyVals(peelingCounter);
//    Poly::evalMersenneMultipoint(polyVals, polynomial, peelingVector);

    while (peelingCounter > 0){
//            cout<<"key = "<<key<<endl;
        key = peelingVector[--peelingCounter];
        firstPosition = first.bucket(key);
        secondPosition = second.bucket(key);
        thirdPosition = third.bucket(key);


        polyVal = tool->getValue(key);
//        Poly::evalMersenne((ZpMersenneLongElement*)&poliVal, polynomial, (ZpMersenneLongElement*)&key);
//        poliVal = polyVals[peelingCounter];
        if (firstEncValues[firstPosition] == 0) {

            if (secondEncValues[secondPosition] == 0) {
                secondEncValues[secondPosition] = prg->getRandom64() >> 3;
            }
            if (thirdEncValues[thirdPosition] == 0) {
                thirdEncValues[thirdPosition] = prg->getRandom64() >> 3;
            }

            firstEncValues[firstPosition] =
                    secondEncValues[secondPosition] ^ thirdEncValues[thirdPosition] ^ polyVal ^ vals[key];
        }
        else if (secondEncValues[secondPosition] == 0) {

            if (thirdEncValues[thirdPosition] == 0) {
                thirdEncValues[thirdPosition] = prg->getRandom64() >> 3;
            }

            secondEncValues[secondPosition] = firstEncValues[firstPosition] ^ thirdEncValues[thirdPosition] ^ polyVal ^ vals[key];

        } else if (thirdEncValues[thirdPosition] == 0) {

            thirdEncValues[thirdPosition] = firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ polyVal ^ vals[key];
        }

    }
    if(print)
        cout<<"peelingCounter = "<<peelingCounter<<endl;
}

void ObliviousDictionaryDB3Tables::checkOutput(){

    uint64_t firstPosition, secondPosition, thirdPosition, key, val, poliVal;

    for (int i=0; i<hashSize; i++){
        key = keys[i];
        val = vals[key];

        poliVal = tool->getValue(key);
//        Poly::evalMersenne((ZpMersenneLongElement*)&poliVal, polynomial, (ZpMersenneLongElement*)&key);
        firstPosition = first.bucket(key);
        secondPosition = second.bucket(key);
        thirdPosition = third.bucket(key);

        if ((firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ thirdEncValues[thirdPosition] ^ poliVal) == val) {
            if (i%100000 == 0)
                cout<<"good value!!! val = "<<val<<endl;
        } else {//if (!hasLoop()){
            cout<<"invalid value :( val = "<<val<<" wrong val = "<<(firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ thirdEncValues[thirdPosition] ^ poliVal)<<endl;
            cout<<"firstEncValues["<<firstPosition<<"] = "<<firstEncValues[firstPosition]<<endl;
            cout<<"secondEncValues["<<secondPosition<<"] = "<<secondEncValues[secondPosition]<<endl;
            cout<<"thirdEncValues["<<thirdPosition<<"] = "<<thirdEncValues[thirdPosition]<<endl;
            cout<<"poliVal = "<<poliVal<<endl;
        }

    }
}

bool ObliviousDictionaryDB3Tables::hasLoop(){
    for (int position = 0; position<tableRealSize; position++) {
        if (first.bucket_size(position) > 1) {
            return true;
        }
    }
    return false;
}


void ObliviousDictionaryDB3Tables::sendData(shared_ptr<ProtocolPartyData> otherParty){
//TODO should be deleted!!
    otherParty->getChannel()->write((byte*)&firstSeed, 8);
    otherParty->getChannel()->write((byte*)&secondSeed, 8);
    otherParty->getChannel()->write((byte*)&thirdSeed, 8);
    if(print) {
        cout << "firstSeed = " << firstSeed << endl;
        cout << "secondSeed = " << secondSeed << endl;
        cout << "thirdSeed = " << thirdSeed << endl;
    }
    otherParty->getChannel()->write((byte*)keys.data(), keys.size()*8);
//TODO until here

    otherParty->getChannel()->write((byte*)firstEncValues.data(), firstEncValues.size()*8);
    otherParty->getChannel()->write((byte*)secondEncValues.data(), secondEncValues.size()*8);
    otherParty->getChannel()->write((byte*)thirdEncValues.data(), thirdEncValues.size()*8);
    int polySize = tool->getSendableDataSize();
    if(print)
        cout<<"polySize = "<<polySize/8<<endl;
    otherParty->getChannel()->write(tool->getSendableData(), polySize);
}

ObliviousDictionaryQuery3Tables::ObliviousDictionaryQuery3Tables(int hashSize, string toolType) : ObliviousDictionaryQuery(hashSize){

    auto key = prg->generateKey(128);
    prg->setKey(key);

    if (toolType.compare("poly") == 0){
        tool = new PolynomialTool(hashSize);
    }
}

void ObliviousDictionaryQuery3Tables::createSets(){
    first = unordered_set<uint64_t, Hasher>(hashSize*(1.25/3.0), Hasher(firstSeed));
    second = unordered_set<uint64_t, Hasher>(hashSize*(1.25/3.0), Hasher(secondSeed));
    third = unordered_set<uint64_t, Hasher>(hashSize*(1.25/3.0), Hasher(thirdSeed));



    tableRealSize = first.bucket_count();
    if(print)
        cout<<"tableRealSize = "<<tableRealSize<<endl;

    while(tableRealSize*3/1.25 < hashSize){
        first = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(firstSeed));
        second = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(secondSeed));
        third = unordered_set<uint64_t, Hasher>(tableRealSize + 1, Hasher(thirdSeed));

        tableRealSize = first.bucket_count();
        if(print)
            cout<<"tableRealSize = "<<tableRealSize<<endl;
    }

    first.max_load_factor(3);
    second.max_load_factor(3);
    third.max_load_factor(3);
    hashSize = tableRealSize*3.0/1.25;
    if(print)
        cout<<"new hashSize = "<<hashSize<<endl;

    tool->setHashSize(hashSize);
}

void ObliviousDictionaryQuery3Tables::readData(shared_ptr<ProtocolPartyData> otherParty){

//TODO should be deleted!!

    otherParty->getChannel()->read((byte*)&firstSeed, 8);
    otherParty->getChannel()->read((byte*)&secondSeed, 8);
    otherParty->getChannel()->read((byte*)&thirdSeed, 8);

    createSets();

    if(print) {
        cout << "firstSeed = " << firstSeed << endl;
        cout << "secondSeed = " << secondSeed << endl;
        cout << "thirdSeed = " << thirdSeed << endl;
    }
    vector<uint64_t> tmpKeys(hashSize);
    otherParty->getChannel()->read((byte*)tmpKeys.data(), tmpKeys.size()*8);

    int numKeysToCheck = 10;
    keys.resize(numKeysToCheck);
    for (int i=0; i<numKeysToCheck; i++){
        keys[i] = tmpKeys[i];//prg->getRandom32() % hashSize;
    }

    int polySize = 5*log2(hashSize)*8;
    if(print)
        cout<<"polySize = "<<polySize/8<<endl;
    byte* polynomial = new byte[polySize];

    firstEncValues.resize(tableRealSize, 0);
    secondEncValues.resize(tableRealSize, 0);
    thirdEncValues.resize(tableRealSize, 0);
//TODO until here

    otherParty->getChannel()->read((byte*)firstEncValues.data(), firstEncValues.size()*8);
    otherParty->getChannel()->read((byte*)secondEncValues.data(), secondEncValues.size()*8);
    otherParty->getChannel()->read((byte*)thirdEncValues.data(), thirdEncValues.size()*8);
    if(print)
        cout<<"before read polynomial"<<endl;
    otherParty->getChannel()->read(polynomial, polySize);
    tool->setSendableData(polynomial);
    if(print)
        cout<<"after read polynomial"<<endl;

    delete polynomial;
}

void ObliviousDictionaryQuery3Tables::calcRealValues(){

    if(print)
        cout<<"vals:"<<endl;
    uint64_t firstPosition, secondPosition, thirdPosition, val, poliVal;
    int size = keys.size();
    vector<uint64_t> vals(size);
    for (int i=0; i<size; i++){
        poliVal = tool->getValue(keys[i]);
//        Poly::evalMersenne((ZpMersenneLongElement*)&poliVal, polynomial, (ZpMersenneLongElement*)&keys[i]);
        firstPosition = first.bucket(keys[i]);
        secondPosition = second.bucket(keys[i]);
        thirdPosition = third.bucket(keys[i]);

        vals[i] = firstEncValues[firstPosition] ^ secondEncValues[secondPosition] ^ thirdEncValues[thirdPosition] ^ poliVal;
        if(print)
            cout<<"key = "<<keys[i]<<" val = "<<vals[i]<<endl;
    }

}


void ObliviousDictionaryQuery3Tables::output(){

}
