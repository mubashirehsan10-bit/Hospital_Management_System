#ifndef ADMIN_H
#define ADMIN_H
#include "Person.h"
#include<iostream>
using namespace std;

class Admin : public Person
{
public:
	Admin(int id = 0, const char* name = "",
		const char* password = "");
	Admin(const Admin& a);
	Admin& operator=(const Admin& d);
	bool operator==(const Admin& a) const;
	friend ostream& operator<<(ostream& os, const Admin& a);
	void display() const override;
	const char* getRole() const override;
};


#endif // !ADMIN_H
