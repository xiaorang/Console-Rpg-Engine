#include <iostream>
#include <fstream>
#include "windows.h"
#include <conio.h>
#include <sstream>
#include "tools.cpp"
#include "audio_clip.cpp"
#include "actions.cpp"
#include <stack>
#include <vector>
#include <algorithm>

using namespace std;

string config= ".\\game.ini";

void drawWindow(int x, int y, int w, int h){
	string up="▁";
	string left= "▕";
	string right= "▏";
	string down="▔";
	// 上边缘修饰 
	for (int i=1; i< w/2- 1; i++){
		gotoxy(x+ i*2, y);
		cout << up;
		gotoxy(x+ i*2, y+h- 1);
		cout << down;
	}
	for (int i=1; i< h- 1; i++){
		gotoxy(x, y+ i);
		cout << left;
		gotoxy(x+ w- 2, y+i);
		cout << right;
	}
	// 内部填充 
	for (int i=1; i< w/2- 1; i++){
		for (int j=1; j< h- 1; j++){
			gotoxy(x+i*2, j+y);
			cout << "　";
		}
	}
}

void textOut(int x, int y, string aText, int aWidth, int aWait){
	int a, b;
	a= 0;
	b= 0;
	for (int i=0; i< aText.size()/ 2; i++){
		gotoxy(x+ a, y);
		cout << aText.substr(i* 2, 2);
		Sleep(aWait);
		a+= 2;
		if (a> aWidth){
			b++;
			a= 0;
		}
	}	
}


struct MenuItem{
	string text;
	ActionList *code;
};

struct Menu{
	string Name;               // 用于标识的位置 
	MenuItem item[10];
	int itemNum;
	int wx, wy, ww, wh;
	int left, top, bottom;     // 限制位置 
	int yy;                    // 当前位置 
	World *world;
	string mark= "·";
	// 锁定模式
	int LockMode;
	// 添加菜单项
	void addItem(string aName, ActionList *onClick){
		item[itemNum].text= aName;
		item[itemNum].code= onClick;
		itemNum++;
	} 
	void clear(){
		itemNum= 0; 
	}
	void loadFromFile(string aName, World *wd){
		world= wd;
		ifstream fin(aName.c_str());
		// cout << aName << endl;
		if (fin){
			//cout << "have file" << endl;
			itemNum= 0;
			while (!fin.eof()){
				string str1, str2;
				getline(fin, str1);
				if (str1==""){
					continue;
				} else if (str1[0]=='#'){
					// 一个新的菜单项的开始
					item[itemNum].text= str1.substr(1); 
					//cout << item[itemNum].text << endl;
					// 后面是代码
					ActionList *temp= new ActionList();
					readActions(fin, *temp, "#END", wd, 0);
					
					item[itemNum].code= temp;
					itemNum++;
				}
			}
		}
		fin.close();
		//getch();
	}
	void showDot(bool aShow){
		setColor(15);
		gotoxy(left, yy);
		if (aShow){
			cout << mark;		
		} else {
			cout << "  ";
		}
	}
	void showMenu(int x, int y, int aLockMode){
		// 首先自动计算窗口的宽高 
		int w=2, h=itemNum;
		for (int i=0; i< itemNum; i++){
			if (item[i].text.size()> w){
				w= item[i].text.size();
			}
		} 
		// 根据宽高，画窗口，宽度留边，加选项点 
		setColor(15);
		drawWindow(x, y, w+ 8, h+ 4);
		// 关闭窗口用的信息 
		wx= x;
		wy= y;
		ww= w+8;
		wh= h+4;
		// 显示文字 
		for (int i=0; i< itemNum; i++){
			textOut(x+ 4, y+ 2+ i, item[i].text, w+6, 0); 
		} 
		// 显示选择点
		left= x+ 2;
		top= y+ 2;
		bottom= y+ 2+ itemNum- 1;
		yy= top;
		showDot(1);
		// 菜单模式打开
		world->menuOn= true; 
		// 是否锁定方式 
		LockMode= aLockMode;
	}
	void hideMenu(){
		world->menuOn= false; 
		world->showMapPart(wx/ 2, wy, ww/ 2+ 2, wh);
	} 
};
struct Map{
	string Name;
	string map[100];
	string mapmove[100];
	string mapcolor[50];
	int Map_Height, Map_Width;
	int Map_Color_Num;
	// 地图背景音乐
	AudioClip sound; 
	// 事件信息 
	EventList eventList;
	// 关联整体世界，以了解英雄的位置
	World *world; 
	// 以下是函数 
	int getColor(string aCode){
		string str1= "", str_else;
		for (int i=0; i< Map_Color_Num; i++){
			int a= mapcolor[i].find("else");
			if (a>=0) {
				str_else= mapcolor[i];
				break;
			}
		}
		for (int i=0; i< Map_Color_Num; i++){
			int a= mapcolor[i].find(aCode);
			if (a>=0) {
				str1= mapcolor[i];
				break;
			}
		}
		//cout << found << endl;
		int cc;
		if (str1!=""){
			int b= str1.find("=");
			string str2= str1.substr(b+ 1, 2);
			cc= str2int(str2);
		} else {
			if (str_else!=""){
				int b= str_else.find("=");
				string str2= str_else.substr(b+ 1, 2);
				cc= str2int(str2);
			} else {
				cc= 15;
			}
		}
		return cc;
	} 
	bool canMove(int x, int y){
		if (mapmove[y][x]=='1'){
			Event *ev= eventList.findEvent(3, x* 2, y);
			if (ev && ev->enabled){
				return 0; 
			}
			ev= eventList.findEvent(5, x* 2, y);
			if (ev && ev->enabled){
				return 0; 
			}
			if (x==world->hero.x/2 && y== world->hero.y){
				return 0;
			}
			return 1;
		} else {
			return 0;
		}
	}

