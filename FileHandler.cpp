#include "FileHandler.h"
#include<iostream>
#include "utility.h"
#include "FileNotFoundException.h"
#include<cstdio>
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

	while (!fin.eof())
	{

		// patient_id,name,pid,gender,contact,password,fee


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
		int age = myatoi(token); // pid extraction

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

	while (!fin.eof())
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

	while (!fin.eof())
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

	while (!fin.eof())
	{

		//appointment_id, patient_id, doctor_id, date, time_slot, status

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

	ifstream fin("bills.txt");
	if (!fin)
	{
		throw FileNotFoundException("bills.txt NOT FOUND"); // throwing exception
	}
	char line[500]; // storation of complete line

	myreadLine(fin, line, 500); //  skip attributed line line

	while (!fin.eof())
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

		Bill b(Bid, Pid, Aid, amount, status, date);
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

	while (!fin.eof())
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
	ofstream fout("patients.txt", ios::app);
	if (!fout)
		throw FileNotFoundException("patients.txt not found!");

	//		// patient_id,name,pid,gender,contact,password,aid

	// convert numbers to strings
	char id[10], age[10], balance[20];
	myitoa(p.getId(), id);
	myitoa(p.getAge(), age);
	myftoa(p.getBalance(), balance);

	// write comma separated
	fout << id << ","
		<< p.getName() << ","
		<< age << ","
		<< p.getGender() << ","
		<< p.getContact() << ","
		<< p.getPassword() << ","
		<< balance << "\n";

	fout.close();
}
void FileHandler::appendDoctor(const Doctor& d)
{
	//doctor_id,name,specialization,contact,password,fee

	ofstream fout("doctors.txt", ios::app);
	if (!fout)
		throw FileNotFoundException("doctors.txt not found!");

	// convert numbers to strings
	char id[10],fee[20];
	myitoa(d.getId(), id);
	myftoa(d.getFees(), fee);

	// write comma separated
	fout << id << ","
		<< d.getName() << ","
		<< d.getSpecialization() << ","
		<< d.getContact() << ","
		<< d.getPassword() << ","
		<< fee << "\n";

	fout.close();

}
void FileHandler::appendAppointment(const Appointment& a)
{
	//appointment_id, patient_id, doctor_id, date, time_slot, Status
	ofstream fout("appointments.txt", ios::app);
	if (!fout)
		throw FileNotFoundException("appointments.txt not found!");

	// convert numbers to strings
	char Aid[10], Pid[20], Did[20];
	myitoa(a.getAppointmentId(), Aid);
	myftoa(a.getPatientId(), Pid);
	myftoa(a.getDoctorId(), Did);


	// write comma separated
	fout << Aid << ","
		<< Pid << ","
		<< Did << ","
		<< a.getAppointmentDate() << ","
		<< a.getAppointmentSlot() << ","
		<< a.getAppointmentStatus() <<"\n";

	fout.close();

}
void FileHandler::appendBill(const Bill& b)
{
	//bill_id, patient_id, appointment_id, amount, status, date
	ofstream fout("bills.txt", ios::app);
	if (!fout)
		throw FileNotFoundException("bills.txt not found!");

	// convert numbers to strings
	char Bid[10], Pid[20], Aid[20], amnt[20];
	myitoa(b.getId(), Bid);
	myitoa(b.getPatientId(), Pid);
	myitoa(b.getAppointmentId(), Aid);
	myitoa(b.getAmount(), amnt);

	// write comma separated
	fout << Bid << ","
		<< Pid << ","
		<< Aid << ","
		<< amnt << ","
		<< b.getStatus() << ","
		<< b.getAppointmentDate() << "\n";

	fout.close();


}
void FileHandler::appendPrescription(const Prescription& p)
{
	//prescription_id, appointment_id, patient_id, doctor_id, date, medicines, notes
	ofstream fout("prescriptions.txt", ios::app);
	if (!fout)
		throw FileNotFoundException("prescriptions.txt not found!");

	// convert numbers to strings
	char Prid[10], Aid[20],Pid[20],Did[20];
	myitoa(p.getId(), Prid);
	myitoa(p.getAppointmentId(), Aid);
	myitoa(p.getPatientId(), Pid);
	myitoa(p.getDoctorId(), Did);

	// write comma separated
	fout << Prid << ","
		<< Aid << ","
		<< Aid << ","
		<< Did << ","
		<< p.getPrescriptionDate() << ","
		<< p.getMedicine() << ","
		<< p.getPrescriptionNotes() << "\n";

	fout.close();


}
// security log
void FileHandler::appendSecurityLog(const char* timestamp, const char* role,
	const char* enteredId, const char* result)
{
	ofstream fout("security_log.txt", ios::app);
	if (!fout)
		throw FileNotFoundException("security_log.txt not found!");

	// write comma separated
	fout << timestamp << ","
		<< role << ","
		<< enteredId << ","
		<< result << "\n";

	fout.close();

}

// just printing file 
void FileHandler::readSecurityLog()
{
	ifstream Fread("security_log.txt");
	if (!Fread)
		throw FileNotFoundException("security_log.txt NOT FOUND!!!");


	char line[500];

	while (!Fread.eof()) {
		myreadLine(Fread, line, 500);
		cout << line << endl;   // print on SFML Screen
	}

	Fread.close();

}


// update by ID
void FileHandler::updatePatient(int id, const Patient& p)
{
	ifstream fin("patients.txt");
	ofstream fout("temp.txt");

	char line[500];
	myreadLine(fin, line, 500);
	fout << line << "\n";  // write header

	while (!fin.eof())
	{
		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue;

		char token[100];
		int pos = 0;
		getToken(line, token, pos);
		int currentId = myatoi(token);

		if (currentId == id)
		{
			// write updated record
			char sid[10], age[10], balance[20];
			myitoa(p.getId(), sid);
			myitoa(p.getAge(), age);
			myftoa(p.getBalance(), balance);
			fout << sid << "," << p.getName() << ","
				<< age << "," << p.getGender() << ","
				<< p.getContact() << "," << p.getPassword() << ","
				<< balance << "\n";
		}
		else
			fout << line << "\n";  // write unchanged record
	}

	fin.close();
	fout.close();
	remove("patients.txt");
	rename("temp.txt", "patients.txt");
}
void FileHandler::updateAppointment(int id, const Appointment& a)
{
	ifstream fin("appointments.txt");
	ofstream fout("temp.txt");

	//appointment_id, patient_id, doctor_id, date, time_slot, Status

	char line[500];
	myreadLine(fin, line, 500);
	fout << line << "\n";  // write header

	while (!fin.eof())
	{
		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue;

		char token[100];
		int pos = 0;
		getToken(line, token, pos);
		int currentId = myatoi(token);

		if (currentId == id)
		{
			// write updated record
			char aid[10], pid[10], did[20];
			myitoa(a.getAppointmentId(), aid);
			myitoa(a.getPatientId(), pid);
			myftoa(a.getDoctorId(), did);
			fout << aid << "," << pid << "," << did 
				<< a.getAppointmentDate() << ","
				<< a.getAppointmentSlot() << ","
				<< a.getAppointmentStatus() <<"\n";
		}
		else
			fout << line << "\n";  // write unchanged record
	}

	fin.close();
	fout.close();
	remove("appointments.txt");
	rename("temp.txt", "appointments.txt");

}
void FileHandler::updateBill(int id, const Bill& b)
{
	ifstream fin("bills.txt");
	ofstream fout("temp.txt");

	//bill_id,patient_id,appointment_id,amount,status,date

	char line[500];
	myreadLine(fin, line, 500);
	fout << line << "\n";  // write header

	while (!fin.eof())
	{
		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue;

		char token[100];
		int pos = 0;
		getToken(line, token, pos);
		int currentId = myatoi(token);

		if (currentId == id)
		{
			// write updated record
			char bid[10], pid[10], aid[20], amnt[20];
			myitoa(b.getId(), bid);
			myitoa(b.getPatientId(), pid);
			myftoa(b.getAppointmentId(), aid);
			myftoa(b.getAmount(), amnt);
			fout << bid << "," << pid << "," << aid << "," << amnt
				<< b.getStatus() << ","
				<< b.getAppointmentDate() << "\n";
		}
		else
			fout << line << "\n";  // write unchanged record
	}

	fin.close();
	fout.close();
	remove("bills.txt");
	rename("temp.txt", "bills.txt");

}

// delete by ID
void FileHandler::deleteDoctor(int id)
{
	ifstream fin("doctors.txt");
	if (!fin)
		throw FileNotFoundException("doctors.txt NOT FOUND!!!");

	ofstream fout("temp.txt");

	char line[500];
	myreadLine(fin, line, 500); // skip header
	fout << line << "\n";       // write header to temp

	while (!fin.eof())
	{
		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue;

		// extract ID from line
		char token[100];
		int pos = 0;
		getToken(line, token, pos);
		int currentId = myatoi(token);

		// only write if ID doesn't match
		if (currentId != id)
			fout << line << "\n";
	}

	fin.close();
	fout.close();

	remove("doctors.txt");
	rename("temp.txt", "doctors.txt");

}
void FileHandler::deletePatient(int id)
{
	ifstream fin("patients.txt");
	if (!fin)
		throw FileNotFoundException("patients.txt not found!");

	ofstream fout("temp.txt");

	char line[500];
	myreadLine(fin, line, 500); // skip header
	fout << line << "\n";       // write header to temp

	while (!fin.eof())
	{
		myreadLine(fin, line, 500);
		if (mystrlen(line) == 0) continue;

		// same as above extraction
		char token[100];
		int pos = 0;
		getToken(line, token, pos);
		int currentId = myatoi(token);

		if (currentId != id) // only write if ID doesn't match
			fout << line << "\n";
	}

	fin.close();
	fout.close();

	remove("patients.txt");
	rename("temp.txt", "patients.txt");
}

// discharged
void FileHandler::appendDischarged(const Patient& p)
{
	//patient_id,name,pid,gender,contact,password,aid
	ofstream fout("discharged.txt", ios::app);
	if (!fout)
		throw FileNotFoundException("discharged.txt not found!");

	// convert numbers to strings
	char id[10], age[10], balance[20];
	myitoa(p.getId(), id);
	myitoa(p.getAge(), age);
	myftoa(p.getBalance(), balance);

	// write comma separated
	fout << id << ","
		<< p.getName() << ","
		<< age << ","
		<< p.getGender() << ","
		<< p.getContact() << ","
		<< p.getPassword() << ","
		<< balance << "\n";

	fout.close();

}