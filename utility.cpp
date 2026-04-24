#include "utility.h" 


int mystrlen(const char* s)
{
	int i = 0;
	while (s[i] != '\0') i++; // length calc
	return i;
}
void mystrcpy(char* dest, const char* src)
{
	int i = 0;
	while (src[i] != '\0') { dest[i] = src[i]; i++; } //copy to copy
	dest[i] = '\0';
}
void mystrcat(char* dest, const char* src) { 
	int i = 0;
	int size = mystrlen(dest);
	while (src[i] != '\0')
	{
		dest[i + size] = src[i]; // copy from after the 1st string ends
		i++;
	}
	dest[i + size] = '\0';
}
int mystrcmp(const char* a, const char* b)
{
	int i = 0;
	while (a[i] != '\0' || b[i] != '\0')
	{
		if (a[i] != b[i])
			return a[i] - b[i];  // positive if a>b, negative if a<b
		i++;
	}
	return 0;  // all characters matched
}
char mytolower(char c)
{
	if (c >= 65 && c <= 90)
		return c + 32;
	else
		return c;
}
int power(int num, int raised)
{
	int rslt = 1 ;
	for (int i = 0; i < raised ; i++)
	{
		rslt*= num;
	}
	return rslt;

}
int myatoi(const char* s)
{
	int num = 0;

	for (int i = 0; i < mystrlen(s); i++)
	{
		num += (s[mystrlen(s) - i - 1] - '0') * power(10, i);// intiger * (10)^position 
	}

	return num;
}

float myatof(const char* s)
{
	int i = 0;
	float num1 = 0;

	// integer part
	while (s[i] != '.' && s[i] != '\0')
	{
		num1 = num1 * 10 + (s[i] - '0');
		i++;
	}

	// no decimal point? return as is
	if (s[i] == '\0') return num1;

	i++; // skip '.'

	// decimal part
	float num2 = 0;
	float place = 0.1f;
	while (s[i] != '\0')
	{
		num2 += (s[i] - '0') * place;
		place *= 0.1f;
		i++;
	}

	return num1 + num2;
}
int mystrcmpIgnoreCase(const char* a, const char* b)
{
	int i = 0;
	while (a[i] != '\0' || b[i] != '\0')
	{
		if (mytolower(a[i]) != mytolower(b[i]))
			return mytolower(a[i]) - mytolower(b[i]);
		i++;
	}
	return 0;
}
void myitoa(int num, char* s)
{
	if (num == 0) { s[0] = '0'; s[1] = '\0'; return; } // returns nothing if already zero
	int i = 0;
	char temp[20];
	while (num > 0)
	{
		temp[i++] = '0' + (num % 10);
		num /= 10;
	}
	// reverse
	int len = i;
	for (int j = 0; j < len; j++)
		s[j] = temp[len - j - 1];
	s[len] = '\0';
}
void myftoa(float num, char* s, int decimals = 2)
{
	int intPart = (int)num;
	float decPart = num - intPart;

	// convert integer part
	myitoa(intPart, s);

	// add decimal point
	int len = mystrlen(s);
	s[len] = '.';
	s[len + 1] = '\0';

	// convert decimal part
	for (int i = 0; i < decimals; i++)
	{
		decPart *= 10;
		int digit = (int)decPart;
		s[mystrlen(s)] = '0' + digit;
		s[mystrlen(s)] = '\0';
		decPart -= digit;
	}
}