/*****************************************************************************************************************
	CTcpAsyncSelect

	�ۼ��� : ������
	�ۼ��� : 2004�� 2�� 25~27��

	��  �� :
		IOCP�� �̿��� Server
	��  �� :
		1. Create�� ����Ͽ� Iocp Server�� �����Ѵ�.
		2. Bind�� Listen�� �Ѵ�. �� ���� �⺻ ���� ���α׷��� �����ϴ�.
		3. Start�� ���ؼ� Iocp Server�� �����Ų��.
		4. Iocp Server�� ���� �ڵ�κ��� OnAccept, OnReceive, OnSend, OnClose �̺�Ʈ�� �߻���Ų��.
		5. Send�� ���ؼ� Iocp Server�� ����Ǿ� �ִ� Ŭ���̾�Ʈ���� �޽����� �����Ѵ�.
		6. CloseSocket�� ���ؼ� Iocp Server�� ����Ǿ� �ִ� Ư�� ������ �ݴ´�.
		7. ��� ����� �ڿ��� Destroy�� ���� ���� ���ҽ��� ���Ÿ� �Ѵ�.

	��  �� : 2004�� 3�� 4��
	��  �� : ����� ������ ������ ����
		1. Accept, Recv, Send, Close �̺�Ʈ �߻��� Child Class���� ó�� �� �� �ֵ��� virtual function ����
		2. Overlapped Object ��ü�� �Ҵ� �ް� ������ ���� �̺�Ʈ�� ó�� �� �� �ֵ��� virtual function ����
*****************************************************************************************************************/

#pragma once

//#include <Winsock2.h>
#include <map>

/*
#include "OverlappedListenSocket.h"
#include "OverlappedCommSocket.h"
#include "..\IO\AsyncIocp.h"
*/
#include "..\..\DataStructure\MemoryPoolSafty.h"

#include "Network.h"
#include "CommSocket.h"

namespace Network
{
	namespace TcpAsyncSelect
	{
		class CTcpAsyncSelect : public INetwork//, IO::IOCP::IIocpHandler
		{
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// ����/�Ҹ���
		public:
			CTcpAsyncSelect( HWND hWindow, UINT uiMessage );
			~CTcpAsyncSelect(void);

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Attributes
		private:
			typedef std::pair<SOCKET, CCommSocket*>					SocketKey_Pair;
			struct TagSocketMap
			{
				std::map <SOCKET, CCommSocket*>						mapSocketKey;
				std::map <SOCKET, CCommSocket*>::const_iterator		IterSocketKey;
			};

			struct TagListener
			{
				SOCKET						sListenSocket;
				INT							nServicerIndex;
				Common::DataStructure::CMemoryPoolSafty<SOCKET>
											CommSocketPool;

			};
			HWND						m_hWindow;
			UINT						m_uiMessage;

			Common::DataStructure::CMemoryPoolSafty<CCommSocket>
										m_ConnectSocketPool;

			TagListener*				m_aListener;

			ITcpNetworkHandler*			m_pNetworkHandler;				// Network handler
			INT							m_nCreatedLister;

			TagSocketMap				m_mapSocket;

		protected:

			// ��� ��Ŷ���� ���� ū ũ��� ������Ų��.
			INT			m_nRecvBuffSize;			// Recv Buffer Size
			INT			m_nSendBuffSize;			// Send Buffer Size
			INT			m_nNumOfConnectSockets;		// TcpAsyncSelect�� ������ �� �ִ� ������ ��
			INT			m_nNumOfListenSockets;		// TcpAsyncSelect�� ������ �� �ִ� Port�� ��

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Operations
		public:
			// �ٸ� ������ ������ �Ѵ�.
			virtual BOOL		Connect( ULONG_PTR lpOwnerKey, const sockaddr* addr, INT namelen );



