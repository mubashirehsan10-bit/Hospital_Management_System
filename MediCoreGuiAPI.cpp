//             Hospital Management system

#include "MediCoreGuiAPI.h"
#include "utility.h"
#include "HospitalException.h"
#include "FileNotFoundException.h"
#include <fstream>
#include "FileHandler.h"
#include "Validator.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"
#include "Storage.h"
#include "InsufficientFundsException.h"
#include "InvalidInputException.h"
#include "SlotUnavailableException.h"
#include <iostream>
#include <ctime>
using namespace std;

int getChoice(Validator& validator, int min, int max = 100)
{
    char input[20];
    int option = -1;

    do {
        try
        {
            cin >> input;

            // check if all characters are digits
            bool isNum = true;
            for (int i = 0; input[i] != '\0'; i++)
            {
                if (input[i] < '0' || input[i] > '9')
                {
                    isNum = false;
                    break;
                }
            }

            if (!isNum)
                throw InvalidInputException("Invalid input! Enter a number.");

            option = myatoi(input);

            if (!validator.isValidMenuChoice(option, min, max))
                throw InvalidInputException("Invalid choice! Enter between 1-4.");
        }
        catch (InvalidInputException& e)
        {
            cout << e.what() << endl;
            option = -1;
        }
    } while (option < min || option > max);

    return option;
}
void loadAllData(Storage<Patient>& patients, Storage<Prescription>& prescriptions, Storage<Bill>& bills,
    Storage<Appointment>& appointments, Storage<Doctor>& doctors, Admin*& admin, FileHandler& fh)
{
    try
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
        fh.loadPatients(patients);
        fh.loadDoctors(doctors);
        fh.loadAdmin(admin);
        fh.loadAppointments(appointments);
        fh.loadBills(bills);
        fh.loadPrescriptions(prescriptions);
        cout << " Data loaded successfully!!" << endl;
    }
    catch (FileNotFoundException& e)
    {
        cout << e.what() << endl;
    }
}
void displayMainMenu()
{
    cout << "\n===============================================\n";
    cout << " Login as :\n";
    cout << " 1. Patient\n";
    cout << " 2. Doctor\n";
    cout << " 3. Admin\n";
    cout << " 4. Exit\n";
}
void displayPatientMenu()
{

    cout << "\n========================\n";
    cout << " 1. Book Appointment\n";
    cout << " 2. Cancel Appointment\n";
    cout << " 3. View My Appointments\n";
    cout << " 4. View My Medical Records\n";
    cout << " 5. View My Bills\n";
    cout << " 6. Pay Bill\n";
    cout << " 7. Top Up Balance\n";
    cout << " 8. Logout\n";
}
Patient* loginPatient(Validator validator, Storage<Patient>& p)
{
    char password[20];
    int id = 1;
    int attempts = 0;

    while (attempts <= 3)
    {

        do
        {
            cout << "Enter vaild ID: ";
            id = getChoice(validator, id);
            cin.ignore();
        } while (!validator.isValidID(id));

        do {
            cout << "Enter Password: ";
            cin.getline(password, 20);
        } while (!validator.isValidPassword(password));

        Patient* patient = p.findByID(id);

        if (patient == nullptr)
        {
            cout << "Patient with this ID Not Found!!!\n";
            cout << "Contact admin.\n";
            attempts++;
            cout << 3 - attempts << " Attempts remaining!!\n";

            continue;
        }

        if (mystrcmp(patient->getPassword(), password) == 0)
        {
            cout << "Logged in successfully!!\n";
            return patient;
        }
        else
        {
            cout << "Invalid Password!!\n";
            attempts++;
            cout << 3 - attempts << " Attempts remaining!!\n";
            continue;
        }

        cout << 3 - attempts << " Attempts remaining!!\n";
        cout << "Account locked. Contact admin.\n";

        return nullptr;
    }

}
void displayPatient(Patient* p)
{
    cout << "\nWelcome, " << p->getName() << endl;
    cout << "Balance:  PKR" << p->getBalance() << endl;
}
// 1
void BookAppointment(Validator& v, FileHandler& fh, Patient* p, Storage<Bill>& b, Storage<Doctor>& d, Storage<Appointment>& a)
{
    char variable[50];
    cout << "Enter Specilaization: ";
    cin >> variable; // input specialization
    cin.ignore();
    // specilaization
    Doctor* doc = d.getAll();
    bool flag = false;

    for (int i = 0; i < d.size(); i++)
    {
        if (mystrcmpIgnoreCase(variable, doc[i].getSpecialization()) == 0)
        {
            flag = true;
            cout << doc[i]; // printing doctor with specific specilaization
        }
    }
    if (!flag) {
        cout << " No doctors available for that specialization.\n";
        return;
    }

    flag = false;
    int id;
    // Doctors ID
    while (flag == false) {
        cout << " Enter Doctor's ID: ";
        id = getChoice(v, 1);
        for (int i = 0; i < d.size(); i++)
        {
            if (mystrcmpIgnoreCase(variable, doc[i].getSpecialization()) == 0)
            {
                if (doc[i].getId() == id) // id checking
                {
                    flag = true;
                }
            }
        }
        if (!flag)
        {
            cout << " Doctor not found.\n";
        }
    }
    int attempts = 0;
    do
    {
        cout << " Enter Valid date (DD-MM-YYYY):";
        cin >> variable; // input date
        if (!v.isValidDate(variable))
        {
            attempts++;
            cout << 3 - attempts << " Attempts remaining!!\n";
        }
        else
            break;
    } while (attempts < 3);
    if (attempts == 3)
    {
        cout << 3 - attempts << " Attempts remaining!!\n";
        cout << "Logging Out........\n";
        return;
    }

    //Appointments Check
    Appointment* all = a.getAll();
    flag = false;
    const char* slots[] = { "09:00","10:00","11:00","12:00",
                        "13:00","14:00","15:00","16:00" };

    char timeSlot[6];
    cout << "1. 09:00\n 2. 10:00\n 3. 11:00\n" <<
        "4. 12:00\n 5. 13:00\n 6. 14:00\n 7. 15:00\n 8. 16:00.\n";
    while (flag == false) {
        try
        {
            cout << "Available slots :" << endl;
            for (int i = 0; i < 8; i++)
            {
                bool taken = false;
                Appointment* all = a.getAll();
                for (int j = 0; j < a.size(); j++)
                {
                    Appointment temp(0, 0, id, variable, slots[i], "pending");
                    if (all[j] == temp)
                    {
                        taken = true;
                        break;
                    }
                }
                if (!taken)
                    cout << slots[i] << endl;
            }

            cout << "Enter time slot: ";
            cin >> timeSlot;
            if (!v.isValidTimeSlot(timeSlot))
                throw InvalidInputException("Invalid time slot!");

            // check if slot taken
            for (int i = 0; i < a.size(); i++)
            {

                Appointment temp(0, 0, id, variable, timeSlot, "pending");
                if (all[i] == temp) // just compare last four parameters as discuused in class
                    throw SlotUnavailableException("Slot already taken!");
            }
            flag = true;
        }
        catch (SlotUnavailableException& e)
        {
            cout << e.what() << endl;
        }
        catch (InvalidInputException& e)
        {
            cout << e.what() << endl;
        }
    }

    // Balance

    Doctor* selectedDoc = d.findByID(id);

    if (p->getBalance() >= selectedDoc->getFees())
    {
        *p -= selectedDoc->getFees();
    }
    else
    {
        try {
            throw InsufficientFundsException("Insufficent Balance");
        }
        catch (InsufficientFundsException& e)
        {
            cout << e.what() << endl;
            return;
        }
    }

    int newId = a.size() + 1;

    Appointment appointment(newId, p->getId(), selectedDoc->getId(), variable, timeSlot, "Pending");
    a.add(appointment);

    Bill bill(newId, newId, p->getId(), selectedDoc->getFees(), "Unpaid", variable);
    b.add(bill);

    fh.appendBill(bill);
    fh.appendAppointment(appointment);
    fh.updatePatient(p->getId(), *p);

    cout << "Appointment Booked Successfully!!!\n";
    cout << "Appointment ID: " << newId << endl << endl;

}
// 2
void CancelAppointment(Validator& v, Storage<Doctor>& d, FileHandler& fh, Patient* p, Storage<Bill>& b, Storage<Appointment>& a)
{
    Appointment* appointment = a.getAll();
    bool checking = false;
    cout << "\n=====List of Patient Pending Appointments=====\n";
    for (int i = 0; i < a.size(); i++)
    {
        if (p->getId() == appointment[i].getPatientId() &&
            mystrcmpIgnoreCase(appointment[i].getAppointmentStatus(), "Cancelled") != 0) {
            cout << appointment[i];
            checking = true;
        }
    }
    if (!checking)
    {
        cout << "\nYou have no pending appointments.\n";
        return;
    }

    checking = false;
    cout << "Enter appointment t Cancel: ";
    int id = getChoice(v, 1);
    for (int i = 0; i < a.size(); i++)
    {
        if (appointment[i].getAppointmentId() == id &&
            p->getId() == appointment[i].getPatientId() &&
            mystrcmpIgnoreCase(appointment[i].getAppointmentStatus(), "Cancelled") != 0)
        {
            Appointment a(appointment[i].getAppointmentId(), appointment[i].getPatientId(),
                appointment[i].getDoctorId(), appointment[i].getAppointmentDate(),
                appointment[i].getAppointmentSlot(), "Cancelled");
            fh.updateAppointment(appointment[i].getAppointmentId(), a);
            Doctor* doc = d.getAll();
            int Did;

            Doctor* selectedDoc = d.findByID(appointment[i].getDoctorId());
            if (selectedDoc != nullptr) {
                *p += selectedDoc->getFees();
                Patient pat(appointment[i].getPatientId(), p->getName(), p->getContact(),
                    p->getPassword(), p->getAge(), p->getGender(), p->getBalance());
                fh.updatePatient(p->getId(), pat);

                Bill* selectedBill = b.findByID(id);
                if (selectedBill != nullptr)
                {
                    Bill updatedBill = *selectedBill;
                    updatedBill.setStatus("Cancelled");
                    fh.updateBill(selectedBill->getId(), updatedBill);
                    cout << "Appointment cancelled. PKR " << selectedDoc->getFees() <<
                        " refunded to your balance.\n\n";
                    checking = true;
                    break;
                }

            }
            else
                cout << "No Such Doctor Exists!!!\n";

        }

    }
    if (!checking)
    {
        cout << "Invalid appointment ID.\n";
        return;
    }

}
// extract year, month, day from "DD-MM-YYYY"
int getYear(const char* date) { return myatoi(date + 6); }  // chars 6-9
int getMonth(const char* date) { return myatoi(date + 3); }  // chars 3-4
int getDay(const char* date) { return myatoi(date); }
bool isDateBefore(const char* d1, const char* d2)
{
    if (getYear(d1) != getYear(d2))   return getYear(d1) < getYear(d2);
    if (getMonth(d1) != getMonth(d2)) return getMonth(d1) < getMonth(d2);
    return getDay(d1) < getDay(d2);
}
// 3
void ViewMyAppointments(Storage<Appointment>& a, Patient* p)
{
    Appointment* appointment = a.getAll();
    Appointment patientAppts[100]; // create a copy so original data remain unchanged
    int count = 0;
    for (int i = 0; i < a.size(); i++)
        if (p->getId() == appointment[i].getPatientId())
            patientAppts[count++] = appointment[i];

    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (isDateBefore(patientAppts[j + 1].getAppointmentDate(),
                patientAppts[j].getAppointmentDate()))
            {
                // swap
                Appointment temp = patientAppts[j];
                patientAppts[j] = patientAppts[j + 1];
                patientAppts[j + 1] = temp;
            }

    bool checking = false;
    cout << "\n=====List of Patient Pending Appointments=====\n";
    for (int i = 0; i < count; i++)
    {
        cout << patientAppts[i];
        checking = true;
    }
    if (!checking)
    {
        cout << "\nYou have no pending appointments.\n";
        return;
    }


}
// 4
void ViewMyMedicalRecords(Storage<Prescription>& p, Patient* pat)
{
    Prescription* prescription = p.getAll();
    Prescription Selectedpres[100];
    int count = 0;

    bool flag = false;
    for (int i = 0; i < p.size(); i++)
        if (pat->getId() == prescription[i].getPatientId()) {
            Selectedpres[count++] = prescription[i];
            flag = true;
        }
    if (!flag) {
        cout << "No medical records found.\n";
        return;
    }

    // sort
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (isDateBefore(Selectedpres[j].getPrescriptionDate(), // via decending
                Selectedpres[j + 1].getPrescriptionDate()))
            {
                Prescription temp = Selectedpres[j];
                Selectedpres[j] = Selectedpres[j + 1];
                Selectedpres[j + 1] = temp;
            }
    // display sorted
    for (int i = 0; i < count; i++)
    {
        cout << Selectedpres[i];
    }

}
// 5
void ViewMyBills(Storage<Bill>& bills, Patient* p)
{
    Bill* b = bills.getAll();
    float totalBill = 0.0f;

    bool flag = false;
    // printing bills
    for (int i = 0; i < bills.size(); i++)
    {
        if (p->getId() == b[i].getPatientId())
        {
            cout << b[i];
            flag = true;
            if (mystrcmpIgnoreCase(b[i].getStatus(), "unpaid") == 0)
                totalBill += b[i].getAmount();
        }
    }
    if (!flag)
    {
        cout << "No bills found.\n";
        return;
    }

    // display total amount

    cout << "The Total Outstanding UNPAID amount: " << totalBill << endl << endl;

}
//6. Pay Bill
void PayBill(Validator& v, Storage<Bill>& bills, Patient* p, FileHandler& fh)
{
    Bill* b = bills.getAll();
    Bill Unpaid[100];
    int count = 0;

    bool flag = false;
    // printing bills
    for (int i = 0; i < bills.size(); i++)
    {
        if (p->getId() == b[i].getPatientId())
        {
            flag = true;
            if (mystrcmpIgnoreCase(b[i].getStatus(), "unpaid") == 0)
            {
                cout << b[i];
                Unpaid[count++] = b[i];
            }
        }
    }
    if (count == 0)
    {
        cout << "No unpaid bills.\n";
        return;
    }

    cout << "Enter Bill id to pay: ";
    int id = getChoice(v, 1);

    flag = false;
    for (int i = 0; i < count; i++)
    {
        try {
            if (Unpaid[i].getId() == id)
            {
                if (Unpaid[i].getAmount() <= p->getBalance()) {
                    *p -= Unpaid[i].getAmount();
                    Bill bil(Unpaid[i].getId(), Unpaid[i].getPatientId(), Unpaid[i].getAppointmentId(),
                        Unpaid[i].getAmount(), "Paid", Unpaid[i].getAppointmentDate());
                    fh.updateBill(Unpaid[i].getId(), bil);
                    Patient pa(p->getId(), p->getName(), p->getContact(), p->getPassword(),
                        p->getAge(), p->getGender(), p->getBalance());
                    fh.updatePatient(p->getId(), pa);
                    cout << "Bill paid successfully. Remaining balance: PKR " << p->getBalance() << "\n";

                    flag = true;
                }
                else
                    throw InsufficientFundsException("Balnce is infufficent to Pay Bill");
            }
        }
        catch (InsufficientFundsException& e)
        {
            cout << e.what() << endl;
            return;
        }
    }
    if (!flag)
    {
        cout << "Invalid Bill id!!\n";
        return;
    }


}
//7. Top Up Balance
void TopUpBalance(Validator& v, FileHandler& fh, Patient* p)
{
    int attempts = 0;
    while (attempts != 3) {
        try {
            cout << "Enter amount to add (PKR): ";
            float amount;
            cin >> amount;
            if (!v.isValidFloat(amount))
            {
                throw InvalidInputException("Amount should be +ve");
                attempts++;
            }
            else {
                *p += amount;
                Patient pa(p->getId(), p->getName(), p->getContact(), p->getPassword(),
                    p->getAge(), p->getGender(), p->getBalance());
                fh.updatePatient(p->getId(), pa);
                cout << "Balance updated.New balance : PKR" << p->getBalance() << endl << endl;
                attempts = 3;
                return;
            }
        }
        catch (InvalidInputException& e)
        {
            cout << e.what() << endl;
            cout << 3 - attempts << " Attempts Reamaining!!\n";
        }
        if (attempts == 3)
        {
            return;
        }
    }

}
//=======================================================================

