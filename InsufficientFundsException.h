#ifndef INSUFFICIENTFUNDSEXCEPTION_H
#define INSUFFICIENTFUNDSEXCEPTION_H
#include "HospitalException.h"

class InsufficentFundsException : public HospitalException {
public:
    InsufficentFundsException(const char* msg) : HospitalException(msg) {}
};

#endif