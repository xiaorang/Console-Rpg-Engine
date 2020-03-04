#include"audio_clip.h"
#include<iostream>
//#include<random>
#include<time.h>
#include<string>

Mci::Mci()
{
	HINSTANCE hins = LoadLibraryA("winmm.dll");
	wmci=(w32mci)GetProcAddress(hins, "mciSendStringA");
	wmcierror = (w32mcierror)GetProcAddress(hins, "mciGetErrorStringA");	
}
Mci::~Mci()
{
	FreeLibrary(hins);
}
bool Mci::send(std::string command)
{
	int errcode = wmci(command.c_str(), buf, 254, 0);
	if (errcode)
	{
		wmcierror(errcode, buf, 254);
		return false;
	}
	return true;
}
AudioClip::AudioClip()
{
	//do nothing
}
AudioClip::~AudioClip()
{
	std::string cmd;
	cmd = "close " + alias;
	mci.send(cmd);
}
bool AudioClip::load(const std::string& _filename)
{
	filename = _filename;
	for (unsigned int i = 0; i < filename.length(); i++)
	{
		if (filename[i] == '/')
			filename[i] = '\\';
	}
	alias = "mp3_";
	srand(time(NULL));
	char randstr[6];
	_itoa(rand() % 65536, randstr, 10);
	alias.append(randstr);
	std::string cmd;
	cmd = "open " + filename + " alias " + alias;
	if (mci.send(cmd) == false)
		return false;
	cmd = "set " + alias + " time format milliseconds";
	if (mci.send(cmd) == false)
		return false;
	cmd = "status " + alias + " length";
	if (mci.send(cmd) == false)
		return false;
	length_ms = atoi(mci.buf);
	return true;
}
bool AudioClip::play(int start_ms , int end_ms)
{
	if (end_ms == -1) end_ms = length_ms;
	std::string cmd;
	char start_str[16], end_str[16];
	_itoa(start_ms, start_str,10);
	_itoa(end_ms, end_str, 10);
	cmd = "play " + alias+" from ";
	cmd.append(start_str);
	cmd.append(" to ");
	cmd.append(end_str);
	return mci.send(cmd);
}
bool AudioClip::stop()
{
	std::string cmd;
	cmd = "stop "+alias;
	if (mci.send(cmd) == false)
		return false;
	cmd = "seek " + alias + " to start";
	if (mci.send(cmd) == false)
		return false;
	return true;
}
bool AudioClip::pause()
{
	std::string cmd;
	cmd = "pause " + alias;
	if (mci.send(cmd) == false)
		return false;
	return true;
}
bool AudioClip::unpause()
{
	std::string cmd;
	cmd = "resume " + alias;
	if (mci.send(cmd) == false)
		return false;
	return true;
}
int AudioClip::milliseconds()
{
	return length_ms;
}

