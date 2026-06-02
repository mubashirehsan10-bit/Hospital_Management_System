# 🏥 MediCore Hospital Management System

> A fully object-oriented Hospital Management System built in C++ with an SFML graphical interface — designed for MediCore, a fictional but fully functional hospital environment.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [OOP Concepts](#oop-concepts)
- [Project Structure](#project-structure)
- [Class Architecture](#class-architecture)
- [File Formats](#file-formats)
- [How to Compile](#how-to-compile)
- [How to Run](#how-to-run)
- [Default Credentials](#default-credentials)
- [GitHub Repository](#github-repository)
- [Student Information](#student-information)

---

## 🔍 Overview

MediCore is a Hospital Management System that supports three user roles:

- **Patient** — Book appointments, pay bills, view medical records
- **Doctor** — Manage appointments, write prescriptions, view patient history
- **Admin** — Full system control including doctor management, patient discharge, and reporting

All data **persists across restarts** through `.txt` file storage. Every change is immediately written to disk and loaded back on startup.

---

## ✨ Features

### 👤 Patient
| Feature | Description |
|---|---|
| Book Appointment | Search by specialization, pick doctor, date, time slot |
| Cancel Appointment | Cancel pending appointments with automatic refund |
| View Appointments | Sorted by date ascending |
| View Medical Records | Prescriptions sorted by date descending |
| View Bills | With total outstanding unpaid amount |
| Pay Bill | Balance check with exception handling |
| Top Up Balance | Add funds with validation |

### 🩺 Doctor
| Feature | Description |
|---|---|
| Today's Appointments | Sorted by time slot ascending |
| Mark Complete | Mark pending appointments as completed |
| Mark No-Show | Mark no-shows, cancel corresponding bill |
| Write Prescription | For completed appointments only |
| Patient Medical History | Access-controlled by appointment history |

### 🔧 Admin
| Feature | Description |
|---|---|
| Add Doctor | Full validation of all fields |
| Remove Doctor | Blocked if pending appointments exist |
| View All Patients | With unpaid bill count |
| View All Doctors | Full details |
| View All Appointments | Sorted by date descending |
| View Unpaid Bills | With overdue flagging |
| Discharge Patient | Archives all records, blocks if unpaid/pending |
| View Security Log | All failed login attempts |
| Generate Daily Report | Revenue, appointment stats, doctor summary |

---

## 🧠 OOP Concepts Demonstrated

| Concept | Implementation |
|---|---|
| **Abstraction** | `Person` is abstract — cannot be instantiated directly |
| **Inheritance** | `Patient`, `Doctor`, `Admin` inherit from `Person` |
| **Polymorphism** | Virtual `display()` and `getRole()` overridden in each class |
| **Encapsulation** | All data private/protected with getters and setters |
| **Operator Overloading** | `+=`, `-=`, `==`, `<<` on `Patient`, `Doctor`, `Appointment` |
| **Templates** | `Storage<T>` generic class for all entity storage |
| **Exception Handling** | Custom exception hierarchy with `try/catch/throw` |
| **File Handling** | Isolated in `FileHandler` — the only class doing I/O |
| **Dynamic Memory** | All pointers allocated with `new`, freed with `delete` |
| **Separation of Concerns** | `Validator` for input, `FileHandler` for I/O, classes for data |

---

## 📁 Project Structure

The repository was reorganized for clarity. Top-level layout:

```
HospitalManagement/
│
├── utility.h / utility.cpp          ← Custom string functions (no STL strings)
│
├── HospitalException.h / .cpp       ← Base exception class
├── FileNotFoundException.h          ← File missing on startup
├── InsufficientFundsException.h     ← Balance too low
├── InvalidInputException.h          ← Bad user input
├── SlotUnavailableException.h       ← Time slot already booked
│
├── Validator.h / .cpp               ← All input validation
│
├── Person.h / .cpp                  ← Abstract base class
├── Patient.h / .cpp                 ← Patient entity
├── Doctor.h / .cpp                  ← Doctor entity
├── Admin.h / .cpp                   ← Admin entity
│
├── Storage.h                        ← Generic template storage (no .cpp)
│
├── Appointment.h / .cpp             ← Appointment entity
├── Bill.h / .cpp                    ← Billing entity
├── Prescription.h / .cpp            ← Prescription entity
│
├── FileHandler.h / .cpp             ← All file I/O operations
│
├── MediCoreGuiAPI.h / .cpp          ← Backend API for GUI
├── MediCoreGui.cpp                  ← SFML window and UI
│
├── patients.txt                     ← Patient data
├── doctors.txt                      ← Doctor data
├── admin.txt                        ← Admin credentials
├── appointments.txt                 ← Appointment records
├── bills.txt                        ← Billing records
├── prescriptions.txt                ← Prescription records
├── security_log.txt                 ← Failed login attempts
└── discharged.txt                   ← Archived discharged patients
```

---

## 🏗️ Class Architecture

```
                    Person (Abstract)
                   /       |        \
              Patient    Doctor     Admin
                 
Storage<T> ──── works with all entity types

FileHandler ──── reads/writes all .txt files

Validator ──── validates all user input

HospitalException
├── FileNotFoundException
├── InsufficientFundsException
├── InvalidInputException
└── SlotUnavailableException
```

---

## 📄 File Formats

All files use comma-separated values with no spaces around commas.

### patients.txt
```
patient_id,name,age,gender,contact,password,balance
1,Ahmed Ali,28,M,03001234567,pass123,5000.00
```

### doctors.txt
```
doctor_id,name,specialization,contact,password,fee
1,Dr. Sara Khan,Cardiology,03111234567,doc456,1500.00
```

### admin.txt
```
admin_id,name,password
1,Admin,admin123
```

### appointments.txt
```
appointment_id,patient_id,doctor_id,date,time_slot,status
1,1,1,15-04-2026,09:00,pending
```

### bills.txt
```
bill_id,patient_id,appointment_id,amount,status,date
1,1,1,1500.00,unpaid,15-04-2026
```

### prescriptions.txt
```
prescription_id,appointment_id,patient_id,doctor_id,date,medicines,notes
1,1,1,1,15-04-2026,Paracetamol 500mg;Amoxicillin 250mg,Take after meals
```

### security_log.txt
```
timestamp,role,entered_id,result
15-04-2026 09:32:11,Patient,99,FAILED
```

---

## 🔨 How to Compile

### Requirements
- **IDE:** Visual Studio 2019 or later (Windows)
- **Framework:** SFML 3.x (linked via Visual Studio project properties)
- **Standard:** C++17 or later

### Steps
1. Clone or download the repository
2. Open `HospitalManagement.sln` in Visual Studio
3. Ensure SFML is linked:
   - `Properties → C/C++ → Additional Include Directories` → path to SFML `include/`
   - `Properties → Linker → Additional Library Directories` → path to SFML `lib/`
   - `Properties → Linker → Input → Additional Dependencies` → add SFML libs
4. Build the solution: `Ctrl + Shift + B`
5. Run: `Ctrl + F5`

---

## ▶️ How to Run

1. Make sure all `.txt` data files are in the **same directory as the executable**
2. Place SFML `.dll` files alongside the executable
3. Place `assets/` folder (containing `font.ttf` and background images) alongside the executable
4. Run the executable
5. Login using your role credentials

### Available Time Slots
```
09:00  10:00  11:00  12:00  13:00  14:00  15:00  16:00
```

### Login Lockout
After **3 consecutive failed login attempts**, the account is locked and the event is logged to `security_log.txt`.

---

## 🔐 Default Credentials

| Role | ID | Password |
|---|---|---|
| Admin | — | admin123 |
| Patient | 1 | pass123 |
| Doctor | 1 | doc456 |

---

## 📌 Important Notes

- `std::vector`, `std::string`, and all built-in string functions (`strcmp`, `strtok` etc.) are **not used** anywhere — all string operations are implemented manually in `utility.cpp`
- No **global variables** are used — all data passed as function parameters
- No **static arrays** except inside `Storage<T>` as required by the specification
- All dynamic memory is properly freed — no memory leaks
- All file I/O is strictly isolated inside `FileHandler`
- All input validation is strictly isolated inside `Validator`

---

## 🔗 GitHub Repository

**[(https://github.com/mubashirehsan10-bit/Hospital_Management_System)]**

---

## 👨‍🎓 Student Information

| Field | Details |
|---|---|
| **Name** | [MubashirEhsan] |
| **Roll Number** | [25L-0635] |
| **Section** | BCS-2A |
| **Course** | Object Oriented Programming (OOP) |
| **Semester** | Spring 2026 |
| **Instructor** | [Mr. Uzair Naqvi] |

---

*MediCore Hospital Management System — Spring 2026*