	void moveNPC(){
		for (int i=0; i< eventList.evtNum; i++){
			if ((eventList.event[i].trigMode==5 || eventList.event[i].trigMode==3) 
				&& eventList.event[i].enabled){
				// 随机移动，或者横向移动，或者纵向 
				int a= rand()% 2;
				int b= rand()% 3 - 1;
				// 这种NPC只能是单个 
				int x= eventList.event[i].pos[0].x;
				int y= eventList.event[i].pos[0].y;
				int x1, y1;
				if (a==0) {
					x1= x+ b* 2;
					y1= y;
				} else {
					x1= x;
					y1= y+ b;
				}
				if (canMove(x1/ 2, y1)){
					gotoxy(x, y);
					setColor(getColor(map[y].substr(x, 2)));
					cout << map[y].substr(x, 2);
					//
					eventList.event[i].pos[0].x= x1;
					eventList.event[i].pos[0].y= y1;
					// 
					gotoxy(x1, y1);
					setColor(eventList.event[i].color);
					cout << eventList.event[i].chart;
				}
			}
		}
	}	
	void showNPC(){
		for (int i=0; i< eventList.evtNum; i++){
			if ((eventList.event[i].trigMode==5 || eventList.event[i].trigMode==3)){
				if (eventList.event[i].enabled){
					// 显示出来 
					// 这种NPC只能是单个 
					int x= eventList.event[i].pos[0].x;
					int y= eventList.event[i].pos[0].y;
					// 
					gotoxy(x, y);
					setColor(eventList.event[i].color);
					cout << eventList.event[i].chart;
				} else {
					// 不显示 
				}
			}
		}
	};
	void hideNPC(){
		for (int i=0; i< eventList.evtNum; i++){
			if ((eventList.event[i].trigMode==5 || eventList.event[i].trigMode==3)){
				if (eventList.event[i].enabled){
					// 该正常显示的NPC不做处理 
				} else {
					// 不显示的NPC直接隐藏 
					// 这种NPC只能是单个 
					int x= eventList.event[i].pos[0].x;
					int y= eventList.event[i].pos[0].y;
					// 
					gotoxy(x, y);
					string str1= map[y].substr(x, 2);
					setColor(getColor(str1));
					cout << str1;
				}
			}
		}
	};	
	void showMap(){
		cout << Map_Height << "," << Map_Width << endl;
		//system("pause");
		//return ;
		for (int i=0; i< Map_Height; i++){
			for (int j= 0; j< Map_Width; j++){
				gotoxy(j*2, i);
				string str1;
				str1= map[i].substr(j* 2, 2);
				setColor(getColor(str1));
				cout << str1;
			}
		}
		showNPC();
	}
	
	void showMapPart(int x, int y, int w, int h){
		//cout << Map_Height << "," << Map_Width << endl;
		//return ;
		int xx, yy;
		for (int i=0; i< h; i++){
			for (int j= 0; j< w; j++){
				xx= x+ j;
				yy= y+ i;
				gotoxy(xx*2, yy);
				string str1;
				str1= map[yy].substr(xx* 2, 2);
				setColor(getColor(str1));
				cout << str1;
			}
		}
		showNPC();
	}

