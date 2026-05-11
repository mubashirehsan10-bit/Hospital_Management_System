#include "MediCoreGuiAPI.h"
#include "utility.h"
#include "FileNotFoundException.h"
#include <fstream>

void med_msgClear(char* buf, int cap)
{
	if (cap <= 0) return;
	buf[0] = '\0';
}

void med_msgCat(char* buf, int cap, const char* part)
{
	if (part == nullptr || cap <= 1) return;
	int len = mystrlen(buf);
	int i = 0;
	while (part[i] != '\0' && len + 1 < cap)
		buf[len++] = part[i++];
	buf[len] = '\0';
}

void med_reloadAll(Storage<Patient>& patients, Storage<Prescription>& prescriptions,
    Storage<Bill>& bills, Storage<Appointment>& appointments, Storage<Doctor>& doctors,
    Admin*& admin, FileHandler& fh)
{
	patients.clear();
	prescriptions.clear();
	bills.clear();
	appointments.clear();
	doctors.clear();
	if (admin != nullptr)
	{
		delete admin;
		admin = nullptr;
	}
	try
	{
		fh.loadPatients(patients);
		fh.loadDoctors(doctors);
		fh.loadAdmin(admin);
		fh.loadAppointments(appointments);
		fh.loadBills(bills);
		fh.loadPrescriptions(prescriptions);
	}
	catch (FileNotFoundException&)
	{
	}
}

int med_maxAppointmentId(const Storage<Appointment>& a)
{
	int m = 0;
	const Appointment* all = a.getAll();
	for (int i = 0; i < a.size(); i++)
		if (all[i].getAppointmentId() > m)
			m = all[i].getAppointmentId();
	return m;
}

int med_maxBillId(const Storage<Bill>& b)
{
	int m = 0;
	const Bill* all = b.getAll();
	for (int i = 0; i < b.size(); i++)
		if (all[i].getId() > m)
			m = all[i].getId();
	return m;
}

int med_maxPrescriptionId(const Storage<Prescription>& p)
{
	int m = 0;
	const Prescription* all = p.getAll();
	for (int i = 0; i < p.size(); i++)
		if (all[i].getId() > m)
			m = all[i].getId();
	return m;
}

int med_maxDoctorId(const Storage<Doctor>& d)
{
	int m = 0;
	const Doctor* all = d.getAll();
	for (int i = 0; i < d.size(); i++)
		if (all[i].getId() > m)
			m = all[i].getId();
	return m;
}

Bill* med_findBillByAppointmentId(Storage<Bill>& b, int appointmentId)
{
	Bill* all = b.getAll();
	for (int i = 0; i < b.size(); i++)
		if (all[i].getAppointmentId() == appointmentId)
			return &all[i];
	return nullptr;
}

