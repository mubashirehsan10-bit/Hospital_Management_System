# MediCore Hospital Management System

## Description
A Hospital Management System built in C++ using Object-Oriented Programming principles. 
The system manages patients, doctors, appointments, prescriptions, and billing for a fictional hospital called MediCore.

## How to Compile

### Requirements
- Visual Studio 2019 or later
- Windows OS
- SFML library (for GUI)

### Steps
1. Clone the repository
2. Open `Hospital_Management_System.sln` in Visual Studio
3. Build the solution (Ctrl + Shift + B)
4. Run the executable (Ctrl + F5)

## How to Run
1. Make sure all `.txt` files are in the same directory as the executable:
   - `patients.txt`
   - `doctors.txt`
   - `admin.txt`
   - `appointments.txt`
   - `bills.txt`
   - `prescriptions.txt`
   - `security_log.txt`
   - `discharged.txt`

2. Run the executable
3. Login as Patient, Doctor or Admin

## File Formats

### patients.txt
patient_id,name,age,gender,contact,password,balance
1,Ahmed Ali,28,M,03001234567,pass123,5000.00

### doctors.txt
doctor_id,name,specialization,contact,password,fee
1,Dr. Sara Khan,Cardiology,03111234567,doc456,1500.00

### admin.txt
admin_id,name,password
1,Admin,admin123

### appointments.txt
appointment_id,patient_id,doctor_id,date,time_slot,status
1,1,1,15-04-2026,09:00,pending

### bills.txt
bill_id,patient_id,appointment_id,amount,status,date
1,1,1,1500.00,unpaid,15-04-2026

### prescriptions.txt
prescription_id,appointment_id,patient_id,doctor_id,date,medicines,notes
1,1,1,1,15-04-2026,Paracetamol 500mg,Take after meals

## OOP Concepts Used
- **Inheritance** — Patient, Doctor, Admin inherit from Person
- **Polymorphism** — Virtual functions, operator overloading
- **Abstract Classes** — Person is abstract with pure virtual methods
- **Templates** — Storage<T> generic class
- **Exception Handling** — Custom exception hierarchy
- **Encapsulation** — Private data with getters/setters
- **File Handling** — FileHandler class for all I/O

## Classes
| Class | Description |
|---|---|
| `Person` | Abstract base class |
| `Patient` | Registered patient |
| `Doctor` | Hospital doctor |
| `Admin` | System administrator |
| `Appointment` | Single appointment |
| `Bill` | Charge from appointment |
| `Prescription` | Doctor's prescription |
| `Storage<T>` | Generic template storage |
| `FileHandler` | File I/O operations |
| `Validator` | Input validation |
| `HospitalException` | Base exception class |

## GitHub Repository
[Insert GitHub Link Here]

## Student Information
- **Name:** [Mubashir Ehsan]
- **Roll Number:** [25L-0635]
- **Section:** BCS-2A
- **Course:** Object Oriented Programming
- **Semester:** Spring 2026