	void loadMap(string aDataPath, string aSndPath, string FName, World *wd){
		string str1;
		// 首先获取纯文件名作为地图名 
		Name= FName;
		world= wd;
		//cout << aDataPath << endl;
		//cout << aSndPath << endl;
		//cout << FName << endl;
		// 载入音频
		char ctemp[100];
		string temp;  
		GetPrivateProfileString("map", FName.c_str(), "", ctemp, 100, config.c_str());
		temp= string(ctemp);
		str1= aSndPath+ temp;	
		//cout << str1 << endl;	
		sound.load(str1);
		//system("pause");
		// 载入地图
		str1= aDataPath+ FName + ".map"; 
		//cout << str1 << endl;
		ifstream fin(str1.c_str());
		int i= 0; 
		Map_Height= 0;
		if (fin){
			//cout << "load map" << endl;
			while (!fin.eof()){
				getline(fin, str1);
				// 同时保存到map中
				if (i==0) {
					Map_Width= str1.size();
				}
				if (str1.size()< Map_Width){
					//Map_Height++;
					str1= str1+ manyChar(" ", Map_Width- str1.size()) ;
				}
				map[i]= str1;
				Map_Height++;
				i++;
			}
			fin.close();	
			Map_Width= Map_Width/ 2;
		}
		// 载入颜色定义
		str1= aDataPath+ FName + "_color.txt"; 
		//cout << str1 << endl;
		fin.open(str1.c_str());
		i= 0; 
		if (fin){
			//cout << "load color map" << endl;
			while (!fin.eof()){
				getline(fin, str1);
				// 同时保存到map中
				mapcolor[i]= str1;
				i++;
			}
			Map_Color_Num= i;
			fin.close();	
			//cout << Map_Color_Num << endl;
		}
		// 载入通行度信息 
		str1= aDataPath+ FName+ "_move.map"; 
		//cout << str1 << endl;
		fin.open(str1.c_str());
		i= 0; 
		if (fin){
			//cout << "load move map" << endl;
			while (!fin.eof()){
				getline(fin, str1);
				// 同时保存到map中
				mapmove[i]= str1;
				i++;
			}
			fin.close();	
		} else {
			// 未定义通行度，代表全部可行
			for (int i= 0; i<Map_Height; i++){
				mapmove[i]= manyChar("1", Map_Width*2); 
			} 
		}
		// 载入地图事件 
		str1= aDataPath+ FName+ "_event.txt"; 
		//cout << str1 << endl;
		eventList.readEvent(str1, wd); 
		//eventList.show();
		// 这一句可以看出懒加载的效果 
		//system("pause");
	}
};

// 战斗角色信息 
struct ActorInfo{
	string Name;
	string Face;
	int Lev, att, def, hpMax;
	int hp, exp;
	void showInfo(int x, int y, int aWait){
		vector <string> info;
		info.push_back(fitStr("【"+ Name+ "】")); 
		info.push_back(fitStr("等级："+ int2str(Lev)));
		info.push_back("攻击："+ fitStr(int2str(att)));
		info.push_back("防御："+ fitStr(int2str(def)));
		info.push_back("体力："+ fitStr(int2str(hp)+ "/"+ int2str(hpMax)));
		info.push_back("经验："+ fitStr(int2str(exp)+ "/100"));
		//	
		setColor(15);	
		int a= 4; 
		int b= 2;
		int w= 22; 
		int h= 10;
		drawWindow(x, y, w, h);
		for (int i=0; i< info.size(); i++){
			textOut(x+ a, y+b+i, info.at(i), w, 0);
		}
		// 暂停一下 
		if (aWait>=0){
			// 显示停止提示
			gotoxy(x+ w- 4, y+ h- 2);
			cout << "◎" ; 
			// 只有选择和取消键，才能继续，方向键不行 
			while(1){
		    	if (kbhit()){
			        char ch1= getch();
			        if (ch1==27 || ch1== 32 || ch1== 13){
		    	    	break;
					}
				} 
				//onTime();
			}  
		}
	} 
}; 

