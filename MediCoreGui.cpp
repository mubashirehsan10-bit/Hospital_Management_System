#include <SFML/Graphics.hpp>
#include "MediCoreGuiAPI.h"
#include "utility.h"
#include "Validator.h"
#include "FileHandler.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include <fstream>
#include <filesystem>

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
    PatientViewAppointments,
    PatientViewRecords,
    PatientViewBills,
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
    AdminDailyReport,
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
    fallbackBg.setFillColor(sf::Color(8, 12, 25));

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
    sf::Text title(font, "MediCore Hospital Management System", 30u);
    title.setFillColor(sf::Color(0, 255, 220));  // bright neon teal
    title.setPosition({ 100.f, 30.f }); // Shifted for logo icon

    sf::Text status(font, sf::String(), 18u);
    status.setFillColor(sf::Color(0, 255, 204)); // Neon cyan status text
    status.setPosition({ 80.f, 655.f });

    sf::Text body(font, sf::String(), 16u);
    body.setFillColor(sf::Color::White);
    body.setPosition({ 100.f, 185.f }); // Shifted inside glass panel

    // --- Clocks and Animations ---
    sf::Clock bgClock;
    sf::Clock cursorClock;

    struct AnimatedButton {
        sf::RectangleShape first;
        sf::Text second;
        float bx, by, bw, bh;
    };

    // Tech Particles
    struct TechParticle {
        sf::Vector2f pos;
        sf::Vector2f vel;
        float radius=0;
        sf::Color color;
    };
    const int NUM_PARTICLES = 30;
    TechParticle particles[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        particles[i].pos = { (float)(rand() % 1280), (float)(rand() % 720) };
        particles[i].vel = { ((rand() % 100) - 50) / 100.f * 25.f, ((rand() % 100) - 50) / 100.f * 25.f };
        particles[i].radius = 1.5f + (rand() % 3);
        particles[i].color = sf::Color(0, 255, 204, 50 + (rand() % 80));
    }

    // Glowing Cyber-Medical Cross Logo
    auto drawLogo = [&](sf::RenderTarget& target, float x, float y) {
        // Outer glowing cross
        sf::RectangleShape hGlow({ 46.f, 16.f });
        hGlow.setPosition({ x - 23.f, y - 8.f });
        hGlow.setFillColor(sf::Color(0, 255, 204, 80));
        target.draw(hGlow);

        sf::RectangleShape vGlow({ 16.f, 46.f });
        vGlow.setPosition({ x - 8.f, y - 23.f });
        vGlow.setFillColor(sf::Color(0, 255, 204, 80));
        target.draw(vGlow);

        // Core solid neon pink cross
        sf::RectangleShape hCore({ 40.f, 10.f });
        hCore.setPosition({ x - 20.f, y - 5.f });
        hCore.setFillColor(sf::Color(255, 0, 127));
        target.draw(hCore);

        sf::RectangleShape vCore({ 10.f, 40.f });
        vCore.setPosition({ x - 5.f, y - 20.f });
        vCore.setFillColor(sf::Color(255, 0, 127));
        target.draw(vCore);
    };

    // Bounded Input Field Helper
    auto drawInputField = [&](const char* label, const char* value, float x, float y, float w, float h, bool isFocused) {
        // Draw label
        sf::Text lbl(font, sf::String::fromUtf8(label, label + mystrlen(label)), 18u);
        lbl.setFillColor(sf::Color(180, 210, 255));
        lbl.setPosition({ x, y + 6.f });
        window.draw(lbl);

        // Draw shadow under input box
        sf::RectangleShape shadow({ w + 4.f, h + 4.f });
        shadow.setPosition({ x + 200.f - 2.f, y - 2.f });
        shadow.setFillColor(sf::Color(0, 0, 0, 90));
        window.draw(shadow);

        // Draw box
        sf::RectangleShape box({ w, h });
        box.setPosition({ x + 200.f, y });
        box.setFillColor(sf::Color(12, 18, 32, 230)); // Sleek dark cyber card
        if (isFocused) {
            box.setOutlineColor(sf::Color(0, 255, 204)); // Glowing Neon Cyan
            box.setOutlineThickness(2.f);
        } else {
            box.setOutlineColor(sf::Color(65, 85, 115, 160));
            box.setOutlineThickness(1.f);
        }
        window.draw(box);

        // Value text with animated blinking cursor
        char displayVal[600];
        mystrcpy(displayVal, value);
        if (isFocused) {
            int tMs = cursorClock.getElapsedTime().asMilliseconds();
            if ((tMs / 400) % 2 == 0) {
                mystrcat(displayVal, "|");
            }
        }

        sf::Text val(font, sf::String::fromUtf8(displayVal, displayVal + mystrlen(displayVal)), 18u);
        if (isFocused) {
            val.setFillColor(sf::Color(255, 255, 255));
        } else {
            val.setFillColor(sf::Color(170, 185, 210));
        }
        val.setPosition({ x + 212.f, y + 6.f });
        window.draw(val);
    };

    // --- Button factory ---
    auto makeButton = [](float x, float y, float w, float h, const char* label, const sf::Font& f)
    {
            sf::RectangleShape r({ w, h });
            r.setPosition({ x, y });
            r.setFillColor(sf::Color(20, 30, 50, 190));
            r.setOutlineColor(sf::Color(80, 110, 160, 150));
            r.setOutlineThickness(1.5f);

            sf::Text t(f, sf::String::fromUtf8(label, label + mystrlen(label)), 18u);
            t.setFillColor(sf::Color(200, 220, 255));
            t.setPosition({ x + 15.f, y + 8.f });

            return AnimatedButton{ r, t, x, y, w, h };
    };

    auto drawPair = [&](sf::RenderTarget& target, AnimatedButton& p)
    {
            sf::Vector2i mPosI = sf::Mouse::getPosition(window);
            sf::Vector2f mPos(static_cast<float>(mPosI.x), static_cast<float>(mPosI.y));
            
            bool hovered = p.first.getGlobalBounds().contains(mPos);
            if (hovered)
            {
                float dw = p.bw * 0.05f; // expand 5%
                float dh = p.bh * 0.05f;
                p.first.setSize({ p.bw + dw, p.bh + dh });
                p.first.setPosition({ p.bx - dw / 2.f, p.by - dh / 2.f });
                
                p.first.setFillColor(sf::Color(255, 0, 127, 220)); // Neon Pink background
                p.first.setOutlineColor(sf::Color(0, 255, 255));     // Neon Cyan Border
                p.first.setOutlineThickness(2.5f);
                
                p.second.setFillColor(sf::Color::White);
                p.second.setPosition({ p.bx - dw / 2.f + 15.f, p.by - dh / 2.f + 8.f });
            }
            else
            {
                p.first.setSize({ p.bw, p.bh });
                p.first.setPosition({ p.bx, p.by });
                
                p.first.setFillColor(sf::Color(20, 30, 50, 190));   // Indigo slate background
                p.first.setOutlineColor(sf::Color(80, 110, 160, 150));
                p.first.setOutlineThickness(1.5f);
                
                p.second.setFillColor(sf::Color(200, 220, 255));
                p.second.setPosition({ p.bx + 15.f, p.by + 8.f });
            }
            
            target.draw(p.first);
            target.draw(p.second);
    };

    // --- Role select buttons ---
    auto btnPatient = makeButton(100.f, 210.f, 260.f, 48.f, "  Patient Login", font);
    auto btnDoctor = makeButton(100.f, 270.f, 260.f, 48.f, "  Doctor Login", font);
    auto btnAdmin = makeButton(100.f, 330.f, 260.f, 48.f, "  Admin Login", font);
    auto btnExit = makeButton(100.f, 390.f, 260.f, 48.f, "  Exit System", font);

    // --- Shared buttons ---
    auto btnLogin = makeButton(900.f, 520.f, 160.f, 40.f, "Login", font);
    auto btnBack = makeButton(900.f, 580.f, 160.f, 40.f, "Back", font);
    auto btnSubmit = makeButton(900.f, 520.f, 160.f, 40.f, "Submit", font);

    // --- Patient menu buttons (expanded to 8 options) ---
    auto pBook = makeButton(80.f, 180.f, 260.f, 40.f, "Book Appointment", font);
    auto pCancel = makeButton(80.f, 230.f, 260.f, 40.f, "Cancel Appointment", font);
    auto pViewAppts = makeButton(80.f, 280.f, 260.f, 40.f, "View Appointments", font);
    auto pViewRecords = makeButton(80.f, 330.f, 260.f, 40.f, "View Medical Records", font);
    auto pViewBills = makeButton(360.f, 180.f, 260.f, 40.f, "View Bills", font);
    auto pPay = makeButton(360.f, 230.f, 260.f, 40.f, "Pay Bill", font);
    auto pTop = makeButton(360.f, 280.f, 260.f, 40.f, "Top Up Balance", font);
    auto pLogout = makeButton(360.f, 330.f, 260.f, 40.f, "Logout", font);

    // --- Doctor menu buttons ---
    auto dToday = makeButton(80.f, 180.f, 280.f, 40.f, "Today's Appointments", font);
    auto dComplete = makeButton(80.f, 230.f, 280.f, 40.f, "Mark Complete", font);
    auto dNoShow = makeButton(80.f, 280.f, 280.f, 40.f, "Mark No-Show", font);
    auto dPrescription = makeButton(80.f, 330.f, 280.f, 40.f, "Write Prescription", font);
    auto dHistory = makeButton(80.f, 380.f, 280.f, 40.f, "Patient History", font);
    auto dLogout = makeButton(80.f, 500.f, 280.f, 40.f, "Logout", font);

    // --- Admin menu buttons ---
    auto aPatients = makeButton(80.f, 180.f, 260.f, 40.f, "View Patients", font);
    auto aDoctors = makeButton(80.f, 230.f, 260.f, 40.f, "View Doctors", font);
    auto aDisch = makeButton(80.f, 280.f, 260.f, 40.f, "Discharge Patient", font);
    auto aAddDr = makeButton(80.f, 330.f, 260.f, 40.f, "Add Doctor", font);
    auto aRemoveDoc = makeButton(360.f, 180.f, 260.f, 40.f, "Remove Doctor", font);
    auto aAppointments = makeButton(360.f, 230.f, 260.f, 40.f, "Appointments", font);
    auto aUnpaid = makeButton(360.f, 280.f, 260.f, 40.f, "Unpaid Bills", font);
    auto aSecLog = makeButton(360.f, 330.f, 260.f, 40.f, "Security Log", font);
    auto aReport = makeButton(360.f, 380.f, 260.f, 40.f, "Daily Report", font);
    auto aLogout = makeButton(80.f, 390.f, 260.f, 40.f, "Logout", font);

    // =====================================================================
    // MAIN LOOP
    while (window.isOpen())
    {
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

                    // Complex form click-to-focus handlers
                    if (state == UiState::PatientBook)
                    {
                        if (pos.x >= 280.f && pos.x <= 630.f)
                        {
                            if (pos.y >= 170.f && pos.y <= 206.f) bookFocus = 0;
                            else if (pos.y >= 220.f && pos.y <= 256.f) bookFocus = 1;
                            else if (pos.y >= 270.f && pos.y <= 306.f) bookFocus = 2;
                            else if (pos.y >= 320.f && pos.y <= 356.f) bookFocus = 3;
                        }
                    }
                    else if (state == UiState::DoctorWritePrescription)
                    {
                        if (pos.x >= 280.f && pos.x <= 630.f)
                        {
                            if (pos.y >= 170.f && pos.y <= 206.f) bookFocus = 0;
                            else if (pos.y >= 220.f && pos.y <= 256.f) bookFocus = 1;
                            else if (pos.y >= 270.f && pos.y <= 306.f) bookFocus = 2;
                        }
                    }
                    else if (state == UiState::AdminAddDoctor)
                    {
                        if (pos.x >= 280.f && pos.x <= 630.f)
                        {
                            if (pos.y >= 170.f && pos.y <= 206.f) bookFocus = 0;
                            else if (pos.y >= 220.f && pos.y <= 256.f) bookFocus = 1;
                            else if (pos.y >= 270.f && pos.y <= 306.f) bookFocus = 2;
                            else if (pos.y >= 320.f && pos.y <= 356.f) bookFocus = 3;
                            else if (pos.y >= 370.f && pos.y <= 406.f) bookFocus = 4;
                        }
                    }
                }
                // focus switch for login fields

                else if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
                {
                        if (mp->position.x >= 280 && mp->position.x <= 630)
                        {
                            if (mp->position.y >= 420 && mp->position.y <= 458)
                                loginFocus = 0;
                            else if (mp->position.y >= 480 && mp->position.y <= 518)
                                loginFocus = 1;
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
                    else if (contains(pos, pViewAppts.first))
                    {
                        // build appointments list for the logged-in patient
                        med_msgClear(bodyText, sizeof bodyText);
                        if (sessionPatient)
                        {
                            const Appointment* all = appointments.getAll();
                            bool found = false;
                            for (int i = 0; i < appointments.size(); i++)
                            {
                                if (all[i].getPatientId() == sessionPatient->getId())
                                {
                                    const Doctor* dd = doctors.findByID(all[i].getDoctorId());
                                    char idbuf[20];
                                    char line[512];
                                    myitoa(all[i].getAppointmentId(), idbuf);
                                    mystrcpy(line, idbuf);
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, dd ? dd->getName() : "?");
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, dd ? dd->getSpecialization() : "?");
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, all[i].getAppointmentDate());
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, all[i].getAppointmentSlot());
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, all[i].getAppointmentStatus());
                                    med_msgCat(line, sizeof line, "\n");
                                    med_msgCat(bodyText, sizeof bodyText, line);
                                    found = true;
                                }
                            }
                            if (!found) med_msgCat(bodyText, sizeof bodyText, "No appointments found.");
                        }
                        else
                            med_msgCat(bodyText, sizeof bodyText, "No patient session.");
                        state = UiState::PatientViewAppointments;
                    }
                    else if (contains(pos, pViewRecords.first))
                    {
                        med_msgClear(bodyText, sizeof bodyText);
                        if (sessionPatient)
                        {
                            const Prescription* all = prescriptions.getAll();
                            bool found = false;
                            for (int i = 0; i < prescriptions.size(); i++)
                            {
                                if (all[i].getPatientId() == sessionPatient->getId())
                                {
                                    char line[1024];
                                    const Doctor* dd = doctors.findByID(all[i].getDoctorId());
                                    mystrcpy(line, all[i].getPrescriptionDate());
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, dd ? dd->getName() : "?");
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, all[i].getMedicine());
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, all[i].getPrescriptionNotes());
                                    med_msgCat(line, sizeof line, "\n");
                                    med_msgCat(bodyText, sizeof bodyText, line);
                                    found = true;
                                }
                            }
                            if (!found) med_msgCat(bodyText, sizeof bodyText, "No medical records found.");
                        }
                        else
                            med_msgCat(bodyText, sizeof bodyText, "No patient session.");
                        state = UiState::PatientViewRecords;
                    }
                    else if (contains(pos, pViewBills.first))
                    {
                        med_msgClear(bodyText, sizeof bodyText);
                        if (sessionPatient)
                        {
                            const Bill* all = bills.getAll();
                            bool found = false;
                            for (int i = 0; i < bills.size(); i++)
                            {
                                if (all[i].getPatientId() == sessionPatient->getId())
                                {
                                    char idbuf[20];
                                    char amtbuf[64];
                                    char line[512];
                                    myitoa(all[i].getId(), idbuf);
                                    myftoa(all[i].getAmount(), amtbuf);
                                    mystrcpy(line, idbuf);
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, "PKR ");
                                    med_msgCat(line, sizeof line, amtbuf);
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, all[i].getStatus());
                                    med_msgCat(line, sizeof line, " | ");
                                    med_msgCat(line, sizeof line, all[i].getAppointmentDate());
                                    med_msgCat(line, sizeof line, "\n");
                                    med_msgCat(bodyText, sizeof bodyText, line);
                                    found = true;
                                }
                            }
                            if (!found) med_msgCat(bodyText, sizeof bodyText, "No bills found.");
                        }
                        else
                            med_msgCat(bodyText, sizeof bodyText, "No patient session.");
                        state = UiState::PatientViewBills;
                    }
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

                // Viewing patient lists -> back
                else if (state == UiState::PatientViewAppointments && contains(pos, btnBack.first))
                    state = UiState::PatientHome;
                else if (state == UiState::PatientViewRecords && contains(pos, btnBack.first))
                    state = UiState::PatientHome;
                else if (state == UiState::PatientViewBills && contains(pos, btnBack.first))
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
                    else if (contains(pos, dLogout.first))
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
                    else if (contains(pos, aReport.first)) 
                        state = UiState::AdminDailyReport;
                    else if (contains(pos, aLogout.first) || contains(pos, btnBack.first))
                        state = UiState::RoleSelect;
                }

                else if (state == UiState::AdminDailyReport && contains(pos, btnBack.first))
                    state = UiState::AdminHome;

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

                //Admin Daily report Back
                else if (contains(pos, aReport.first))
                    state = UiState::AdminDailyReport;

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
        // -----------------------------------------------------------------
        // DRAW BACKGROUND & PARTICLES
        // -----------------------------------------------------------------
        window.clear(sf::Color(5, 8, 20));

        // Vertical background gradient
        sf::VertexArray grad(sf::PrimitiveType::TriangleStrip, 4);
        grad[0].position = { 0.f, 0.f };
        grad[0].color = sf::Color(5, 8, 20);
        grad[1].position = { 1280.f, 0.f };
        grad[1].color = sf::Color(5, 8, 20);
        grad[2].position = { 0.f, 720.f };
        grad[2].color = sf::Color(10, 20, 45);
        grad[3].position = { 1280.f, 720.f };
        grad[3].color = sf::Color(10, 20, 45);
        window.draw(grad);

        // Tech mesh grid animation
        float time = bgClock.getElapsedTime().asSeconds();
        float gridOffset = fmod(time * 15.f, 40.f);
        for (float x = gridOffset; x < 1280.f; x += 40.f) {
            sf::Vertex v1;
            v1.position = { x, 0.f };
            v1.color = sf::Color(0, 200, 170, 8);            sf::Vertex v2;
            v2.position = { x, 720.f };
            v2.color = sf::Color(0, 255, 204, 12);
            sf::Vertex line[2] = { v1, v2 };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }
        for (float yVal = gridOffset; yVal < 720.f; yVal += 40.f) {
            sf::Vertex v1;
            v1.position = { 0.f, yVal };
            v1.color = sf::Color(0, 200, 170, 8);            sf::Vertex v2;
            v2.position = { 1280.f, yVal };
            v2.color = sf::Color(0, 255, 204, 12);
            sf::Vertex line[2] = { v1, v2 };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }

        // Drifting Tech Particles
        float dt = 1.f / 60.f;
        for (int i = 0; i < NUM_PARTICLES; ++i) {
            particles[i].pos += particles[i].vel * dt;
            if (particles[i].pos.x < 0.f) particles[i].pos.x = 1280.f;
            if (particles[i].pos.x > 1280.f) particles[i].pos.x = 0.f;
            if (particles[i].pos.y < 0.f) particles[i].pos.y = 720.f;
            if (particles[i].pos.y > 720.f) particles[i].pos.y = 0.f;

            sf::CircleShape c(particles[i].radius);
            c.setPosition(particles[i].pos);
            c.setFillColor(particles[i].color);
            window.draw(c);
        }

        // Underlay images if loaded
        if (state == UiState::RoleSelect || state == UiState::LoginPatient ||
            state == UiState::LoginDoctor || state == UiState::LoginAdmin)
        {
            if (hasMain) {
                sprMain.setColor(sf::Color(255, 255, 255, 25));
                window.draw(sprMain);
            }
        }
        else if (state == UiState::PatientHome || state == UiState::PatientBook ||
            state == UiState::PatientCancel || state == UiState::PatientPay ||
            state == UiState::PatientTopUp || state == UiState::PatientViewAppointments ||
            state == UiState::PatientViewRecords || state == UiState::PatientViewBills)
        {
            if (hasPat) {
                sprPatient.setColor(sf::Color(255, 255, 255, 25));
                window.draw(sprPatient);
            }
        }
        else if (state == UiState::DoctorHome || state == UiState::DoctorTodayAppts ||
            state == UiState::DoctorMarkComplete || state == UiState::DoctorMarkNoShow ||
            state == UiState::DoctorWritePrescription || state == UiState::DoctorViewHistory)
        {
            if (hasDoc) {
                sprDoctor.setColor(sf::Color(255, 255, 255, 25));
                window.draw(sprDoctor);
            }
        }
        else
        {
            if (hasAdm) {
                sprAdmin.setColor(sf::Color(255, 255, 255, 25));
                window.draw(sprAdmin);
            }
        }

        // Glassmorphic Container Panel
        sf::RectangleShape glassCard({ 1200.f, 530.f });
        glassCard.setPosition({ 40.f, 100.f });
        glassCard.setFillColor(sf::Color(10, 18, 35, 200));
        glassCard.setOutlineColor(sf::Color(0, 255, 204, 80));
        glassCard.setOutlineThickness(1.5f);
        window.draw(glassCard);

        // Header Title, Cyber Cross Logo & Decorative Underline
        window.draw(title);
        drawLogo(window, 65.f, 48.f);
        sf::Text subtitle(font, "Advanced Healthcare Management Platform  |  v1.0", 14u);
        subtitle.setFillColor(sf::Color(0, 180, 160, 180));
        subtitle.setPosition({ 100.f, 62.f });
        window.draw(subtitle);

        sf::RectangleShape headerUnderline({ 1120.f, 2.f });
        headerUnderline.setPosition({ 80.f, 90.f });
        headerUnderline.setFillColor(sf::Color(0, 255, 204, 150));
        window.draw(headerUnderline);

        // Configure general body text layout
        body.setCharacterSize(16u);
        body.setFillColor(sf::Color::White);

        // Helper for formatting text list screens
        auto drawListCard = [&]() {
            sf::RectangleShape listCard({ 1120.f, 390.f });
            listCard.setPosition({ 80.f, 160.f });
            listCard.setFillColor(sf::Color(10, 16, 28, 235)); // Solid dark container
            listCard.setOutlineColor(sf::Color(65, 85, 115, 140));
            listCard.setOutlineThickness(1.f);
            window.draw(listCard);
            body.setPosition({ 100.f, 180.f });
        };

        // -----------------------------------------------------------------
        // DRAW UI PER STATE
        // -----------------------------------------------------------------
        if (state == UiState::RoleSelect)
        {
            // left panel
            sf::RectangleShape leftPanel({ 320.f, 320.f });
            leftPanel.setPosition({ 60.f, 175.f });
            leftPanel.setFillColor(sf::Color(8, 16, 32, 220));
            leftPanel.setOutlineColor(sf::Color(0, 255, 204, 100));
            leftPanel.setOutlineThickness(1.5f);
            window.draw(leftPanel);

            sf::Text selectText(font, "Select Your Role", 18u);
            selectText.setFillColor(sf::Color(0, 200, 180));
            selectText.setPosition({ 105.f, 183.f });
            window.draw(selectText);

            drawPair(window, btnPatient);
            drawPair(window, btnDoctor);
            drawPair(window, btnAdmin);
            drawPair(window, btnExit);

            // right panel — system info
            sf::RectangleShape rightPanel({ 740.f, 320.f });
            rightPanel.setPosition({ 420.f, 175.f });
            rightPanel.setFillColor(sf::Color(5, 12, 24, 200));
            rightPanel.setOutlineColor(sf::Color(0, 255, 204, 60));
            rightPanel.setOutlineThickness(1.f);
            window.draw(rightPanel);

            sf::Text sysTitle(font, "Welcome to MediCore", 26u);
            sysTitle.setFillColor(sf::Color(0, 255, 220));
            sysTitle.setPosition({ 450.f, 210.f });
            window.draw(sysTitle);

            sf::Text sysDesc(font, "A complete hospital management solution\nfor patients, doctors and administrators.\n\nManage appointments, prescriptions,\nbilling and medical records with ease.", 17u);
            sysDesc.setFillColor(sf::Color(160, 190, 210));
            sysDesc.setPosition({ 450.f, 258.f });
            window.draw(sysDesc);

            // divider
            sf::RectangleShape div({ 1.f, 280.f });
            div.setPosition({ 415.f, 180.f });
            div.setFillColor(sf::Color(0, 255, 204, 80));
            window.draw(div);
        }
        else if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
        {
            sf::Text hint(font, "Click input boxes directly to focus. Tab=switch. Backspace=delete.", 16u);
            hint.setFillColor(sf::Color(170, 195, 220)); hint.setPosition({ 80.f, 380.f });
            window.draw(hint);

            drawInputField("User ID:", idBuf, 80.f, 420.f, 350.f, 38.f, loginFocus == 0);
            drawInputField("Password:", passBuf, 80.f, 480.f, 350.f, 38.f, loginFocus == 1);
            
            drawPair(window, btnLogin);
            drawPair(window, btnBack);
        }
        else if (state == UiState::LoginAdmin)
        {
            sf::Text hint(font, "Enter Admin Password. Click box to focus. Backspace=delete.", 16u);
            hint.setFillColor(sf::Color(170, 195, 220)); hint.setPosition({ 80.f, 380.f });
            window.draw(hint);

            drawInputField("Password:", passBuf, 80.f, 440.f, 350.f, 38.f, true);
            
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
                wt.setFillColor(sf::Color::White); wt.setPosition({ 80.f, 115.f }); window.draw(wt);
                
                char bal[64] = "Account Balance: PKR ";
                char bstr[32]; myftoa(sessionPatient->getBalance(), bstr);
                mystrcat(bal, bstr);
                sf::Text bt(font, sf::String::fromUtf8(bal, bal + mystrlen(bal)), 18u);
                bt.setFillColor(sf::Color(0, 255, 204)); bt.setPosition({ 80.f, 142.f }); window.draw(bt);
            }
            drawPair(window, pBook); drawPair(window, pCancel);
            drawPair(window, pViewAppts); drawPair(window, pViewRecords);
            drawPair(window, pViewBills); drawPair(window, pPay);
            drawPair(window, pTop); drawPair(window, pLogout);
        }
        else if (state == UiState::PatientBook)
        {
            sf::Text hint(font, "Click input fields to select. Tab=next field. Backspace=delete.", 16u);
            hint.setFillColor(sf::Color(170, 195, 220)); hint.setPosition({ 80.f, 125.f });
            window.draw(hint);

            drawInputField("Specialization:", specBuf, 80.f, 170.f, 350.f, 36.f, bookFocus == 0);
            drawInputField("Doctor ID:", docIdBuf, 80.f, 220.f, 350.f, 36.f, bookFocus == 1);
            drawInputField("Date (DD-MM-YYYY):", dateBuf, 80.f, 270.f, 350.f, 36.f, bookFocus == 2);
            drawInputField("Time Slot:", slotBuf, 80.f, 320.f, 350.f, 36.f, bookFocus == 3);

            sf::Text slotHelp(font, "Available slots: 09:00, 10:00, 11:00, 12:00, 13:00, 14:00, 15:00, 16:00", 15u);
            slotHelp.setFillColor(sf::Color(0, 255, 204)); slotHelp.setPosition({ 80.f, 380.f }); window.draw(slotHelp);

            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::PatientCancel)
        {
            drawInputField("Appointment ID:", apptCancelBuf, 80.f, 220.f, 350.f, 38.f, true);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::PatientPay)
        {
            drawInputField("Bill ID to Pay:", billPayBuf, 80.f, 220.f, 350.f, 38.f, true);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::PatientTopUp)
        {
            drawInputField("Amount (PKR):", amountBuf, 80.f, 220.f, 350.f, 38.f, true);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::PatientViewAppointments)
        {
            drawListCard();
            body.setCharacterSize(14u);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body);
            drawPair(window, btnBack);
        }
        else if (state == UiState::PatientViewRecords)
        {
            drawListCard();
            body.setCharacterSize(14u);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body);
            drawPair(window, btnBack);
        }
        else if (state == UiState::PatientViewBills)
        {
            drawListCard();
            body.setCharacterSize(14u);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body);
            drawPair(window, btnBack);
        }

        // --- Doctor states ---
        else if (state == UiState::DoctorHome)
        {
            if (sessionDoctor)
            {
                char welcome[128] = "Welcome, ";
                mystrcat(welcome, sessionDoctor->getName());
                mystrcat(welcome, "  |  ");
                mystrcat(welcome, sessionDoctor->getSpecialization());
                sf::Text wt(font, sf::String::fromUtf8(welcome, welcome + mystrlen(welcome)), 22u);
                wt.setFillColor(sf::Color::White); wt.setPosition({ 80.f, 120.f }); window.draw(wt);
            }
            drawPair(window, dToday); drawPair(window, dComplete);
            drawPair(window, dNoShow); drawPair(window, dPrescription);
            drawPair(window, dHistory); drawPair(window, dLogout);
        }
        else if (state == UiState::DoctorTodayAppts)
        {
            drawListCard();
            med_formatTodayAppointments(appointments, patients, sessionDoctor, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::DoctorMarkComplete || state == UiState::DoctorMarkNoShow)
        {
            // draw label separately above the input box
            const char* label = (state == UiState::DoctorMarkComplete) ?
                "Mark Appointment Complete" : "Mark Appointment No-Show";

            sf::Text heading(font, sf::String::fromUtf8(label, label + mystrlen(label)), 20u);
            heading.setFillColor(sf::Color(0, 255, 220));
            heading.setPosition({ 80.f, 185.f });
            window.draw(heading);

            drawInputField("Appointment ID:", apptIdBuf, 80.f, 230.f, 350.f, 38.f, true);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
            }
        else if (state == UiState::DoctorWritePrescription)
        {
            sf::Text hint(font, "Click input fields to select. Tab=next field. Backspace=delete.", 16u);
            hint.setFillColor(sf::Color(170, 195, 220)); hint.setPosition({ 80.f, 125.f });
            window.draw(hint);

            drawInputField("Appointment ID:", apptIdBuf, 80.f, 170.f, 350.f, 36.f, bookFocus == 0);
            drawInputField("Medicines:", medBuf, 80.f, 220.f, 350.f, 36.f, bookFocus == 1);
            drawInputField("Notes:", notesBuf, 80.f, 270.f, 350.f, 36.f, bookFocus == 2);

            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::DoctorViewHistory)
        {
            drawInputField("Patient ID:", patHistIdBuf, 80.f, 220.f, 350.f, 38.f, true);
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
            drawPair(window, aReport);
        }
        else if (state == UiState::AdminPatients)
        {
            drawListCard();
            med_formatPatientList(patients, bills, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminDoctors)
        {
            drawListCard();
            med_formatDoctorList(doctors, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminDischarge)
        {
            drawInputField("Patient ID:", adminPidBuf, 80.f, 220.f, 350.f, 38.f, true);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminAddDoctor)
        {
            sf::Text hint(font, "Click input fields to select. Tab=next field. Backspace=delete.", 16u);
            hint.setFillColor(sf::Color(170, 195, 220)); hint.setPosition({ 80.f, 125.f });
            window.draw(hint);

            drawInputField("Doctor Name:", adName, 80.f, 170.f, 350.f, 36.f, bookFocus == 0);
            drawInputField("Specialization:", adSpec, 80.f, 220.f, 350.f, 36.f, bookFocus == 1);
            drawInputField("Contact:", adContact, 80.f, 270.f, 350.f, 36.f, bookFocus == 2);
            drawInputField("Password:", adPass, 80.f, 320.f, 350.f, 36.f, bookFocus == 3);
            drawInputField("Fee (PKR):", adFeeBuf, 80.f, 370.f, 350.f, 36.f, bookFocus == 4);

            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminRemoveDoctor)
        {
            drawInputField("Doctor ID to Remove:", removeDocBuf, 80.f, 220.f, 350.f, 38.f, true);
            drawPair(window, btnSubmit); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminAppointments)
        {
            drawListCard();
            med_formatAppointmentList(appointments, patients, doctors, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminUnpaidBills)
        {
            drawListCard();
            med_formatUnpaidBillsAll(bills, patients, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body); drawPair(window, btnBack);
        }
        else if (state == UiState::AdminDailyReport)
        {
            drawListCard();
            med_formatDailyReport(appointments, bills, patients, doctors, bodyText, sizeof bodyText);
            body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
            window.draw(body);
            drawPair(window, btnBack);
        }
        else if (state == UiState::AdminSecurityLog)
        {
            drawListCard();
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
        sf::RectangleShape statusBar({ 1280.f, 32.f });
        statusBar.setPosition({ 0.f, 645.f });
        statusBar.setFillColor(sf::Color(5, 10, 20, 220));
        statusBar.setOutlineColor(sf::Color(0, 255, 204, 60));
        statusBar.setOutlineThickness(1.f);
        window.draw(statusBar);

        // smart status color
        if (mystrlen(statusMsg) > 0)
        {
            const char* errWords[] = { "invalid", "error", "not found", "insufficient", "cannot", "denied", "locked", "failed" };
            bool isErr = false;
            for (int w = 0; w < 8 && !isErr; w++) {
                const char* h = statusMsg;
                int nLen = mystrlen(errWords[w]);
                while (*h) {
                    bool match = true;
                    for (int j = 0; j < nLen; j++)
                        if (mytolower(h[j]) != mytolower(errWords[w][j])) { match = false; break; }
                    if (match) { isErr = true; break; }
                    h++;
                }
            }
            status.setFillColor(isErr ? sf::Color(255, 60, 60) : sf::Color(0, 255, 180));
        }
        else
            status.setFillColor(sf::Color(0, 200, 160, 150));
        // Status bar
        status.setString(sf::String::fromUtf8(statusMsg, statusMsg + mystrlen(statusMsg)));
        window.draw(status);
        window.display();
    }
    
    if (admin != nullptr) delete admin;
    return 0;
}