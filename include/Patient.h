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
    Patient(int id = 0, const char* name ="", const char* contact ="",
        const char* password = "", int age = 0, char gender = ' ', float balance = 0.0f);
    Patient(const Patient& p);
    float getBalance() const;
    Patient& operator=(const Patient& other);

    Patient& setBalance(float b);
    Patient& operator+=(float amount);
    Patient& operator-=(float amount);
    bool operator==(const Patient& p) const;
    friend ostream& operator<<(ostream& os, const Patient& p);
    void display() const override;
    const char* getRole() const override;
    ~Patient();
};


#endif // !