Doctor* loginDoctor(Validator validator, Storage<Doctor>& d)
{
    char password[20];
    int id = 1;
    int attempts = 0;

    while (attempts <= 3)
    {

        do
        {
            cout << "Enter vaild ID: ";
            id = getChoice(validator, id);
            cin.ignore();
        } while (!validator.isValidID(id));

        do {
            cout << "Enter Password: ";
            cin.getline(password, 20);
        } while (!validator.isValidPassword(password));

        Doctor* doctor = d.findByID(id);

        if (doctor == nullptr)
        {
            cout << "Doctor with this ID Not Found!!!\n";
            attempts++;
            cout << 3 - attempts << " Attempts remaining!!\n";
            continue;
        }

        if (mystrcmp(doctor->getPassword(), password) == 0)
        {
            cout << "Logged in successfully!!\n";
            return doctor;
        }
        else
        {
            cout << "Invalid Password!!\n";
            attempts++;
            cout << 3 - attempts << " Attempts remaining!!\n";
            continue;
        }

        cout << 3 - attempts << " Attempts remaining!!\n";
        cout << "Account locked. Contact admin.\n";

        return nullptr;
    }

}
void displayDoctor(Doctor* d)
{

    cout << "Welcome, Dr." << d->getName() << " | Specialization: " << d->getSpecialization() << endl << endl;

}
void displayDoctorMenu()
{
    cout << "\n===============================================\n";
    cout << "1. View Today's Appointments\n";
    cout << "2. Mark Appointment Complete\n";
    cout << "3. Mark Appointment No-Show\n";
    cout << "4. Write Prescription\n";
    cout << "5. View Patient Medical History\n";
    cout << "6. Logout\n\n";
}

