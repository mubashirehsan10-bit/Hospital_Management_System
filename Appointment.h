#ifndef APPOINTMENT_H
#define APPOINTMENT_H
#include<iostream>
using namespace std;
//appointment_id,patient_id,doctor_id,date,time_slot,status
//1, 1, 1, 15 - 04 - 2025, 09:00, completed

class Appointment
{
private:
	int appointment_id;
	int patient_id;
	int doctor_id;
	char* date;
	char* time_slot;
	char* status;
public:
	Appointment(int Aid = 0, int Pid = 0, int Did = 0, const char* d = "",
		const char* ts = "", const char* stat = "no-show");
    Appointment(const Appointment& s);
	Appointment& operator=(const Appointment& s);

	int getPatientId() const;
	int getDoctorId() const;
	Appointment& setAppointmentId(int id);
	Appointment& setPatientId(int pid);
	Appointment& setDoctorId(int did);
	Appointment& setAppointmentStatus(const char* );
	char* getAppointmentStatus()const;
	int getAppointmentId() const;
	char* getAppointmentDate() const;
	Appointment& changeDate(const char*);
	char* getAppointmentSlot() const;
	Appointment& changeSlot(const char*);
	~Appointment();
	friend ostream& operator<<(ostream& os, const Appointment& a);
	bool operator==(const Appointment& a) const;// timeslot,id,and date and status checker

};


#endif // !APPOINTMENT_H
