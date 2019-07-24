//
// Created by moriya on 7/24/19.
//

#include "ProtocolParty.h"

ProtocolParty::ProtocolParty(int argc, char* argv[]) : Protocol("ObliviousDictionary", argc, argv)
{

    partyId = stoi(this->getParser().getValueByKey(arguments, "partyID"));

    this->times = stoi(this->getParser().getValueByKey(arguments, "internalIterationsNumber"));
    this->hashSize = stoi(this->getParser().getValueByKey(arguments, "hashSize"));

    vector<string> subTaskNames{"Online"};
    timer = new Measurement(*this, subTaskNames);

    MPCCommunication comm;
    string partiesFile = this->getParser().getValueByKey(arguments, "partiesFile");

//    otherParty = comm.setCommunication(io_service, partyId, 2, partiesFile)[0];
//
//    string tmp = "init times";
//    //cout<<"before sending any data"<<endl;
//    byte tmpBytes[20];
//    if (otherParty->getID() < partyId){
//        otherParty->getChannel()->write(tmp);
//        otherParty->getChannel()->read(tmpBytes, tmp.size());
//    } else {
//        otherParty->getChannel()->read(tmpBytes, tmp.size());
//        otherParty->getChannel()->write(tmp);
//    }

}

void ProtocolParty::run() {

    for (iteration=0; iteration<times; iteration++){

        auto t1start = high_resolution_clock::now();
        timer->startSubTask("Online", iteration);
        runOnline();
        timer->endSubTask("Online", iteration);

        auto t2end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(t2end-t1start).count();

        cout << "time in milliseconds for protocol: " << duration << endl;
    }


}

void DBParty::init(int firstSeed, int secondSeed){

    dic = new ObliviousDictionaryDB(hashSize, firstSeed, secondSeed);
}

void DBParty::runOnline() {

    auto start = high_resolution_clock::now();
    auto t1 = high_resolution_clock::now();

    dic->fillTables();
    auto t2 = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(t2-t1).count();
    cout << "fillTables took in milliseconds: " << duration << endl;

    t1 = high_resolution_clock::now();
    dic->peeling();

    t2 = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(t2-t1).count();
    cout << "peeling took in milliseconds: " << duration << endl;

    t1 = high_resolution_clock::now();
    dic->calcPolynomial();
    t2 = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(t2-t1).count();
    cout << "calcPolinomial took in milliseconds: " << duration << endl;

    t1 = high_resolution_clock::now();
    dic->unpeeling();

    t2 = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(t2-t1).count();
    cout << "unpeeling took in milliseconds: " << duration << endl;

    t1 = high_resolution_clock::now();
    //dic->send();

    t2 = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(t2-t1).count();
    cout << "send took in milliseconds: " << duration << endl;

    auto end = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(end-start).count();
    cout << "all protocol took in milliseconds: " << duration << endl;

    dic->checkOutput();

}

void QueryParty::init(int firstSeed, int secondSeed){

    dic = new ObliviousDictionaryQuery(hashSize, firstSeed, secondSeed);
}


void QueryParty::runOnline() {

    auto start = high_resolution_clock::now();
    auto t1 = high_resolution_clock::now();

    dic->readData(otherParty);
    auto t2 = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(t2-t1).count();
    cout << "fillTables took in milliseconds: " << duration << endl;


    auto end = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(end-start).count();
    cout << "all protocol took in milliseconds: " << duration << endl;

    dic->calcRealValues();

}

