#include "Validator.h"
#include "utility.h"
#include <ctime>

bool Validator::isValidID(int id)
{
	return id > 0;
}
bool Validator::isValidDate(const char* date)
{
    // checking length
    if (mystrlen(date) != 10)
        return false;

    // year
    int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 +
        (date[8] - '0') * 10 + (date[9] - '0');
    if (year < getCurrentCalendarYear())
        return false;

    // day
    if (date[0] < '0' || date[0] > '3')
        return false;
    if (date[0] == '3') {
        if (date[1] < '0' || date[1] > '1')
            return false;
    }
    else {
        if (date[1] < '0' || date[1] > '9')
            return false;
    }  

    // dashes
    if (date[2] != '-' || date[5] != '-')
        return false;

    // month
    if (date[3] == '0' && date[4] == '0')
        return false;
    if (date[3] < '0' || date[3] > '1')
        return false;
    if (date[3] == '1')
        if (date[4] < '0' || date[4] > '2')
            return false;
    if (date[4] < '1' || date[4] > '9')
        return false;

    return true;
}
bool Validator::isValidTimeSlot(const char* slot)
{
	if (mystrlen(slot) != 5)
		return false;

    // checking all time slots
	return (((slot[0] == '0' && slot[1] == '9') or (slot[0] == '1' && slot[1] == '0') or
		(slot[0] == '1' && slot[1] == '1')
		or (slot[0] == '1' && slot[1] == '2') or (slot[0] == '1' && slot[1] == '3')
		or (slot[0] == '1' && slot[1] == '4') or (slot[0] == '1' && slot[1] == '5')
		or (slot[0] == '1' && slot[1] == '6')) && slot[2] == ':' &&
		slot[3] == '0' && slot[4] == '0');
}
bool Validator::isValidContact(const char* contact)
{
	bool flag = false;
	if (mystrlen(contact) == 11)
		flag = true;

	int i = 0;
	while (contact[i] != '\0') {
		if (contact[i] < '0' or contact[i]>'9')
			flag = false;
		i++;
	}

	return flag;
}
bool Validator::isValidPassword(const char* password)
{
	return mystrlen(password) >= 6;
}
bool Validator::isValidFloat(float num)
{
	return num > 0;
}
bool Validator::isValidMenuChoice(int choice, int min, int max)
{
	return choice >= min && choice <= max;
}