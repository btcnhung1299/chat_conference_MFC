// Project1.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "Server.h"
#include "CommonLib.h"
#include <afxsock.h>
#include <conio.h>
#include <Windows.h>
#include <fstream>
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LOGIN_SUC 0
#define LOGIN_PASS_WRONG 1
#define LOGIN_NOT_EXIST 2

// The one and only application object

//CWinApp theApp;

using namespace std;

void WriteUserChatLog(string& from, string& to, string& message) {
	string dir1, dir2;
	if (to >= from)
	{
		dir1 = from;
		dir2 = to;
	}
	else
	{
		dir1 = to;
		dir2 = from;
	}
	int curPartNo = 0, curLineLength = 1; 
	string dirPath = "userdb/" + dir1 + "/" + dir2 + "/";
	string fPath = dirPath + "info.txt";
	ifstream info(fPath,ios::in);
	ofstream out;
	if (info.is_open())
	{
		info >> curPartNo >> curLineLength;
	}
	else
	{
		string d1Path = "userdb/" + dir1;
		CreateDirectoryA(d1Path.data(), NULL);
		CreateDirectoryA(dirPath.data(), NULL);
	}
	fPath = dirPath + std::to_string(curPartNo)+".txt";

	out.open(fPath, ios::app);
	if (!out.is_open()) {
		out.open(fPath, ios::out);
	}

	out << from;
	out << " : ";
	out << message;
	out << endl;
	out.close();

	if (curLineLength < 20)
	{
		++curLineLength;
	}
	else
	{
		++curPartNo;
		curLineLength = 1;
	}
	fPath = dirPath + "info.txt";
	out.open(fPath, ios::out);
	out << curPartNo << endl;
	out << curLineLength << endl;
	out.close();
}
void WriteGroupChatLog(string& from, string groupID, string& message) {
	int curPartNo = 0, curLineLength = 1;
	string grPath = "groupdb/" + groupID + "/";
	string fPath = grPath + "info.txt";
	ifstream info(fPath, ios::in);
	ofstream out;
	if (info.is_open())
	{
		info >> curPartNo >> curLineLength;
	}
	else
	{
		CreateDirectoryA(grPath.data(), NULL);
	}

	fPath = grPath + std::to_string(curPartNo) + ".txt";
	out.open(fPath, ios::app);
	if (!out.is_open()) {
		out.open(fPath, ios::out);
	}

	out << from;
	out << " : ";
	out << message;
	out << endl;
	out.close();

	if (curLineLength < 20)
	{
		++curLineLength;
	}
	else
	{
		++curPartNo;
		curLineLength = 1;
	}
	fPath = grPath + "info.txt";
	out.open(fPath, ios::out);
	out << curPartNo << endl;
	out << curLineLength << endl;
	out.close();
}

void SendDataToUser(User * userCache, int index, CommonData& data) {
	if (userCache[index].available)
	{
		return;
	}
	CSocket sock;
	sock.Create();
	while (userCache[index].isBlocking)
	{
	}
	userCache[index].isBlocking = true;
	sock.Connect(userCache[index]._address, userCache[index].inboxPort);
	SendCommonData(sock, data);
	userCache[index].isBlocking = false;
	sock.Close();
}

void SendThreadDataToUser(User * userCache, int index, CommonData data) {
	thread th(SendDataToUser, userCache, index, ref(data));
	th.detach();
}

void Wait4Free(bool& isBlocked) {
	while (isBlocked)
	{

	}
	isBlocked = true;
}

void Wait4Free(DlgLogger& logger) {
	while (!(logger.dlg->IsWindowVisible()) || logger.isBlocked)
	{

	}
	logger.isBlocked = true;
}

bool checkNickName(string nickname, User* users) {
	for (int i = 0; i < 5; i++)
	{
		if (strcmp(nickname.data(), users[i].nickname.data()) == 0)
		{
			return false;
		}

	}
	return true;
}

void SendLog(DlgLogger& logger, CString& message, int type = MESSAGE_NEW_LOG) {
	Wait4Free(logger);
	LPARAM lpr = (LPARAM)(LPCTSTR)message;
	logger.dlg->SendMessage(type, NULL, lpr);
	logger.isBlocked = false;
}

