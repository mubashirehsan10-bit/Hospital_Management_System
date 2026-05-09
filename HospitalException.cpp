#include "HospitalException.h"
#include "utility.h"
#include<iostream>
using namespace std;

HospitalException::HospitalException(const char* msg)
{
	mystrcpy(message, msg);
}
const char* HospitalException::what() const
{
	return message;
}