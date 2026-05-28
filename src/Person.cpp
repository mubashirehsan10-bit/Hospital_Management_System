#include "Person.h"
#include "Validator.h"
#include "utility.h"
#include<iostream>
using namespace std;

Person::Person(int id, const char* name, const char* contact,
    const char* password, int age, char gender)
{
    this->id = id;
    this->name = new char[mystrlen(name) + 1];
    mystrcpy(this->name, name);
    this->contact = new char[mystrlen(contact) + 1];
    mystrcpy(this->contact, contact);
    this->password = new char[mystrlen(password) + 1];
    mystrcpy(this->password, password);
    this->age = age;
    this->gender = gender;
}
Person::Person(const Person& p)
{
    id = p.id;
    age = p.age;
    gender = p.gender;

    name = new char[mystrlen(p.name) + 1];
    mystrcpy(name, p.name);

    contact = new char[mystrlen(p.contact) + 1];
    mystrcpy(contact, p.contact);

    password = new char[mystrlen(p.password) + 1];
    mystrcpy(password, p.password);
}

Person& Person::operator=(const Person& p)
{
    if (this == &p)return *this;
    delete[] name;
    delete[] contact;
    delete[] password;

    id = p.id;
    age = p.age;
    gender = p.gender;

    name = new char[mystrlen(p.name) + 1];
    mystrcpy(name, p.name);

    contact = new char[mystrlen(p.contact) + 1];
    mystrcpy(contact, p.contact);

    password = new char[mystrlen(p.password) + 1];
    mystrcpy(password, p.password);

}



Person& Person::setId(int n) { id = n; return *this; }

int Person::getId() const { return id; }

Person& Person::setName(const char* n)
{
    delete[] name;
    this->name = new char[mystrlen(n) + 1];
    mystrcpy(this->name, n);
    return *this;
}

char* Person::getName() const{ return name;}

Person& Person::setContact(const char* c)
{
    delete[] contact;
    this->contact = new char[mystrlen(c) + 1];
    mystrcpy(this->contact, c);
    return *this;
}
char* Person::getContact() const { return contact; }
Person& Person::setPassword(const char* p)
{
    delete[] password;
    password = new char[mystrlen(p) + 1];
    mystrcpy(password, p);
    return *this;
}
char* Person::getPassword() const { return password; }
Person& Person::setAge(int a) { age = a; return*this; }
int Person::getAge() const { return age; }
Person& Person::setGender(char g) { gender = g; return *this; }
char Person::getGender() const { return gender; }
Person::~Person()
{
    if (name != nullptr) { delete[] name; name = nullptr; }
    if (contact != nullptr) { delete[] contact; contact = nullptr; }
    if (password != nullptr) { delete[] password; password = nullptr; }
}