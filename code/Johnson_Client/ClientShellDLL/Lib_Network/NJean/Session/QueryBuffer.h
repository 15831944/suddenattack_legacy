/*****************************************************************************************************************
*****************************************************************************************************************/

#pragma once

#include "..\Application\Peer.h"
#include "../Application/Packet.h"
using namespace Application::Packets;

namespace GamehiSolution{	namespace Session{		namespace Manage{
	class CQueryBuffer
	{
		// ���� / �Ҹ���
	public:
		CQueryBuffer()
			:	m_pData			( NULL	)
			,	c_nBufferSize	( CPacket::PredefineLength::GetQuerySize	)
			,	m_nDataSize		( 0		)
			,	m_dwHandle		( NULL	)
		{}
		virtual ~CQueryBuffer(VOID){}

		// Attributes
	private:
		CHAR		m_Buffer[CPacket::PredefineLength::GetQuerySize];		// ������ ���� �޸� ����
		const INT	c_nBufferSize;											// ���� ������ ũ��

		CHAR*		m_pData;		// ��ȿ�� �������� ������ : Send�� ��� ©���� ������ �Ǹ� �� ���� �޶�����.
		INT			m_nDataSize;	// ������ ũ��

		DWORD		m_dwHandle;		// �� ��Ŷ�� �������� �ڵ�

	public:
		VOID		CopyData		( const CHAR* pData, INT nDataSize, DWORD dwHandle = 0 );

		VOID		ReplaceDataPos	( INT nProcessedSize );

		// Get / Set
		BOOL		IsCreated	()
		{
			return m_Buffer != NULL;
		}
		CHAR*		GetData		()
		{
			return	m_pData;
		}
		VOID		SetDataSize ( INT nDataSize )
		{
			m_nDataSize = nDataSize;
		}
		INT			GetDataSize	()
		{
			return	m_nDataSize;
		}
		BOOL		GetHandle	()
		{
			return	m_dwHandle;
		}
	};// CQueryBuffer

	class CRequestObject
	{
	private:
		Application::CPeer*		m_pPeer;
		UINT					m_uiMessage;
		CQueryBuffer*			m_pQueryBuffer;
	public:
		void SetProperty( Application::CPeer* pPeer, UINT uiMessage, CQueryBuffer* pQueryBuffer )
		{
			m_pPeer			= pPeer;
			m_uiMessage		= uiMessage;
			m_pQueryBuffer	= pQueryBuffer;
		}
		Application::CPeer*	GetPeer()		{ return m_pPeer;		}
		UINT				GetMessage()	{ return m_uiMessage;	}
		CQueryBuffer*		GetQueryBuffer(){ return m_pQueryBuffer;}
	};
}/* namespace Manage */		}/* namespace Session */	}/* namespace GamehiSolution */
