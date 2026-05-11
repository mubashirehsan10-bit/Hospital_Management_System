#ifndef UTILS_H
#define UTILS_H
#include<fstream>

int mystrlen(const char* s);
void mystrcpy(char* dest, const char* src);
void mystrcat(char* dest, const char* src);
int mystrcmp(const char* a, const char* b);
char mytolower(char c);
int myatoi(const char* s);
float myatof(const char* s);
int mystrcmpIgnoreCase(const char* a, const char* b);
void myitoa(int num, char* s);
void myftoa(float num, char* s, int decimals = 2);
void myreadLine(std::ifstream& fin, char* buffer, int maxSize);
void getToken(const char* line, char* token, int& pos);
void getStandardTimeSlot(int index, char* out);
int getCurrentCalendarYear();
void formatTimestampFull(char* buf, int cap);

#endif