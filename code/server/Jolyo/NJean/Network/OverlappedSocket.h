/*****************************************************************************************************************
	COverlappedSocket

	�ۼ��� : ������
	�ۼ��� : 2004�� 2�� 25~27��

	��  �� :
		IOCP�� ��ϵǴ� Overlapped Socket Class
		Accept, Recv, Send ���� �̺�Ʈ�� �ޱ� ���� ���Ǵ� Class�� base Class
	��  �� :
		1. IOCP�κ��� �Ϸ� �̺�Ʈ�� ������, COverlappedSocket���� Ref�� �޴´�.
		2. COverlappedSocket�� vitual function�� GetIOType�� ���ؼ� ���� ��ü�� Ÿ���� �޴´�.
		   ���� �� Ŭ������ ��ӹ��� Ŭ������ �ݵ�� GetIOType�� �����ؾ� �Ѵ�.
*****************************************************************************************************************/


#pragma once

#include <Winsock2.h>
#include <Windows.h>

namespace Network
{
	namespace TcpIOCP
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		// OverlappedSocket�� BaseClass
		class COverlappedSocket
		{
		public:
			COverlappedSocket(void){}
			~COverlappedSocket(void){}

		public:
			enum ErrorCode
			{
				BufferCreateFail		= 1,	// ���� ���� ����
				SocketCreateFail		= 2,	// ���� ���� ����
				SetSockOptFail			= 3,	// ���� �ɼ� ���� ����
				AcceptExFail			= 4,	// AcceptEx ��� ����
				AcceptPoolCreateFail	= 5		// Accept Pool ���� ����
			};

			enum IOType
			{
				NoneType		= 0,
				AcceptType		= 1,
				AcceptRecvType	= 2,
				RecvType		= 3,
				SendType		= 4
			};

			// Attributtes
		public:
			SOCKET			m_sSocket;
			COverlappedSocket*
				m_pParentOvSocket;

		protected:
			DWORD			m_dwErrCode;			// ������ ���� �ڵ�

			ULONG_PTR		m_lpdwOwnerKey;			// �� ������ ����ϴ� ��ü�� Ű
			ULONG_PTR		m_lpAssemblerKey;		// �� �������κ��� Data Assemble�� �ϴ� ��ü�� Ű
			DWORD			m_dwAssemblerHandle;

			int				m_nServiceIndex;		// ������ ���� �ε���


			// Operations
		public:
			// IOCP�� ����� IO �ڵ��� ��ȯ�Ѵ�.
			HANDLE	GetIOHandle()
			{
				return (HANDLE) m_sSocket;
			}

			// ������ ������ ��ȯ�Ѵ�.
			DWORD	GetLastError(void)
			{
				return m_dwErrCode;
			}

			void	SetOwnerKey( ULONG_PTR dwOwnerKey )
			{
				m_lpdwOwnerKey = dwOwnerKey;
			}
			ULONG_PTR	GetOwnerKey()
			{
				return m_lpdwOwnerKey;
			}

			void	SetAssemblerKey( ULONG_PTR lpAssemblerKey, DWORD dwAssemblerHandle )
			{
				m_lpAssemblerKey	= lpAssemblerKey;
				m_dwAssemblerHandle	= dwAssemblerHandle;
			}
			void	GetAssemblerKey( ULONG_PTR& lpAssemblerKey, DWORD& dwAssemblerHandle )
			{
				lpAssemblerKey		= m_lpAssemblerKey;
				dwAssemblerHandle	= m_dwAssemblerHandle;
			}

			int		GetServiceIndex()//CreateListener������,�Է�������˵,�ܶ���ΪServicerIndex::Client(1)
			{
				return m_nServiceIndex;
			}
			void	SetServiceIndex( int nServiceIndex )
			{
				m_nServiceIndex = nServiceIndex;
			}

			// Overroadable
		public:
			// IOType�� ��ȯ�Ѵ�.
			virtual IOType GetIOType( LPOVERLAPPED lpOv )
			{
				return IOType::NoneType;
			}

			virtual void	Init()
			{
				m_pParentOvSocket = NULL;
			}
		};// class COverlappedSocket

	}// namespace TcpIOCP
}// namespace Network
