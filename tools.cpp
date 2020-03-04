#include<iostream>
#include<fstream>
#include"windows.h"
#include <conio.h>
#include <sstream>
#include "tools.h"
#include <time.h>              // 时间管理 
#include <direct.h>			   // 文件夹管理
#include <vector> 

using namespace std;

int str2int(string aStr){
	stringstream res;
	res << aStr;
	int ret;
	res >> ret;
	return ret;
}

string int2str(int aInt){
	stringstream res;
	res << aInt;
	string ret;
	res >> ret;
	return ret;
}

string manyChar(string aCell, int aLength){
	string ret= "";
	for (int i=0; i< aLength; i++){
		ret= ret+ aCell;
	}
	return ret;
}

void setColor(int cc){
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), cc);
}

void HideCursor(){
    CONSOLE_CURSOR_INFO cursor_info = {1, 0};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void gotoxy(unsigned char x,unsigned char y){
    //COORD是Windows API中定义的一种结构，表示一个字符在控制台屏幕上的坐标
    COORD cor;
    //句柄
    HANDLE hout;
    //设定我们要定位到的坐标
    cor.X = x;
    cor.Y = y;
    //GetStdHandle函数获取一个指向特定标准设备的句柄，包括标准输入，标准输出和标准错误。
    //STD_OUTPUT_HANDLE正是代表标准输出（也就是显示屏）的宏
    hout = GetStdHandle(STD_OUTPUT_HANDLE);
    //SetConsoleCursorPosition函数用于设置控制台光标的位置
    SetConsoleCursorPosition(hout, cor);
}

string getTimeName(){
	struct tm *local;
	time_t t; 
	t=time(NULL);
	local=localtime(&t);
	string ret= int2str(100 +local->tm_mon).substr(1)+ "-"+ int2str(100+ local->tm_mday).substr(1)+ "-"+ int2str(100+ local->tm_hour).substr(1)+ "-"+ int2str(100+ local->tm_min).substr(1)+ "-"+ int2str(100+ local->tm_sec).substr(1);
	return ret;
}

void makeDir(string aPath){
	if (_access(aPath.c_str(), 0) == -1)	//如果文件夹不存在
		_mkdir(aPath.c_str());	
}

string getExePath(){
	char szFilePath[MAX_PATH + 1]={0};
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	(strrchr(szFilePath, '\\'))[0] = 0; // 删除文件名，只获得路径字串
	string path = szFilePath;
	return path;
}

void getFiles(string path, string aExt, vector<string>& files ){  
    //文件句柄  
    //long hFile = 0;  //win7
    intptr_t hFile = 0;   //win10
    //文件信息  
    struct _finddata_t fileinfo;  
    string p;  
    if((hFile = _findfirst(p.assign(path).append("\\*.").append(aExt).c_str(),&fileinfo)) !=  -1){
        do{
           //cout << fileinfo.name << endl;
           files.push_back(fileinfo.name);  
        }while(_findnext(hFile, &fileinfo)  == 0);  
        _findclose(hFile);  
    } 
}
void setConsoleSize(int w, int h){
	string str1;
	str1="mode con cols="+ int2str(w* 2)+" lines="+ int2str(h);
    system(str1.c_str());
}

void setConsoleTitle(string aText){
	string str1= "title "+ aText;
    system(str1.c_str());
} 

string lowCase(string s){
    int len=s.size();
    string ret= "";
    for(int i=0; i<len; i++){
    	char c= s[i];
        if(c>='A'&&c<='Z'){
            c= c- 'A'+ 'a';
        }
        ret= ret+ c;
    }
    return ret;
}
string upCase(string s){
    int len=s.size();
    string ret= "";
    for(int i=0; i<len; i++){
    	char c= s[i];
        if(c>='a'&&c<='z'){
            c= c- 'a'+ 'A';
        }
        ret= ret+ c;
    }
    return ret;
}
void split2(string aStr, char aChar, string &st1, string &st2){
	int a= aStr.find(aChar);
	if (a>=0){
		st1= aStr.substr(0, a);
		st2= aStr.substr(a+ 1);		
	} else {
		st1= aStr;
		st2= "";
	}
}
string getBetween(string aStr, string aFrom, string aTo){
	int a= aStr.find(aFrom);
	int b= aStr.find(aTo, a);
	string ret;
	if (a>=0){
		ret=  aStr.substr(a+ aFrom.size(), b- a- aFrom.size());
	} else {
		ret= "";
	}
	return ret;
}

// 常见的凑整 
string fitStr(string aStr){
	int a= aStr.size();
	if (a % 2==1){
		aStr= aStr+ " ";
	} 
	return aStr;
}
