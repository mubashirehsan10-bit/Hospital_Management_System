cmake_minimum_required(VERSION 3.22)
project(MediCoreHospital LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_STATIC_LIBRARIES ON)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

set(MEDICORE_SOURCES
    Admin.cpp
    Appointment.cpp
    Backend.cpp
    Bill.cpp
    Doctor.cpp
    FileHandler.cpp
    HospitalException.cpp
    Patient.cpp
    Person.cpp
    Prescription.cpp
    utility.cpp
    Validator.cpp
    MediCoreGuiAPI.cpp
    MediCoreGui.cpp
)

add_executable(MediCore ${MEDICORE_SOURCES})
target_include_directories(MediCore PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(MediCore PRIVATE SFML::Graphics SFML::Window SFML::System)

if (WIN32)
    target_link_libraries(MediCore PRIVATE winmm gdi32 user32)
endif()

add_custom_command(TARGET MediCore POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:MediCore>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/patients.txt" "$<TARGET_FILE_DIR:MediCore>/patients.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/doctors.txt" "$<TARGET_FILE_DIR:MediCore>/doctors.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/admin.txt" "$<TARGET_FILE_DIR:MediCore>/admin.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/appointments.txt" "$<TARGET_FILE_DIR:MediCore>/appointments.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/bills.txt" "$<TARGET_FILE_DIR:MediCore>/bills.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/prescriptions.txt" "$<TARGET_FILE_DIR:MediCore>/prescriptions.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/security_log.txt" "$<TARGET_FILE_DIR:MediCore>/security_log.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/discharged.txt" "$<TARGET_FILE_DIR:MediCore>/discharged.txt"
)
