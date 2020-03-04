#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;

int str2int(string aStr);
string int2str(int aInt);

string manyChar(string aCell, int aLength);

void HideCursor();
void setColor(int cc);
void setConsoleSize();
void setConsoleTitle(string aText); 
void gotoxy(unsigned char x,unsigned char y);

string lowCase(string s);
string upCase(string s);
void split2(string aStr, char aChar, string &st1, string &st2);
string getBetween(string aStr, string aFrom, string aTo);

string getTimeName();
string getExePath();
void makeDir(string aPath);
void getFiles(string path, string aExt, vector<string>& files );

