#ifndef VALIDATOR_H
#define VALIDATOR_H

class Validator 
{
public:
    bool isValidID(int id);
    bool isValidDate(const char* date);
    bool isValidTimeSlot(const char* slot);
    bool isValidContact(const char* contact);
    bool isValidPassword(const char* password);
    bool isValidFloat(float num);
    bool isValidMenuChoice(int choice, int min, int max);
};


#endif // !VALIDATOR_H

