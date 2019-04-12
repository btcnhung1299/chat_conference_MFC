#pragma once

#include "CServerSettings.h"
#include "CommonLib.h"
#include "CLogin.h"
#include "CNoti.h"
#include <utility>
#include <afxsock.h>

class CClientChatDoc : public CDocument {
	private:
		CSocket clntSock, listenerConv, listenerUser, receiverConv, receiverUser;

		UINT serverPort;
		CString serverIP;
		UINT contactPort;
		UINT myPort;

		CString username;
		CString password;

	protected:
		CClientChatDoc() noexcept;
		DECLARE_DYNCREATE(CClientChatDoc)
		virtual ~CClientChatDoc();

	public:
		void InitListenerConv();
		void InitListenerUser();

	public:
		virtual BOOL OnNewDocument();
		virtual void Serialize(CArchive& ar);
	#ifdef SHARED_HANDLERS
		virtual void InitializeSearchContent();
		virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
	#endif // SHARED_HANDLERS

	public:
		CString GetUsername() { return username; }
		BOOL Send(CommonData&, CommonData&);
		void ReceiveConv(CommonData& receiveData);
		void ReceiveUser();

	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif

	protected:

	// Generated message map functions
	protected:
		DECLARE_MESSAGE_MAP()



	#ifdef SHARED_HANDLERS
		// Helper function that sets search content for a Search Handler
		void SetSearchContent(const CString& value);
	#endif // SHARED_HANDLERS
};