void med_guiBookAppointment(Validator& v, FileHandler& fh, Patient* p, Storage<Bill>& bills,
    Storage<Doctor>& doctors, Storage<Appointment>& appointments,
    const char* specialization, int doctorId, const char* date, const char* timeSlot,
    char* msg, int msgCap)
{
	med_msgClear(msg, msgCap);

	bool foundSpec = false;
	Doctor* docList = doctors.getAll();
	for (int i = 0; i < doctors.size(); i++)
	{
		if (mystrcmpIgnoreCase(specialization, docList[i].getSpecialization()) == 0)
		{
			foundSpec = true;
			break;
		}
	}
	if (!foundSpec)
	{
		med_msgCat(msg, msgCap, "No doctors available for that specialization.");
		return;
	}

	Doctor* selectedDoc = doctors.findByID(doctorId);
	if (selectedDoc == nullptr)
	{
		med_msgCat(msg, msgCap, "Doctor not found.");
		return;
	}
	if (mystrcmpIgnoreCase(specialization, selectedDoc->getSpecialization()) != 0)
	{
		med_msgCat(msg, msgCap, "Doctor not found.");
		return;
	}

	if (!v.isValidDate(date))
	{
		med_msgCat(msg, msgCap, "Invalid date. Use format DD-MM-YYYY.");
		return;
	}

	if (!v.isValidTimeSlot(timeSlot))
	{
		med_msgCat(msg, msgCap, "Invalid time slot.");
		return;
	}

	Appointment* all = appointments.getAll();
	for (int i = 0; i < appointments.size(); i++)
	{
		Appointment probe(0, 0, doctorId, date, timeSlot, "pending");
		if (all[i] == probe)
		{
			med_msgCat(msg, msgCap, "Slot unavailable.");
			return;
		}
	}

	if (p->getBalance() < selectedDoc->getFees())
	{
		med_msgCat(msg, msgCap, "Insufficient funds.");
		return;
	}

	*p -= selectedDoc->getFees();

	int newApptId = med_maxAppointmentId(appointments) + 1;
	int newBillId = med_maxBillId(bills) + 1;

	Appointment appointment(newApptId, p->getId(), selectedDoc->getId(), date, timeSlot, "pending");
	appointments.add(appointment);

	Bill bill(newApptId, newBillId, p->getId(), selectedDoc->getFees(), "unpaid", date);
	bills.add(bill);

	fh.appendBill(bill);
	fh.appendAppointment(appointment);
	fh.updatePatient(p->getId(), *p);

	med_msgCat(msg, msgCap, "Appointment booked successfully. Appointment ID: ");
	char tmp[32];
	myitoa(newApptId, tmp);
	med_msgCat(msg, msgCap, tmp);
}

void med_guiCancelAppointment(FileHandler& fh, Patient* p, Storage<Doctor>& doctors,
    Storage<Bill>& bills, Storage<Appointment>& appointments, int appointmentId,
    char* msg, int msgCap)
{
	med_msgClear(msg, msgCap);
	Appointment* all = appointments.getAll();
	int idx = -1;
	for (int i = 0; i < appointments.size(); i++)
	{
		if (all[i].getAppointmentId() == appointmentId &&
			all[i].getPatientId() == p->getId() &&
			mystrcmpIgnoreCase(all[i].getAppointmentStatus(), "pending") == 0)
		{
			idx = i;
			break;
		}
	}
	if (idx < 0)
	{
		med_msgCat(msg, msgCap, "Invalid appointment ID.");
		return;
	}

	Doctor* selectedDoc = doctors.findByID(all[idx].getDoctorId());
	if (selectedDoc == nullptr)
	{
		med_msgCat(msg, msgCap, "Doctor not found.");
		return;
	}

	Appointment updated(all[idx].getAppointmentId(), all[idx].getPatientId(),
		all[idx].getDoctorId(), all[idx].getAppointmentDate(),
		all[idx].getAppointmentSlot(), "cancelled");
	fh.updateAppointment(appointmentId, updated);

	*p += selectedDoc->getFees();
	fh.updatePatient(p->getId(), *p);

	Bill* b = med_findBillByAppointmentId(bills, appointmentId);
	if (b != nullptr)
	{
		Bill ub = *b;
		ub.setStatus("cancelled");
		fh.updateBill(b->getId(), ub);
	}

	med_msgCat(msg, msgCap, "Appointment cancelled. Refund processed.");
}

void med_guiPayBill(Validator& v, FileHandler& fh, Patient* p, Storage<Bill>& bills,
    int billId, char* msg, int msgCap)
{
	(void)v;
	med_msgClear(msg, msgCap);
	Bill* b = bills.findByID(billId);
	if (b == nullptr || b->getPatientId() != p->getId() ||
		mystrcmpIgnoreCase(b->getStatus(), "unpaid") != 0)
	{
		med_msgCat(msg, msgCap, "Invalid bill.");
		return;
	}
	if (p->getBalance() < b->getAmount())
	{
		med_msgCat(msg, msgCap, "Insufficient funds.");
		return;
	}
	*p -= b->getAmount();
	Bill paid(b->getAppointmentId(), b->getId(), b->getPatientId(), b->getAmount(), "paid", b->getAppointmentDate());
	fh.updateBill(b->getId(), paid);
	fh.updatePatient(p->getId(), *p);
	med_msgCat(msg, msgCap, "Bill paid successfully.");
}

