#include "Prescription.h"
#include "utility.h"



Prescription::Prescription(int Pid , int Aid, int Ptid, int Did, const char* d ,
	const char* md , const char* n )
	:prescription_id(Pid), appointment_id(Aid), patient_id(Ptid),doctor_id(Did)
{
	date = new char[mystrlen(d) + 1];
	mystrcpy(date, d);

	medicine = new char[mystrlen(md) + 1];
	mystrcpy(medicine, md);

	notes = new char[mystrlen(n) + 1];
	mystrcpy(notes, n);
}
Prescription::Prescription(const Prescription& s)
	:prescription_id(s.prescription_id), appointment_id(s.appointment_id)
	, patient_id(s.patient_id), doctor_id(s.doctor_id)
{
	date = new char[mystrlen(s.date) + 1];
	mystrcpy(date, s.date);

	medicine = new char[mystrlen(s.medicine) + 1];
	mystrcpy(medicine, s.medicine);

	notes = new char[mystrlen(s.notes) + 1];
	mystrcpy(notes, s.notes);
}

Prescription& Prescription::setPrescriptionId(int id)
{
	prescription_id = id;
	return *this;
}
Prescription& Prescription::setAppointmentId(int id)
{
	appointment_id = id;
	return *this;
}
Prescription& Prescription::setPatientId(int id)
{
	patient_id = id;
	return *this;
}
Prescription& Prescription::setDoctorId(int id)
{
	doctor_id = id;
	return *this;
}
Prescription& Prescription::setMedicine(const char* m)
{
	delete[] medicine;
	medicine = new char[mystrlen(m) + 1];
	mystrcpy(medicine,m);

	return *this;
}
Prescription& Prescription::setNotes(const char* n)
{
	delete[] notes;
	notes = new char[mystrlen(n) + 1];
	mystrcpy(notes, n);

	return *this;
}
Prescription& Prescription::setDate(const char* d)
{
	delete[] date;
	date = new char[mystrlen(d) + 1];
	mystrcpy(date, d);

	return *this;
}

char* Prescription::getMedicine()const { return medicine; }
int Prescription::getPatientId() const { return patient_id; }
int Prescription::getDoctorId() const { return doctor_id; }
int Prescription::getId() const { return prescription_id;} // prescription
int Prescription::getAppointmentId() const { return appointment_id; }
char* Prescription::getPrescriptionDate() const { return date; }
char* Prescription::getPrescriptionNotes() const { return notes; }

Prescription::~Prescription()
{
	delete[] notes;
	delete[] medicine;
	delete[] date;
}

ostream& operator<<(ostream& os, const Prescription& a)
{
	os << "==========Prescription Id: " << a.getId() << "==========";
	os << "\nPatient Id: " << a.getPatientId();
	os << "\nAppointment Id: " << a.getAppointmentId();
	os << "\nDoctor Id: " << a.getDoctorId();
	os << "\nMedicine: " << a.getMedicine();
	os << "\nNotes: " << a.getPrescriptionNotes();
	os << "\nDate: " << a.getPrescriptionDate();
	os << endl;

	return os;
}
bool Prescription::operator==(const Prescription& a) const// check prescription
{
	return prescription_id == a.prescription_id;

}
