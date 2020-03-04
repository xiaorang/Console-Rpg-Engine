#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include "tools.h"

using namespace std;

struct Var{
	string name;
	string value;
};
struct VarList{
	Var var[100];
	int varNum;
	void setValue(string aName, string aValue){
		// 寻找变量是否存在，大小写不敏感 
		int find= -1;
		for (int i= 0; i< varNum; i++){
			if (upCase(aName)== upCase(var[i].name)){
				find= i;
				break;
			}
		} 
		string str1;
		if (aValue[0]=='%'){
			str1= getValue(aValue.substr(1));
		} else {
			str1= aValue;
		}
		if (find>=0){
			// 如果找到，直接修改值 
			var[find].value= str1; 
		} else {
			// 如果是新变量，则添加
			var[varNum].name= aName;
			var[varNum].value= str1;
			varNum++; 
		}
	} 
	string getValue(string aName){
		// 寻找变量是否存在，大小写不敏感 
		int find= -1;
		for (int i= 0; i< varNum; i++){
			if (upCase(aName)== upCase(var[i].name)){
				find= i;
				break;
			}
		} 
		if (find>=0){
			// 如果找到
			return var[find].value; 
		} else {
			// 如果是不存在 
			return ""; 
		}
	} 
	// 计算条件表达式（目前只支持最简单的） 
	bool calcRelaExp(string aExp){
		for (int i= 0; i< varNum; i++){
			//cout << var[i].name << " : " << var[i].value << endl;
		} 
		//cout << aExp << endl;
		string str1, str2;
		split2(aExp, '=', str1, str2);
		string vst1, vst2;
		if (str1!=""){
			if (str1[0]=='%'){
				vst1= getValue(str1.substr(1));
			} else {
				vst1= str1;
			}
			//cout << "a1: " << str1 << endl;
			//cout << "a2: " << str2 << endl;
			//cout << "b1: " << vst1 << endl;
			if (str2==""){
				return str2int(vst1);
			} else {
				if (str2[0]=='%'){
					vst2= getValue(str2.substr(1));
				} else {
					vst2= str2;
				} 
				//cout << "b2: " << vst2 << endl;
				return (vst2==vst1);
			}
		} else {
			return false;
		}
	}
};

struct Pos{
	int x, y;
	void setValue(string aStr){
		string str1, str2;
		split2(aStr, ',', str1, str2);
		x= str2int(str1);
		y= str2int(str2); 
	}
};

struct ActionList;
struct Event;

struct World{
	VarList varList;
	int CurrentMap; 
	Pos hero;
	bool menuOn;
	virtual void changeMap(string aName, int x, int y){}
	virtual void showMap(){}
	virtual void showMapPart(int x, int y, int w, int h){}
	virtual void showMenu(string aName, int x, int y, int aLockMode){}
	virtual void showSpecMenu(string aName, int x, int y, ActionList *aCode){}
	virtual void talk(string Who, string Words, int aWait){}
	virtual void save(){}
	virtual void load(string fName){}
	virtual int fightOn(string aTeam, ActionList *win, ActionList *lose){}
	virtual void callBackFunc(){}
	virtual void Info(){}
	virtual void InitVar(){}
	virtual void updateNPC(){}
	virtual void hideNPC(){}
	virtual void hideEvent(Event *evt){}
};

struct Action{
	string cmd; 
	Event *event;
	World *world;
	virtual void execute(){
		if (upCase(cmd)=="EXIT"){
			exit(0);
		} else if (upCase(cmd)=="SAVE"){
			world->save(); 
		} else if (upCase(cmd)=="INIT"){
			world->InitVar(); 
		} else if (upCase(cmd)=="HIDESELF"){
			world->hideEvent(event);
		} else if (upCase(cmd)=="INFO"){
			world->Info(); 
		} else if (upCase(cmd)=="CALLBACK"){
			world->callBackFunc(); 
		} else if (upCase(cmd)=="UPDATENPC"){
			world->updateNPC(); 
		} else if (upCase(cmd)=="HIDENPC"){
			world->hideNPC(); 
		}
	}
	virtual void show(){
		cout << "cmd: " << cmd << endl;
	}
};
// 由于在两个地方用到类似的结构，且希望用一个函数统一处理
// 故建立结构 
struct ActionList{
	// 这里必须用指针，可以指向其继承对象 
	Action *cmd[50];
	int cmdNum;
	void show(){
		cout << "指令数： " << cmdNum  << " {"<<endl;
		for (int i=0; i< cmdNum; i++){
			cmd[i]->show();
		}
		cout << "}" << endl; 
	}
	void execute(){
		for (int i=0; i< cmdNum; i++){
			gotoxy(0, 31);
			cmd[i]->execute();
		}
	}
};