void med_guiTopUp(Validator& v, FileHandler& fh, Patient* p, float amount, char* msg, int msgCap)
{
	med_msgClear(msg, msgCap);
	if (!v.isValidFloat(amount))
	{
		med_msgCat(msg, msgCap, "Invalid amount.");
		return;
	}
	*p += amount;
	fh.updatePatient(p->getId(), *p);
	med_msgCat(msg, msgCap, "Balance updated.");
}

void med_guiAddDoctor(Validator& v, FileHandler& fh, Storage<Doctor>& doctors,
    const char* name, const char* spec, const char* contact, const char* password, float fee,
    char* msg, int msgCap)
{
	med_msgClear(msg, msgCap);
	if (!v.isValidContact(contact))
	{
		med_msgCat(msg, msgCap, "Invalid contact.");
		return;
	}
	if (!v.isValidPassword(password))
	{
		med_msgCat(msg, msgCap, "Invalid password.");
		return;
	}
	if (!v.isValidFloat(fee))
	{
		med_msgCat(msg, msgCap, "Invalid fee.");
		return;
	}
	int id = med_maxDoctorId(doctors) + 1;
	Doctor d(id, name, contact, password, spec, fee);
	doctors.add(d);
	fh.appendDoctor(d);
	med_msgCat(msg, msgCap, "Doctor added. ID: ");
	char t[16];
	myitoa(id, t);
	med_msgCat(msg, msgCap, t);
}

void med_guiRemoveDoctor(FileHandler& fh, Storage<Doctor>& doctors, Storage<Appointment>& appointments,
    int doctorId, char* msg, int msgCap)
{
	med_msgClear(msg, msgCap);
	Appointment* all = appointments.getAll();
	for (int i = 0; i < appointments.size(); i++)
	{
		if (all[i].getDoctorId() == doctorId &&
			mystrcmpIgnoreCase(all[i].getAppointmentStatus(), "pending") == 0)
		{
			med_msgCat(msg, msgCap, "Cannot remove doctor with pending appointments.");
			return;
		}
	}
	if (doctors.findByID(doctorId) == nullptr)
	{
		med_msgCat(msg, msgCap, "Doctor not found.");
		return;
	}
	fh.deleteDoctor(doctorId);
	doctors.removeByID(doctorId);
	med_msgCat(msg, msgCap, "Doctor removed.");
}

static bool patientHasUnpaid(const Storage<Bill>& bills, int patientId)
{
	const Bill* all = bills.getAll();
	for (int i = 0; i < bills.size(); i++)
	{
		if (all[i].getPatientId() == patientId &&
			mystrcmpIgnoreCase(all[i].getStatus(), "unpaid") == 0)
			return true;
	}
	return false;
}

static bool patientHasPendingAppt(const Storage<Appointment>& appointments, int patientId)
{
	const Appointment* all = appointments.getAll();
	for (int i = 0; i < appointments.size(); i++)
	{
		if (all[i].getPatientId() == patientId &&
			mystrcmpIgnoreCase(all[i].getAppointmentStatus(), "pending") == 0)
			return true;
	}
	return false;
}