void InboxUsers(DlgLogger& logger, string senderName, CommonData &data, User * userCache) {
	CSocket sender;
	bool status = false;
	string toName = data.to;

	//Log message to screen
	CString logLine;
	logLine += CHATP;
	logLine += data.from.c_str();
	logLine += L" -> ";
	logLine += data.to.c_str();
	logLine += L" : ";
	logLine += data.message.c_str();
	SendLog(logger, logLine);

	//Log message to file
	WriteUserChatLog(data.from, data.to, data.message);

	//Send Message to other user
	for (int i = 0; i < UCACHE_LENGTH; i++)
	{
		if (!userCache[i].available && (userCache[i].nickname == toName || userCache[i].nickname == senderName))
		{
			//Try to connect to user in user cache.
			sender.Create();
			while (userCache[i].isBlocking)
			{

			}
			userCache[i].isBlocking = true;
			status = sender.Connect(userCache[i]._address.GetString(), userCache[i].inboxPort);
			if (!(status))
			{
				// User is unavailable -> Free the slot.
				disconnectUser(logger, userCache, i);
				break;
			}
			data.from = senderName;
			//Send common data
			SendCommonData(sender, data);
			sender.Close();
			userCache[i].isBlocking = false;
		}
	}


}

void AssignNewUser(DlgLogger& logger, CSocket * tmpSockPtr, CommonData &data, bool& end) {
	int userNameLen = data.fileSize;
	string id = data.message.substr(0, userNameLen);
	string pass = data.message.substr(userNameLen);
	int freeSlot = -1;
	CommonData respond;

	int exists = checkUser(id, pass);

	if (exists != LOGIN_NOT_EXIST) {
		respond.type = "dup";
		SendCommonData(*tmpSockPtr, respond);
		return;
	}

	ofstream fo;
	string path = "userdb/userInfo.txt";
	fo.open(path.data(), ios::app);
	fo << id << endl;
	fo << pass << endl;
	fo.close();

	//Send log to LogDisplay
	CString logLine;
	logLine += HANDLR;
	logLine += id.c_str();
	logLine += " Registered";
	SendLog(logger, logLine);

	respond.type = "suc";
	SendCommonData(*tmpSockPtr, respond);
}
	
int checkUser(string id, string pass) {
	ifstream fi;
	string path = "userdb\\userInfo.txt";
	string IdInDB;
	string PassInDB;
	fi.open(path.data());

	bool existID = false;
	if (!fi.is_open())
		return -2;

	while (!fi.eof()) {
		getline(fi, IdInDB);
		getline(fi, PassInDB);

		if (IdInDB == id)
			existID = true;

		if (IdInDB == id && PassInDB == pass)
			return LOGIN_SUC;
	}
	
	if (existID)
		return LOGIN_PASS_WRONG;
	else
		return LOGIN_NOT_EXIST;
}

void LogIn(DlgLogger& logger, CSocket * tmpSockPtr, CommonData &data, User *userCache, bool& end) {
	int userNameLen = data.fileSize;
	string id = data.message.substr(0, userNameLen);
	string pass = data.message.substr(userNameLen);
	UINT tmp;
	CommonData respond;
	CString logLine;
	logLine += HANDLR;
	logLine += id.c_str();
	int status = checkUser(id, pass);
	if (status == LOGIN_SUC) {
		for (size_t i = 0; i < UCACHE_LENGTH; i++)
			if (userCache[i].available) {
				userCache[i].inboxPort = stoi(data.from);
				tmpSockPtr->GetPeerName(userCache[i]._address, tmp);
				userCache[i].nickname = id;
				userCache[i].available = false;
				userCache[i].isBlocking = false;

				respond.type = "lisuc";
				CommonData noti;
				noti.type = "newUserLogin";
				noti.message = id;
				//List of connected users
				for (int j = 0; j < UCACHE_LENGTH; j++)
				{
					if (!userCache[j].available && userCache[j].nickname != id)
					{
						SendDataToUser(userCache, j, noti);
						respond.message += userCache[j].nickname;
						respond.message += "\n";
					}
				}
				SendCommonData(*tmpSockPtr, respond);

				//Send log to LogDisplay
				logLine += " Logged in";
				SendLog(logger, logLine);
				logLine = id.c_str();
				SendLog(logger, logLine, MESSAGE_NEW_USER);

				return;
			}
	}
	else {
		respond.type = "fail";
		SendCommonData(*tmpSockPtr, respond);
		logLine += " Attempt failed";
		SendLog(logger, logLine);
	}
}