struct CLoad:Action{
	string Name;
	void show(){
		cout << "load from file " << Name <<endl;
	}
	void execute(){
		//cout << Name << endl;
		world->load(world->varList.getValue(Name));
	}
};

struct CJump:Action{
	string mapName;
	Pos newp;
	void show(){
		cout << "jump to " << mapName << ":" << newp.x << "-" << newp.y <<endl;
	}
	void execute(){
		world->changeMap(mapName, newp.x, newp.y);
	}
};

struct CTalk:Action{
	string Name[20];
	string Speach[20];
	int sentNum; 
	void execute(){
		for (int i=0; i< sentNum; i++){
			world->talk(Name[i], Speach[i], 0);
		}
		world->showMap();
	}
	void show(){
		cout << "Talk of " << sentNum << endl;
		for (int i=0; i< sentNum; i++){
			cout << "    sent " << Name[i] << "：" << Speach[i] << endl;
		}
	}
};

struct CVar:Action{
	string Name;
	string Value;
	void show(){
		cout << "变量赋值 " << Name << "," << Value << endl;
	}
	void execute(){
		world->varList.setValue(Name, Value);
	}
};

struct CMenu:Action{
	string Name;
	Pos mpos;
	int Mode;   // 锁定模式
	void show(){
		cout << "打开菜单 " << Name << endl;
	}
	void execute(){
		world->showMenu(Name, mpos.x, mpos.y, Mode); 
	}
};

// 特殊菜单 
struct CSpecMenu:Action{
	string Name;    // 类型
	Pos mpos;
	ActionList onClick;
	void show(){
		cout << "打开特殊菜单 " << Name << endl;
	}
	void execute(){
		//onClick.show();
		world->showSpecMenu(Name, mpos.x, mpos.y, &onClick); 
	}
};

struct CIf:Action{
	string exp;
	ActionList block; 
	ActionList elseblock; 
	void show(){
		cout << "if (" << exp << ")"<<endl;
		block.show();
		cout << "else" << endl;
		elseblock.show();
	}
	void execute(){
		//cout << exp;
		if (world->varList.calcRelaExp(exp)) {
			block.execute();
		} else {
			elseblock.execute();
		}
	}
}; 

struct CFight:Action{
	string team;
	ActionList win; 
	ActionList lose; 
	void show(){
		cout << "fight win with (" << team << ")"<<endl;
		win.show();
		cout << "lose" << endl;
		lose.show();
	}
	void execute(){
		//cout << exp;
		world->fightOn(team, &win, &lose);
	}
}; 

struct Event{
	int trigMode;        // 1=走到触发  
					     // 2=探索（撞击）触发  
						 // 3=随机走动的探索触发  
						 // 4=自动触发 
						 // 5=随机行走的接触触发 
	Pos pos[20];         // 触发位置，支持多个位置 
	int posNum;		     // 位置数量 
	ActionList block; 
	string chart;       // NPC图像
	int color;			// 图像的颜色 
	bool enabled;        // 默认是打开的，可以关闭，即隐藏 
	// 通过一个字符串，增加一个位置点 
	void addPos(string aStr){
			pos[posNum].setValue(aStr);
			posNum++;
	}
	void addPosList(string aStr){
		string str1, str2;
		while(true) {
			split2(aStr, ';', str1, str2); 
			addPos(str1);
			if (aStr != str1) {
				aStr= str2;
			} else {
				break;
			}
		}
	} 
	//
	bool check(int aMode, int x, int y){
		if (trigMode== aMode){
			for(int i= 0; i< posNum; i++){
				if (pos[i].x==x && pos[i].y==y){
					return true;
				}
			}
		}
		return false;
	}
	//
	bool isNear(int aMode, int x, int y){
		if (trigMode== aMode){
			for(int i= 0; i< posNum; i++){
				if ((pos[i].x==x+2 && pos[i].y==y) ||
					(pos[i].x==x-2 && pos[i].y==y) ||
					(pos[i].x==x && pos[i].y==y+1) ||
					(pos[i].x==x && pos[i].y==y-1) ){
					//
					//gotoxy(70, 8);
					//cout << x << "," << y << "     ";
					return true;
				}
			}
		}
		return false;
	}
	//
	void show(){
		block.show();
	}
};

