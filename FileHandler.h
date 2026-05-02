#ifndef FILEHANDLER_H
#define FILEHANDLER_H
#include <fstream>
#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"

class FileHandler {
public:
    // load
    void loadPatients(Storage<Patient>& storage);
    void loadDoctors(Storage<Doctor>& storage);
    void loadAdmin(Admin*& admin);
    void loadAppointments(Storage<Appointment>& storage);
    void loadBills(Storage<Bill>& storage);
    void loadPrescriptions(Storage<Prescription>& storage);

    // append
    void appendPatient(const Patient& p);
    void appendDoctor(const Doctor& d);
    void appendAppointment(const Appointment& a);
    void appendBill(const Bill& b);
    void appendPrescription(const Prescription& p);
    void appendSecurityLog(const char* timestamp, const char* role,
        const char* enteredId, const char* result);
    void readSecurityLog();

    // update by ID
    void updatePatient(int id, const Patient& p);
    void updateAppointment(int id, const Appointment& a);
    void updateBill(int id, const Bill& b);

    // delete by ID
    void deleteDoctor(int id);
    void deletePatient(int id);

    // discharged
    void appendDischarged(const Patient& p);
};
#endif