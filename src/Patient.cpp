#include "Patient.h"
#include "utility.h"

Patient::Patient(int id, const char* name, const char* contact,
    const char* password, int age, char gender, float balance)
    :Person(id, name, contact, password, age, gender)
{
    this->balance = balance;
}
Patient::Patient(const Patient& p):Person(p)
{
    balance = p.balance;
}
Patient& Patient::operator=(const Patient& other)
{
    if (this == &other) return *this;  // self assignment check

    Person::operator=(other);  // call parent first, so that all mirrored parameters are assigned perfectly

    // copy new memory
    balance = other.balance;   // then copy own members

    return *this;
}
float Patient::getBalance() const { return balance; }
Patient& Patient::setBalance(float b) { balance = b; return *this; }
Patient& Patient::operator+=(float amount) { balance += amount; return *this; }
Patient& Patient::operator-=(float amount) { balance -= amount; return *this; }
bool Patient::operator==(const Patient& p) const
{
    return id == p.id;
}
ostream& operator<<(ostream& os, const Patient& p)
{
    os << "\n======Patient(" << p.getName() << ") Details======";
    os << "\nID: " << p.getId();
    os << "\nContact: " << p.getContact();
    os << "\nAge: " << p.getAge();
    os << "\nGender: " << p.getGender();
    os << "\nBalance: " << p.getBalance() << endl;

    return os;
}
void Patient::display() const 
{
    cout << *this;
}
const char* Patient::getRole() const
{
    return "Patient";
}
Patient::~Patient()
{
    if (name != nullptr) { delete[] name; name = nullptr; }
    if (contact != nullptr) { delete[] contact; contact = nullptr; }
    if (password != nullptr) { delete[] password; password = nullptr; }
}