// 这是一个通用函数，返回值，是为了帮助if...else...endif这种判断 
	int readActions(ifstream &ff, 	ActionList &block, string aStop, World *wd, Event *evt){ 
		block.cmdNum= 0;
		while (!ff.eof()){
			string str1, str2;
			getline(ff, str1); 
			if (str1==""){
				continue;
			} else if (str1[0]==';'){
				// 分号开头代表注释 
				continue;
			} else if (upCase(str1)==upCase(aStop)){
				break;
			} else if (str1[0]=='@'){
				// 一个新的指令的开始 
				str1= str1.substr(1);
				if (upCase(str1)=="JUMP"){
					//cout << "jump" << endl;
					CJump *jump= new CJump();
					block.cmd[block.cmdNum]= jump;
					jump->world= wd;
					getline(ff, str2); 
					jump->mapName= str2; 
					getline(ff, str2); 
					jump->newp.setValue(str2);
				} else if (upCase(str1)=="EXIT" || upCase(str1)=="SAVE" || 
						   upCase(str1)=="INIT" || upCase(str1)=="HIDESELF" ||
						   upCase(str1)=="INFO" || upCase(str1)=="UPDATENPC" ||
						   upCase(str1)=="HIDENPC" ){
					Action *act= new Action();
					block.cmd[block.cmdNum]= act;
					act->cmd= upCase(str1);
					act->world= wd;
				} else if (upCase(str1)=="LOAD"){
					CLoad *cld= new CLoad();
					cld->world= wd;
					block.cmd[block.cmdNum]= cld;
					getline(ff, str2); 
					cld->Name= str2;
				} else if (upCase(str1)=="MENU" || upCase(str1)=="LOCKMENU"){
					CMenu *cmn= new CMenu();
					cmn->world= wd;
					block.cmd[block.cmdNum]= cmn;
					getline(ff, str2); 
					cmn->Name= str2;
					getline(ff, str2); 
					cmn->mpos.setValue(str2);
					if (upCase(str1)=="LOCKMENU"){
						cmn->Mode= 1;      // 锁定模式的菜单不可关闭，如封面菜单 
					} else {
						cmn->Mode= 0;     // 普通模式，不锁定 
					}
				} else if (upCase(str1)=="SPECMENU"){
					CSpecMenu *cmn= new CSpecMenu();
					cmn->world= wd;
					block.cmd[block.cmdNum]= cmn;
					getline(ff, str2); 
					cmn->Name= str2;
					getline(ff, str2); 
					cmn->mpos.setValue(str2);
					readActions(ff, cmn->onClick, "@endspec", wd, evt);
				} else if (upCase(str1)=="TALK"){
					//cout << "talk" << endl;
					CTalk *talk= new CTalk();
					talk->world= wd;
					block.cmd[block.cmdNum]= talk;
					talk->sentNum= 0;
					do{
						getline(ff, str2); 
						if (str2[0]!='@'){
							split2(str2, ',', talk->Name[talk->sentNum], talk->Speach[talk->sentNum]);
							talk->sentNum++;
						} else {
							break;
						}
					} while (1); 				
				} else if (upCase(str1)=="VAR"){
					//cout << "var" << endl;
					CVar *cvar= new CVar();
					cvar->world= wd;
					block.cmd[block.cmdNum]= cvar;
					getline(ff, str2); 
					split2(str2, '=', cvar->Name, cvar->Value);
				} else if (upCase(str1)=="IF"){
					CIf *cif= new CIf();
					cif->world= wd;
					block.cmd[block.cmdNum]= cif;
					getline(ff, str2);
					cif->exp= str2;
					int a= readActions(ff, cif->block, "@endif", wd, evt);
					if (a==1) {
						// 说明的读到@ELSE结束的，继续读 
						readActions(ff, cif->elseblock, "@endif", wd, evt);
					}
				} else if (upCase(str1)=="FIGHT"){
					CFight *cfit= new CFight();
					cfit->world= wd;
					block.cmd[block.cmdNum]= cfit;
					getline(ff, str2);
					cfit->team= str2;
					int a= readActions(ff, cfit->win, "@endfight", wd, evt);
					if (a==2) {
						// 说明的读到@ELSE结束的，继续读 
						readActions(ff, cfit->lose, "@endfight", wd, evt);
					}
				} else if (upCase(str1)=="ELSE"){
					// 出现else，如果不是语法错误
					//cout << "else"<< endl;
					if (upCase(aStop)=="@ENDIF") {
						// 这是正常的语法，前一个block完成 
						// cout << "else2"<< endl;
						return 1;
					} else if (upCase(aStop)=="@ENDFIGHT") {
						return 2; 
					} else {
						cout << "else 错误：" << str1;
						throw 1;
					} 
				} else {
					cout << "未知指令："+ str1 << endl;
					throw 2;
				}
				block.cmd[block.cmdNum]->world= wd;
				block.cmd[block.cmdNum]->event= evt;
				block.cmdNum++;
			} else {
				// 这里也是报错 
				cout << "指令错误：" << str1 << endl;
				throw 3;
			}
		}
		return 0;
	}

