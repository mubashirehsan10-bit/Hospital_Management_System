#include <SFML/Graphics.hpp>

#include "MediCoreGuiAPI.h"
#include "utility.h"
#include "Validator.h"
#include "FileHandler.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"

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
// Updates sessionPatient and sessionDoctor pointers based on logged IDs
void refreshSessions(Storage<Patient>& patients, Storage<Doctor>& doctors, int loggedPatientId, int loggedDoctorId, Patient*& sessionPatient, Doctor*& sessionDoctor)
{
	if (loggedPatientId >= 0)
		sessionPatient = patients.findByID(loggedPatientId);
	else
		sessionPatient = nullptr;
	if (loggedDoctorId >= 0)
		sessionDoctor = doctors.findByID(loggedDoctorId);
	else
		sessionDoctor = nullptr;
}
// Example usage in main():
static bool loadTextureFromAssets(sf::Texture& tex, const char* fileName)
{
	char path[260];
	if (!buildAssetPath(path, sizeof path, fileName))
		return false;
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
	LoginAdmin,
	AdminHome,
	AdminPatients,
	AdminDoctors,
	AdminDischarge,
	AdminAddDoctor
};

static void appendTypedChar(char* buf, int cap, char32_t code)
{
	if (code < 32 || code > 126)
		return;
	int n = mystrlen(buf);
	if (n + 1 >= cap)
		return;
	buf[n] = static_cast<char>(code);
	buf[n + 1] = '\0';
}

static void backspaceChar(char* buf)
{
	int n = mystrlen(buf);
	if (n <= 0) return;
	buf[n - 1] = '\0';
}

static bool tryLoginPatient(Storage<Patient>& patients, FileHandler& fh, int id, const char* password, char* err, int errCap)
{
	med_msgClear(err, errCap);
	Patient* p = patients.findByID(id);
	if (p == nullptr)
	{
		med_msgCat(err, errCap, "Invalid credentials.");
		return false;
	}
	if (mystrcmp(p->getPassword(), password) != 0)
	{
		med_msgCat(err, errCap, "Invalid credentials.");
		return false;
	}
	return true;
}

static bool tryLoginDoctor(Storage<Doctor>& doctors, FileHandler& fh, int id, const char* password, char* err, int errCap)
{
	(void)fh;
	med_msgClear(err, errCap);
	Doctor* d = doctors.findByID(id);
	if (d == nullptr)
	{
		med_msgCat(err, errCap, "Invalid credentials.");
		return false;
	}
	if (mystrcmp(d->getPassword(), password) != 0)
	{
		med_msgCat(err, errCap, "Invalid credentials.");
		return false;
	}
	return true;
}

static bool tryLoginAdmin(Admin* admin, FileHandler& fh, const char* password, char* err, int errCap)
{
	(void)fh;
	med_msgClear(err, errCap);
	if (admin == nullptr)
	{
		med_msgCat(err, errCap, "No admin loaded.");
		return false;
	}
	if (mystrcmp(admin->getPassword(), password) != 0)
	{
		med_msgCat(err, errCap, "Invalid credentials.");
		return false;
	}
	return true;
}

