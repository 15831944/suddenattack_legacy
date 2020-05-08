#pragma once

#include <Windows.h>

#include "..\..\DataStructure\InnerObject.h"

namespace GamehiSolution
{
	namespace Application
	{
		class CPeer
		{
		public:
			CPeer			( void )
			{
				m_InnerObjectForPing.Init( this );
			}
			virtual ~CPeer	( void ){}

		private:
			BOOL		m_bCloseWait;		// CloseWait ���� ����
			BOOL		m_bRequestWait;		// Request�Ǿ��� ��� �ݳ� ���
			BOOL		m_bAliveManaged;	// Alive ���� ��� ����
			INT			m_nServicerIndex;	// Peer�� ���� �ε���
			INT			m_nRequestCount;	// Request�� ����
			Common::DataStructure::CInnerObject<CPeer>
						m_InnerObjectForPing;

			unsigned long	m_lpNetworkKey;		// ��Ʈ��ũ Ű

			char*		m_pAssembleBuffer;		// Assemble ����
			int			m_nAssemble;

			BOOL		m_bBeginConnect;

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

				m_bBeginConnect		= FALSE;
			}

			void		SetBeginConnect( BOOL bBegin )
			{
				m_bBeginConnect		= bBegin;
			}
			BOOL		IsBeginConnect()
			{
				return m_bBeginConnect;
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

			VOID		SetNetworkKey( unsigned long lpNetworkKey )
			{
				m_lpNetworkKey = lpNetworkKey;
			}
			unsigned long	GetNetworkKey()
			{
				return m_lpNetworkKey;
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
}// namespace GamehiSolution