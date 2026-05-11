#include <SFML/Graphics.hpp>
#include "MediCoreGuiAPI.h"
#include "utility.h"
#include "Validator.h"
#include "FileHandler.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include <fstream>

namespace AssetPaths
{
    static const char kDir[] = "assets/";
    static const char kMainBg[] = "Main_page.png";
    static const char kPatientBg[] = "Patient_service.png";
    static const char kDoctorBg[] = "Doctor_console.png";
    static const char kAdminBg[] = "Admin_panel.png";
}

static bool buildAssetPath(char* out, int cap, const char* fileName)
{
    med_msgClear(out, cap);
    med_msgCat(out, cap, AssetPaths::kDir);
    med_msgCat(out, cap, fileName);
    return mystrlen(out) > 0;
}

static bool loadTextureFromAssets(sf::Texture& tex, const char* fileName)
{
    char path[260];
    if (!buildAssetPath(path, sizeof path, fileName)) return false;
    return tex.loadFromFile(path);
}

static bool contains(sf::Vector2f p, const sf::RectangleShape& r)
{
    return r.getGlobalBounds().contains(p);
}

enum class UiState
{
    RoleSelect,
    LoginPatient,
    PatientHome,
    PatientBook,
    PatientCancel,
    PatientPay,
    PatientTopUp,
    LoginDoctor,
    DoctorHome,
    DoctorTodayAppts,
    DoctorMarkComplete,
    DoctorMarkNoShow,
    DoctorWritePrescription,
    DoctorViewHistory,
    LoginAdmin,
    AdminHome,
    AdminPatients,
    AdminDoctors,
    AdminDischarge,
    AdminAddDoctor,
    AdminRemoveDoctor,
    AdminAppointments,
    AdminUnpaidBills,
    AdminSecurityLog
};

static void appendTypedChar(char* buf, int cap, char32_t code)
{
    if (code < 32 || code > 126) return;
    int n = mystrlen(buf);
    if (n + 1 >= cap) return;
    buf[n] = static_cast<char>(code);
    buf[n + 1] = '\0';
}

static void backspaceChar(char* buf)
{
    int n = mystrlen(buf);
    if (n <= 0) return;
    buf[n - 1] = '\0';
}

static bool tryLoginPatient(Storage<Patient>& patients, int id, const char* password, char* err, int errCap)
{
    med_msgClear(err, errCap);
    Patient* p = patients.findByID(id);
    if (p == nullptr) { med_msgCat(err, errCap, "Invalid credentials."); return false; }
    if (mystrcmp(p->getPassword(), password) != 0) { med_msgCat(err, errCap, "Invalid credentials."); return false; }
    return true;
}

static bool tryLoginDoctor(Storage<Doctor>& doctors, int id, const char* password, char* err, int errCap)
{
    med_msgClear(err, errCap);
    Doctor* d = doctors.findByID(id);
    if (d == nullptr) { med_msgCat(err, errCap, "Invalid credentials."); return false; }
    if (mystrcmp(d->getPassword(), password) != 0) { med_msgCat(err, errCap, "Invalid credentials."); return false; }
    return true;
}

