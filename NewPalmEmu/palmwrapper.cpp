#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <thread>
#include <chrono>
#include <random>
#include <mutex>

#include "palmwrapper.h"

#include "prcfile.h"
#include "palmapi.h"
#include "displaydriver.h"
#include "datamanager.h"
#include "appselector.h"
#include "launchroutines.h"
#include "minifunc.h"

#include "eventqueue.h"


//new list
#include "m68k.h"
#include "newcpu.h"


//Universal data storage/heap/osvalues
vector<palmdb> apps;

//OS data
void (*TouchDriver)(int,int,bool) = nullptr;
void (*KeyDriver)(char,bool) = nullptr;
void (*ButtonDriver)(int,bool) = nullptr;

int curapp;//open app
int curoverlay;//open apps overlay(language file)
CPTR appcall;//fake data saying how and why the app was launched
string username;
string clipboard;
ULONG totalticks;
ULONG keymask;
chrono::high_resolution_clock::time_point starttime;
bool truerandom = false;//hack

//events
CPTR appexceptionlist;

//ui
CPTR oslcdwindow,lcdbitmaptype;

//random numbers (not exposed to other files)
random_device seedstore;
mt19937 gentype(seedstore());
uniform_int_distribution<uint32> getrandom(0,UINT32_MAX);

//i/o thread safety
//mutex getlcd_audio,getbutton_touch;
mutex os_data_lock;

//debug
string lasttrap;
//End of universal data


uint32 randomnumber(){
	return getrandom(gentype);
}

string directory;
shared_img palm;
thread palmcpu;
bool running = false,started = false;
bool hasbootableapp = false;

//audio stream buffer
UWORD conversionaudiobuffer[100];

//uses bigest memory setting (16 bpp & 320*480)
UWORD conversionbuffer[LCDMAXBYTES];


UWORD* formattedaudiobuffer = conversionaudiobuffer;
UWORD* formattedgfxbuffer = conversionbuffer;

void updatescreendata(){
	if(!running)return;

	offset_68k i;
	size_t_68k total = LCDW * LCDH;
	UWORD* start = get_real_address(lcd_start);
	inc_for(i,total){
		formattedgfxbuffer[i] = start[i];
	}
}

UWORD* validdisplaydata(){
	updatescreendata();
	return formattedgfxbuffer;
}

void palmabrt(){
	if(!lasttrap.empty())dbgprintf(lasttrap.c_str());
	abort();
}

void backupram(){
	//need to do this!!
}

bool start(int bootfile){
	if(running)return false;

	if(!hasbootableapp)return false;
	if(apps.size() < bootfile || !apps[bootfile].exe)return false;
	bool launched = launchapp(bootfile);
	if(!launched)return false;

	palmcpu = thread(CPU,&palm);
    CPU_start(&palm);

	//input drivers
	TouchDriver = &appTouchDriver;
	KeyDriver = &appKeyDriver;
	ButtonDriver = nullptr;//hack

	running = true;
	started = true;
	return true;
}

bool resume(){
	if(!started)return false;

	BTNTBL = 0x00000000;
	PENX = 0x0000;
	PENY = 0x0000;
	starttime = chrono::high_resolution_clock::now();
    CPU_start(&palm);
	running = true;
	return true;
}

bool halt(){
	if(!running)return false;

	totalticks += (chrono::high_resolution_clock::now() - starttime) / palmTicks(1);
    CPU_stop(&palm);
	running = false;
	return true;
}

bool end(){
	if(started){
		started = false;
		running = false;
		palm.CpuReq = cpuExit;
		palmcpu.join();
		if(!palm.ForceExit)backupram();

		full_deinit();//clean up

		return true;
	}
	return false;
}



UWORD buttontovchr[7] = {520,0,0,516,517,518,519};

void sendbutton(int button, bool state){
	if(!running)return;

	if(state){
		BTNTBL |= bit(button);
		if(buttontovchr[button] && (bit(button) & ~keymask)){
			osevent keypress;
			keypress.type = keyDownEvent;
			keypress.data.push_back(0);//chr
			keypress.data.push_back(buttontovchr[button]);//virtual key num
			keypress.data.push_back(8);//modifiers
			addnewevent(keypress);
		}
	}
	else{
		BTNTBL &= ~bit(button);
	}
}

void sendtouch(int x, int y, bool pressed){
	if(TouchDriver)TouchDriver(x,y,pressed);
}

void sendkeyboardchar(char thiskey,bool state){
	if(KeyDriver)KeyDriver(thiskey,state);
}

string getclipboard(){
	string state;
	os_data_lock.lock();
	state = clipboard;
	os_data_lock.unlock();
	return state;
}

void setclipboard(string data){
	os_data_lock.lock();
	clipboard = data;
	os_data_lock.unlock();
}

void uniprint(string output,...){
	//output.
}

