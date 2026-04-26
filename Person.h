#ifndef PERSON_H
#define PERSON_H

class Person // abstarct class
{
protected:
    int id;
    char* name;
    char* contact;
    char* password;
    int age;
    char gender;
public:
    Person(int id, const char* name, const char* contact,
        const char* password, int age, char gender);
    Person& setId(int n);
    int getId() const;
    Person& setName(const char* n);
    char* getName() const;
    Person& setContact(const char* c);
    char* getContact() const;
    Person& setPassword(const char* p);
    char* getPassword() const;
    Person& setAge(int a);
    int getAge() const;
    Person& setGender(char g);
    char getGender() const;
    virtual void display() const = 0;
    virtual const char* getRole() const = 0;
    ~Person();
};

#endif // !PERSON_H