void med_guiDischargePatient(FileHandler& fh, Storage<Patient>& patients, Storage<Appointment>& appointments,
    Storage<Bill>& bills, Storage<Prescription>& prescriptions, int patientId, char* msg, int msgCap)
{
	med_msgClear(msg, msgCap);
	Patient* p = patients.findByID(patientId);
	if (p == nullptr)
	{
		med_msgCat(msg, msgCap, "Patient not found.");
		return;
	}
	if (patientHasUnpaid(bills, patientId))
	{
		med_msgCat(msg, msgCap, "Cannot discharge patient with unpaid bills.");
		return;
	}
	if (patientHasPendingAppt(appointments, patientId))
	{
		med_msgCat(msg, msgCap, "Cannot discharge patient with pending appointments.");
		return;
	}

	ifstream finA("appointments.txt");
	if (finA)
	{
		char line[600];
		myreadLine(finA, line, 600);
		while (!finA.eof())
		{
			myreadLine(finA, line, 600);
			if (mystrlen(line) == 0) continue;
			char tok[120];
			int pos = 0;
			getToken(line, tok, pos);
			getToken(line, tok, pos);
			int pid = myatoi(tok);
			if (pid == patientId)
				fh.appendArchivedRecord(line);
		}
		finA.close();
	}

	ifstream finB("bills.txt");
	if (finB)
	{
		char line[600];
		myreadLine(finB, line, 600);
		while (!finB.eof())
		{
			myreadLine(finB, line, 600);
			if (mystrlen(line) == 0) continue;
			char tok[120];
			int pos = 0;
			getToken(line, tok, pos);
			getToken(line, tok, pos);
			int pid = myatoi(tok);
			if (pid == patientId)
				fh.appendArchivedRecord(line);
		}
		finB.close();
	}

	ifstream finP("prescriptions.txt");
	if (finP)
	{
		char line[800];
		myreadLine(finP, line, 800);
		while (!finP.eof())
		{
			myreadLine(finP, line, 800);
			if (mystrlen(line) == 0) continue;
			char tok[120];
			int pos = 0;
			getToken(line, tok, pos);
			getToken(line, tok, pos);
			getToken(line, tok, pos);
			int pid = myatoi(tok);
			if (pid == patientId)
				fh.appendArchivedRecord(line);
		}
		finP.close();
	}

	fh.appendDischarged(*p);
	fh.deleteAppointmentsByPatientId(patientId);
	fh.deleteBillsByPatientId(patientId);
	fh.deletePrescriptionsByPatientId(patientId);
	fh.deletePatient(patientId);
	patients.removeByID(patientId);
	med_msgCat(msg, msgCap, "Patient discharged and archived.");
}

void med_formatPatientList(const Storage<Patient>& patients, const Storage<Bill>& bills, char* out, int cap)
{
	(void)bills;
	med_msgClear(out, cap);
	const Patient* all = patients.getAll();
	for (int i = 0; i < patients.size(); i++)
	{
		char idbuf[20];
		char line[256];
		myitoa(all[i].getId(), idbuf);
		mystrcpy(line, idbuf);
		med_msgCat(line, sizeof line, " | ");
		med_msgCat(line, sizeof line, all[i].getName());
		med_msgCat(line, sizeof line, "\n");
		med_msgCat(out, cap, line);
	}
}

void med_formatDoctorList(const Storage<Doctor>& doctors, char* out, int cap)
{
	med_msgClear(out, cap);
	const Doctor* all = doctors.getAll();
	for (int i = 0; i < doctors.size(); i++)
	{
		char idbuf[20];
		char line[256];
		myitoa(all[i].getId(), idbuf);
		mystrcpy(line, idbuf);
		med_msgCat(line, sizeof line, " | ");
		med_msgCat(line, sizeof line, all[i].getName());
		med_msgCat(line, sizeof line, "\n");
		med_msgCat(out, cap, line);
	}
}

void med_formatAppointmentList(const Storage<Appointment>& appointments,
    const Storage<Patient>& patients, const Storage<Doctor>& doctors, char* out, int cap)
{
	med_msgClear(out, cap);
	const Appointment* all = appointments.getAll();
	for (int i = 0; i < appointments.size(); i++)
	{
		const Patient* pp = patients.findByID(all[i].getPatientId());
		const Doctor* dd = doctors.findByID(all[i].getDoctorId());
		char idbuf[20];
		char line[320];
		myitoa(all[i].getAppointmentId(), idbuf);
		mystrcpy(line, idbuf);
		med_msgCat(line, sizeof line, " | ");
		med_msgCat(line, sizeof line, pp ? pp->getName() : "?");
		med_msgCat(line, sizeof line, " | ");
		med_msgCat(line, sizeof line, dd ? dd->getName() : "?");
		med_msgCat(line, sizeof line, "\n");
		med_msgCat(out, cap, line);
	}
}

