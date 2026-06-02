cmake_minimum_required(VERSION 3.22)
project(MediCoreHospital LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_STATIC_LIBRARIES ON)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

set(MEDICORE_SOURCES
    src/Admin.cpp
    src/Appointment.cpp
    src/Bill.cpp
    src/Doctor.cpp
    src/helpers/FileHandler.cpp
    src/helpers/HospitalException.cpp
    src/Patient.cpp
    src/Person.cpp
    src/Prescription.cpp
    src/helpers/utility.cpp
    src/helpers/Validator.cpp
    src/gui/MediCoreGuiAPI.cpp
    src/gui/MediCoreGui.cpp
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
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:MediCore>/data"
)
