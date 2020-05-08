#pragma once
#include "iocp.h"

/*****************************************************************************************************************
	CAsynCAsyncIocp

	�ۼ��� : ������
	�ۼ��� : 2004�� 2�� 20��

	��  �� :
		IOCP�� Ŭ�����ν� ������ �Ѵ�.
		CAsyncIocp Ŭ������ ��ӹ޾� �񵿱� ó���� �����ϰ� �̺�Ʈ�� �����Ѵ�.
	��  �� :
		1. Method Create�� ����Ͽ� IOCP �ڵ��� �����Ѵ�. �� �� ó���� Thread�� ������ ������ �� �ִ�.
		2. AssignIoCompletionPort�� ����Ͽ� IO �ڵ��� Key���� ������ IOCP �ڵ鿡 ��Ͻ�Ų��.
		3. IO �ڵ��� ����� ���� Key���� NULL�� �Ǿ�� �ȵȴ�.
		4. ��ϵ� IO �ڵ��� �Ϸ� �̺�Ʈ�� �ޱ� ���ؼ��� IIocp �������̽��� ��ӹ޾� OnGetQuededCompletionStatus�� �����Ѵ�.

	��  �� : 2004�� 3�� 8��
		CAsyncIocp Blocking Ŭ������ ���������ν� IOCP API�� ����ϴ� ��Ŀ��� CAsyncIocp�� ����ϴ� ������� ��ȯ
*****************************************************************************************************************/

#include <Windows.h>

namespace IO
{
	namespace IOCP
	{
		// ��ϵ� IO �ڵ��� �Ϸ� �̺�Ʈ�� �ޱ� ���� �������̽�
		interface IIocpHandler
		{
			virtual void OnGetQueuedCompletionStatus( BOOL bRet, DWORD dwNumberOfBytesTransferred, ULONG_PTR lpCompletionKey, LPOVERLAPPED lpOverlapped ) = 0;
		};

		class CAsyncIocp : public CIocp
		{
			// ���� / �Ҹ���
		public:
			CAsyncIocp(void);
			~CAsyncIocp(void);

			// Attributes
		private:
			IIocpHandler*	m_pIocpHandler;			// IOCP(OnGetQueuedCompletionStatus)�� ó���� ��ü
			INT				m_nNumOfThreads;		// IOCP�� �������� ����, ����Ʈ�δ� �������� ���� * 2�� �Ѵ�.

			const ULONG_PTR	c_lpThreadDie;			// ����Ϊ�˳�key

			// Operations
		public:
			// IOCP�� �����Ѵ�.
			//		nNumberOfThreads : �������� ���� : 0�̸� CPU���� * 2�� �����ȴ�.
			//		dwWaitMilliSeconds : wait �ð�
			virtual BOOL	Create( IIocpHandler* pIocpHandler, int nNumberOfThreads = 0, DWORD dwWaitMilliSeconds = INFINITE );

			// IOCP�� �����Ѵ�.
			virtual void	Destroy();

		private:
			// ComletionKey���� ���Ḧ ���� �������� ���Ѵ�.
			BOOL	IsExitCompletionKey( ULONG_PTR CompletionKey )
			{
				return( CompletionKey == c_lpThreadDie );
			}

			// IOCP�� �����ϴ� ������
			static	DWORD __stdcall IocpWorkerThreadStartingPoint(PVOID pvParam);
		};
	}// namespace IOCP

}// namespace IO