void med_formatUnpaidBillsAll(const Storage<Bill>& bills, const Storage<Patient>& patients, char* out, int cap)
{
	med_msgClear(out, cap);
	const Bill* all = bills.getAll();
	for (int i = 0; i < bills.size(); i++)
	{
		if (mystrcmpIgnoreCase(all[i].getStatus(), "unpaid") != 0) continue;
		const Patient* pp = patients.findByID(all[i].getPatientId());
		char idbuf[20];
		char line[256];
		myitoa(all[i].getId(), idbuf);
		mystrcpy(line, idbuf);
		med_msgCat(line, sizeof line, " | ");
		med_msgCat(line, sizeof line, pp ? pp->getName() : "?");
		med_msgCat(line, sizeof line, "\n");
		med_msgCat(out, cap, line);
	}
}
void med_guiMarkComplete(FileHandler& fh, Doctor* doc,
	Storage<Appointment>& appointments, int apptId, char* msg, int msgCap)
{
	med_msgClear(msg, msgCap);
	if (doc == nullptr) { med_msgCat(msg, msgCap, "No doctor session."); return; }

	char today[15];
	getTodayDate(today);

	Appointment* all = appointments.getAll();
	for (int i = 0; i < appointments.size(); i++)
	{
		if (all[i].getAppointmentId() == apptId &&
			all[i].getDoctorId() == doc->getId() &&
			mystrcmpIgnoreCase(all[i].getAppointmentStatus(), "pending") == 0 &&
			mystrcmp(all[i].getAppointmentDate(), today) == 0)
		{
			Appointment updated(all[i].getAppointmentId(), all[i].getPatientId(),
				all[i].getDoctorId(), all[i].getAppointmentDate(),
				all[i].getAppointmentSlot(), "completed");
			fh.updateAppointment(apptId, updated);
			med_msgCat(msg, msgCap, "Appointment marked as completed.");
			return;
		}
	}
	med_msgCat(msg, msgCap, "Appointment not found or not eligible.");
}

void med_guiMarkNoShow(FileHandler& fh, Doctor* doc,
	Storage<Appointment>& appointments, Storage<Bill>& bills,
	int apptId, char* msg, int msgCap)
{
	med_msgClear(msg, msgCap);
	if (doc == nullptr) { med_msgCat(msg, msgCap, "No doctor session."); return; }

	char today[15];
	getTodayDate(today);

	Appointment* all = appointments.getAll();
	for (int i = 0; i < appointments.size(); i++)
	{
		if (all[i].getAppointmentId() == apptId &&
			all[i].getDoctorId() == doc->getId() &&
			mystrcmpIgnoreCase(all[i].getAppointmentStatus(), "pending") == 0 &&
			mystrcmp(all[i].getAppointmentDate(), today) == 0)
		{
			Appointment updated(all[i].getAppointmentId(), all[i].getPatientId(),
				all[i].getDoctorId(), all[i].getAppointmentDate(),
				all[i].getAppointmentSlot(), "no-show");
			fh.updateAppointment(apptId, updated);

			Bill* allBills = bills.getAll();
			for (int j = 0; j < bills.size(); j++)
			{
				if (allBills[j].getAppointmentId() == apptId)
				{
					Bill ub = allBills[j];
					ub.setStatus("cancelled");
					fh.updateBill(allBills[j].getId(), ub);
					break;
				}
			}
			med_msgCat(msg, msgCap, "Appointment marked as no-show.");
			return;
		}
	}
	med_msgCat(msg, msgCap, "Appointment not found or not eligible.");
}

