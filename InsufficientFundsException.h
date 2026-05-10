#ifndef INSUFFICIENTFUNDSEXCEPTION_H
#define INSUFFICIENTFUNDSEXCEPTION_H
#include "HospitalException.h"

class InsufficientFundsException : public HospitalException {
public:
    InsufficientFundsException(const char* msg) : HospitalException(msg) {}
};

#endif