struct EventList{
	Event event[50];
	int evtNum;
	
	void readEvent(string aName, World *wd){
		//cout << "Event file: " << aName << endl;
		//system("pause");
		ifstream fin(aName.c_str());
		if (fin){
			while (!fin.eof()){
				string str1, str2;
				getline(fin, str1);
				if (str1==""){
					continue;
				} else if (str1[0]=='#'){
					// 一个新的事件开始
					event[evtNum].trigMode= (str1[1]- 'A')+ 1; 
					event[evtNum].enabled= true; 
					// 
					if (str1.size()>6){
						int a= str1.find(":");
						if (a>0){
							string str2= getBetween(str1, "(", ")");
							string str3, str4;
							split2(str2, ',', str3, str4);
							event[evtNum].chart= str3;
							event[evtNum].color= str2int(str4);
						} else {
							event[evtNum].chart= "女";
							event[evtNum].color= 10;
						}
					} else {
						event[evtNum].chart= "男";
						event[evtNum].color= 12;
					} 
					// 后面必然是位置定义
					getline(fin, str2);
					event[evtNum].addPosList(str2); 
					readActions(fin, event[evtNum].block, "#END", wd, &event[evtNum]);
					//event[evtNum].show(); 
					evtNum++;
				}
			}
		}
		fin.close();
		//system("pause");
	}
	// 调试的必备函数
	void show(){
		for (int i=0; i< evtNum; i++){
			cout << "Event " << i << endl;
			event[i].show();
		}
	}
	// 根据坐标，定位event 
	Event* findEvent(int aMode, int x, int y){
		for (int i=0; i< evtNum; i++){
			if (event[i].check(aMode, x, y)){
				return &event[i];
			}
		}
		return 0;
	}
	Event* nearEvent(int aMode, int x, int y){
		for (int i=0; i< evtNum; i++){
			if (event[i].isNear(aMode, x, y)){
				return &event[i];
			}
		}
		return 0;
	}
	// 直接运行
	void runEvent(int aMode, int x, int y){
        Event *evt; 
		evt= findEvent(aMode, x, y);
		if (evt) {
			if (evt->enabled){
				evt->block.execute();
			}
		} else if (aMode==5) {
			evt= nearEvent(aMode, x, y);
			if (evt) {
				if (evt->enabled){
					evt->block.execute();
				}
			}
		} 
	} 
	// 特殊操作，NPC刷新 
	void updateEvent(int aMode){
		for (int i=0; i< evtNum; i++){
			if (event[i].trigMode==aMode){
				event[i].enabled= true;
			}
		}
	} 
	// 更加特殊的操作，隐藏所有NPC
	void hideNPC(int aMode){
		for (int i=0; i< evtNum; i++){
			if (event[i].trigMode==aMode){
				event[i].enabled= false;
			}
		}
	} 
}; 