// 1. View Today's Appointments

void ViewTodaysAppointments(Doctor* doc, Storage<Appointment>& a,
    Storage<Patient>& patients)
{
    char today[15];
    getTodayDate(today);

    // extract today's appointments for this doctor
    Appointment todayAppts[100];
    int count = 0;

    Appointment* all = a.getAll();
    for (int i = 0; i < a.size(); i++)
    {
        if (all[i].getDoctorId() == doc->getId() &&
            mystrcmp(all[i].getAppointmentDate(), today) == 0)
        {
            todayAppts[count++] = all[i];
        }
    }

    if (count == 0)
    {
        cout << "No appointments scheduled for today.\n";
        return;
    }

    // sort by time slot ascending
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if ((mystrcmp(todayAppts[j].getAppointmentSlot(),
                todayAppts[j + 1].getAppointmentSlot()) > 0))
            {
                Appointment temp = todayAppts[j];
                todayAppts[j] = todayAppts[j + 1];
                todayAppts[j + 1] = temp;
            }

    // display
    for (int i = 0; i < count; i++)
    {
        cout << todayAppts[i];
    }
}
// 2. Mark Appointment Complete
void MarkAppointmentComplete(Validator v, Doctor* doc, Storage<Appointment>& a,
    Storage<Patient>& patients, FileHandler& fh)
{
    char today[15];
    getTodayDate(today);

    // extract today's appointments for this doctor
    Appointment todayAppts[100];
    int count = 0;

    Appointment* all = a.getAll();
    for (int i = 0; i < a.size(); i++)
    {
        if (all[i].getDoctorId() == doc->getId() &&
            mystrcmp(all[i].getAppointmentDate(), today) == 0 &&
            mystrcmpIgnoreCase(all[i].getAppointmentStatus(), "Pending") == 0)
        {
            todayAppts[count++] = all[i];
        }
    }

    if (count == 0)
    {
        cout << "No appointments scheduled for today.\n";
        return;
    }

    cout << "Enter Appointment ID:\n";
    int id = getChoice(v, 1);
    bool flag = false;
    for (int i = 0; i < count; i++)
    {
        if (todayAppts[i].getAppointmentId() == id)
        {
            Appointment ap(todayAppts[i].getAppointmentId(), todayAppts[i].getPatientId(),
                todayAppts[i].getDoctorId(), todayAppts[i].getAppointmentDate(),
                todayAppts[i].getAppointmentSlot(), "Completed");
            fh.updateAppointment(todayAppts[i].getAppointmentId(), ap);
            cout << "Appointment marked as completed.\n";
            flag = true;
        }
    }
    if (!flag)
        cout << "No such Appointment id exis\n";
}
// 3. Mark No Show
void MarkNoShow(Validator v, Doctor* doc, Storage<Appointment>& a,
    Storage<Patient>& patients, FileHandler& fh, Storage<Bill>& bills)
{
    char today[15];
    getTodayDate(today);

    // extract today's appointments for this doctor
    Appointment todayAppts[100];
    int count = 0;

    Appointment* all = a.getAll();
    for (int i = 0; i < a.size(); i++)
    {
        if (all[i].getDoctorId() == doc->getId() &&
            mystrcmp(all[i].getAppointmentDate(), today) == 0 &&
            mystrcmpIgnoreCase(all[i].getAppointmentStatus(), "Pending") == 0)
        {
            todayAppts[count++] = all[i];
        }
    }

    if (count == 0)
    {
        cout << "No appointments scheduled for today.\n";
        return;
    }

    cout << "Enter Appointment ID:\n";
    int id = getChoice(v, 1);
    bool flag = false;

    Bill* allBills = bills.getAll();

    for (int i = 0; i < count; i++)
    {
        if (todayAppts[i].getAppointmentId() == id)
        {
            Appointment ap(todayAppts[i].getAppointmentId(), todayAppts[i].getPatientId(),
                todayAppts[i].getDoctorId(), todayAppts[i].getAppointmentDate(),
                todayAppts[i].getAppointmentSlot(), "no-show");
            fh.updateAppointment(todayAppts[i].getAppointmentId(), ap);
            for (int j = 0; j < bills.size(); j++)
            {
                if (allBills[j].getAppointmentId() == id)  // match by id
                {
                    Bill updatedBill = allBills[j];
                    updatedBill.setStatus("cancelled");
                    fh.updateBill(allBills[j].getId(), updatedBill);
                    flag = true;
                    break;
                }
            }
            cout << "Appointment marked as no show.\n";

        }
    }
    if (!flag)
        cout << "No such Appointment id exis\n";
}
// 4. Write Prescription
void WritePrescription(FileHandler& fh, Validator& v, Storage<Appointment>& a, Storage<Prescription>& p, Doctor* d)
{

    int attempts = 0;
    int id;

    while (attempts != 3) {
        try
        {
            cout << "Enter Appointment ID: ";
            id = getChoice(v, 1);
            if (!v.isValidID(id))
            {
                throw InvalidInputException("Invalid ID Entered!!");
            }
            else
                break;
        }
        catch (InvalidInputException& e)
        {
            cout << e.what() << endl;
            attempts++;

            cout << 3 - attempts << " Attempts Reamaining!!\n";
        }
    }
    if (attempts == 3)
    {
        cout << "0 Attempts left\n\n";
        return;
    }

    Appointment* app = a.getAll();
    Prescription* pres = p.getAll();
    bool flag = false;

    for (int i = 0; i < a.size(); i++)
    {
        if (app[i].getAppointmentId() == id && mystrcmpIgnoreCase(app[i].getAppointmentStatus(), "Completed") == 0)
        {
            for (int j = 0; j < p.size(); j++)
            {
                if (pres[j].getAppointmentId() == id)
                {
                    cout << "Prescription already written for this appointment.\n";
                    return;
                }
                else if (pres[j].getAppointmentId() == id && mystrcmp(pres[j].getPrescriptionNotes(), "") == 0)
                {
                    cout << "Enter medicines (format: MedicineName Dosage; e.g Paracetamol 500mg; Amoxicillin 250mg): ";
                    char medicine[500];
                    cin.getline(medicine, 500);
                    cout << "Enter notes (max 300 chars): ";
                    char notes[300];
                    cin.ignore();
                    cin.getline(notes, 300);
                    cin.ignore();

                    int newid = p.size() + 1;

                    char today[15];
                    getTodayDate(today);
                    Prescription prescrip(newid, id, pres[j].getPatientId(), d->getId(), today,
                        medicine, notes);
                    fh.appendPrescription(prescrip);
                    cout << "Prescription Written Successfully\n";
                    return;

                }
            }

        }

    }
    cout << "No such Prescription Exists.\n";



}
// 5. ViewPatientMedicalHistory
void ViewPatientMedicalHistory(Validator& v, Storage<Patient>& p, Doctor* d, Storage<Prescription>& pres,
    Storage<Appointment>& app)
{
    int attempts = 0;
    int id;

    Patient* patient = p.getAll();
    while (attempts != 3) {
        try
        {
            cout << "Enter Patient ID: ";
            id = getChoice(v, 1);
            if (!v.isValidID(id))
            {
                throw InvalidInputException("Invalid ID Entered!!");
            }
            else
            {
                Patient* found = p.findByID(id);
                if (found == nullptr)
                {
                    cout << "No Such Patient Exists\n";
                    return;
                }
                break;
            }

        }
        catch (InvalidInputException& e)
        {
            cout << e.what() << endl;
            attempts++;

            cout << 3 - attempts << " Attempts Reamaining!!\n";
        }
    }
    if (attempts == 3)
    {
        cout << "0 Attempts left\n\n";
        return;
    }


    Prescription* pr = pres.getAll();
    Appointment* ap = app.getAll();
    bool flag = false;
    Prescription selected[100];
    int count = 0;

    bool hasAccess = false;
    for (int i = 0; i < app.size(); i++)
    {
        if (ap[i].getPatientId() == id &&
            ap[i].getDoctorId() == d->getId() &&
            mystrcmpIgnoreCase(ap[i].getAppointmentStatus(), "completed") == 0)
        {
            hasAccess = true;
            break;
        }
    }
    if (!hasAccess)
    {
        cout << "Access denied. You can only view records of your own patients.\n";
        return;
    }

    for (int i = 0; i < pres.size(); i++)
    {
        if (pr[i].getPatientId() == id &&
            pr[i].getDoctorId() == d->getId())
        {
            selected[count++] = pr[i];
        }
    }

    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (mystrcmp(selected[j].getPrescriptionDate(), selected[j + 1].getPrescriptionDate()) > 0)
            {
                Prescription temp = selected[j];
                selected[j] = selected[j + 1];
                selected[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < count; i++)
    {
        cout << selected[i];
    }

}
//===============================================================================================
void DisplayAdmin()
{
    cout << "\nAdmin Panel  MediCore\n";
    cout << "======================\n";
    cout << "1. Add Doctor\n";
    cout << "2. Remove Doctor\n";
    cout << "3. View All Patients\n";
    cout << "4. View All Doctors\n";
    cout << "5. View All Appointments\n";
    cout << "6. View Unpaid Bills\n";
    cout << "7. Discharge Patient\n";
    cout << "8. View Security Log\n";
    cout << "9. Generate Daily Report\n";
    cout << "10. Logout\n";

}
Admin* loginAdmin(Validator& validator, Admin* a, FileHandler& fh)
{
    char password[20];
    int attempts = 0;

    while (attempts < 3)
    {
        cout << "Enter Password: ";
        cin >> password;

        if (mystrcmp(a->getPassword(), password) == 0)
        {
            cout << "Logged in successfully!\n";
            return a;
        }
        attempts++;
        cout << "Invalid Password! " << 3 - attempts << " attempts remaining.\n";
    }

    cout << "Account locked. Contact admin.\n";
    char timestamp[40];
    formatTimestampFull(timestamp, sizeof timestamp);
    char sid[10];
    myitoa(a->getId(), sid);
    fh.appendSecurityLog(timestamp, "Admin", sid, "FAILED");
    return nullptr;
}

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
