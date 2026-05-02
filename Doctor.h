#ifndef DOCTOR_H
#define DOCTOR_H
#include "Person.h"
#include<iostream>
using namespace std;

class Doctor :public Person
{
private:
	char* specialization;
	float fees;
public:
	Doctor(int id, const char* name, const char* contact,
		const char* password, const char* spec,float fees);
	Doctor(const Doctor& d);
	char* getSpecialization() const;
	float getFees() const;
	Doctor& setSpecialization(const char* s);
	Doctor& setFees(float fees);
	bool operator==(const Doctor& d) const; 
	friend ostream& operator<<(ostream& os, const Doctor& d);
	void display() const override;
	const char* getRole() const override;
	~Doctor();
	
};

#endif // !DOCTOR_H