// 一场战斗过程
struct Fight{
	ActorInfo *left;          // 左边通常为玩家 
	ActorInfo *right;	      // 右边通常为NPC 
	int rnd, half;            // 回合数，半场数 
	int result;				  // 战斗结果 0=战斗中 1=左边胜，2=右边胜 
	ActionList *win;		  // 胜利脚本 
	ActionList *lose;         // 失败脚本 
	void fightRound(){
		if (result!=0){
			// 已经分出胜负，无战斗
			return; 
		}
		// 分两个半场分别动作
		if (half==0){
			int a;
			a= right->hp- (left->att - right->def);
			if (a<= 0){
				a= 0;
				result= 1;
			}
			right->hp= a;
			half= 1;
		} else {
			int a;
			a= left->hp- (right->att - left->def);
			if (a<= 0){
				a= 0;
				result= 2;
			}
			left->hp= a;
			half= 0;
			rnd++;
		}
		if (result==1){
			// 执行胜利脚本 
		} else if (result==2){
			// 执行失败脚本 
		} 
	} 
	void init(ActorInfo *a1, ActorInfo *a2, ActionList *s1, ActionList *s2){
		left= a1;
		right= a2;
		win= s1;
		lose= s2;
		rnd= 1;
		half= 0;
		result= 0;
	}
	void showScene(){
		// 窗口打底 
		drawWindow(0, 1, 60, 12);
		// 显示信息
		left->showInfo(2, 2, -1); 
		right->showInfo(36, 2, -1); 
		// 显示回合数及半场数
		//drawWindow(24, 6, 12, 4);
		string str1;
		str1= "第"+fitStr(int2str(rnd))+"回合"; 
		textOut(26, 7, fitStr(str1), 10, 0); 
		if (half== 0){
			str1= "  我方";
		} else {
			str1= "  敌方";
		}
		textOut(26, 8, fitStr(str1), 20, 0); 
	}
}; 

