#include "Bill.h"
#include "utility.h"

Bill::Bill(int Aid, int Bid, int Pid, float am,const char* st, const char* d)
	:appointment_id(Aid), bill_id(Bid), patient_id(Pid), amount(am)
{
	status = new char[mystrlen(st)+1];
	mystrcpy(status, st);

	date = new char[mystrlen(d) + 1];
	mystrcpy(date, d);
}
Bill::Bill(const Bill& b) :appointment_id(b.appointment_id),
bill_id(b.bill_id), patient_id(b.patient_id), amount(b.amount)
{
	status = new char[mystrlen(b.status) + 1];
	mystrcpy(status, b.status);

	date = new char[mystrlen(b.date) + 1];
	mystrcpy(date, b.date);
}
Bill& Bill::operator=(const Bill& b)
{
	if (this == &b)return *this;

	appointment_id = b.appointment_id;
	bill_id = b.bill_id; 
	patient_id = b.patient_id;
	amount = b.amount;

	delete[] status;
	delete[] date;

	status = new char[mystrlen(b.status) + 1];
	mystrcpy(status, b.status);

	date = new char[mystrlen(b.date) + 1];
	mystrcpy(date, b.date);

	return *this;
}


Bill& Bill::setStatus(const char* s)
{
	delete[] status;
	status = new char[mystrlen(s) + 1];
	mystrcpy(status, s);
	return *this;
}
Bill& Bill::setAmount(float amount)
{
	this->amount = amount;
	return *this;
}
Bill& Bill::setAppointmentId(int id)
{
	appointment_id = id;
	return *this;
}
Bill& Bill::setPatientId(int id)
{
	patient_id = id;
	return *this;
}
Bill& Bill::setBillId(int id)
{
	bill_id = id;
	return *this;

}
Bill& Bill::setBillDate(const char* d)
{
	delete[] date;
	date = new char[mystrlen(d) + 1];
	mystrcpy(date, d);
	return *this;

}
char* Bill::getAppointmentDate() const { return date; }
char* Bill::getStatus()const { return status; }
float Bill::getAmount() const { return amount; }
int Bill::getPatientId() const { return patient_id; }
int Bill::getAppointmentId() const { return appointment_id; }
int Bill::getId() const { return bill_id; }
ostream& operator<<(ostream& os, const Bill& b)
{
	os << "\n==========Bill Id: " << b.getId() << "============";
	os << "\nAppointment Id: " << b.getAppointmentId();
	os << "\nPatient Id: " << b.getPatientId();
	os << "\nAmount: " << b.getAmount();
	os << "\nDate: " << b.getAppointmentDate() << endl;
	return os;
}
bool Bill::operator==(const Bill& b) const // checks bill id similarity
{
	return bill_id == b.bill_id;
}
Bill::~Bill()
{
	delete[] status;
	delete[] date;
}
