#ifndef UTILS_H
#define UTILS_H

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


#endif