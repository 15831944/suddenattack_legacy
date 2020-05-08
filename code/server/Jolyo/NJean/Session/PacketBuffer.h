/*****************************************************************************************************************
*****************************************************************************************************************/

#pragma once

#include "../Application/Packet.h"

namespace Session{		namespace Manage{
	class CPacketBuffer		//���ݰ���������ʵ���Ϲ���Session::Message�и���
	{
		// ���� / �Ҹ���
	public:
		CPacketBuffer()
			:	m_pData			( m_Buffer)
			,	c_nBufferSize	( CPacket::PredefineLength::BasePacketSize	)
			,	m_nDataSize		( 0		)
			,	m_dwHandle		( NULL	)
		{}
		virtual ~CPacketBuffer(VOID){}

		// Attributes
	private:
		CHAR		m_Buffer[CPacket::PredefineLength::BasePacketSize+2];		// ������ ���� �޸� ����
		const INT	c_nBufferSize;											// ���� ������ ũ��

		CHAR*		m_pData;		// ��ȿ�� �������� ������ : Send�� ��� ©���� ������ �Ǹ� �� ���� �޶�����.
		INT			m_nDataSize;	// ������ ũ��

		DWORD		m_dwHandle;		// �� ��Ŷ�� �������� �ڵ�
		DWORD		m_dwSendIndex;	// Send ���� ��ȣ

	public:
		VOID		CopyData		( const CHAR* pData, INT nDataSize );
		VOID		CopyData		( const CHAR* pData, INT nDataSize, DWORD dwHandle, DWORD dwSendIndex );

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
		INT			GetBufferSize()
		{
			return	c_nBufferSize;
		}
		DWORD		GetHandle	()
		{
			return	m_dwHandle;
		}
		DWORD		GetSendIndex()
		{
			return m_dwSendIndex;
		}
		VOID		SetSendIndex( DWORD dwSendIndex )
		{
			m_dwSendIndex = dwSendIndex;
		}
	};// CPacketBuffer

}/* namespace Manage */		}/* namespace Session */
