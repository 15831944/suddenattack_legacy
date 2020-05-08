/*
	InternetWrite�� Read���� ū �뷮 ó�� �κ��� �����ִ�.
	����� ���� Command ������ �ۼ����̶� ������ ������,
	��뷮 ó���� ���ؼ� ������ �صξ�� �Ѵ�.
*/

#pragma once

#include "..\..\DataStructure\MemoryPoolSafty.h"
#include "IQueryHandler.h"
#include <wininet.h>


namespace Query
{
	namespace HTTP
	{
		class CAsyncInet
		{
		public:
			CAsyncInet(void);
			~CAsyncInet(void);

		public:
			enum	InternetType
			{
				HTTP	= 0,
			};

		private:
			enum NextStep
			{
				HttpSendRequestEx,
				InternetWriteFile,
				HttpEndRequest,
			};

			struct RequestContext
			{
				BOOL		bUsed;
				int			nIndex;
				HINTERNET	hConnect;
				HINTERNET	hRequest;
				CAsyncInet*	pThis;
				ULONG_PTR		lpContext;
				BOOL		bEndRequest;

				NextStep	nextStep;
				LPCTSTR		lpszBody;

				HANDLE		hCreatedEvent;
			};

			struct TagHttpHandle
			{
				HINTERNET	hConnect;
				HINTERNET	hRequest;
			};

			struct TagHttpOpenRequest
			{
				HINTERNET	hConnect;
				LPCTSTR		lpszVerb;
				LPCTSTR		lpszObjectName;
				LPCTSTR		lpszVersion;
				LPCTSTR		lpszReferer;
				LPCTSTR*	lpszAcceptTypes;
				DWORD		dwFlags;
			};

			struct TagHttpSendRequestEx
			{
				HINTERNET			hRequest;
				LPINTERNET_BUFFERS	lpBuffersIn;
				LPINTERNET_BUFFERS	lpBuffersOut;
				DWORD				dwFlags;
			};


			Query::IQueryHandler*
				m_pHandler;

			HINTERNET		m_hOpen;

			Common::DataStructure::CMemoryPoolSafty<RequestContext>
				m_RequestContextPool;


			int				m_nCount;

		public:
			BOOL	Create	( int nInetCount, Query::IQueryHandler* pHandler );
			void	Destroy	( void	);

			BOOL	Connect	( ULONG_PTR& lpOutInetKey, InternetType type, LPCTSTR lpszServerName, ULONG_PTR lpContext, LPCTSTR lpszUserName = NULL, LPCTSTR lpszPassword = NULL, DWORD dwFlags= 0 );

			void	HttpRequestPost(
				ULONG_PTR	lpInetKey,
				LPCTSTR		lpszObjectName,
				LPCTSTR		lpszBody );

			void	HttpRequestGet(
				ULONG_PTR	lpContext,
				LPCTSTR		lpszURL );

			int		HttpResult( ULONG_PTR lpInetKey );

			DWORD	HttpResponse( ULONG_PTR lpInetKey, char* lpBuffersOut, DWORD dwBufferSize );

			void	Close( ULONG_PTR lpInetKey );


			BOOL	HttpOpenRequest(
				LPCTSTR		lpszVerb,
				LPCTSTR		lpszObjectName,
				LPCTSTR		lpszVersion		= NULL,
				LPCTSTR		lpszReferer		= NULL,
				LPCTSTR*	lpszAcceptTypes	= NULL,
				DWORD		dwFlags			= 0
				);

		private:
			void AsyncInternetWriteFile( RequestContext* pContext );
			void AsyncHttpEndRequest( RequestContext* pContext );

			static void __stdcall InetCallBack(HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
		};// class CAsyncInet

	}// namespace HTTP

}// namespace Query
