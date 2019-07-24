//
// Created by moriya on 7/17/19.
//

#ifndef BENNYPROJECT_EXTENDEDUNORDEREDSET_H
#define BENNYPROJECT_EXTENDEDUNORDEREDSET_H


class ExtendedUnorderedSet {
private:
    int hashSize;
    unsigned int realSize;
    vector<unordered_set<uint32_t>> set;

public:
    ExtendedUnorderedSet(int hashSize) hashSize(hashSize){
        set.resize(hashSize);
    }

    uint32_t insertKey(uint32_t key){
        uint32_t index = key % realSize;
        set[index].insert(key);
        return index;
    }

    int bucketSize(int index){
        return set[index].size();
    }

    uint32_t emptyBucket(int index){
        set[index].
        set[index].clear();
    }
};


#endif //BENNYPROJECT_EXTENDEDUNORDEREDSET_H
