#ifndef MEDICORE_GUI_API_H
#define MEDICORE_GUI_API_H

#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"
#include "Admin.h"
#include "FileHandler.h"
#include "Validator.h"

void med_reloadAll(Storage<Patient>& patients, Storage<Prescription>& prescriptions,
    Storage<Bill>& bills, Storage<Appointment>& appointments, Storage<Doctor>& doctors,
    Admin*& admin, FileHandler& fh);

void med_msgClear(char* buf, int cap);
void med_msgCat(char* buf, int cap, const char* part);

int med_maxAppointmentId(const Storage<Appointment>& a);
int med_maxBillId(const Storage<Bill>& b);
int med_maxPrescriptionId(const Storage<Prescription>& p);
int med_maxDoctorId(const Storage<Doctor>& d);

Bill* med_findBillByAppointmentId(Storage<Bill>& b, int appointmentId);

void med_guiBookAppointment(Validator& v, FileHandler& fh, Patient* p, Storage<Bill>& bills,
    Storage<Doctor>& doctors, Storage<Appointment>& appointments,
    const char* specialization, int doctorId, const char* date, const char* timeSlot,
    char* msg, int msgCap);

void med_guiCancelAppointment(FileHandler& fh, Patient* p, Storage<Doctor>& doctors,
    Storage<Bill>& bills, Storage<Appointment>& appointments, int appointmentId,
    char* msg, int msgCap);

void med_guiPayBill(Validator& v, FileHandler& fh, Patient* p, Storage<Bill>& bills,
    int billId, char* msg, int msgCap);

void med_guiTopUp(Validator& v, FileHandler& fh, Patient* p, float amount, char* msg, int msgCap);

void med_guiAddDoctor(Validator& v, FileHandler& fh, Storage<Doctor>& doctors,
    const char* name, const char* spec, const char* contact, const char* password, float fee,
    char* msg, int msgCap);

void med_guiRemoveDoctor(FileHandler& fh, Storage<Doctor>& doctors, Storage<Appointment>& appointments,
    int doctorId, char* msg, int msgCap);

void med_guiDischargePatient(FileHandler& fh, Storage<Patient>& patients, Storage<Appointment>& appointments,
    Storage<Bill>& bills, Storage<Prescription>& prescriptions, int patientId, char* msg, int msgCap);

void med_formatPatientList(const Storage<Patient>& patients, const Storage<Bill>& bills, char* out, int cap);
void med_formatDoctorList(const Storage<Doctor>& doctors, char* out, int cap);
void med_formatAppointmentList(const Storage<Appointment>& appointments,
    const Storage<Patient>& patients, const Storage<Doctor>& doctors, char* out, int cap);
void med_formatUnpaidBillsAll(const Storage<Bill>& bills, const Storage<Patient>& patients, char* out, int cap);

void med_guiMarkComplete(FileHandler& fh, Doctor* doc,
    Storage<Appointment>& appointments, int apptId, char* msg, int msgCap);

void med_guiMarkNoShow(FileHandler& fh, Doctor* doc,
    Storage<Appointment>& appointments, Storage<Bill>& bills,
    int apptId, char* msg, int msgCap);

void med_guiWritePrescription(FileHandler& fh, Doctor* doc,
    Storage<Appointment>& appointments, Storage<Prescription>& prescriptions,
    int apptId, const char* medicines, const char* notes,
    char* msg, int msgCap);

void med_formatTodayAppointments(const Storage<Appointment>& appointments,
    const Storage<Patient>& patients, const Doctor* doc,
    char* out, int cap);

void med_formatPatientHistory(const Storage<Prescription>& prescriptions,
    const Storage<Appointment>& appointments, const Doctor* doc,
    int patientId, char* out, int cap);
#endif
