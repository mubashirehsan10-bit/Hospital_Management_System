#ifndef PRESCRIPTION_H
#define PRESCRIPTION_H
#include<iostream>
using namespace std;
class Prescription
{
private:
	//prescription_id,appointment_id,patient_id,doctor_id,date,medicines,notes
	//	1, 1, 1, 1, 15 - 04 - 2025, Paracetamol 500mg; Amoxicillin 250mg, Take after meals
	int prescription_id;
	int appointment_id;
	int patient_id;
	int doctor_id;
	char* date;
	char* medicine;
	char* notes;
public:
	Prescription(int Pid = 0, int Aid = 0, int Ptid = 0,int Did=0 , const char* d = "",
		const char* md = "", const char* n = "");
	Prescription(const Prescription& s);
	Prescription& operator=(const Prescription& s);
	
	Prescription& setPrescriptionId(int id);
	Prescription& setAppointmentId(int id);
	Prescription& setPatientId(int id);
	Prescription& setDoctorId(int id);
	Prescription& setMedicine(const char*);
	Prescription& setNotes(const char*);
	Prescription& setDate(const char*);

	char* getMedicine()const;
	int getPatientId() const;
	int getDoctorId() const;
	int getId() const; // prescription
	int getAppointmentId() const;
	char* getPrescriptionDate() const;
	char* getPrescriptionNotes() const;

	~Prescription();

	friend ostream& operator<<(ostream& os, const Prescription& a);
	bool operator==(const Prescription& a) const;// check prescription

};


#endif // !PRESCRIPTION_H
