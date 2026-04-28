#ifndef PATIENT_H
#define PATIENT_H
#include "Person.h"
#include <iostream>
using namespace std;

class Patient:public Person
{
private:
	float balance;
public:
    Patient(int id, const char* name, const char* contact,
        const char* password, int age, char gender, float balance);
    Patient(const Patient& p);
    float getBalance() const;
    Patient& setBalance(float b);
    Patient& operator+=(float amount);
    Patient& operator-=(float amount);
    bool operator==(const Patient& p) const;
    friend ostream& operator<<(ostream& os, const Patient& p);
    void display() const override;
    const char* getRole() const override;

};


#endif // !
