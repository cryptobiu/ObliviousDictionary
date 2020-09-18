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

    reportStatistics = stoi(this->getParser().getValueByKey(arguments, "reportStatistics"));




    if(reportStatistics==0) {
    otherParty = comm.setCommunication(io_service, partyId, 2, partiesFile)[0];


        string tmp = "init times";
        //cout<<"before sending any data"<<endl;
        byte tmpBytes[20];
        if (otherParty->getID() < partyId) {
            otherParty->getChannel()->write(tmp);
            otherParty->getChannel()->read(tmpBytes, tmp.size());
        } else {
            otherParty->getChannel()->read(tmpBytes, tmp.size());
            otherParty->getChannel()->write(tmp);
        }
    }

}

ProtocolParty::~ProtocolParty(){

    delete timer;
}

void ProtocolParty::run() {

    auto total_duration = 0;
    for (iteration=0; iteration<times; iteration++){

        auto t1start = high_resolution_clock::now();
        timer->startSubTask("Online", iteration);
        runOnline();
        timer->endSubTask("Online", iteration);

        auto t2end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(t2end-t1start).count();

        total_duration += duration;
        
    }
    cout << "average time per run =  " << (float)total_duration/(float)times << " milliseconds" << endl;


}

DBParty::DBParty(int argc, char *argv[]): ProtocolParty(argc, argv){

    auto version = this->getParser().getValueByKey(arguments, "version");
    auto tool = this->getParser().getValueByKey(arguments, "tool");

    auto batchSize = stoi(this->getParser().getValueByKey(arguments, "batchSize"));

    auto tableRatio = stof(this->getParser().getValueByKey(arguments, "tableRatio"));


    auto processId = this->getParser().getValueByKey(arguments, "processId");


    if (version.compare("2Tables") == 0) {
        dic = new ObliviousDictionaryDB2Tables(hashSize, tool);
    } else if (version.compare("3Tables") == 0) {
        dic = new ObliviousDictionaryDB3Tables(hashSize, tool,batchSize, processId, tableRatio);
    }
    dic->setReportStatstics(reportStatistics);

}

DBParty::~DBParty(){
    delete dic;
}

void DBParty::runOnline() {
        

        dic->init();

        // auto start = high_resolution_clock::now();
        // auto t1 = high_resolution_clock::now();

        dic->fillTables();
        // auto t2 = high_resolution_clock::now();

        // duration = duration_cast<milliseconds>(t2 - t1).count();
        // cout << "fillTables took in milliseconds: " << duration << endl;

        // t1 = high_resolution_clock::now();
        dic->peeling();

        // t2 = high_resolution_clock::now();

        // duration = duration_cast<milliseconds>(t2 - t1).count();
        // cout << "peeling took in milliseconds: " << duration << endl;

        if (reportStatistics == 1)
            dic->updateIteration(iteration);

        // if (reportStatistics == 0) {
        //     t1 = high_resolution_clock::now();
        //     dic->generateExternalToolValues();
        //     t2 = high_resolution_clock::now();

        //     duration = duration_cast<milliseconds>(t2 - t1).count();
        //     cout << "calcPolinomial took in milliseconds: " << duration << endl;


        //     t1 = high_resolution_clock::now();
        //     dic->unpeeling();

        //     t2 = high_resolution_clock::now();

        //     duration = duration_cast<milliseconds>(t2 - t1).count();
        //     cout << "unpeeling took in milliseconds: " << duration << endl;

        //     t1 = high_resolution_clock::now();
        //     dic->sendData(otherParty);

        //     t2 = high_resolution_clock::now();

        //     duration = duration_cast<milliseconds>(t2 - t1).count();
        //     cout << "send took in milliseconds: " << duration << endl;

        //     auto end = high_resolution_clock::now();

        //     duration = duration_cast<milliseconds>(end - start).count();
        //     cout << "all protocol took in milliseconds: " << duration << endl;

        //     dic->checkOutput();
        // }
}

QueryParty::QueryParty(int argc, char *argv[]) : ProtocolParty(argc, argv){

    auto version = this->getParser().getValueByKey(arguments, "version");
    auto tool = this->getParser().getValueByKey(arguments, "tool");

    if (version.compare("2Tables") == 0) {
        dic = new ObliviousDictionaryQuery2Tables(hashSize, tool);
    } else if (version.compare("3Tables") == 0) {
        dic = new ObliviousDictionaryQuery3Tables(hashSize, tool);
    }
}

QueryParty::~QueryParty() {
    delete dic;
}



void QueryParty::runOnline() {

    auto start = high_resolution_clock::now();
    auto t1 = high_resolution_clock::now();

    dic->readData(otherParty);
    auto t2 = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(t2-t1).count();
    cout << "read data took in milliseconds: " << duration << endl;

    t1 = high_resolution_clock::now();
    dic->calcRealValues();
    t2 = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(t2-t1).count();
    cout << "calc real values took in milliseconds: " << duration << endl;

    auto end = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(end-start).count();
    cout << "all protocol took in milliseconds: " << duration << endl;
}