			/////////////////////////////////////////////
			// Create Bind ���� ������ ������ �����Ѵ�.
			// n���� Accept�� ���ؼ�


			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Overloadable
		public:
			// ���ҽ����� �����Ѵ�.
			//		nNumOfCommSockets : Iocp Server�� ������ų �� �ִ� ������ ����
			//		nRecvBuffSize : Iocp Server�� Ŭ���̾�Ʈ�� ����� �ϴ� �� ��ü�� Recv Buffer�� ũ��
			//		nSendBuffSize : Iocp Server�� Ŭ���̾�Ʈ�� ����� �ϴ� �� ��ü�� Send Buffer�� ũ��
			//		nAcceptPoolSize : Accept�� ���� ������ �ϱ� ���ؼ� �̸� ����� Accept�� Pool�� ũ��
			//		bAcceptRecvEvent : Accept �̺�Ʈ�� ������ ����, True�̸� Accept�̺�Ʈ�� ���� �ʰ�, ������ Recv �߻��� AcceptRecv �̺�Ʈ�� �޴´�.
			virtual BOOL	Create( ITcpNetworkHandler* pNetworkHandler, INT nNumOfListener, INT nNumOfConnectSockets, INT nRecvBuffSize, INT nSendBuffSize/*, UINT nAcceptPoolSize, BOOL bAcceptRecvEvent = FALSE*/ );

			// ���ҽ����� �����Ѵ�.
			//		nNumOfCommSockets : Iocp Server�� ������ų �� �ִ� ������ ����
			//		nRecvBuffSize : Iocp Server�� Ŭ���̾�Ʈ�� ����� �ϴ� �� ��ü�� Recv Buffer�� ũ��
			//		nSendBuffSize : Iocp Server�� Ŭ���̾�Ʈ�� ����� �ϴ� �� ��ü�� Send Buffer�� ũ��
			//		nAcceptPoolSize : Accept�� ���� ������ �ϱ� ���ؼ� �̸� ����� Accept�� Pool�� ũ��
			//		bAcceptRecvEvent : Accept �̺�Ʈ�� ������ ����, True�̸� Accept�̺�Ʈ�� ���� �ʰ�, ������ Recv �߻��� AcceptRecv �̺�Ʈ�� �޴´�.
			virtual BOOL	CreateListener( INT nServerIndex, INT nNumOfCommSockets, INT nAcceptPoolSize, BOOL bAcceptRecvEvent = FALSE );

			// ���ҽ����� �����Ѵ�.
			virtual void	Destroy();

			virtual void	FirePacket( DWORD wParam, DWORD lParam );


			virtual BOOL	Bind( INT nServerIndex, const sockaddr* addr, INT namelen, INT nListenQueue = 5 );
			virtual BOOL	Bind( INT nServerIndex, INT nPort, u_long hostAddr = INADDR_ANY, short family = AF_INET, INT nListenQueue = 5 );

			virtual BOOL	StartListen( INT nServerIndex );






			// �ش� Ű�� �ش��ϴ� context�� send buffer�� ref�� ��ȯ�Ѵ�.
			//		In	: lpNetworkKey - IOCP�� ����� Completion Key
			//		Ret	: Send Buffer PoINTer
			//virtual char*		GetSendBufferRef( ULONG_PTR lpNetworkKey );

			// sendBuff�� �� ���� ���۷� ī�Ǹ� �Ͽ� �����Ѵ�.
			//		In	: lpNetworkKey - IOCP�� ����� Completion Key
			//			: sendBuff - ������ �޽��� ����
			//			: nLen - ������ �޽����� ����
			//		Out	: nOutError - ���� ���н� �����ڵ�
			//		Ret : ���� ���� ����
			virtual BOOL		Send( ULONG_PTR lpNetworkKey, const char* sendBuff, INT nLen, INT& nOutError );

			// �ش� Ű�� ���� ��ü�� �ݴ´�
			//		In	: lpNetworkKey - IOCP�� ����� Completion Key
			virtual void		CloseSocket( ULONG_PTR lpNetworkKey );

			// �ش�Ű�� ���� ��ü�� Shutdown�Ѵ�.
			//		In	: lpNetworkKey - IOCP�� ����� Completion Key
			virtual void		ShutDown( ULONG_PTR lpNetworkKey );

			virtual void	SetConnectRecvEvent			( ULONG_PTR lpCompletionKey );
			virtual void	ResetConnectRecvEvent		( ULONG_PTR lpCompletionKey );
			virtual void	WaitConnectRecvEvent		( ULONG_PTR lpCompletionKey );
			virtual void	SetRecvDisconnectEvent		( ULONG_PTR lpCompletionKey );
			virtual void	ResetRecvDisconnectEvent	( ULONG_PTR lpCompletionKey );
			virtual void	WaitRecvDisconnectEvent		( ULONG_PTR lpCompletionKey );

