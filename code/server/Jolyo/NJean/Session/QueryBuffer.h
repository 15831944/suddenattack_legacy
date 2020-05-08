/*****************************************************************************************************************
*****************************************************************************************************************/

#pragma once

#import "c:\Program Files\Common Files\System\ADO\msado15.dll" \
    no_namespace rename("EOF", "EndOfFile")

#include "..\Application\Peer.h"
#include "../Application/Packet.h"
using namespace Application::Packets;

namespace Session
{
	namespace Manage
	{
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

		class CDbConnObject
		{
		private:
			_ConnectionPtr		m_dbConn;
		public:
			_ConnectionPtr&		GetDbConn()
			{
				return m_dbConn;
			}
		};

		class CDbObject
		{
		private:
			Application::CPeer*		m_pPeer;
			UINT					m_uiMessage;
			BOOL					m_bSuccess;
			_CommandPtr				m_pCommand;
			void *					m_pArg;
	//		_RecordsetPtr			m_pRS;

		//public:
		//	CDbObject()
		//	{
		//		m_pCommand.CreateInstance( __uuidof(Command) );
		//		m_pCommand->CommandType = adCmdStoredProc;
		//	}
		public:
			void SetProperty( Application::CPeer* pPeer, UINT uiMessage, void * i_pArg )
			{
				m_pPeer			= pPeer;
				m_uiMessage		= uiMessage;
				m_pArg			= i_pArg;
			}
			Application::CPeer*	GetPeer()		{ return m_pPeer;		}
			UINT				GetMessage()	{ return m_uiMessage;	}
			_CommandPtr&		GetCommand()	{ return m_pCommand;	}
	//		_RecordsetPtr&		GetRecordSet()	{ return m_pRS;			}
			BOOL				IsSuccess()		{ return m_bSuccess;	}
			void				SetSuccess( BOOL bSuccess )
			{
				m_bSuccess = bSuccess;
			}
			void *				GetArg()
			{
				return m_pArg;
			}
		};
	}/* namespace Manage */
}/* namespace Session */