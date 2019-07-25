//
// Created by moriya on 7/17/19.
//

#include "ObliviousDictionary.h"

uint64_t ObliviousDictionary::getPolynomialValue(uint64_t key){
    ZpMersenneLongElement b;
    Poly::evalMersenne(&b, polynomial, (ZpMersenneLongElement*)&key);

    return *(uint64_t*)&b;
}
