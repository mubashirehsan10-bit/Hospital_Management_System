#include "utility.h"
#include "Doctor.h"

Doctor::Doctor(int id, const char* name, const char* contact,
	const char* password, const char* spec, float fees)
	:Person(id, name, contact, password)
{
	specialization = new char[mystrlen(spec) + 1];
	mystrcpy(specialization, spec);
	this->fees = fees;
}
Doctor::Doctor(const Doctor& d) :Person(d)
{
	specialization = new char[mystrlen(d.specialization) + 1];
	mystrcpy(specialization, d.specialization);
	fees = d.fees;
}
char* Doctor::getSpecialization() const { return specialization; }
float Doctor::getFees() const { return fees; }
Doctor& Doctor::setSpecialization(const char* s)
{
	delete[] specialization;
	specialization = new char[mystrlen(s) + 1];
	mystrcpy(specialization, s);
	return *this;
}
Doctor& Doctor::setFees(float fees) { this->fees = fees; return *this; }
bool Doctor::operator==(const Doctor& d) const
{ 
	return id == d.id;
}
ostream& operator<<(ostream& os, const Doctor& d)
{
	os << "\n======Dr." << d.getName() << " Details======";
	os << "\nID: " << d.getId();
	os << "\nContact: " << d.getContact();
	os << "\nAge: " << d.getAge();
	os << "\nGender: " << d.getGender();
	os << "\nFees: " << d.getFees();
	os << "\nSpecialization: " << d.getSpecialization() << endl << endl;
	return os;

}
void Doctor::display() const { cout << *this; }
const char* Doctor::getRole() const { return "Doctor"; }
Doctor::~Doctor() { delete[] specialization; }