void med_guiWritePrescription(FileHandler& fh, Doctor* doc,
	Storage<Appointment>& appointments, Storage<Prescription>& prescriptions,
	int apptId, const char* medicines, const char* notes,
	char* msg, int msgCap)
{
	med_msgClear(msg, msgCap);
	if (doc == nullptr) { med_msgCat(msg, msgCap, "No doctor session."); return; }

	Appointment* allAppts = appointments.getAll();
	bool found = false;
	int patientId = -1;
	for (int i = 0; i < appointments.size(); i++)
	{
		if (allAppts[i].getAppointmentId() == apptId &&
			allAppts[i].getDoctorId() == doc->getId() &&
			mystrcmpIgnoreCase(allAppts[i].getAppointmentStatus(), "completed") == 0)
		{
			found = true;
			patientId = allAppts[i].getPatientId();
			break;
		}
	}
	if (!found)
	{
		med_msgCat(msg, msgCap, "Appointment not found or not completed.");
		return;
	}

	Prescription* allPres = prescriptions.getAll();
	for (int i = 0; i < prescriptions.size(); i++)
	{
		if (allPres[i].getAppointmentId() == apptId)
		{
			med_msgCat(msg, msgCap, "Prescription already written.");
			return;
		}
	}

	char today[15];
	getTodayDate(today);
	int newId = med_maxPrescriptionId(prescriptions) + 1;

	Prescription pr(newId, apptId, patientId, doc->getId(), today, medicines, notes);
	prescriptions.add(pr);
	fh.appendPrescription(pr);
	med_msgCat(msg, msgCap, "Prescription saved.");
}

void med_formatTodayAppointments(const Storage<Appointment>& appointments,
	const Storage<Patient>& patients, const Doctor* doc,
	char* out, int cap)
{
	med_msgClear(out, cap);
	if (doc == nullptr) return;

	char today[15];
	getTodayDate(today);

	const Appointment* all = appointments.getAll();
	bool found = false;
	for (int i = 0; i < appointments.size(); i++)
	{
		if (all[i].getDoctorId() == doc->getId() &&
			mystrcmp(all[i].getAppointmentDate(), today) == 0)
		{
			const Patient* p = patients.findByID(all[i].getPatientId());
			char idbuf[20];
			char line[256];
			myitoa(all[i].getAppointmentId(), idbuf);
			mystrcpy(line, idbuf);
			med_msgCat(line, sizeof line, " | ");
			med_msgCat(line, sizeof line, p ? p->getName() : "?");
			med_msgCat(line, sizeof line, " | ");
			med_msgCat(line, sizeof line, all[i].getAppointmentSlot());
			med_msgCat(line, sizeof line, " | ");
			med_msgCat(line, sizeof line, all[i].getAppointmentStatus());
			med_msgCat(line, sizeof line, "\n");
			med_msgCat(out, cap, line);
			found = true;
		}
	}
	if (!found)
		med_msgCat(out, cap, "No appointments scheduled for today.");
}

void med_formatPatientHistory(const Storage<Prescription>& prescriptions,
	const Storage<Appointment>& appointments, const Doctor* doc,
	int patientId, char* out, int cap)
{
	med_msgClear(out, cap);
	if (doc == nullptr) return;

	// check access
	const Appointment* allAppts = appointments.getAll();
	bool hasAccess = false;
	for (int i = 0; i < appointments.size(); i++)
	{
		if (allAppts[i].getPatientId() == patientId &&
			allAppts[i].getDoctorId() == doc->getId() &&
			mystrcmpIgnoreCase(allAppts[i].getAppointmentStatus(), "completed") == 0)
		{
			hasAccess = true;
			break;
		}
	}
	if (!hasAccess)
	{
		med_msgCat(out, cap, "Access denied.");
		return;
	}

	const Prescription* all = prescriptions.getAll();
	bool found = false;
	for (int i = 0; i < prescriptions.size(); i++)
	{
		if (all[i].getPatientId() == patientId &&
			all[i].getDoctorId() == doc->getId())
		{
			char line[512];
			mystrcpy(line, all[i].getPrescriptionDate());
			med_msgCat(line, sizeof line, " | ");
			med_msgCat(line, sizeof line, all[i].getMedicine());
			med_msgCat(line, sizeof line, " | ");
			med_msgCat(line, sizeof line, all[i].getPrescriptionNotes());
			med_msgCat(line, sizeof line, "\n");
			med_msgCat(out, cap, line);
			found = true;
		}
	}
	if (!found)
		med_msgCat(out, cap, "No medical records found.");
}