void connectResolver(User * userCache, string& toScreen, int * portsStatus, bool& end, DlgLogger& logger) {
	std::this_thread::sleep_for(20ms);
	CSocket mainServer, tempSlot;
	CSocket * tmpSockPtr;
	mainServer.Create(1234);
	CString tmpAddr;
	UINT tmpPort;

	mainServer.GetSockNameEx(tmpAddr, tmpPort);
	CT2CA pszConvertedAnsiString(tmpAddr);
	string ipAddr(pszConvertedAnsiString);
	int stt = 0, errCode = 0;
	CSocket tmpSock;

	CString cString =  RESOL;
	cString += " Started at port 1234";
	SendLog(logger, cString);

	while (!end)
	{
		mainServer.Listen();
		//tmpSockPtr = new CSocket();
		mainServer.Accept(tmpSock);
		
		errCode = mainServer.GetLastError();
		if (errCode > 0)
		{
			//cout << RESOL << " Errcode: " << errCode << endl;
			continue;
		}		
		

		UINT returnPort = 1234;
		for (int i = 0; i < SUBPORT_LENGTH; i++)
		{
			if (portsStatus[i] == PORT_STT_FREE)
			{
				returnPort += i + 1;
				tmpSock.Send(&returnPort, 4, 0);
				break;
			}
		}
		tempSlot.Close();
		tmpSock.Close();
		//delete tmpSockPtr;
	}
}

void receiveListener(int portNumber, int& portStatus, User * userCache, vector<Group> &groupCache, bool& end, DlgLogger& logger) {
	std::this_thread::sleep_for(100ms);
	CSocket subServer, sock;
	subServer.Create(portNumber);
	int messageLen = 0;
	string message;
	char buffer[255];
	CString clientAddress;
	UINT clientPort;
	CommonData data;
	
	CString cString = HANDLR;
	cString += " Started at port: ";
	cString += std::to_string(portNumber).data();
	SendLog(logger, cString);

	int stt = 0;
	char ch = ' ';
	while (!end)
	{
		stt = subServer.Listen();
		subServer.Accept(sock);
		portStatus = PORT_STT_BUSY;
		sock.GetPeerName(clientAddress, clientPort);
	
		ReceiveCommonData(sock, data);

		//Get userName
		string senderName = data.from;
		message = data.message;

		if (data.type == "re")
		{
			AssignNewUser(logger, &sock, data, end);
		}
		else if (data.type == "li") { //Log in
			LogIn(logger, &sock, data, userCache, end);
		}
		else if (data.type == "cg") { //create new Group
			createGroup(logger, data, groupCache, senderName);
			saveGroupCache(groupCache);

			//Phan hoi ve cho tat ca cac client trong group de tao group chat
			Group group = groupCache.back();
			CommonData respond;
			respond.type = "cg";
			respond.message = group.groupID;
			SendCommonData(sock, respond);
			InboxGroup(logger, respond, group, userCache);
		}
		else if (data.type == "mu") //chat User -> User
		{
			SendCommonData(sock, data);
			InboxUsers(logger, senderName, data, userCache);
		}
		else if (data.type == "mg") {
			string groupID = data.to;
			int index = findGroup(groupID, groupCache);
			if (index == -1)
				return;
			Group group = groupCache[index];
			SendCommonData(sock, data);
			InboxGroup(logger, data, group, userCache);
		}
		// File sending
		else if (data.type == "fu")
		{
			//Recieve metadata of the file to transfer
			string fileName = data.message;
			SendCommonData(sock, data);
			ReceiveFile(logger, sock, data, senderName);
			
			//Send metadata of the file to user
			CommonData respond;
			respond.type = "fu"; 
			respond.from = senderName;
			respond.to = data.to;
			respond.message = fileName;
			InboxUsers(logger, senderName, respond, userCache);
		}
		else if (data.type == "fg") { //send file to group
			string fileName = data.message;
			SendCommonData(sock, data);
			ReceiveFile(logger, sock, data, senderName);

			//Create metadata to send to group chat
			CommonData respond;
			respond.type = "fg";
			respond.from = senderName;
			respond.to = data.to;
			respond.message = fileName;

			//find group
			string groupID = data.to;
			int index = findGroup(groupID, groupCache);
			if (index == -1)
				return;

			Group group = groupCache[index];

			InboxGroup(logger, respond, group, userCache);
		}
		else if (data.type == "gf") { //get a file from chatbox
			SendCommonData(sock, data);
			SendFileUser(logger, sock, data);
		}
		else if (data.type == "isConnecting") {
			CommonData rep;
			rep.type = "no";
			for (int i = 0; i < UCACHE_LENGTH; i++)
			{
				if (!userCache[i].available && userCache[i].nickname == senderName)
				{
					rep.type = "yes";
				}
			}
			SendCommonData(sock, rep);
		}
		else if (data.type == "forceKick") {
			disconnectUser(logger, userCache, data.message);
		}
		sock.Close();
		portStatus = PORT_STT_FREE;
	}
}


