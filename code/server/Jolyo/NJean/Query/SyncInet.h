/*
	InternetWrite�� Read���� ū �뷮 ó�� �κ��� �����ִ�.
	����� ���� Command ������ �ۼ����̶� ������ ������,
	��뷮 ó���� ���ؼ� ������ �صξ�� �Ѵ�.
*/

#pragma once

#include "IQueryHandler.h"
#include <wininet.h>

namespace Query
{
	namespace HTTP
	{
		class CSyncInet
		{
		public:
			CSyncInet(void);
			~CSyncInet(void);

		public:

		private:
			HINTERNET		m_hOpen;
			HINTERNET		m_hRequest;

			int				m_nCount;

		public:
			bool		Create	( void );
			void		Destroy	( void );

			BOOL		Request	( LPCTSTR lpszQuery, DWORD &dwError );	// Get���
			BOOL		Response( ULONG_PTR lpInetKey, CHAR* lpBuffersOut, DWORD dwBufferSize, DWORD &dwNumberOfBytesRead );
			INT			HttpResult( ULONG_PTR lpInetKey );

			bool		Connect	(
				LPCTSTR lpszServerName,			// ������ ���� �ּ� ex> 61.106.140.203
				LPCTSTR lpszObjectName,			// ���� ��� ex> /scripts/test.asp
				LPCTSTR lpszBody,
				unsigned short port,		// ���� ��Ʈ
				LPCTSTR lpszUserName, 
				LPCTSTR lpszPassword, 
				DWORD dwFlags,
				char* lpBuffersOut, 
				DWORD dwBufferSize,
				DWORD * dwReturnSize
				);

		};// class CSyncInet

	}// namespace HTTP

}// namespace Query
