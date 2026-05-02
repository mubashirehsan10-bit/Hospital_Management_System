#include "FileHandler.h"
#include<iostream>
#include "utility.h"
#include "FileNotFoundException.h"
using namespace std;

// load
void FileHandler::loadPatients(Storage<Patient>& storage)
{
	ifstream fin("patients.txt");
	if (!fin)
	{
		throw FileNotFoundException("patients.txt NOT FOUND"); // throwing exception
	}
	char line[500]; // storation of complete line

	myreadLine(fin, line, 500); //  skip attributed line line

	while (fin.eof())
	{

		// patient_id,name,age,gender,contact,password,balance


		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue; // skip empty lines

		char token[100]; // chunk of comlete line
		int pos = 0; // position of string

		getToken(line, token, pos); 
		int id = myatoi(token); // getting id

		char name[100];
		getToken(line, token, pos); // name extraction
		mystrcpy(name, token);

		getToken(line, token, pos);
		int age = myatoi(token); // age extraction

		char gender;
		getToken(line, token, pos); // gender extraction
		gender = token[0];

		char contact[100];
		getToken(line, token, pos); // conatact extraction
		mystrcpy(contact, token);

		char pass[100];
		getToken(line, token, pos); // password extraction
		mystrcpy(pass, token);

		getToken(line, token, pos);
		float balance = myatof(token);

		Patient p(id, name, contact, pass, age, gender, balance);
		storage.add(p); // patient added to storage

	}
}
void FileHandler::loadDoctors(Storage<Doctor>& storage)
{

	ifstream fin("doctors.txt");
	if (!fin)
	{
		throw FileNotFoundException("doctors.txt NOT FOUND"); // throwing exception
	}
	char line[500]; // storation of complete line

	myreadLine(fin, line, 500); //  skip attributed line line

	while (fin.eof())
	{
		//doctor_id,name,specialization,contact,password,fee

		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue; // skip empty lines

		char token[100]; // chunk of comlete line
		int pos = 0; // position of string

		getToken(line, token, pos);
		int id = myatoi(token); // getting id

		char name[100];
		getToken(line, token, pos); // name extraction
		mystrcpy(name, token);

		char spec[100];
		getToken(line, token, pos);
		mystrcpy(spec,token); // specilization extraction

		char contact[100];
		getToken(line, token, pos); // conatact extraction
		mystrcpy(contact, token);

		char pass[100];
		getToken(line, token, pos); // password extraction
		mystrcpy(pass, token);

		getToken(line, token, pos);
		float fees = myatof(token); // fee extraction

		Doctor d(id, name,contact, pass,spec,fees);
		storage.add(d); // doctor added to storage

	}
}
void FileHandler::loadAdmin(Admin*& admin) // just 1 admin that's why pointer
{
	ifstream fin("admin.txt");
	if (!fin)
	{
		throw FileNotFoundException("admin.txt NOT FOUND!!"); // throwing exception
	}
	char line[500]; // storation of complete line

	myreadLine(fin, line, 500); //  skip attributed line line

	while (fin.eof())
	{
		//admin_id, name, password

		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue; // skip empty lines

		char token[100]; // chunk of comlete line
		int pos = 0; // position of string

		getToken(line, token, pos);
		int id = myatoi(token); // getting id

		char name[100];
		getToken(line, token, pos); // name extraction
		mystrcpy(name, token);

		char pass[100];
		getToken(line, token, pos); // password extraction
		mystrcpy(pass, token);

		admin = new Admin(id, name,pass); // singular existance in whole system

	}
}
void FileHandler::loadAppointments(Storage<Appointment>& storage)
{

	ifstream fin("appointments.txt");
	if (!fin)
	{
		throw FileNotFoundException("appointments.txt NOT FOUND"); // throwing exception
	}
	char line[500]; // storation of complete line

	myreadLine(fin, line, 500); //  skip attributed line line

	while (fin.eof())
	{

		//appointment_id, patient_id, doctor_id, date, time_slot, notes

		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue; // skip empty lines

		char token[100]; // chunk of comlete line
		int pos = 0; // position of string

		getToken(line, token, pos);
		int Aid = myatoi(token); // getting id of appointment

		getToken(line, token, pos);
		int Pid = myatoi(token); // getting id of patient

		getToken(line, token, pos);
		int Did = myatoi(token); // getting id of doctor

		char date[100];
		getToken(line, token, pos); // date extraction
		mystrcpy(date, token);

		char time[100];
		getToken(line, token, pos); // Time Slot extraction
		mystrcpy(time, token);

		char status[100];
		getToken(line, token, pos); // Status extraction
		mystrcpy(status, token);

		Appointment ap(Aid, Pid, Did, date, time, status);
		storage.add(ap); // patient added to storage

	}
}
void FileHandler::loadBills(Storage<Bill>& storage)
{

	ifstream fin("appointments.txt");
	if (!fin)
	{
		throw FileNotFoundException("appointments.txt NOT FOUND"); // throwing exception
	}
	char line[500]; // storation of complete line

	myreadLine(fin, line, 500); //  skip attributed line line

	while (fin.eof())
	{

		//bill_id, patient_id, appointment_id, amount, status, date

		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue; // skip empty lines

		char token[100]; // chunk of comlete line
		int pos = 0; // position of string

		getToken(line, token, pos);
		int Bid = myatoi(token); // getting id of Bill

		getToken(line, token, pos);
		int Pid = myatoi(token); // getting id of patient

		getToken(line, token, pos);
		int Aid = myatoi(token); // getting id of appointment

		
		getToken(line, token, pos); // amount extraction
		float amount = myatof(token);

		char status[100];
		getToken(line, token, pos); // Status extraction
		mystrcpy(status, token);


		char date[100];
		getToken(line, token, pos); // Time Slot extraction
		mystrcpy(date, token);

		Bill b(Aid, Pid, Aid, amount, status, date);
		storage.add(b); // patient added to storage

	}
}
void FileHandler::loadPrescriptions(Storage<Prescription>& storage)
{
	ifstream fin("prescriptions.txt");
	if (!fin)
	{
		throw FileNotFoundException("prescriptions.txt NOT FOUND"); // throwing exception
	}
	char line[500]; // storation of complete line

	myreadLine(fin, line, 500); //  skip attributed line line

	while (fin.eof())
	{

		// prescription_id, appointment_id, patient_id, doctor_id, date, medicines, notes

		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue; // skip empty lines

		char token[100]; // chunk of comlete line
		int pos = 0; // position of string

		getToken(line, token, pos);
		int Prid = myatoi(token); // getting id of prescription

		getToken(line, token, pos);
		int Aid = myatoi(token); // getting id of appointment

		getToken(line, token, pos);
		int Pid = myatoi(token); // getting id of patient

		getToken(line, token, pos);
		int Did = myatoi(token); // getting id of doctor

		char date[100];
		getToken(line, token, pos); // date extraction
		mystrcpy(date, token);

		char medicine[100];
		getToken(line, token, pos); // medicine extraction
		mystrcpy(medicine, token);

		char notes[100];
		getToken(line, token, pos); // notes extraction
		mystrcpy(notes, token);

		Prescription pr(Prid, Aid, Pid, Did, date, medicine, notes);
		storage.add(pr); // patient added to storage

	}
}

// append
void FileHandler::appendPatient(const Patient& p)
{

}
void FileHandler::appendDoctor(const Doctor& d)
{

}
void FileHandler::appendAppointment(const Appointment& a)
{

}
void FileHandler::appendBill(const Bill& b)
{

}
void FileHandler::appendPrescription(const Prescription& p)
{

}
// security log
void FileHandler::appendSecurityLog(const char* timestamp, const char* role,
	const char* enteredId, const char* result)
{

}

// jsut pring file 
void FileHandler::readSecurityLog()
{

}


// update by ID
void FileHandler::updatePatient(int id, const Patient& p)
{

}
void FileHandler::updateAppointment(int id, const Appointment& a)
{

}
void FileHandler::updateBill(int id, const Bill& b)
{

}

// delete by ID
void FileHandler::deleteDoctor(int id)
{

}
void FileHandler::deletePatient(int id)
{

}

// discharged
void FileHandler::appendDischarged(const Patient& p)
{

}