static bool tryLoginAdmin(Admin* admin, const char* password, char* err, int errCap)
{
    med_msgClear(err, errCap);
    if (admin == nullptr) { med_msgCat(err, errCap, "No admin loaded."); return false; }
    if (mystrcmp(admin->getPassword(), password) != 0) { med_msgCat(err, errCap, "Invalid credentials."); return false; }
    return true;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1280u, 720u }), "MediCore Hospital", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    sf::Font font;
    if (!font.openFromFile("assets/font.ttf"))
        font.openFromFile("C:/Windows/Fonts/arial.ttf");

    // --- Data ---
    int loggedPatientId = -1;
    int loggedDoctorId = -1;
    Patient* sessionPatient = nullptr;
    Doctor* sessionDoctor = nullptr;

    Storage<Patient>     patients;
    Storage<Doctor>      doctors;
    Storage<Appointment> appointments;
    Storage<Bill>        bills;
    Storage<Prescription>prescriptions;
    Admin* admin = nullptr;
    FileHandler fh;
    Validator   validator;

    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);

    auto refreshSessions = [&]()
        {
            sessionPatient = (loggedPatientId >= 0) ? patients.findByID(loggedPatientId) : nullptr;
            sessionDoctor = (loggedDoctorId >= 0) ? doctors.findByID(loggedDoctorId) : nullptr;
        };
    refreshSessions();

    // --- Textures ---
    sf::Texture texMain, texPatient, texDoctor, texAdmin;
    bool hasMain = loadTextureFromAssets(texMain, AssetPaths::kMainBg);
    bool hasPat = loadTextureFromAssets(texPatient, AssetPaths::kPatientBg);
    bool hasDoc = loadTextureFromAssets(texDoctor, AssetPaths::kDoctorBg);
    bool hasAdm = loadTextureFromAssets(texAdmin, AssetPaths::kAdminBg);

    sf::Sprite sprMain(texMain), sprPatient(texPatient), sprDoctor(texDoctor), sprAdmin(texAdmin);

    auto scaleSpr = [&](sf::Sprite& s, bool has)
        {
            if (!has) return;
            auto sz = s.getTexture().getSize();
            if (sz.x == 0 || sz.y == 0) return;
            s.setScale({ (float)window.getSize().x / sz.x, (float)window.getSize().y / sz.y });
        };
    scaleSpr(sprMain, hasMain); scaleSpr(sprPatient, hasPat);
    scaleSpr(sprDoctor, hasDoc); scaleSpr(sprAdmin, hasAdm);

    sf::RectangleShape fallbackBg({ (float)window.getSize().x, (float)window.getSize().y });
    fallbackBg.setFillColor(sf::Color(25, 40, 60));

    UiState state = UiState::RoleSelect;

    // --- Input buffers ---
    char idBuf[32] = "";
    char passBuf[64] = "";
    int  loginFocus = 0;
    int  bookFocus = 0;
    char specBuf[64] = "";
    char docIdBuf[32] = "";
    char dateBuf[32] = "";
    char slotBuf[16] = "";
    char apptCancelBuf[32] = "";
    char billPayBuf[32] = "";
    char amountBuf[32] = "";
    char adminPidBuf[32] = "";
    char adName[64] = "";
    char adSpec[64] = "";
    char adContact[16] = "";
    char adPass[64] = "";
    char adFeeBuf[32] = "";
    char apptIdBuf[32] = "";
    char patHistIdBuf[32] = "";
    char medBuf[500] = "";
    char notesBuf[300] = "";
    char removeDocBuf[32] = "";

    char statusMsg[1024] = "";
    char bodyText[8192] = "";
    int  loginFails = 0;

    // --- Text objects ---
    sf::Text title(font, "MediCore Hospital Management System", 28u);
    title.setFillColor(sf::Color::White);
    title.setPosition({ 40.f, 30.f });

    sf::Text status(font, sf::String(), 18u);
    status.setFillColor(sf::Color(220, 255, 220));
    status.setPosition({ 40.f, 660.f });

    sf::Text body(font, sf::String(), 16u);
    body.setFillColor(sf::Color::White);
    body.setPosition({ 40.f, 120.f });

    // --- Button factory ---
    auto makeButton = [](float x, float y, float w, float h, const char* label, const sf::Font& f)
        {
            sf::RectangleShape r({ w, h });
            r.setPosition({ x, y });
            r.setFillColor(sf::Color(60, 90, 130, 200));
            r.setOutlineColor(sf::Color::White);
            r.setOutlineThickness(1.f);
            sf::Text t(f, sf::String::fromUtf8(label, label + mystrlen(label)), 18u);
            t.setFillColor(sf::Color::White);
            t.setPosition({ x + 10.f, y + 8.f });
            return std::pair<sf::RectangleShape, sf::Text>(r, t);
        };

    auto drawPair = [&](sf::RenderTarget& target, std::pair<sf::RectangleShape, sf::Text>& p)
        {
            target.draw(p.first);
            target.draw(p.second);
        };

    // --- Role select buttons ---
    auto btnPatient = makeButton(80.f, 200.f, 220.f, 44.f, "1. Patient", font);
    auto btnDoctor = makeButton(80.f, 260.f, 220.f, 44.f, "2. Doctor", font);
    auto btnAdmin = makeButton(80.f, 320.f, 220.f, 44.f, "3. Admin", font);
    auto btnExit = makeButton(80.f, 380.f, 220.f, 44.f, "4. Exit", font);

    // --- Shared buttons ---
    auto btnLogin = makeButton(900.f, 520.f, 160.f, 40.f, "Login", font);
    auto btnBack = makeButton(900.f, 580.f, 160.f, 40.f, "Back", font);
    auto btnSubmit = makeButton(900.f, 520.f, 160.f, 40.f, "Submit", font);

    // --- Patient menu buttons ---
    auto pBook = makeButton(80.f, 180.f, 280.f, 40.f, "Book Appointment", font);
    auto pCancel = makeButton(80.f, 230.f, 280.f, 40.f, "Cancel Appointment", font);
    auto pPay = makeButton(80.f, 280.f, 280.f, 40.f, "Pay Bill", font);
    auto pTop = makeButton(80.f, 330.f, 280.f, 40.f, "Top Up Balance", font);
    auto pLogout = makeButton(80.f, 500.f, 280.f, 40.f, "Logout", font);

    // --- Doctor menu buttons ---
    auto dToday = makeButton(80.f, 180.f, 280.f, 40.f, "Today's Appointments", font);
    auto dComplete = makeButton(80.f, 230.f, 280.f, 40.f, "Mark Complete", font);
    auto dNoShow = makeButton(80.f, 280.f, 280.f, 40.f, "Mark No-Show", font);
    auto dPrescription = makeButton(80.f, 330.f, 280.f, 40.f, "Write Prescription", font);
    auto dHistory = makeButton(80.f, 380.f, 280.f, 40.f, "Patient History", font);
    auto dLogout = makeButton(80.f, 500.f, 280.f, 40.f, "Logout", font);

    // --- Admin menu buttons ---
    auto aPatients = makeButton(80.f, 180.f, 280.f, 40.f, "View Patients", font);
    auto aDoctors = makeButton(80.f, 230.f, 280.f, 40.f, "View Doctors", font);
    auto aDisch = makeButton(80.f, 280.f, 280.f, 40.f, "Discharge Patient", font);
    auto aAddDr = makeButton(80.f, 330.f, 280.f, 40.f, "Add Doctor", font);
    auto aRemoveDoc = makeButton(380.f, 180.f, 280.f, 40.f, "Remove Doctor", font);
    auto aAppointments = makeButton(380.f, 230.f, 280.f, 40.f, "View Appointments", font);
    auto aUnpaid = makeButton(380.f, 280.f, 280.f, 40.f, "Unpaid Bills", font);
    auto aSecLog = makeButton(380.f, 330.f, 280.f, 40.f, "Security Log", font);
    auto aLogout = makeButton(80.f, 560.f, 280.f, 40.f, "Logout", font);

    // =====================================================================
    // MAIN LOOP
    // =====================================================================
    while (window.isOpen())
    {
        // -----------------------------------------------------------------
        // EVENT HANDLING
        // -----------------------------------------------------------------
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) { window.close(); break; }

            // --- Mouse click ---
            if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mp->button != sf::Mouse::Button::Left) continue;
                sf::Vector2f pos({ (float)mp->position.x, (float)mp->position.y });

                // Role select
                if (state == UiState::RoleSelect)
                {
                    if (contains(pos, btnPatient.first))
                    {
                        state = UiState::LoginPatient;
                        med_msgClear(idBuf, sizeof idBuf);
                        med_msgClear(passBuf, sizeof passBuf);
                        loginFocus = 0; loginFails = 0;
                        med_msgClear(statusMsg, sizeof statusMsg);
                    }
                    else if (contains(pos, btnDoctor.first))
                    {
                        state = UiState::LoginDoctor;
                        med_msgClear(idBuf, sizeof idBuf);
                        med_msgClear(passBuf, sizeof passBuf);
                        loginFocus = 0; loginFails = 0;
                    }
                    else if (contains(pos, btnAdmin.first))
                    {
                        state = UiState::LoginAdmin;
                        med_msgClear(passBuf, sizeof passBuf);
                        loginFails = 0;
                    }
                    else if (contains(pos, btnExit.first))
                        window.close();
                }

                // Login screens
                else if (state == UiState::LoginPatient || state == UiState::LoginDoctor || state == UiState::LoginAdmin)
                {
                    if (contains(pos, btnBack.first))
                    {
                        state = UiState::RoleSelect;
                        loginFails = 0;
                    }
                    else if (contains(pos, btnLogin.first))
                    {
                        int id = myatoi(idBuf);
                        bool ok = false;
                        if (state == UiState::LoginPatient) ok = tryLoginPatient(patients, id, passBuf, statusMsg, sizeof statusMsg);
                        else if (state == UiState::LoginDoctor)  ok = tryLoginDoctor(doctors, id, passBuf, statusMsg, sizeof statusMsg);
                        else                                     ok = tryLoginAdmin(admin, passBuf, statusMsg, sizeof statusMsg);

                        if (ok)
                        {
                            if (state == UiState::LoginPatient) { loggedPatientId = id; refreshSessions(); state = UiState::PatientHome; }
                            else if (state == UiState::LoginDoctor) { loggedDoctorId = id; refreshSessions(); state = UiState::DoctorHome; }
                            else state = UiState::AdminHome;
                            loginFails = 0;
                            med_msgClear(statusMsg, sizeof statusMsg);
                        }
                        else
                        {
                            loginFails++;
                            char ts[48]; formatTimestampFull(ts, sizeof ts);
                            if (loginFails >= 3)
                            {
                                char idstr[16];
                                if (state == UiState::LoginAdmin && admin != nullptr) myitoa(admin->getId(), idstr);
                                else myitoa(id, idstr);
                                if (state == UiState::LoginAdmin)   fh.appendSecurityLog(ts, "Admin", idstr, "FAILED");
                                else if (state == UiState::LoginPatient) fh.appendSecurityLog(ts, "Patient", idstr, "FAILED");
                                else                                     fh.appendSecurityLog(ts, "Doctor", idstr, "FAILED");
                                med_msgCat(statusMsg, sizeof statusMsg, " Account locked. Contact admin.");
                                state = UiState::RoleSelect;
                                loginFails = 0;
                            }
                        }
                    }
                    // focus switch for login fields
                    if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
                    {
                        // clicking approximate password area switches focus
                        if (mp->position.y > 490 && mp->position.y < 530 && mp->position.x < 400)
                            loginFocus = 1;
                        else if (mp->position.y > 450 && mp->position.y < 490 && mp->position.x < 400)
                            loginFocus = 0;
                    }
                }

                // Patient home
                else if (state == UiState::PatientHome)
                {
                    if (contains(pos, pBook.first))
                    {
                        state = UiState::PatientBook; bookFocus = 0;
                        med_msgClear(specBuf, sizeof specBuf);
                        med_msgClear(docIdBuf, sizeof docIdBuf);
                        med_msgClear(dateBuf, sizeof dateBuf);
                        med_msgClear(slotBuf, sizeof slotBuf);
                    }
                    else if (contains(pos, pCancel.first)) { state = UiState::PatientCancel; med_msgClear(apptCancelBuf, sizeof apptCancelBuf); }
                    else if (contains(pos, pPay.first)) { state = UiState::PatientPay;    med_msgClear(billPayBuf, sizeof billPayBuf); }
                    else if (contains(pos, pTop.first)) { state = UiState::PatientTopUp;  med_msgClear(amountBuf, sizeof amountBuf); }
                    else if (contains(pos, pLogout.first) || contains(pos, btnBack.first))
                    {
                        loggedPatientId = -1; sessionPatient = nullptr; state = UiState::RoleSelect;
                    }
                }

                // Patient book submit
                else if (state == UiState::PatientBook && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiBookAppointment(validator, fh, sessionPatient, bills, doctors, appointments,
                        specBuf, myatoi(docIdBuf), dateBuf, slotBuf, msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::PatientHome;
                }
                else if (state == UiState::PatientBook && contains(pos, btnBack.first))
                    state = UiState::PatientHome;

                // Patient cancel submit
                else if (state == UiState::PatientCancel && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiCancelAppointment(fh, sessionPatient, doctors, bills, appointments, myatoi(apptCancelBuf), msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::PatientHome;
                }
                else if (state == UiState::PatientCancel && contains(pos, btnBack.first))
                    state = UiState::PatientHome;

                // Patient pay submit
                else if (state == UiState::PatientPay && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiPayBill(validator, fh, sessionPatient, bills, myatoi(billPayBuf), msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::PatientHome;
                }
                else if (state == UiState::PatientPay && contains(pos, btnBack.first))
                    state = UiState::PatientHome;

                // Patient topup submit
                else if (state == UiState::PatientTopUp && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiTopUp(validator, fh, sessionPatient, myatof(amountBuf), msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::PatientHome;
                }
                else if (state == UiState::PatientTopUp && contains(pos, btnBack.first))
                    state = UiState::PatientHome;

                // Doctor home
                else if (state == UiState::DoctorHome)
                {
                    if (contains(pos, dToday.first))
                        state = UiState::DoctorTodayAppts;
                    else if (contains(pos, dComplete.first))
                    {
                        med_msgClear(apptIdBuf, sizeof apptIdBuf);
                        state = UiState::DoctorMarkComplete;
                    }
                    else if (contains(pos, dNoShow.first))
                    {
                        med_msgClear(apptIdBuf, sizeof apptIdBuf);
                        state = UiState::DoctorMarkNoShow;
                    }
                    else if (contains(pos, dPrescription.first))
                    {
                        med_msgClear(apptIdBuf, sizeof apptIdBuf);
                        med_msgClear(medBuf, sizeof medBuf);
                        med_msgClear(notesBuf, sizeof notesBuf);
                        bookFocus = 0;
                        state = UiState::DoctorWritePrescription;
                    }
                    else if (contains(pos, dHistory.first))
                    {
                        med_msgClear(patHistIdBuf, sizeof patHistIdBuf);
                        state = UiState::DoctorViewHistory;
                    }
                    else if (contains(pos, dLogout.first) || contains(pos, btnBack.first))
                    {
                        loggedDoctorId = -1; sessionDoctor = nullptr;
                        state = UiState::RoleSelect;
                    }
                }

                // Doctor today appts back
                else if (state == UiState::DoctorTodayAppts && contains(pos, btnBack.first))
                    state = UiState::DoctorHome;

                // Doctor mark complete
                else if (state == UiState::DoctorMarkComplete && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiMarkComplete(fh, sessionDoctor, appointments, myatoi(apptIdBuf), msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::DoctorHome;
                }
                else if (state == UiState::DoctorMarkComplete && contains(pos, btnBack.first))
                    state = UiState::DoctorHome;

                // Doctor mark no show
                else if (state == UiState::DoctorMarkNoShow && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiMarkNoShow(fh, sessionDoctor, appointments, bills, myatoi(apptIdBuf), msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::DoctorHome;
                }
                else if (state == UiState::DoctorMarkNoShow && contains(pos, btnBack.first))
                    state = UiState::DoctorHome;

                // Doctor write prescription
                else if (state == UiState::DoctorWritePrescription && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiWritePrescription(fh, sessionDoctor, appointments, prescriptions,
                        myatoi(apptIdBuf), medBuf, notesBuf, msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::DoctorHome;
                }
                else if (state == UiState::DoctorWritePrescription && contains(pos, btnBack.first))
                    state = UiState::DoctorHome;

                // Doctor view history
                else if (state == UiState::DoctorViewHistory && contains(pos, btnSubmit.first))
                {
                    med_formatPatientHistory(prescriptions, appointments, sessionDoctor,
                        myatoi(patHistIdBuf), bodyText, sizeof bodyText);
                    state = UiState::DoctorTodayAppts;
                }
                else if (state == UiState::DoctorViewHistory && contains(pos, btnBack.first))
                    state = UiState::DoctorHome;

                // Admin home
                else if (state == UiState::AdminHome)
                {
                    if (contains(pos, aPatients.first))
                        state = UiState::AdminPatients;
                    else if (contains(pos, aDoctors.first))
                        state = UiState::AdminDoctors;
                    else if (contains(pos, aDisch.first))
                    {
                        med_msgClear(adminPidBuf, sizeof adminPidBuf);
                        state = UiState::AdminDischarge;
                    }
                    else if (contains(pos, aAddDr.first))
                    {
                        bookFocus = 0;
                        med_msgClear(adName, sizeof adName); med_msgClear(adSpec, sizeof adSpec);
                        med_msgClear(adContact, sizeof adContact); med_msgClear(adPass, sizeof adPass);
                        med_msgClear(adFeeBuf, sizeof adFeeBuf);
                        state = UiState::AdminAddDoctor;
                    }
                    else if (contains(pos, aRemoveDoc.first))
                    {
                        med_msgClear(removeDocBuf, sizeof removeDocBuf);
                        state = UiState::AdminRemoveDoctor;
                    }
                    else if (contains(pos, aAppointments.first))
                        state = UiState::AdminAppointments;
                    else if (contains(pos, aUnpaid.first))
                        state = UiState::AdminUnpaidBills;
                    else if (contains(pos, aSecLog.first))
                        state = UiState::AdminSecurityLog;
                    else if (contains(pos, aLogout.first) || contains(pos, btnBack.first))
                        state = UiState::RoleSelect;
                }

                // Admin patients back
                else if (state == UiState::AdminPatients && contains(pos, btnBack.first))
                    state = UiState::AdminHome;

                // Admin doctors back
                else if (state == UiState::AdminDoctors && contains(pos, btnBack.first))
                    state = UiState::AdminHome;

                // Admin discharge
                else if (state == UiState::AdminDischarge && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiDischargePatient(fh, patients, appointments, bills, prescriptions, myatoi(adminPidBuf), msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::AdminHome;
                }
                else if (state == UiState::AdminDischarge && contains(pos, btnBack.first))
                    state = UiState::AdminHome;

                // Admin add doctor
                else if (state == UiState::AdminAddDoctor && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiAddDoctor(validator, fh, doctors, adName, adSpec, adContact, adPass, myatof(adFeeBuf), msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::AdminHome;
                }
                else if (state == UiState::AdminAddDoctor && contains(pos, btnBack.first))
                    state = UiState::AdminHome;

                // Admin remove doctor
                else if (state == UiState::AdminRemoveDoctor && contains(pos, btnSubmit.first))
                {
                    char msg[512];
                    med_guiRemoveDoctor(fh, doctors, appointments, myatoi(removeDocBuf), msg, sizeof msg);
                    med_msgClear(statusMsg, sizeof statusMsg);
                    med_msgCat(statusMsg, sizeof statusMsg, msg);
                    med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
                    refreshSessions(); state = UiState::AdminHome;
                }
                else if (state == UiState::AdminRemoveDoctor && contains(pos, btnBack.first))
                    state = UiState::AdminHome;

                // Admin appointments back
                else if (state == UiState::AdminAppointments && contains(pos, btnBack.first))
                    state = UiState::AdminHome;

                // Admin unpaid bills back
                else if (state == UiState::AdminUnpaidBills && contains(pos, btnBack.first))
                    state = UiState::AdminHome;

                // Admin security log back
                else if (state == UiState::AdminSecurityLog && contains(pos, btnBack.first))
                    state = UiState::AdminHome;
            }

            // --- Text input ---
            if (const auto* txt = event->getIf<sf::Event::TextEntered>())
            {
                char32_t c = txt->unicode;
                if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
                {
                    if (loginFocus == 0) appendTypedChar(idBuf, sizeof idBuf, c);
                    else                 appendTypedChar(passBuf, sizeof passBuf, c);
                }
                else if (state == UiState::LoginAdmin)
                    appendTypedChar(passBuf, sizeof passBuf, c);
                else if (state == UiState::PatientBook)
                {
                    if (bookFocus == 0) appendTypedChar(specBuf, sizeof specBuf, c);
                    else if (bookFocus == 1) appendTypedChar(docIdBuf, sizeof docIdBuf, c);
                    else if (bookFocus == 2) appendTypedChar(dateBuf, sizeof dateBuf, c);
                    else                     appendTypedChar(slotBuf, sizeof slotBuf, c);
                }
                else if (state == UiState::PatientCancel)  appendTypedChar(apptCancelBuf, sizeof apptCancelBuf, c);
                else if (state == UiState::PatientPay)     appendTypedChar(billPayBuf, sizeof billPayBuf, c);
                else if (state == UiState::PatientTopUp)   appendTypedChar(amountBuf, sizeof amountBuf, c);
                else if (state == UiState::AdminDischarge) appendTypedChar(adminPidBuf, sizeof adminPidBuf, c);
                else if (state == UiState::AdminAddDoctor)
                {
                    if (bookFocus == 0) appendTypedChar(adName, sizeof adName, c);
                    else if (bookFocus == 1) appendTypedChar(adSpec, sizeof adSpec, c);
                    else if (bookFocus == 2) appendTypedChar(adContact, sizeof adContact, c);
                    else if (bookFocus == 3) appendTypedChar(adPass, sizeof adPass, c);
                    else                     appendTypedChar(adFeeBuf, sizeof adFeeBuf, c);
                }
                else if (state == UiState::DoctorMarkComplete || state == UiState::DoctorMarkNoShow)
                    appendTypedChar(apptIdBuf, sizeof apptIdBuf, c);
                else if (state == UiState::DoctorWritePrescription)
                {
                    if (bookFocus == 0) appendTypedChar(apptIdBuf, sizeof apptIdBuf, c);
                    else if (bookFocus == 1) appendTypedChar(medBuf, sizeof medBuf, c);
                    else                     appendTypedChar(notesBuf, sizeof notesBuf, c);
                }
                else if (state == UiState::DoctorViewHistory)
                    appendTypedChar(patHistIdBuf, sizeof patHistIdBuf, c);
                else if (state == UiState::AdminRemoveDoctor)
                    appendTypedChar(removeDocBuf, sizeof removeDocBuf, c);
            }

            // --- Key press (Tab / Backspace) ---
            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Tab)
                {
                    if (state == UiState::LoginPatient || state == UiState::LoginDoctor) loginFocus = loginFocus == 0 ? 1 : 0;
                    else if (state == UiState::PatientBook)          bookFocus = (bookFocus + 1) % 4;
                    else if (state == UiState::AdminAddDoctor)       bookFocus = (bookFocus + 1) % 5;
                    else if (state == UiState::DoctorWritePrescription) bookFocus = (bookFocus + 1) % 3;
                }
                if (key->code == sf::Keyboard::Key::Backspace)
                {
                    if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
                    {
                        if (loginFocus == 1) backspaceChar(passBuf); else backspaceChar(idBuf);
                    }
                    else if (state == UiState::LoginAdmin)     backspaceChar(passBuf);
                    else if (state == UiState::PatientBook)
                    {
                        if (bookFocus == 3) backspaceChar(slotBuf);
                        else if (bookFocus == 2) backspaceChar(dateBuf);
                        else if (bookFocus == 1) backspaceChar(docIdBuf);
                        else                     backspaceChar(specBuf);
                    }
                    else if (state == UiState::PatientCancel)  backspaceChar(apptCancelBuf);
                    else if (state == UiState::PatientPay)     backspaceChar(billPayBuf);
                    else if (state == UiState::PatientTopUp)   backspaceChar(amountBuf);
                    else if (state == UiState::AdminDischarge) backspaceChar(adminPidBuf);
                    else if (state == UiState::AdminAddDoctor)
                    {
                        if (bookFocus == 4) backspaceChar(adFeeBuf);
                        else if (bookFocus == 3) backspaceChar(adPass);
                        else if (bookFocus == 2) backspaceChar(adContact);
                        else if (bookFocus == 1) backspaceChar(adSpec);
                        else                     backspaceChar(adName);
                    }
                    else if (state == UiState::DoctorMarkComplete || state == UiState::DoctorMarkNoShow)
                        backspaceChar(apptIdBuf);
                    else if (state == UiState::DoctorWritePrescription)
                    {
                        if (bookFocus == 2) backspaceChar(notesBuf);
                        else if (bookFocus == 1) backspaceChar(medBuf);
                        else                     backspaceChar(apptIdBuf);
                    }
                    else if (state == UiState::DoctorViewHistory)  backspaceChar(patHistIdBuf);
                    else if (state == UiState::AdminRemoveDoctor)  backspaceChar(removeDocBuf);
                }
            }
        } // end event loop

        // -----------------------------------------------------------------
        // DRAW BACKGROUND
        // -----------------------------------------------------------------
        window.clear(sf::Color::Black);
        if (state == UiState::RoleSelect || state == UiState::LoginPatient ||
            state == UiState::LoginDoctor || state == UiState::LoginAdmin)
        {
            if (hasMain) window.draw(sprMain); else window.draw(fallbackBg);
        }
        else if (state == UiState::PatientHome || state == UiState::PatientBook ||
            state == UiState::PatientCancel || state == UiState::PatientPay ||
            state == UiState::PatientTopUp)
        {
            if (hasPat) window.draw(sprPatient); else window.draw(fallbackBg);
        }
        else if (state == UiState::DoctorHome || state == UiState::DoctorTodayAppts ||
            state == UiState::DoctorMarkComplete || state == UiState::DoctorMarkNoShow ||
            state == UiState::DoctorWritePrescription || state == UiState::DoctorViewHistory)
        {
            if (hasDoc) window.draw(sprDoctor); else window.draw(fallbackBg);
        }
        else
        {
            if (hasAdm) window.draw(sprAdmin); else window.draw(fallbackBg);
        }

        window.draw(title);

        // -----------------------------------------------------------------
        // DRAW UI PER STATE
        // -----------------------------------------------------------------
        if (state == UiState::RoleSelect)
        {
            drawPair(window, btnPatient);
            drawPair(window, btnDoctor);
            drawPair(window, btnAdmin);
            drawPair(window, btnExit);
        }
        else if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
        {
            sf::Text hint(font, "ID (top) | Password (bottom). Tab=switch. Backspace=delete.", 16u);
            hint.setFillColor(sf::Color::White); hint.setPosition({ 40.f, 400.f });
            window.draw(hint);
            sf::Text idLabel(font, "ID:", 18u);    idLabel.setFillColor(sf::Color::White);  idLabel.setPosition({ 40.f, 455.f }); window.draw(idLabel);
            sf::Text idv(font, sf::String::fromUtf8(idBuf, idBuf + mystrlen(idBuf)), 20u);
            idv.setPosition({ 80.f, 455.f }); idv.setFillColor(loginFocus == 0 ? sf::Color::Yellow : sf::Color::White); window.draw(idv);
            sf::Text pLabel(font, "PW:", 18u);     pLabel.setFillColor(sf::Color::White);   pLabel.setPosition({ 40.f, 495.f }); window.draw(pLabel);
            sf::Text pv(font, sf::String::fromUtf8(passBuf, passBuf + mystrlen(passBuf)), 20u);
            pv.setPosition({ 80.f, 495.f }); pv.setFillColor(loginFocus == 1 ? sf::Color::Yellow : sf::Color::White); window.draw(pv);
            drawPair(window, btnLogin);
            drawPair(window, btnBack);
        }
        else if (state == UiState::LoginAdmin)
        {
            sf::Text hint(font, "Admin password:", 18u);
            hint.setFillColor(sf::Color::White); hint.setPosition({ 40.f, 460.f }); window.draw(hint);
            sf::Text pv(font, sf::String::fromUtf8(passBuf, passBuf + mystrlen(passBuf)), 20u);
            pv.setPosition({ 40.f, 495.f }); pv.setFillColor(sf::Color::Yellow); window.draw(pv);
            drawPair(window, btnLogin);
            drawPair(window, btnBack);
        }
        else if (state == UiState::PatientHome)
        {
            if (sessionPatient)
            {
                char welcome[128] = "Welcome, ";
                mystrcat(welcome, sessionPatient->getName());
                sf::Text wt(font, sf::String::fromUtf8(welcome, welcome + mystrlen(welcome)), 22u);
                wt.setFillColor(sf::Color::White); wt.setPosition({ 40.f, 120.f }); window.draw(wt);
                char bal[64] = "Balance: PKR ";
                char bstr[32]; myftoa(sessionPatient->getBalance(), bstr);
                mystrcat(bal, bstr);
                sf::Text bt(font, sf::String::fromUtf8(bal, bal + mystrlen(bal)), 18u);
                bt.setFillColor(sf::Color(220, 255, 180)); bt.setPosition({ 40.f, 148.f }); window.draw(bt);
            }
            drawPair(window, pBook); drawPair(window, pCancel);
            drawPair(window, pPay);  drawPair(window, pTop);
            drawPair(window, pLogout);
        }
        else if (state == UiState::PatientBook)
        {
            sf::Text hint(font, "Tab=next field  Backspace=delete", 15u);
            hint.setFillColor(sf::Color::White); hint.setPosition({ 40.f, 120.f }); window.draw(hint);
            med_msgClear(bodyText, sizeof bodyText);
            med_msgCat(bodyText, sizeof bodyText, "Specialization: "); med_msgCat(bodyText, sizeof bodyText, specBuf);
            med_msgCat(bodyText, sizeof bodyText, "\nDoctor ID:      "); med_msgCat(bodyText, sizeof bodyText, docIdBuf);
            med_msgCat(bodyText, sizeof bodyText, "\nDate (DD-MM-YYYY): "); med_msgCat(bodyText, sizeof bodyText, dateBuf);
            med_msgCat(bodyText, sizeof bodyText, "\nTime Slot:      "); med_msgCat(bodyText, sizeof bodyText, slotBuf);
            med_msgCat(bodyText, sizeof bodyText, "\n\nSlots: 09:00 10:00 11:00 12:00 13:00 14:00 15:00 16:00");
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::PatientCancel)
        {
            sf::Text t(font, "Enter Appointment ID to cancel:", 18u);
            t.setFillColor(sf::Color::White); t.setPosition({ 40.f, 200.f }); window.draw(t);
            sf::Text v(font, sf::String::fromUtf8(apptCancelBuf, apptCancelBuf + mystrlen(apptCancelBuf)), 22u);
            v.setPosition({ 40.f, 240.f }); v.setFillColor(sf::Color::Yellow); window.draw(v);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::PatientPay)
        {
            sf::Text t(font, "Enter Bill ID to pay:", 18u);
            t.setFillColor(sf::Color::White); t.setPosition({ 40.f, 200.f }); window.draw(t);
            sf::Text v(font, sf::String::fromUtf8(billPayBuf, billPayBuf + mystrlen(billPayBuf)), 22u);
            v.setPosition({ 40.f, 240.f }); v.setFillColor(sf::Color::Yellow); window.draw(v);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::PatientTopUp)
        {
            sf::Text t(font, "Enter amount to add (PKR):", 18u);
            t.setFillColor(sf::Color::White); t.setPosition({ 40.f, 200.f }); window.draw(t);
            sf::Text v(font, sf::String::fromUtf8(amountBuf, amountBuf + mystrlen(amountBuf)), 22u);
            v.setPosition({ 40.f, 240.f }); v.setFillColor(sf::Color::Yellow); window.draw(v);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }

        // --- Doctor states ---
        else if (state == UiState::DoctorHome)
        {
            if (sessionDoctor)
            {
                char welcome[128] = "Welcome, Dr. ";
                mystrcat(welcome, sessionDoctor->getName());
                mystrcat(welcome, " | ");
                mystrcat(welcome, sessionDoctor->getSpecialization());
                sf::Text wt(font, sf::String::fromUtf8(welcome, welcome + mystrlen(welcome)), 20u);
                wt.setFillColor(sf::Color::White); wt.setPosition({ 40.f, 120.f }); window.draw(wt);
            }
            drawPair(window, dToday); drawPair(window, dComplete);
            drawPair(window, dNoShow); drawPair(window, dPrescription);
            drawPair(window, dHistory); drawPair(window, dLogout);
        }
        else if (state == UiState::DoctorTodayAppts)
        {
            med_formatTodayAppointments(appointments, patients, sessionDoctor, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::DoctorMarkComplete || state == UiState::DoctorMarkNoShow)
        {
            const char* label = (state == UiState::DoctorMarkComplete) ?
                "Mark Complete — Enter Appointment ID:" : "Mark No-Show — Enter Appointment ID:";
            sf::Text t(font, label, 18u);
            t.setFillColor(sf::Color::White); t.setPosition({ 40.f, 200.f }); window.draw(t);
            sf::Text v(font, sf::String::fromUtf8(apptIdBuf, apptIdBuf + mystrlen(apptIdBuf)), 22u);
            v.setPosition({ 40.f, 240.f }); v.setFillColor(sf::Color::Yellow); window.draw(v);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::DoctorWritePrescription)
        {
            sf::Text hint(font, "Tab=next field  Backspace=delete", 15u);
            hint.setFillColor(sf::Color::White); hint.setPosition({ 40.f, 120.f }); window.draw(hint);
            med_msgClear(bodyText, sizeof bodyText);
            med_msgCat(bodyText, sizeof bodyText, "Appointment ID: "); med_msgCat(bodyText, sizeof bodyText, apptIdBuf);
            med_msgCat(bodyText, sizeof bodyText, "\nMedicines:     "); med_msgCat(bodyText, sizeof bodyText, medBuf);
            med_msgCat(bodyText, sizeof bodyText, "\nNotes:         "); med_msgCat(bodyText, sizeof bodyText, notesBuf);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::DoctorViewHistory)
        {
            sf::Text t(font, "Enter Patient ID then Submit:", 18u);
            t.setFillColor(sf::Color::White); t.setPosition({ 40.f, 200.f }); window.draw(t);
            sf::Text v(font, sf::String::fromUtf8(patHistIdBuf, patHistIdBuf + mystrlen(patHistIdBuf)), 22u);
            v.setPosition({ 40.f, 240.f }); v.setFillColor(sf::Color::Yellow); window.draw(v);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }

        // --- Admin states ---
        else if (state == UiState::AdminHome)
        {
            drawPair(window, aPatients); drawPair(window, aDoctors);
            drawPair(window, aDisch);    drawPair(window, aAddDr);
            drawPair(window, aRemoveDoc); drawPair(window, aAppointments);
            drawPair(window, aUnpaid);   drawPair(window, aSecLog);
            drawPair(window, aLogout);
        }
        else if (state == UiState::AdminPatients)
        {
            med_formatPatientList(patients, bills, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminDoctors)
        {
            med_formatDoctorList(doctors, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminDischarge)
        {
            sf::Text t(font, "Enter Patient ID to discharge:", 18u);
            t.setFillColor(sf::Color::White); t.setPosition({ 40.f, 200.f }); window.draw(t);
            sf::Text v(font, sf::String::fromUtf8(adminPidBuf, adminPidBuf + mystrlen(adminPidBuf)), 22u);
            v.setPosition({ 40.f, 240.f }); v.setFillColor(sf::Color::Yellow); window.draw(v);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminAddDoctor)
        {
            sf::Text hint(font, "Tab=next field  Backspace=delete", 15u);
            hint.setFillColor(sf::Color::White); hint.setPosition({ 40.f, 120.f }); window.draw(hint);
            med_msgClear(bodyText, sizeof bodyText);
            med_msgCat(bodyText, sizeof bodyText, "Name:    "); med_msgCat(bodyText, sizeof bodyText, adName);
            med_msgCat(bodyText, sizeof bodyText, "\nSpec:    "); med_msgCat(bodyText, sizeof bodyText, adSpec);
            med_msgCat(bodyText, sizeof bodyText, "\nContact: "); med_msgCat(bodyText, sizeof bodyText, adContact);
            med_msgCat(bodyText, sizeof bodyText, "\nPass:    "); med_msgCat(bodyText, sizeof bodyText, adPass);
            med_msgCat(bodyText, sizeof bodyText, "\nFee:     "); med_msgCat(bodyText, sizeof bodyText, adFeeBuf);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminRemoveDoctor)
        {
            sf::Text t(font, "Enter Doctor ID to remove:", 18u);
            t.setFillColor(sf::Color::White); t.setPosition({ 40.f, 200.f }); window.draw(t);
            sf::Text v(font, sf::String::fromUtf8(removeDocBuf, removeDocBuf + mystrlen(removeDocBuf)), 22u);
            v.setPosition({ 40.f, 240.f }); v.setFillColor(sf::Color::Yellow); window.draw(v);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminAppointments)
        {
            med_formatAppointmentList(appointments, patients, doctors, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminUnpaidBills)
        {
            med_formatUnpaidBillsAll(bills, patients, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminSecurityLog)
        {
            med_msgClear(bodyText, sizeof bodyText);
            std::ifstream fin("security_log.txt");
            if (fin)
            {
                char line[500];
                while (!fin.eof())
                {
                    myreadLine(fin, line, 500);
                    if (mystrlen(line) == 0) continue;
                    med_msgCat(bodyText, sizeof bodyText, line);
                    med_msgCat(bodyText, sizeof bodyText, "\n");
                }
                fin.close();
            }
            if (mystrlen(bodyText) == 0)
                med_msgCat(bodyText, sizeof bodyText, "No security events logged.");
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }

        // Status bar
        status.setString(sf::String::fromUtf8(statusMsg, statusMsg + mystrlen(statusMsg)));
        window.draw(status);
        window.display();
    }

    if (admin != nullptr) delete admin;
    return 0;
}