#pragma once

#include <Windows.h>

#include "..\..\DataStructure\InnerObject.h"

namespace Application
{
	class CPeer //��ɫ״̬
	{
	public:
		CPeer			( void )
		{
			m_InnerObjectForPing.Init( this );
		}
		virtual ~CPeer	( void ){}

	private:
		BOOL		m_bCloseWait;		// �Ƿ�Ϊ�Ⱥ��˳�״̬
		BOOL		m_bRequestWait;		// 
		BOOL		m_bAliveManaged;	// Alive ���� ��� ����
		INT			m_nServicerIndex;	// Peer�� ���� �ε���
		INT			m_nRequestCount;	// �����˳��Ĵ�������ֻҪ����һ���������Ϊ��
		Common::DataStructure::CInnerObject<CPeer>
					m_InnerObjectForPing;	//����CSessionGate����m_AliveManager�й���

		ULONG_PTR	m_lpNetworkKey;		// ��Ʈ��ũ Ű
		DWORD		m_dwHandle;

		char*		m_pAssembleBuffer;		// Assemble ����
		int			m_nAssemble;

	public:
		void		Create( int nMaxPacketSize )
		{
			m_pAssembleBuffer	= new char[nMaxPacketSize];
			m_nAssemble			= 0;
		}

		void		Destroy()
		{
			delete[] m_pAssembleBuffer;
		}

		char*		GetAssembleBuffer()
		{
			return m_pAssembleBuffer;
		}

		int			GetAssembleSize()
		{
			return m_nAssemble;
		}
		void		SetAssembleSize( int nAssembleSize )
		{
			m_nAssemble = nAssembleSize;
		}

		virtual void	Init()
		{
			m_bCloseWait		= FALSE;
			m_nRequestCount		= 0;
			m_bRequestWait		= FALSE;
		}

		BOOL	IsAliveManaged()
		{
			return m_bAliveManaged;
		}
		void	SetAliveManaged( BOOL bAliveManaged )
		{
			m_bAliveManaged = bAliveManaged;
		}

		BOOL	IsCloseWait()
		{
			return m_bCloseWait;
		}

		VOID	SetCloseWait()
		{
			m_bCloseWait = TRUE;
		}

		VOID		SetNetworkKey( ULONG_PTR lpNetworkKey, DWORD dwHandle )
		{
			m_lpNetworkKey	= lpNetworkKey;
			m_dwHandle		= dwHandle;
		}
		ULONG_PTR	GetNetworkKey()
		{
			return m_lpNetworkKey;
		}
		DWORD		GetHandle()
		{
			return m_dwHandle;
		}

		Common::DataStructure::CInnerObject<CPeer>*
					GetInnerObjectForPing()
		{
			return &m_InnerObjectForPing;
		}

		VOID		SetServicerIndex( INT nIndex )
		{
			m_nServicerIndex = nIndex;
		}
		INT			GetServicerIndex()
		{
			return m_nServicerIndex;
		}

		VOID		IncrementRequestCount()
		{
			m_nRequestCount++;
		}
		VOID		DecrementRequestCount()
		{
			m_nRequestCount--;
		}
		BOOL		IsRequested()
		{
			return( m_nRequestCount > 0 );
		}

		BOOL		IsRequestWait()
		{
			return m_bRequestWait;
		}
		void		SetRequestWait()
		{
			m_bRequestWait = TRUE;
		}

	};// class CSessionGate

}// namespace Session