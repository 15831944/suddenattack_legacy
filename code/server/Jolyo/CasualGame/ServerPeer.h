#pragma once

#include "..\NJean\Application\Peer.h"
#include "..\DataStructure\InnerObject.h"
#include "CasualPacket.h"	// ID�� ���̸� �������� ���� ���

namespace GamehiSolution
{
	namespace CasualGame
	{
		using namespace CasualPacket;
		class CServerPeer : public Application::CPeer
		{
			/*
		public:
			enum	SessionStatus
			{
				Ready	= 0,	// ���ӵ� ����
				Login	= 1,	// �α��ε� ���� : ���� ������ ��쿡 Login ���°� �Ǹ�, Unique�� ��쿡 Service ���°� �ȴ�.
				Service	= 2,	// ���� ���� ����

				// �帧
				// Ready -> Login -> Service	: ����ó���� ���� ������ �� ���
				// Ready -> Service				: Unique�� ������ �� ���
			};

		private:
			long		m_lIdd;
			char		m_strID[ CCasualPacketInfo::StringLength::UserID + 1 ];		// NULL ���� + 1

			BOOL		m_bLogined;		// ���� ���� ����
			BOOL		m_bService;		// ���� ���� ���� ����

			int			m_nRoomIndex;	// ����ڰ� ���� Room�� Index

			Common::DataStructure::CInnerObject<CServerPeer>
					m_InnerObjectForRoom;	// Room �ȿ� ���ϱ� ���� Inner Object
		public:
			CServerPeer();
			virtual ~CServerPeer();

		public:
			Common::DataStructure::CInnerObject<CServerPeer>*
				GetInnerObjectForRoom()
			{
				return &m_InnerObjectForRoom;
			}

			long		GetIdd()
			{
				return m_lIdd;
			}
			void		SetIdd( long lIdd )
			{
				m_lIdd = lIdd;
			}

			LPCTSTR		GetID()
			{
				return m_strID;
			}
			void		SetID( LPCTSTR strID, int nLen )
			{
				if( nLen > CCasualPacketInfo::StringLength::UserID )
					nLen = CCasualPacketInfo::StringLength::UserID;

				::memcpy( m_strID, strID, nLen );
				m_strID[nLen] = NULL;
			}

			BOOL		IsLogined()
			{
				return m_bLogined;
			}
			void		SetLogined( BOOL bLogined )
			{
				m_bLogined = bLogined;
			}

			BOOL		IsService()
			{
				return m_bService;
			}
			void		SetService( BOOL bService )
			{
				m_bService = bService;
			}

			int		GetRoomIndex()
			{
				return m_nRoomIndex;
			}
			void	SetRoomIndex( int nRoomIndex )
			{
				m_nRoomIndex = nRoomIndex;
			}

			*/
		};// class CServerPeer

	}// namespace CasualGame

}// namespace GamehiSolution