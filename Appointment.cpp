#include "Appointment.h"
#include "utility.h"


Appointment::Appointment(int Aid , int Pid, int Did , const char* d ,
	const char* ts , const char* stat )
{
	appointment_id = Aid;
	patient_id = Pid;
	doctor_id = Did;
	date = new char[mystrlen(d) + 1];
	mystrcpy(date, d);
	time_slot = new char[mystrlen(ts) + 1];
	mystrcpy(time_slot, ts);
	status = new char[mystrlen(stat) + 1];
	mystrcpy(status, stat);

}
Appointment::Appointment(const Appointment& s)
{
	appointment_id = s.appointment_id;
	patient_id = s.patient_id;
	doctor_id = s.doctor_id;

	date = new char[mystrlen(s.date) + 1]; // new date
	mystrcpy(date, s.date);

	time_slot = new char[mystrlen(s.time_slot) + 1]; // new time slot
	mystrcpy(time_slot, s.time_slot);

	status = new char[mystrlen("Pending") + 1]; // status
	mystrcpy(status, "Pending");
}
Appointment& Appointment::operator=(const Appointment& s)
{
	if (this == &s) return *this;

	appointment_id = s.appointment_id;
	patient_id = s.patient_id;
	doctor_id = s.doctor_id;

	delete[] date;
	delete[] time_slot;
	delete[] status;

	date = new char[mystrlen(s.date) + 1]; // new date
	mystrcpy(date, s.date);

	time_slot = new char[mystrlen(s.time_slot) + 1]; // new time slot
	mystrcpy(time_slot, s.time_slot);

	status = new char[mystrlen("Pending") + 1]; // status
	mystrcpy(status, "Pending");

	return *this;

}


Appointment& Appointment::setAppointmentStatus(const char* s)
{
	delete[] status;

	status = new char[mystrlen(s) + 1];
	mystrcpy(status, s);

	return *this;
}
int Appointment::getPatientId() const { return patient_id; }

int Appointment::getDoctorId() const { return doctor_id;}
Appointment& Appointment::setAppointmentId(int id)
{
	appointment_id = id;
	return *this;
}

Appointment& Appointment::setPatientId(int pid)
{
	patient_id = pid;
	return *this;
}

Appointment& Appointment::setDoctorId(int did)
{
	doctor_id = did;
	return *this;
}
char* Appointment::getAppointmentStatus()const { return status; }
int Appointment::getAppointmentId() const { return appointment_id; }
char* Appointment::getAppointmentDate() const { return date; }
Appointment& Appointment::changeDate(const char* d)
{
	delete[] date; // free memory

	date = new char[mystrlen(d) + 1];
	mystrcpy(date, d);

	return *this;
}
char* Appointment::getAppointmentSlot() const { return time_slot; }
Appointment& Appointment::changeSlot(const char* s)
{
	delete[] time_slot;

	time_slot = new char[mystrlen(s) + 1];
	mystrcpy(time_slot, s);

	return *this;

}
Appointment::~Appointment()
{
	if (date != nullptr) { delete[] date; date = nullptr; }
	if (time_slot != nullptr) { delete[] time_slot; time_slot = nullptr; }
	if (status != nullptr) { delete[] status; status = nullptr; }
}
ostream& operator<<(ostream& os, const Appointment& a)
{
	os << "\n=========Appointment Details===========\n";
	os << "\nAppointment ID: " << a.getAppointmentId();
	os << "\nPatient's ID: " << a.getPatientId();
	os << "\nDoctor's ID: " << a.getDoctorId();
	os << "\nTime Slot: " << a.getAppointmentSlot();
	os << "\nDate: " << a.getAppointmentDate();
	os << "\nStatus: " << a.getAppointmentStatus();
	os << endl;

	return os;

}
bool Appointment::operator==(const Appointment& a) const
{
	// if either is cancelled 
	if (mystrcmpIgnoreCase(status, "cancelled") == 0) return false;
	if (mystrcmpIgnoreCase(a.status, "cancelled") == 0) return false;

	// checking if same doctor, same date, same slot
	return (doctor_id == a.doctor_id &&
		mystrcmp(date, a.date) == 0 &&
		mystrcmp(time_slot, a.time_slot) == 0);
}
