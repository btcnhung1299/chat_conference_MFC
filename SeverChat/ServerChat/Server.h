#pragma once

#include "resource.h"
#include "CommonLib.h"
#include <string>
#include <thread>

#define FILE_BUFFER_SIZE 102400 // The size of buffer for receiving file in bytes
#define UCACHE_LENGTH 125
#define SUBPORT_LENGTH 5
#define CHAT	L"[CHAT] "
#define RESOL	L"[RESOLVER] "
#define HANDLR	L"[HANDLER] "

#define PORT_STT_FREE 1
#define PORT_STT_BUSY 0

#define MESSAGE_NEW_USER	WM_USER + 1
#define MESSAGE_NEW_LOG	WM_USER + 2

#include "stdafx.h"
#include <vector>

using namespace std;

struct User {
	bool available;
	CString _address;
	UINT inboxPort;
	std::string nickname;
};

struct Group {
	std::string groupID;
	std::vector<string> userInGroup;
};

bool checkNickName(std::string nickname, User* users);

void InboxUsers(string senderName, CommonData &data, User * userCache);

void InboxGroup(CommonData data, Group &group, User *userCache);

bool IdInGroup(string id, Group group);

int findGroup(string groupID, vector<Group> &groupCache);

//void AssignNewUser(CSocket * tmpSockPtr, User * userCache, bool& end);

void connectResolver(User * userCache, std::string& toScreen, int * portsStatus, bool& end, bool& coutBlocked, CDialog& dlg);

void receiveListener(int portNumber, int& portStatus, User * userCache, vector<Group> &groupCache, bool& end, bool& coutBlocked, CDialog& dlg);

void terminateListen(bool * end);

void loadGroupCache(vector<Group> &group);

int checkUser(CDialog& dlg, string id, string pass);

void saveGroupCache(vector<Group> & groupCache);

void ReceiveFile(CDialog& dlg, CSocket& sock, CommonData& data, string senderName);

void SendFileUser(CSocket& sock, CommonData& data);