void terminateListen(bool * end) {
	char ch = ' ';
	ch = _getch();
	while (ch != '\r')
	{
		ch = _getch();
	}
	//cout << "END" << endl;
	*end = true;    
	return;
}

bool IdInGroup(string id, Group group) {
	for (int i = 0; i < group.userInGroup.size(); i++) {
		if (id == group.userInGroup[i]) {
			return true;
		}
	}
	return false;
}

void InboxGroup(DlgLogger& logger, CommonData data, Group &group, User *userCache) {
	CSocket sender;
	bool status = false;
	//Log to screen
	CString logLine;
	logLine += CHATG;
	logLine += data.from.c_str();
	logLine += L" -> ";
	logLine += data.to.c_str();
	logLine += L" : ";
	logLine += data.message.c_str();
	SendLog(logger, logLine);

	//Log to file
	WriteGroupChatLog(data.from, group.groupID, data.message);

	//Send to group users
	for (int i = 0; i < UCACHE_LENGTH; i++) {
		if (!userCache[i].available && IdInGroup(userCache[i].nickname, group)) {
			sender.Create();
			 while (userCache[i].isBlocking)
			 {

			 }
			userCache[i].isBlocking = true;
			status = sender.Connect(userCache[i]._address.GetString(), userCache[i].inboxPort);

			if (!status) {
				disconnectUser(logger, userCache, i);
				continue;
			}

			
			SendCommonData(sender, data);

			sender.Close();
			userCache[i].isBlocking = false;
		}
	}
}

void loadGroupCache(vector<Group> &group) {
	ifstream fi;
	fi.open("groupdb/groups.txt");

	if (!fi.is_open())
		return;

	while (!fi.eof()) {
		Group tmp;
		int n;
		char c;
		fi >> tmp.groupID;
		if (tmp.groupID == "")
			break;
		fi >> n;
		for (int i = 0; i < n; i++) {
			string user;
			getline(fi, user);
			if (user == "") {
				i--;
				continue;
			}
			tmp.userInGroup.push_back(user);
		}
		group.push_back(tmp);
	}
	fi.close();
}

int findGroup(string groupID, vector<Group> &groupCache) {
	for (int i = 0; i < groupCache.size(); i++) {
		if (groupID == groupCache[i].groupID)
			return i;
	}
	return -1;
}

void createGroup(DlgLogger &logger, CommonData data, vector<Group> &groupCache, string senderName) {
	Group grp;
	grp.groupID = std::to_string(groupCache.size());
	string allUsers = data.message;
	vector<string> userNames;

	userNames.push_back(senderName);

	int start = 0;
	int end = 0;
	for (int i = 0; i < allUsers.length(); i++) {
		if (allUsers[i] == '\n') {
			end = i;
			string sub = allUsers.substr(start, end - start);
			start = end + 1;
			userNames.push_back(sub);
		}
	}
	grp.userInGroup = userNames;

	CString logLine;
	logLine += HANDLR;
	logLine += data.from.c_str();
	logLine += L" Create group ";
	logLine += grp.groupID.c_str();
	SendLog(logger, logLine);

	groupCache.push_back(grp);
}

void saveGroupCache(vector<Group> & groupCache) {

	ofstream fo;
	fo.open("groupdb/groups.txt");

	for (int i = 0; i < groupCache.size(); i++)
	{
		fo << groupCache[i].groupID << endl;
		fo << groupCache[i].userInGroup.size() << endl;
		for (int j = 0; j < groupCache[i].userInGroup.size(); j++)
		{
			fo << groupCache[i].userInGroup[j] << endl;
		}
	}

	fo.close();
}

