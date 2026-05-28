#ifndef BILL_H
#define BILL_H
#include<iostream>
using namespace std;

class Bill
{
	//bill_id,patient_id,appointment_id,amount,status,date
	//1, 1, 1, 1500.00, paid, 15 - 04 - 2025
private:
	int appointment_id;
	int bill_id;
	int patient_id;
	float amount;
	char* status;
	char* date;
public:
	Bill(int Aid = 0, int Bid = 0, int Pid = 0, float am =0.0f,const char* st = "", const char* d = "");
	Bill(const Bill& b);
	Bill& operator=(const Bill& b);
	Bill& setStatus(const char* s);
	Bill& setAmount(float amount);
    Bill& setAppointmentId(int id);
	Bill& setPatientId(int id);
	Bill& setBillId(int id);
	Bill& setBillDate(const char* d);
	char* getAppointmentDate() const;
	char* getStatus()const;
	float getAmount() const;
	int getPatientId() const;
	int getAppointmentId() const;
	int getId() const;
	friend ostream& operator<<(ostream& os, const Bill& b);
	bool operator==(const Bill& b)const; // checks bill id similarity
	~Bill();


};

#endif // !BILL_H
