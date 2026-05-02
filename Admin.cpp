#include "Admin.h"

Admin::Admin(int id, const char* name,
	const char* password):Person(id,name,password){ }
Admin::Admin(const Admin& a):Person(a){}
bool Admin::operator==(const Admin& a) const
{
	return id == a.id;
}
ostream& operator<<(ostream& os, const Admin& a)
{
	os << "\n======Admin(" << a.getName() << ") Details======";
	os << "\nID: " << a.getId();
	os << "\nContact: " << a.getContact();
	os << "\nAge: " << a.getAge();
	os << "\nGender: " << a.getGender() << endl << endl;
	return os;

}
void Admin::display() const { cout << *this; }
const char* Admin::getRole() const { return "Admin"; }