struct MapWorld:World{
	Map mapList[20];
	int mapNum;
	string currentMapName; 
	string dataPath;
	string savePath;
	string sndPath;
	// 英雄基本信息 
	ActorInfo info; 
	ActorInfo enemy; 	
	// 战斗过程
	Fight fight; 
	ActionList callBack; 
	// 菜单列表 
	Menu menu[50];
	int menuNum;
	int currentMenu;
	stack <int> menuStack;
	// 调试模式：显示坐标信息，辅助
	bool debugOn; 
	// 定时器
	bool stopTimer;
	int pre_time; 
	// 音乐播放器 
	AudioClip sound;
	// 显示基本信息
	void Info(){
		loadInfoFromVar();
		info.showInfo(2, 2, 0);
		showMap();
	} 
	// 播放短音效的函数
	void playSound(string aName) {
		char ctemp[100];
		string temp, str1, str2;  
		GetPrivateProfileString("sound", aName.c_str(), "", ctemp, 100, config.c_str());
		temp= string(ctemp);

		sound.load(sndPath+ temp);
		sound.play();
	}
	// 战斗函数
	int fightOn(string aTeam, ActionList *win, ActionList *lose){
		// 战斗开始，音乐转换
		mapList[CurrentMap].sound.stop(); 
		playSound("fight");
		// 战斗时建立敌人信息  
		char ctemp[100];
		string temp, str1, str2;  
		GetPrivateProfileString("team", aTeam.c_str(), "", ctemp, 100, config.c_str());
		temp= string(ctemp);
		//
		split2(temp, ',', str1, str2);
		enemy.Name= str1;
		temp= str2;
		split2(temp, ',', str1, str2);
		enemy.Lev= str2int(str1);
		temp= str2;
		split2(temp, ',', str1, str2);
		enemy.att= str2int(str1);
		temp= str2;
		split2(temp, ',', str1, str2);
		enemy.def= str2int(str1);
		temp= str2;
		split2(temp, ',', str1, str2);
		enemy.hpMax= str2int(str1);
		enemy.hp= enemy.hpMax;
		enemy.exp= 0; 
		loadInfoFromVar(); 
		fight.init(&info, &enemy, win, lose);
		fight.showScene();
		showSpecMenu("fight", 24, 13, &callBack);
		// 只有选择和取消键，才能继续，方向键不行 
	}
	// 战斗菜单回调函数
	void callBackFunc(){
		// 从变量中获得刚才选择的代码
		string s1= varList.getValue("Fight_Id");
		if (s1== "0") {
			// 攻击
			Sleep(50);
			playSound("att1");
			fight.fightRound(); 
			// 数据变化保存
			saveInfoToVar(); 
			fight.showScene();
			Sleep(1000);
			if (fight.result==1){
				// 胜利的情况下，增加经验值
				sound.stop();
				playSound("win");
				fight.left->exp += ((fight.right->Lev+1)* 40/ (fight.left->Lev+1));
				if (fight.left->exp>= 100){
					Sleep(1000);
					sound.stop();
					playSound("upLevel");
				}
				while (fight.left->exp>= 100){
					fight.left->Lev +=1;
					fight.left->att +=2;
					fight.left->def +=1;
					fight.left->hpMax +=5; 
					fight.left->exp -=100;
				}
				saveInfoToVar();
				fight.win->execute();
				// 执行代码后画地图，战败的NPC会消失 
				showMap();
				sound.stop();
				mapList[CurrentMap].sound.play(); 
			} else {
				playSound("att2");
				fight.fightRound(); 
				fight.showScene();
				if (fight.result==2){
					showMap();
					sound.stop();
					mapList[CurrentMap].sound.play(); 
					fight.lose->execute();
				} else {
					// 再次显示菜单，无须传回调脚本，故此可以输入0 
					showSpecMenu("fight", 24, 13, 0);
				}
			}
		} else if (s1=="1") {
			// 逃跑，直接退出 
			sound.stop();
			mapList[CurrentMap].sound.play(); 
			showMap();
		}
	} 
	//
	virtual void updateNPC(){
		// 必然是刷新当前地图
		mapList[CurrentMap].eventList.updateEvent(5);
	}
	virtual void hideNPC(){
		// 同时刷新当前地图
		mapList[CurrentMap].eventList.hideNPC(5);
		mapList[CurrentMap].hideNPC();
	}
	//  
	void savetoFile(string aName){
		ofstream fout(aName.c_str());
		// 所需保存的信息
		// 当前所在地图
		fout << mapList[CurrentMap].Name<< endl; 
		// 当前位置
		fout << hero.x <<"," << hero.y << endl; 
		// 当前全部已定义的变量 
		fout << varList.varNum << endl;
		for (int i=0; i< varList.varNum; i++){
			fout << varList.var[i].name << "=" << varList.var[i].value << endl;
		}
		// 英雄数据保存
		// 这里不能这样操作 saveInfoToVar();
		// 关闭 
		fout.close();
	}
	void save(){
		// 如果不存在，则创建文件夹
		//cout << getExePath()+ "\\"+ savePath << endl;
		makeDir(getExePath()+ "\\"+ savePath); 
		// 获取文件名 
		string str1= getTimeName();
		// 对齐两位字符，以确保显示正常 
		str1= str1+ manyChar(" ", 15- str1.size());
		str1= str1+ "@"+ mapList[CurrentMap].Name;
		//
		// cout << str1 << endl;
		// 保存
		savetoFile(savePath+ str1+ ".sav"); 
	}
	void loadFromFile(string aName){
		ifstream fin(aName.c_str());
		// 当前所在地图
		fin >> currentMapName; 
		// 当前位置
		string str1;
		fin >> str1;
		hero.setValue(str1);
		// 当前全部已定义的变量 
		fin >> varList.varNum;
		for (int i=0; i< varList.varNum; i++){
			fin >> str1;
			string str2, str3;
			split2(str1, '=', str2, str3);
			varList.var[i].name= str2;
			varList.var[i].value= str3;
		}
		// 英雄数据读取 
		loadInfoFromVar();
		// 关闭 
		fin.close();
	}
	void load(string fName){
		//cout << fName;
		loadFromFile(savePath+ fName+".sav");
		changeMap(currentMapName, hero.x, hero.y);
	}
	void loadData(){
		// 涉及了基本架构的直接常量，但不包含数据内容 
		string str1, str2, str3;
		char cpath[100];
		GetPrivateProfileString("system", "datapath", ".\\data\\", cpath, 100, config.c_str());
		dataPath= string(cpath);
		GetPrivateProfileString("system", "savepath", ".\\save\\", cpath, 100, config.c_str());
		savePath= string(cpath);
		GetPrivateProfileString("system", "soundpath", ".\\sound\\", cpath, 100, config.c_str());
		sndPath= string(cpath);
		// 调试模式 
		debugOn= GetPrivateProfileInt("system", "debug", 1, config.c_str()); 
		// 修改窗口名称，修改窗口大小 
		char ctemp[100];
		string temp;  
		GetPrivateProfileString("windows", "title", "", ctemp, 100, config.c_str());
		temp= string(ctemp);
		setConsoleTitle(temp);
		//
		int ww= GetPrivateProfileInt("windows", "width", 30, config.c_str());
		int hh= GetPrivateProfileInt("windows", "height", 30, config.c_str());
		setConsoleSize(ww, hh);
		// 载入地图主菜单信息 
		//GetPrivateProfileString("system", "mainMenu", "", ctemp, 100, config.c_str());
		//temp= string(ctemp);
		// 初始信息 
		GetPrivateProfileString("system", "enter", "", ctemp, 100, config.c_str());
		currentMapName=string(ctemp);
		//CurrentMap= GetPrivateProfileInt("system", "enter", 0, config.c_str());
		GetPrivateProfileString("system", "initPos", "", ctemp, 100, config.c_str());
		temp= string(ctemp);
		hero.setValue(temp);
		//mainMenu.loadFromFile(path+temp, this);
		//system("pause"); 
	}
	void hideEvent(Event *evt){
		evt->enabled= false;
	}
	void loadInfoFromVar(){
		info.Face= varList.getValue("_Face");
		info.Name= varList.getValue("_Name");
		info.att= str2int(varList.getValue("_att"));
		info.def= str2int(varList.getValue("_def"));
		info.hp= str2int(varList.getValue("_hp"));
		info.hpMax= str2int(varList.getValue("_hpMax"));
		info.Lev= str2int(varList.getValue("_Lev"));
		info.exp= str2int(varList.getValue("_Exp"));
	}
	void saveInfoToVar(){
		varList.setValue("_Face", info.Face);
		varList.setValue("_Name", info.Name);
		varList.setValue("_Att", int2str(info.att));
		varList.setValue("_def", int2str(info.def));
		varList.setValue("_hp", int2str(info.hp));
		varList.setValue("_hpMax", int2str(info.hpMax));
		varList.setValue("_Lev", int2str(info.Lev));
		varList.setValue("_Exp", int2str(info.exp));
	}
	void InitVar(){
		varList.varNum= 0;
		char ctemp[100];
		string temp;  
		GetPrivateProfileString("hero", "name", "", ctemp, 100, config.c_str());
		info.Name= string(ctemp);
		GetPrivateProfileString("hero", "Face", "", ctemp, 100, config.c_str());
		info.Face= string(ctemp);
		info.att= GetPrivateProfileInt("hero", "att", 1, config.c_str());
		info.def= GetPrivateProfileInt("hero", "def", 1, config.c_str());
		info.Lev= GetPrivateProfileInt("hero", "Lev", 1, config.c_str());
		info.hpMax= GetPrivateProfileInt("hero", "hp", 1, config.c_str());
		info.hp= info.hpMax;
		info.exp= 0;
		// 同时保存到变量系统中
		saveInfoToVar(); 
		//system("pause");
	}
	void init(){
		CurrentMap= -1;
		HideCursor();
		pre_time= clock();
		// 实现回调功能 
		callBack.cmd[0]= new Action();
		callBack.cmd[0]->cmd= "CALLBACK";
		callBack.cmd[0]->world= this;
		callBack.cmdNum= 1;
	} 
	void changeMap(string aName, int x, int y){
		// 首先停止旧音乐
		if (CurrentMap>=0){
			mapList[CurrentMap].sound.stop();
		}
		// 查找地图 
		bool found= false;
		for (int i=0; i< mapNum; i++){
			if (mapList[i].Name== aName){
				CurrentMap= i;
				found= true;
				break;
			}
		}
		if (!found){
			// 未找到则加载 
			mapList[mapNum].loadMap(dataPath, sndPath, aName, this);
			CurrentMap= mapNum;
			mapNum++;
		}
		// 换地图播放音乐
		mapList[CurrentMap].sound.play(); 
		//cout << str1;
		//system("pause");
		// 
		hero.x= x;
		hero.y= y;
		// 执行指令前显示地图，会让自动指令中隐藏的NPC留下痕迹 
		showMap();
		// 执行自动指令 
        Event *evt; 
		evt= mapList[CurrentMap].eventList.findEvent(4, -1, -1);
		if (evt) {
			//system("pause");
			evt->block.execute();
		}		
		// 执行指令后，再显示地图，会阻碍指令中弹出的菜单 
		// showMap();
	}
	void talk(string Who, string Words, int aWait){
		int x, y, w, h; 
		x= 2;
		w= 60;
		h= 10;
		// 根据位置，选择在哪里画窗口 
		setColor(15);
		if (hero.y< 15){
			drawWindow(0, 20, w, h);
			y= 22;
		} else {
			drawWindow(0, 0, w, h);
			y= 2;
		}
		for (int i=0; i< Who.size()/ 2; i++){
			gotoxy(x+2+i*2, y);
			cout << Who.substr(i*2, 2);
			Sleep(50);
		}
		int b= 0;
		int a= 2; 
		for (int i=0; i< Words.size()/ 2; i++){
			gotoxy(x+a, y+2+b);
			cout << Words.substr(i* 2, 2);
			Sleep(50);
			a+= 2;
			if (a> 50){
				b++;
				a= 2;
			}
		}
		if (aWait>=0){
			// 说一句之后，暂停一下 
			// 显示一个三角形作为提示
			gotoxy(x+ w- 8, y+ h- 5);
			cout << "◎" ; 
			// 只有选择和取消键，才能继续，方向键不行 
			while(1){
		    	if (kbhit()){
			        char ch1= getch();
			        if (ch1==27 || ch1== 32 || ch1== 13){
		    	    	break;
					}
				} 
				//onTime();
			} 
		}
	} 
	bool checkSpeak(int x, int y){
		// 2 是固定不变的探索点 
		mapList[CurrentMap].eventList.runEvent(2, x, y);
		// 3 是可自由移动的NPC
		mapList[CurrentMap].eventList.runEvent(3, x, y);
		// 5 是可自由移动的敌人，撞击同样触发 
		mapList[CurrentMap].eventList.runEvent(5, x, y);
	} 
	void showSpecMenu(string aName, int x, int y, ActionList *aCode){
		// 首先找到特殊菜单是否已经占位 
		int find= -1;
		int lock;
		for (int i=0; i< menuNum; i++){
			// 特殊菜单 
			if (menu[i].Name== aName){
				find= i;
			}
		}
		if (find>=0){
			currentMenu= find;
		} else {
			currentMenu= menuNum;
			menu[currentMenu].world= this;
			menu[menuNum].Name= aName;
			menuNum++;
			// 新建菜单 
			if (upCase(aName)=="FIGHT") {
		    	menu[currentMenu].addItem("攻击", aCode);
		    	menu[currentMenu].addItem("逃跑", aCode);
			}
			lock= 1;
		}
		// 特殊菜单不存在已经打开的问题，总是重新载入
		if (upCase(aName)=="SAVE") {
			vector <string> files;
			getFiles(savePath, "sav", files);
			menu[currentMenu].clear();
			sort(files.begin(), files.end());
			int fsum= files.size();
			for (int i = 1; i <= fsum && i<= 5; i++) {
				string str1= files.at(fsum- i);
				int a= str1.find_last_of(".");
				string str2= str1.substr(0, a);
				
				//cout << str1 << endl;
				//aCode->show();
			    menu[currentMenu].addItem(str2, aCode);
			}
			lock= 0;
		}
		// 
		menu[currentMenu].showMenu(x, y, lock);
		menuOn= true; 
	}
	void showMenu(string aName, int x, int y, int aLockMode){
		// 如果新打开的菜单来自其他的菜单项，则记录之 
		// 如果菜单已经载入，则直接运行，否则，先载入
		int find= -1;
		for (int i=0; i< menuNum; i++){
			if (menu[i].Name== aName){
				find= i;
			}
		}
		if (find>=0){
			currentMenu= find;
		} else {
			menu[menuNum].loadFromFile(dataPath+ aName+ ".txt", this);
			menu[menuNum].Name= aName;
			currentMenu= menuNum;
			menuNum++;
		}
		menu[currentMenu].showMenu(x, y, aLockMode);
		menuOn= true; 
	}
	void showHero(){
		gotoxy(hero.x, hero.y);
		setColor(mapList[CurrentMap].getColor(info.Face));
		cout << info.Face;
	}
	void moveHero(int x, int y){
		// 首先判断是否允许走到下一个点
		if (debugOn){ 
			gotoxy(70, 4);
			cout << mapList[CurrentMap].Map_Width << "-" << mapList[CurrentMap].Map_Height;
			gotoxy(70, 5);
			cout << x << "," << y <<"    ";
			gotoxy(70, 6);
			cout << hero.x << "," << hero.y <<"    ";
		} 
		if (x< 0 || y< 0 || x/ 2>= mapList[CurrentMap].Map_Width || y>= mapList[CurrentMap].Map_Height){
			// 超边，也出声音 
			if (!checkSpeak(x, y)){
				Beep(200, 100);
			}
			return; 
		}
		if (mapList[CurrentMap].canMove(x/ 2, y)) {
			//
			string under;
			under= mapList[CurrentMap].map[hero.y].substr(hero.x, 2);
			gotoxy(hero.x, hero.y);
			setColor(mapList[CurrentMap].getColor(under));
			cout << under;
			hero.x= x;
			hero.y= y;
			showHero();
			// 实际发生移动，才触发移动事件 
			mapList[CurrentMap].eventList.runEvent(1, hero.x, hero.y);
		} else {
			// 给声音提示 
			if (!checkSpeak(x, y)){
				Beep(200, 100);
			}
		} 
	}
	void showMap(){
		system("cls");
		mapList[CurrentMap].showMap();
		showHero();
	}
	void showMapPart(int x, int y, int w, int h){
		mapList[CurrentMap].showMapPart(x, y, w, h);
		showHero();
	}
	void mapControl(char ch1){
        // 按一个键，返回两个值，第一个是224 
        switch(ch1){
	        case 72:
	        	// cout << "向上移动" << endl; 
	        	moveHero(hero.x, hero.y- 1);
	            break;
	        case 80:
	        	//cout << "向下移动" << endl; 
	        	moveHero(hero.x, hero.y+ 1);
	            break;
	        case 75:
	        	//cout << "向左移动" << endl; 
	        	moveHero(hero.x-2, hero.y);
	            break;
	        case 77:
	        	//cout << "向右移动" << endl; 
	        	moveHero(hero.x+2, hero.y);
	            break;
	        case 27:
	        	//cout << "取消" << endl; 
	        	showMenu("mainMenu", 5, 5, 0);
	        case 13:
	        case 32:
	        	//cout << "探索" << endl; 
	            break;
	    } 
	}
	void menuControl(char ch1){
        switch(ch1){
	        case 72:
	        	// cout << "向上移动" << endl; 
				menu[currentMenu].showDot(0);
	        	if (menu[currentMenu].yy> menu[currentMenu].top ){
	        		menu[currentMenu].yy= menu[currentMenu].yy- 1;
				} else {
					menu[currentMenu].yy= menu[currentMenu].bottom;					
				}
				menu[currentMenu].showDot(1);
	            break;
	        case 80:
	        	//cout << "向下移动" << endl; 
				menu[currentMenu].showDot(0);
	        	if (menu[currentMenu].yy< menu[currentMenu].bottom){
	        		menu[currentMenu].yy= menu[currentMenu].yy+ 1;
				} else {
					menu[currentMenu].yy= menu[currentMenu].top;					
				}
				menu[currentMenu].showDot(1);
	            break;
	        case 27:	        	
				// 菜单模式关闭
				if (!menu[currentMenu].LockMode){
					menu[currentMenu].hideMenu(); 
					// 如果菜单栈不空 
					// 此时应当重新启动前一个菜单 
					if (!menuStack.empty()){
						currentMenu= menuStack.top();
						menuStack.pop(); 
						// 把原菜单的当前选项保留 
						int a= menu[currentMenu].yy;
						menu[currentMenu].showMenu(menu[currentMenu].wx, menu[currentMenu].wy, menu[currentMenu].LockMode);
						menuOn= true; 						
						// 同时，保留前一菜单的选项位置 
						menu[currentMenu].showDot(0);
						menu[currentMenu].yy= a;
						menu[currentMenu].showDot(1);
					} 
				}
				break;
	        case 32:
	        case 13:
	        	//选择了选项
				int a= menu[currentMenu].yy- menu[currentMenu].top;
				// 将选择的内容和ID号记录在变量中，以便脚本使用
				varList.setValue(menu[currentMenu].Name+"_id", int2str(a)); 
				varList.setValue(menu[currentMenu].Name+"_name", menu[currentMenu].item[a].text); 
				//gotoxy(0, 0);
				//setColor(15);
				//cout << menu[currentMenu].Name+"_id" << endl;
				// 暂时记录当前的菜单号 
				int m1= currentMenu;
				//gotoxy(70, 9);
				//cout << "execute" << endl;
				//item[a].code.show(); 
				// 这里，menuOn的开关必须放在执行指令之前
				// 否则执行的指令如果有换场景并打开菜单的操作，将异常
				menu[currentMenu].hideMenu(); 
				menu[currentMenu].item[a].code->execute(); 
				// 如果运行过后，菜单号发生变化，则出现菜单递进的问题。
				if (currentMenu!=m1){
					menuStack.push(m1);
				} else {
					// 没有打开菜单，则所有菜单序列将清空
					while (!menuStack.empty()){
						menuStack.pop();
					} 
				}
	            break;
        }
	} 
	void onTime(){
		if (stopTimer || menuOn){
			return;
		}
		int x= clock();
		if (x- pre_time> CLOCKS_PER_SEC){
			mapList[CurrentMap].moveNPC();
			// NPC移动后，立即测试贴近触发
			mapList[CurrentMap].eventList.runEvent(5, hero.x, hero.y); 
			pre_time= x;
		}
	}
	void run(){
		if (CurrentMap>=0){
			showMap(); 
			// 执行自动指令 
			mapList[CurrentMap].eventList.runEvent(4, -1, -1);
		} else {
			changeMap(currentMapName, hero.x, hero.y);
		}
		unsigned char ch1;
	    while(true){
	    	if (kbhit()){
		        ch1=getch();
				if (debugOn){ 
			        gotoxy(70, 7);
		    	    cout << int(ch1) << "," << menuOn << "    ";
		    	} 
		        if (menuOn) {
		        	menuControl(ch1);
				} else {
					mapControl(ch1);
				}
			} else {
				onTime();
			}
		}
	}
};

MapWorld world;

int main(){
	world.loadData();
	world.init();
	//system("pause");
	world.run();
	/*
	int a= menuMove("开始新游戏,载入进度,退出游戏");
	if (a== 0){
		mapMove(hero);
	}
	*/
}

