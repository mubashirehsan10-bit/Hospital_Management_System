#include "HospitalException.h"
#include "utility.h"

HospitalException::HospitalException(const char* msg)
{
	mystrcpy(message, msg);

}
const char* HospitalException::what() const
{
	return message;
}