int main()
{
	sf::RenderWindow window(sf::VideoMode({ 1280u, 720u }), "MediCore Hospital", sf::Style::Titlebar | sf::Style::Close);
	window.setVerticalSyncEnabled(true);

	sf::Font font;
	if (!font.openFromFile("assets/font.ttf"))
		font.openFromFile("C:/Windows/Fonts/arial.ttf");

	int loggedPatientId = -1;
	int loggedDoctorId = -1;
	Patient* sessionPatient = nullptr;
	Doctor* sessionDoctor = nullptr;

	Storage<Patient> patients;
	Storage<Doctor> doctors;
	Storage<Appointment> appointments;
	Storage<Bill> bills;
	Storage<Prescription> prescriptions;
	Admin* admin = nullptr;
	FileHandler fh;
	Validator validator;

	med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);

	// With this, passing all required arguments:
	refreshSessions(patients, doctors, loggedPatientId, loggedDoctorId, sessionPatient, sessionDoctor);
	sessionDoctor = doctors.findByID(loggedDoctorId);

    
	sf::Texture texMain, texPatient, texDoctor, texAdmin;
	bool hasMain = loadTextureFromAssets(texMain, AssetPaths::kMainBg);
	bool hasPat = loadTextureFromAssets(texPatient, AssetPaths::kPatientBg);
	bool hasDoc = loadTextureFromAssets(texDoctor, AssetPaths::kDoctorBg);
	bool hasAdm = loadTextureFromAssets(texAdmin, AssetPaths::kAdminBg);

	sf::Sprite sprMain(texMain);
	sf::Sprite sprPatient(texPatient);
	sf::Sprite sprDoctor(texDoctor);
	sf::Sprite sprAdmin(texAdmin);

	auto scaleSpriteToWindow = [&](sf::Sprite& s, bool hasTex)
	{
		if (!hasTex) return;
		auto sz = s.getTexture().getSize();
		if (sz.x == 0 || sz.y == 0) return;
		float sx = static_cast<float>(window.getSize().x) / static_cast<float>(sz.x);
		float sy = static_cast<float>(window.getSize().y) / static_cast<float>(sz.y);
		s.setScale({ sx, sy });
	};
	scaleSpriteToWindow(sprMain, hasMain);
	scaleSpriteToWindow(sprPatient, hasPat);
	scaleSpriteToWindow(sprDoctor, hasDoc);
	scaleSpriteToWindow(sprAdmin, hasAdm);

	sf::RectangleShape fallbackBg;
	fallbackBg.setSize({ static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y) });
	fallbackBg.setFillColor(sf::Color(25, 40, 60));

	UiState state = UiState::RoleSelect;

	auto refreshSessions = [&]()
	{
		if (loggedPatientId >= 0)
			sessionPatient = patients.findByID(loggedPatientId);
		else
			sessionPatient = nullptr;
		if (loggedDoctorId >= 0)
			sessionDoctor = doctors.findByID(loggedDoctorId);
		else
			sessionDoctor = nullptr;
	};

	char idBuf[32] = "";
	char passBuf[64] = "";
	int loginFocus = 0;
	int bookFocus = 0;
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

	char statusMsg[1024] = "";
	char bodyText[8192] = "";

	int loginFails = 0;

	sf::Text title(font, "MediCore Hospital Management System", 28u);    
    // With this line to call the lambda version:
    refreshSessions();
      
	title.setFillColor(sf::Color::White);
	title.setPosition({ 40.f, 30.f });

	sf::Text status(font, sf::String(), 18u);
	status.setFillColor(sf::Color(220, 255, 220));
	status.setPosition({ 40.f, 660.f });

	sf::Text body(font, sf::String(), 16u);
	body.setFillColor(sf::Color::White);
	body.setPosition({ 40.f, 120.f });

	auto makeButton = [](float x, float y, float w, float h, const char* label, const sf::Font& f) {
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

	auto drawPair = [&](sf::RenderTarget& target, std::pair<sf::RectangleShape, sf::Text>& p) {
		target.draw(p.first);
		target.draw(p.second);
	};

	std::pair<sf::RectangleShape, sf::Text> btnPatient = makeButton(80.f, 200.f, 220.f, 44.f, "1. Patient", font);
	std::pair<sf::RectangleShape, sf::Text> btnDoctor = makeButton(80.f, 260.f, 220.f, 44.f, "2. Doctor", font);
	std::pair<sf::RectangleShape, sf::Text> btnAdmin = makeButton(80.f, 320.f, 220.f, 44.f, "3. Admin", font);
	std::pair<sf::RectangleShape, sf::Text> btnExit = makeButton(80.f, 380.f, 220.f, 44.f, "4. Exit", font);

	std::pair<sf::RectangleShape, sf::Text> btnLogin = makeButton(900.f, 520.f, 160.f, 40.f, "Login", font);
	std::pair<sf::RectangleShape, sf::Text> btnBack = makeButton(900.f, 580.f, 160.f, 40.f, "Back", font);

	std::pair<sf::RectangleShape, sf::Text> pBook = makeButton(80.f, 180.f, 280.f, 40.f, "Book appointment", font);
	std::pair<sf::RectangleShape, sf::Text> pCancel = makeButton(80.f, 230.f, 280.f, 40.f, "Cancel appointment", font);
	std::pair<sf::RectangleShape, sf::Text> pPay = makeButton(80.f, 280.f, 280.f, 40.f, "Pay bill", font);
	std::pair<sf::RectangleShape, sf::Text> pTop = makeButton(80.f, 330.f, 280.f, 40.f, "Top up balance", font);
	std::pair<sf::RectangleShape, sf::Text> pLogout = makeButton(80.f, 500.f, 280.f, 40.f, "Logout", font);

	std::pair<sf::RectangleShape, sf::Text> btnSubmit = makeButton(900.f, 520.f, 160.f, 40.f, "Submit", font);

	std::pair<sf::RectangleShape, sf::Text> aPatients = makeButton(80.f, 180.f, 280.f, 40.f, "View patients", font);
	std::pair<sf::RectangleShape, sf::Text> aDoctors = makeButton(80.f, 230.f, 280.f, 40.f, "View doctors", font);
	std::pair<sf::RectangleShape, sf::Text> aDisch = makeButton(80.f, 280.f, 280.f, 40.f, "Discharge patient", font);
	std::pair<sf::RectangleShape, sf::Text> aAddDr = makeButton(80.f, 330.f, 280.f, 40.f, "Add doctor", font);

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
				break;
			}
			if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>())
			{
				if (mp->button != sf::Mouse::Button::Left)
					continue;
				sf::Vector2f pos({ static_cast<float>(mp->position.x), static_cast<float>(mp->position.y) });

				if (state == UiState::RoleSelect)
				{
					if (contains(pos, btnPatient.first))
					{
						state = UiState::LoginPatient;
						med_msgClear(idBuf, sizeof idBuf);
						med_msgClear(passBuf, sizeof passBuf);
						loginFocus = 0;
						loginFails = 0;
						med_msgClear(statusMsg, sizeof statusMsg);
					}
					else if (contains(pos, btnDoctor.first))
					{
						state = UiState::LoginDoctor;
						med_msgClear(idBuf, sizeof idBuf);
						med_msgClear(passBuf, sizeof passBuf);
						loginFocus = 0;
						loginFails = 0;
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
						if (state == UiState::LoginPatient)
							ok = tryLoginPatient(patients, fh, id, passBuf, statusMsg, sizeof statusMsg);
						else if (state == UiState::LoginDoctor)
							ok = tryLoginDoctor(doctors, fh, id, passBuf, statusMsg, sizeof statusMsg);
						else
							ok = tryLoginAdmin(admin, fh, passBuf, statusMsg, sizeof statusMsg);

						if (ok)
						{
							if (state == UiState::LoginPatient)
							{
								loggedPatientId = id;
								refreshSessions();
								state = UiState::PatientHome;
							}
							else if (state == UiState::LoginDoctor)
							{
								loggedDoctorId = id;
								refreshSessions();
								state = UiState::DoctorHome;
							}
							else
								state = UiState::AdminHome;
							loginFails = 0;
							med_msgClear(statusMsg, sizeof statusMsg);
						}
						else
						{
							loginFails++;
							char ts[48];
							formatTimestampFull(ts, sizeof ts);
							if (loginFails >= 3)
							{
								char idstr[16];
								if (state == UiState::LoginAdmin && admin != nullptr)
									myitoa(admin->getId(), idstr);
								else
									myitoa(id, idstr);
								if (state == UiState::LoginAdmin)
									fh.appendSecurityLog(ts, "Admin", idstr, "FAILED");
								else if (state == UiState::LoginPatient)
									fh.appendSecurityLog(ts, "Patient", idstr, "FAILED");
								else
									fh.appendSecurityLog(ts, "Doctor", idstr, "FAILED");
								med_msgCat(statusMsg, sizeof statusMsg, " Account locked. Contact admin.");
								state = UiState::RoleSelect;
								loginFails = 0;
							}
						}
					}
				}
				else if (state == UiState::PatientHome)
				{
					if (contains(pos, pBook.first))
					{
						state = UiState::PatientBook;
						bookFocus = 0;
						med_msgClear(specBuf, sizeof specBuf);
						med_msgClear(docIdBuf, sizeof docIdBuf);
						med_msgClear(dateBuf, sizeof dateBuf);
						med_msgClear(slotBuf, sizeof slotBuf);
						med_msgClear(bodyText, sizeof bodyText);
						med_msgCat(bodyText, sizeof bodyText, "Enter fields below (keyboard), then Submit.\n");
					}
					else if (contains(pos, pCancel.first))
					{
						state = UiState::PatientCancel;
						med_msgClear(apptCancelBuf, sizeof apptCancelBuf);
					}
					else if (contains(pos, pPay.first))
					{
						state = UiState::PatientPay;
						med_msgClear(billPayBuf, sizeof billPayBuf);
					}
					else if (contains(pos, pTop.first))
					{
						state = UiState::PatientTopUp;
						med_msgClear(amountBuf, sizeof amountBuf);
					}
					else if (contains(pos, pLogout.first))
					{
						loggedPatientId = -1;
						sessionPatient = nullptr;
						state = UiState::RoleSelect;
					}
					else if (contains(pos, btnBack.first))
					{
						loggedPatientId = -1;
						sessionPatient = nullptr;
						state = UiState::RoleSelect;
					}
				}
				else if (state == UiState::PatientBook && contains(pos, btnSubmit.first))
				{
					int docId = myatoi(docIdBuf);
					char msg[512];
					med_guiBookAppointment(validator, fh, sessionPatient, bills, doctors, appointments,
						specBuf, docId, dateBuf, slotBuf, msg, sizeof msg);
					med_msgClear(statusMsg, sizeof statusMsg);
					med_msgCat(statusMsg, sizeof statusMsg, msg);
					med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
					refreshSessions();
					state = UiState::PatientHome;
				}
				else if (state == UiState::PatientCancel && contains(pos, btnSubmit.first))
				{
					char msg[512];
					med_guiCancelAppointment(fh, sessionPatient, doctors, bills, appointments, myatoi(apptCancelBuf), msg, sizeof msg);
					med_msgClear(statusMsg, sizeof statusMsg);
					med_msgCat(statusMsg, sizeof statusMsg, msg);
					med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
					refreshSessions();
					state = UiState::PatientHome;
				}
				else if (state == UiState::PatientPay && contains(pos, btnSubmit.first))
				{
					char msg[512];
					med_guiPayBill(validator, fh, sessionPatient, bills, myatoi(billPayBuf), msg, sizeof msg);
					med_msgClear(statusMsg, sizeof statusMsg);
					med_msgCat(statusMsg, sizeof statusMsg, msg);
					med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
					refreshSessions();
					state = UiState::PatientHome;
				}
				else if (state == UiState::PatientTopUp && contains(pos, btnSubmit.first))
				{
					char msg[512];
					med_guiTopUp(validator, fh, sessionPatient, myatof(amountBuf), msg, sizeof msg);
					med_msgClear(statusMsg, sizeof statusMsg);
					med_msgCat(statusMsg, sizeof statusMsg, msg);
					med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
					refreshSessions();
					state = UiState::PatientHome;
				}
				else if (state == UiState::DoctorHome && contains(pos, btnBack.first))
				{
					loggedDoctorId = -1;
					sessionDoctor = nullptr;
					state = UiState::RoleSelect;
				}
				else if (state == UiState::AdminHome)
				{
					if (contains(pos, aPatients.first))
						state = UiState::AdminPatients;
					else if (contains(pos, aDoctors.first))
						state = UiState::AdminDoctors;
					else if (contains(pos, aDisch.first))
						state = UiState::AdminDischarge;
					else if (contains(pos, aAddDr.first))
					{
						bookFocus = 0;
						med_msgClear(adName, sizeof adName);
						med_msgClear(adSpec, sizeof adSpec);
						med_msgClear(adContact, sizeof adContact);
						med_msgClear(adPass, sizeof adPass);
						med_msgClear(adFeeBuf, sizeof adFeeBuf);
						state = UiState::AdminAddDoctor;
					}
					else if (contains(pos, btnBack.first))
						state = UiState::RoleSelect;
				}
				else if (state == UiState::AdminPatients && contains(pos, btnBack.first))
					state = UiState::AdminHome;
				else if (state == UiState::AdminDoctors && contains(pos, btnBack.first))
					state = UiState::AdminHome;
				else if (state == UiState::AdminDischarge && contains(pos, btnSubmit.first))
				{
					char msg[512];
					med_guiDischargePatient(fh, patients, appointments, bills, prescriptions, myatoi(adminPidBuf), msg, sizeof msg);
					med_msgClear(statusMsg, sizeof statusMsg);
					med_msgCat(statusMsg, sizeof statusMsg, msg);
					med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
					refreshSessions();
					state = UiState::AdminHome;
				}
				else if (state == UiState::AdminDischarge && contains(pos, btnBack.first))
					state = UiState::AdminHome;
				else if (state == UiState::AdminAddDoctor && contains(pos, btnSubmit.first))
				{
					char msg[512];
					med_guiAddDoctor(validator, fh, doctors, adName, adSpec, adContact, adPass, myatof(adFeeBuf), msg, sizeof msg);
					med_msgClear(statusMsg, sizeof statusMsg);
					med_msgCat(statusMsg, sizeof statusMsg, msg);
					med_reloadAll(patients, prescriptions, bills, appointments, doctors, admin, fh);
					refreshSessions();
					state = UiState::AdminHome;
				}
				else if (state == UiState::AdminAddDoctor && contains(pos, btnBack.first))
					state = UiState::AdminHome;
			}
			if (const auto* txt = event->getIf<sf::Event::TextEntered>())
			{
				char32_t c = txt->unicode;
				if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
				{
					if (loginFocus == 0)
						appendTypedChar(idBuf, sizeof idBuf, c);
					else
						appendTypedChar(passBuf, sizeof passBuf, c);
				}
				else if (state == UiState::LoginAdmin)
					appendTypedChar(passBuf, sizeof passBuf, c);
				else if (state == UiState::PatientBook)
				{
					if (bookFocus == 0) appendTypedChar(specBuf, sizeof specBuf, c);
					else if (bookFocus == 1) appendTypedChar(docIdBuf, sizeof docIdBuf, c);
					else if (bookFocus == 2) appendTypedChar(dateBuf, sizeof dateBuf, c);
					else appendTypedChar(slotBuf, sizeof slotBuf, c);
				}
				else if (state == UiState::PatientCancel)
					appendTypedChar(apptCancelBuf, sizeof apptCancelBuf, c);
				else if (state == UiState::PatientPay)
					appendTypedChar(billPayBuf, sizeof billPayBuf, c);
				else if (state == UiState::PatientTopUp)
					appendTypedChar(amountBuf, sizeof amountBuf, c);
				else if (state == UiState::AdminDischarge)
					appendTypedChar(adminPidBuf, sizeof adminPidBuf, c);
				else if (state == UiState::AdminAddDoctor)
				{
					int af = bookFocus;
					if (af == 0) appendTypedChar(adName, sizeof adName, c);
					else if (af == 1) appendTypedChar(adSpec, sizeof adSpec, c);
					else if (af == 2) appendTypedChar(adContact, sizeof adContact, c);
					else if (af == 3) appendTypedChar(adPass, sizeof adPass, c);
					else appendTypedChar(adFeeBuf, sizeof adFeeBuf, c);
				}
			}
			if (const auto* key = event->getIf<sf::Event::KeyPressed>())
			{
				if (key->code == sf::Keyboard::Key::Tab)
				{
					if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
						loginFocus = loginFocus == 0 ? 1 : 0;
					else if (state == UiState::PatientBook)
						bookFocus = (bookFocus + 1) % 4;
					else if (state == UiState::AdminAddDoctor)
						bookFocus = (bookFocus + 1) % 5;
				}
				if (key->code == sf::Keyboard::Key::Backspace)
				{
					if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
					{
						if (loginFocus == 1)
							backspaceChar(passBuf);
						else
							backspaceChar(idBuf);
					}
					else if (state == UiState::LoginAdmin)
						backspaceChar(passBuf);
					else if (state == UiState::PatientBook)
					{
						if (bookFocus == 3) backspaceChar(slotBuf);
						else if (bookFocus == 2) backspaceChar(dateBuf);
						else if (bookFocus == 1) backspaceChar(docIdBuf);
						else backspaceChar(specBuf);
					}
					else if (state == UiState::PatientCancel)
						backspaceChar(apptCancelBuf);
					else if (state == UiState::PatientPay)
						backspaceChar(billPayBuf);
					else if (state == UiState::PatientTopUp)
						backspaceChar(amountBuf);
					else if (state == UiState::AdminDischarge)
						backspaceChar(adminPidBuf);
					else if (state == UiState::AdminAddDoctor)
					{
						if (bookFocus == 4) backspaceChar(adFeeBuf);
						else if (bookFocus == 3) backspaceChar(adPass);
						else if (bookFocus == 2) backspaceChar(adContact);
						else if (bookFocus == 1) backspaceChar(adSpec);
						else backspaceChar(adName);
					}
				}
			}
		}

		window.clear(sf::Color::Black);
		if (state == UiState::RoleSelect || state == UiState::LoginPatient || state == UiState::LoginDoctor || state == UiState::LoginAdmin)
		{
			if (hasMain) window.draw(sprMain);
			else window.draw(fallbackBg);
		}
		else if (state == UiState::PatientHome || state == UiState::PatientBook || state == UiState::PatientCancel
			|| state == UiState::PatientPay || state == UiState::PatientTopUp)
		{
			if (hasPat) window.draw(sprPatient);
			else window.draw(fallbackBg);
		}
		else if (state == UiState::DoctorHome)
		{
			if (hasDoc) window.draw(sprDoctor);
			else window.draw(fallbackBg);
		}
		else
		{
			if (hasAdm) window.draw(sprAdmin);
			else window.draw(fallbackBg);
		}

		window.draw(title);

		if (state == UiState::RoleSelect)
		{
			drawPair(window, btnPatient);
			drawPair(window, btnDoctor);
			drawPair(window, btnAdmin);
			drawPair(window, btnExit);
		}
		else if (state == UiState::LoginPatient || state == UiState::LoginDoctor)
		{
			sf::Text hint(font,("ID and password. Press Tab to switch field. Backspace to delete."), 18u);
			hint.setFillColor(sf::Color::White);
			hint.setPosition({ 40.f, 420.f });
			window.draw(hint);
			sf::Text idv(font, sf::String::fromUtf8(idBuf, idBuf + mystrlen(idBuf)), 20u);
			idv.setPosition({ 40.f, 460.f });
			idv.setFillColor(sf::Color::White);
			sf::Text pv(font, sf::String::fromUtf8(passBuf, passBuf + mystrlen(passBuf)), 20u);
			pv.setPosition({ 40.f, 500.f });
			pv.setFillColor(sf::Color::White);
			window.draw(idv);
			window.draw(pv);
			drawPair(window, btnLogin);
			drawPair(window, btnBack);
		}
		else if (state == UiState::LoginAdmin)
		{
			sf::Text hint(font,("Admin password"), 18u);
			hint.setFillColor(sf::Color::White);
			hint.setPosition({ 40.f, 460.f });
			window.draw(hint);
			sf::Text pv(font, sf::String::fromUtf8(passBuf, passBuf + mystrlen(passBuf)), 20u);
			pv.setPosition({ 40.f, 500.f });
			pv.setFillColor(sf::Color::White);
			window.draw(pv);
			drawPair(window, btnLogin);
			drawPair(window, btnBack);
		}
		else if (state == UiState::PatientHome)
		{
			drawPair(window, pBook);
			drawPair(window, pCancel);
			drawPair(window, pPay);
			drawPair(window, pTop);
			drawPair(window, pLogout);
			drawPair(window, btnBack);
		}
		else if (state == UiState::PatientBook)
		{
			med_msgClear(bodyText, sizeof bodyText);
			med_msgCat(bodyText, sizeof bodyText, "Spec: ");
			med_msgCat(bodyText, sizeof bodyText, specBuf);
			med_msgCat(bodyText, sizeof bodyText, "\nDoctor ID: ");
			med_msgCat(bodyText, sizeof bodyText, docIdBuf);
			med_msgCat(bodyText, sizeof bodyText, "\nDate: ");
			med_msgCat(bodyText, sizeof bodyText, dateBuf);
			med_msgCat(bodyText, sizeof bodyText, "\nSlot: ");
			med_msgCat(bodyText, sizeof bodyText, slotBuf);
			body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
			window.draw(body);
			drawPair(window, btnSubmit);
			drawPair(window, btnBack);
		}
		else if (state == UiState::PatientCancel || state == UiState::PatientPay || state == UiState::PatientTopUp)
		{
			med_msgClear(bodyText, sizeof bodyText);
			if (state == UiState::PatientCancel)
			{
				med_msgCat(bodyText, sizeof bodyText, "Appointment ID: ");
				med_msgCat(bodyText, sizeof bodyText, apptCancelBuf);
			}
			else if (state == UiState::PatientPay)
			{
				med_msgCat(bodyText, sizeof bodyText, "Bill ID: ");
				med_msgCat(bodyText, sizeof bodyText, billPayBuf);
			}
			else
			{
				med_msgCat(bodyText, sizeof bodyText, "Amount: ");
				med_msgCat(bodyText, sizeof bodyText, amountBuf);
			}
			body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
			window.draw(body);
			drawPair(window, btnSubmit);
			drawPair(window, btnBack);
		}
		else if (state == UiState::DoctorHome)
		{
			sf::Text t(font,("Doctor console: use full CLI build for all doctor tools."), 20u);
			t.setFillColor(sf::Color::White);
			t.setPosition({ 40.f, 200.f });
			window.draw(t);
			drawPair(window, btnBack);
		}
		else if (state == UiState::AdminHome)
		{
			drawPair(window, aPatients);
			drawPair(window, aDoctors);
			drawPair(window, aDisch);
			drawPair(window, aAddDr);
			drawPair(window, btnBack);
		}
		else if (state == UiState::AdminPatients)
		{
			med_formatPatientList(patients, bills, bodyText, sizeof bodyText);
			body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
			window.draw(body);
			drawPair(window, btnBack);
		}
		else if (state == UiState::AdminDoctors)
		{
			med_formatDoctorList(doctors, bodyText, sizeof bodyText);
			body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
			window.draw(body);
			drawPair(window, btnBack);
		}
		else if (state == UiState::AdminDischarge)
		{
			sf::Text t(font,("Patient ID to discharge:"), 18u);
			t.setFillColor(sf::Color::White);
			t.setPosition({ 40.f, 200.f });
			window.draw(t);
			sf::Text v(font, sf::String::fromUtf8(adminPidBuf, adminPidBuf + mystrlen(adminPidBuf)), 20u);
			v.setPosition({ 40.f, 230.f });
			v.setFillColor(sf::Color::White);
			window.draw(v);
			drawPair(window, btnSubmit);
			drawPair(window, btnBack);
		}
		else if (state == UiState::AdminAddDoctor)
		{
			med_msgClear(bodyText, sizeof bodyText);
			med_msgCat(bodyText, sizeof bodyText, "Name: ");
			med_msgCat(bodyText, sizeof bodyText, adName);
			med_msgCat(bodyText, sizeof bodyText, "\nSpec: ");
			med_msgCat(bodyText, sizeof bodyText, adSpec);
			med_msgCat(bodyText, sizeof bodyText, "\nContact: ");
			med_msgCat(bodyText, sizeof bodyText, adContact);
			med_msgCat(bodyText, sizeof bodyText, "\nPass: ");
			med_msgCat(bodyText, sizeof bodyText, adPass);
			med_msgCat(bodyText, sizeof bodyText, "\nFee: ");
			med_msgCat(bodyText, sizeof bodyText, adFeeBuf);
			body.setString(sf::String::fromUtf8(bodyText, bodyText + mystrlen(bodyText)));
			window.draw(body);
			drawPair(window, btnSubmit);
			drawPair(window, btnBack);
		}

		status.setString(sf::String::fromUtf8(statusMsg, statusMsg + mystrlen(statusMsg)));
		window.draw(status);
		window.display();
	}

	if (admin != nullptr)
		delete admin;
	return 0;
}