void ReceiveFile(DlgLogger& logger, CSocket& sock, CommonData& data, string senderName){
	
	ofstream fo;
	string fPath = "filedb/" + senderName + "_" + data.message;
	fo.open(fPath.data(), ios::binary);
	int toWriteSize = 0;
	int temp = 0;

	CString logLine;
	logLine += FILHAN;
	logLine += data.from.c_str();
	logLine += L" Send a File ";
	logLine += data.message.c_str();
	logLine += L" to ";
	logLine += data.to.c_str();
	SendLog(logger, logLine);

	//Receive real file
	char Buffer[8];
	do
	{
		sock.Receive(&temp, sizeof(long long), 0);
		toWriteSize = temp;
		if (toWriteSize <= 0)
			break;
		sock.Receive(&Buffer, toWriteSize, 0);
		fo.write(Buffer, toWriteSize);
		temp = 0;
	} while (toWriteSize > 0 && toWriteSize <= sizeof(long long));

	int check = 1;
	sock.Send(&check, sizeof(int), 0); //Thong bao ket thuc nhan file cho Client

	fo.close();
}

void SendFileUser(DlgLogger& logger, CSocket& sock, CommonData& data) {
	string fPath = "filedb/" + data.from + "_" + data.message;
	long long Buffer;

	CString logLine;
	logLine += FILHAN;
	logLine += data.to.c_str();
	logLine += L" Request a File ";
	logLine += data.message.c_str();
	logLine += L" Sent from ";
	logLine += data.from.c_str();
	SendLog(logger, logLine);

	FILE *fp = fopen(fPath.data(), "rb");

	if (fp == NULL) 
		return;

	int byteReaded = 0;

	do {
		byteReaded = fread(&Buffer, 1, sizeof(long long), fp);
		sock.Send(&byteReaded, sizeof(long long), 0);
		sock.Send(&Buffer, byteReaded);
	} while (byteReaded == sizeof(long long));

	char zero = 0;
	sock.Send(&zero, sizeof(char), 0); //Signal of End of file to client;
	fclose(fp);

	// Waiting until server received file successfully
	int fullReceive = 0;
	do {
		sock.Receive(&fullReceive, sizeof(int), 0);
	} while (fullReceive != 1);
}


void userConnectionChecker(User* userCache, bool& end, DlgLogger& logger) {
	std::this_thread::sleep_for(150ms);
	CSocket checker;

	CString logLine;
	logLine = CHECKR;
	logLine += L"Connection Checker Started.";
	SendLog(logger, logLine);

	while (!end)
	{
		bool changed = false;
		std::this_thread::sleep_for(100ms);

		//CommonData data;
		//data.type = "updateUsers";

		for (int i = 0; i < UCACHE_LENGTH; i++)
		{
			if (userCache[i].available || userCache[i].isBlocking)
				continue;

			checker.Create();
			int status = checker.Connect(userCache[i]._address, userCache[i].inboxPort);
			if (!status)
			{
				int err = checker.GetLastError();
				logLine = CHECKR;
				logLine += L"ErrCode: ";
				logLine += to_string(err).c_str();
				logLine += L" - ";
				logLine += to_string(userCache[i].inboxPort).c_str();
				SendLog(logger,logLine);
				disconnectUser(logger, userCache, i);
				checker.Close();
				continue;
			}
			checker.Close();
		}
	}
}

void disconnectUser(DlgLogger& logger, User* userCache, int index) {
	userCache[index].available = true;
	CString logLine;
	logLine = userCache[index].nickname.c_str();

	SendLog(logger, logLine, MESSAGE_REMOVE_USER);

	logLine = HANDLR;
	logLine += userCache[index].nickname.c_str();
	logLine += L" Disconnected.";
	SendLog(logger, logLine);
	CommonData data;
	data.type = "dis";
	data.message = userCache[index].nickname;
	for (int i = 0; i < UCACHE_LENGTH; i++)
	{
		if (userCache[i].available || i == index)
			continue;
		SendDataToUser(userCache, i, data);
	}
	SendLog(logger, logLine);
}


void disconnectUser(DlgLogger& logger, User* userCache, string UName) {
	int index = -1;
	for (int i = 0; i < UCACHE_LENGTH; i++)
	{
		if (userCache[i].nickname == UName)
		{
			index = i;
			break;
		}
	}
	if (index == -1)
	{
		return;
	}
	disconnectUser(logger, userCache, index);
}
