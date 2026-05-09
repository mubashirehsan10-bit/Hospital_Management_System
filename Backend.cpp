//             Hospital Management system
#include "utility.h"
#include "FileHandler.h"
#include "Validator.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"
#include "Storage.h"
#include "HospitalException.h"
#include "FileNotFoundException.h"
#include "InsufficientFundsException.h"
#include "InvalidInputException.h"
#include "SlotUnavailableException.h"
#include <iostream>
#include <ctime>
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
void loadAllData(Storage<Patient> &patients,Storage<Prescription> &prescriptions,Storage<Bill> &bills,
    Storage<Appointment> &appointments,Storage<Doctor> &doctors,Admin* &admin ,FileHandler &fh)
{
	try
	{
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
Patient* loginPatient(Validator validator,Storage<Patient>& p)
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

void BookAppointment(Validator& v,FileHandler& fh,Patient* p,Storage<Bill>& b,Storage<Doctor>& d,Storage<Appointment>& a)
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
        for (int i = 0; i < d.size() ; i++)
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
    } while (attempts<3);
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
            throw InsufficentFundsException("Insufficent Balance");
        }
        catch (InsufficentFundsException& e)
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

void CancelAppointment(Validator& v,Storage<Doctor>& d,FileHandler& fh,Patient* p,Storage<Bill>& b,Storage<Appointment>& a)
{
    Appointment* appointment = a.getAll();
    bool checking = false;
    cout << "\n=====List of Patient Pending Appointments=====\n";
    for (int i = 0; i < a.size(); i++)
    {
        if (p->getId() == appointment[i].getPatientId() && 
            mystrcmpIgnoreCase(appointment[i].getAppointmentStatus(),"Cancelled") != 0) {
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
            fh.updateAppointment(appointment[i].getAppointmentId(),a);
            Doctor* doc = d.getAll();
            int Did;
            for (int j = 0; j < d.size(); j++)
            {
                if (doc[j].getId() == appointment[i].getDoctorId())
                {
                    Doctor* selectedDoc = d.findByID(appointment[i].getDoctorId());
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
                    }
                    cout << "Appointment cancelled. PKR " << selectedDoc->getFees() <<
                        " refunded to your balance.\n\n";
                    checking = true;
                    break;
                }
            }
        }
        if (!checking)
        {
            cout << "Invalid appointment ID.\n";
            return;
        }
    }
    

}

using namespace std;
int main()
{
    // Loading Data from the files
    Storage<Patient> patients;
    Storage<Doctor> doctors;
    Storage<Appointment> appointments;
    Storage<Bill> bills;
    Storage<Prescription> prescriptions;
    Admin* admin = nullptr;
    FileHandler fh;
    Validator validator;// check validations through validator object
    int option;
    loadAllData(patients, prescriptions, bills, appointments, doctors, admin, fh);// data loading......
    try {
        throw InvalidInputException("TEST MESSAGE");
    }
    catch (InvalidInputException& e) {
        cout << e.what() << endl;
    }
    cout << " Welcome to MediCore Hospital Management System\n";
    do {
        displayMainMenu();
        option = getChoice(validator, 1, 4);

        switch (option)
        {
        case 1:
        {
            Patient* p = loginPatient(validator, patients);
            if (p != nullptr)
            {
                displayPatient(p);
                displayPatientMenu();
                int pChoice = getChoice(validator, 1, 8);
                switch (pChoice)
                {
                case 1:
                {
                    BookAppointment(validator, fh, p, bills, doctors, appointments);
                    break;
                }
                case 2:
                {
                    CancelAppointment();
                }
                case 3:
                {

                }
                }
            }

           
        }
        }
        
    } while (option != 4);
    cout << "\nThank You for using Hospital Mangament System!!!!\n";
    if (admin != nullptr)
    {
        delete admin;
        admin = nullptr;  // set to nullptr after delete
    }
    return 0;
}