			virtual void	ResetRttTime				( ULONG_PTR lpNetworkKey );
			virtual BOOL	CanStartRTT					( ULONG_PTR lpNetworkKey );
			virtual void	SetRttStartTime				( ULONG_PTR lpNetworkKey );
			virtual void	SetRttEndTime				( ULONG_PTR lpNetworkKey );
			virtual void	GetRttTime					( ULONG_PTR lpNetworkKey, DWORD& out_dwRttTime );

			// �ش� Ű�� Servicer �ε����� �����´�.
			virtual INT			GetServicerIndex( ULONG_PTR lpNetworkKey )
			{
				CCommSocket* pCommSocket;
				pCommSocket = (CCommSocket*)lpNetworkKey;

				if( pCommSocket->m_pParentSocket )
				{
					return pCommSocket->m_pParentSocket->GetServiceIndex();
				}
				else
				{
					return pCommSocket->GetServiceIndex();
				}
			}
			virtual void		SetServicerIndex( ULONG_PTR lpNetworkKey, INT nServicerIndex )
			{
				CCommSocket* pCommSocket;
				pCommSocket = (CCommSocket*)lpNetworkKey;

				pCommSocket->SetServiceIndex( nServicerIndex );
			}

			virtual ULONG_PTR GetOwnerKey( ULONG_PTR lpNetworkKey )
			{
				CCommSocket* pSocket = (CCommSocket*)lpNetworkKey;
				return pSocket->GetOwnerKey();
			}

			virtual void SetOwnerKey( ULONG_PTR lpNetworkKey, ULONG_PTR lpOwnerKey )
			{
				CCommSocket* pSocket = (CCommSocket*)lpNetworkKey;
				pSocket->SetOwnerKey( lpOwnerKey );
			}

			virtual void GetAssemblerKey( ULONG_PTR lpNetworkKey, ULONG_PTR& lpAssemblerKey, DWORD& dwAssemblerHandle )
			{
				CCommSocket* pSocket = (CCommSocket*)lpNetworkKey;
				pSocket->GetAssemblerKey( lpAssemblerKey, dwAssemblerHandle );
			}

			virtual void SetAssemblerKey( ULONG_PTR lpNetworkKey, ULONG_PTR lpAssemblerKey, DWORD dwAssemblerHandle )
			{
				CCommSocket* pSocket = (CCommSocket*)lpNetworkKey;
				pSocket->SetAssemblerKey( lpAssemblerKey, dwAssemblerHandle );
			}

			virtual void FreeNetworkKey( ULONG_PTR lpNetworkKey )
			{
				CCommSocket*						pCommSocket;
				Common::DataStructure::CMemoryPoolSafty<CCommSocket>*	pCommSocketPool;

				pCommSocket		= (CCommSocket*)lpNetworkKey;
				pCommSocketPool	= pCommSocket->GetCommSocketPool();

				pCommSocketPool->Free( pCommSocket );
			}

			virtual LPCTSTR GetLocalAddr( ULONG_PTR lpNetworkKey )
			{
				CCommSocket* pSocket = (CCommSocket*)lpNetworkKey;
				return pSocket->GetLocalAddr();
			}

			virtual LPCTSTR GetPeerAddr( ULONG_PTR lpNetworkKey )
			{
				CCommSocket* pSocket = (CCommSocket*)lpNetworkKey;
				return pSocket->GetPeerAddr();
			}


		private:
			void	ProcessAccept	( SOCKET sSocket );
			void	ProcessConnect	( SOCKET sSocket );
			void	ProcessClose	( SOCKET sSocket );
			void	ProcessRecv		( SOCKET sSocket );
			void	ProcessSend		( SOCKET sSocket );
			/*
			// ��ϵ� IO�ڵ� ó���� Accept
			void	ProcessAcceptCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped );
			// ��ϵ� IO�ڵ� ó���� Accept and Recv
			void	ProcessAcceptRecvCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped );
			// ��ϵ� IO�ڵ� ó���� Recv
			void	ProcessRecvCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped );
			// ��ϵ� IO�ڵ� ó���� Send
			void	ProcessSendCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped );
			*/

			/*
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Implementation
		private:
			// IIocp Implementation : 

			// ��ϵ� IO �ڵ��� �Ϸ� �̺�Ʈ�� �ޱ� ���� �������̽�
			virtual void OnGetQueuedCompletionStatus( BOOL bRet, DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped );
			*/
		};// class CTcpAsyncSelect
	}// namespace TcpAsyncSelect
}// namespace Network
