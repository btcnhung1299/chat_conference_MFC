#pragma once

#include "resource.h"
#include "CommonLib.h"
#include <string>
#include <thread>

#define FILE_BUFFER_SIZE 102400 // The size of buffer for receiving file in bytes
#define UCACHE_LENGTH 125
#define SUBPORT_LENGTH 5
#define FILHAN	L"[FILE] "
#define CHATG	L"[CHAT GROUP] "
#define CHATP	L"[CHAT PRIVATE] "
#define RESOL	L"[RESOLVER] "
#define HANDLR	L"[HANDLER] "
#define CHECKR	L"[CHECKER] "

#define PORT_STT_FREE 1
#define PORT_STT_BUSY 0

#define MESSAGE_NEW_USER	WM_USER + 1
#define MESSAGE_REMOVE_USER	WM_USER + 2
#define MESSAGE_NEW_LOG	WM_USER + 3

#include "stdafx.h"
#include <vector>

using namespace std;

struct User {
	bool available;
	bool isBlocking; //Flag to check if user is receiving message and checker should ignore
	CString _address;
	UINT inboxPort;
	std::string nickname;
};

struct Group {
	std::string groupID;
	std::vector<string> userInGroup;
};

struct DlgLogger {
	CDialog* dlg;
	bool isBlocked;
};

bool checkNickName(std::string nickname, User* users);

void InboxUsers(DlgLogger& dlg, string senderName, CommonData &data, User * userCache);

void InboxGroup(DlgLogger& dlg, CommonData data, Group &group, User *userCache);

bool IdInGroup(string id, Group group);

int findGroup(string groupID, vector<Group> &groupCache);

//void AssignNewUser(CSocket * tmpSockPtr, User * userCache, bool& end);

void connectResolver(User * userCache, std::string& toScreen, int * portsStatus, bool& end, DlgLogger& dlg);

void receiveListener(int portNumber, int& portStatus, User * userCache, vector<Group> &groupCache, bool& end, DlgLogger& dlg);

void terminateListen(bool * end);

void loadGroupCache(vector<Group> &group);

int checkUser(string id, string pass);

void saveGroupCache(vector<Group> & groupCache);

void ReceiveFile(DlgLogger& dlg, CSocket& sock, CommonData& data, string senderName);

void SendFileUser(DlgLogger& logger, CSocket& sock, CommonData& data);

void createGroup(DlgLogger &dlg, CommonData data, vector<Group> &groupCache, string senderName);

void userConnectionChecker(User* userCache, bool& end, DlgLogger& logger);

void disconnectUser(DlgLogger& dlg, User* userCache, int index);
void disconnectUser(DlgLogger& logger, User* userCache, string UName);