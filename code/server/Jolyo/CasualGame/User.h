#pragma once

#include <string>

#include "..\NJean\Application\Peer.h"
#include "..\DataStructure\InnerObject.h"
#include "CasualPacket.h"	// ID�� ���̸� �������� ���� ���
#include "Pocket.h"

namespace CasualGame
{
	using namespace CasualPacket;
	using namespace std;

	class CUser : public Application::CPeer //��ɫ��������
	{
	public:
		CUser();
		virtual ~CUser();

	private:
		Common::DataStructure::CInnerObject<CUser>
				m_InnerObjectForRoom;		// Room �ȿ� ���ϱ� ���� Inner Object

		Common::DataStructure::CInnerObject<CUser>
				m_InnerObjectForChannel;	// Channel �ȿ� ���ϱ� ���� Inner Object

		Common::DataStructure::CInnerObject<CUser>
				m_InnerObjectForGame;		// Game �ȿ� ���ϱ� ���� Inner Object

		CPocket*	m_pPocket;				// ������� ���Ӻ� Ưȭ�� Ŭ����

	protected:
		string		m_strUserNo;				// ����� ���̵� �ε���
		string		m_strID;				// ����� ���̵�
		string		m_strNick;				// ����� ��

		UINT		m_nGrade;				// ����� ���
		__int64		m_nExp;					// ����� ����ġ
		BOOL		m_bMale;				// ����

		BOOL		m_bLogined;				// ���� ���� ����
		BOOL		m_bService;				// ���� ���� ���� ����
		BOOL		m_bMapped;				// �ʿ� ��ϵǾ����� ����

//			string		m_strUserProperty;		// ����� ����
		string		m_strIP;				// p2p������ IP
		INT			m_nPort;				// p2p������ Port

		BOOL		m_bCaptine;				// ���� ����
		BOOL		m_bP2PCaptine;			// p2p���� ������ �� �� �ִ��� ���� : �ý��� ��Ȳ
		BOOL		m_bGameCaptine;			// ������ �� �� �ִ��� ���� : ���� �� ��Ȳ

		CHAR		m_cRoomInnerIndex;		// �� �ȿ����� �ڽ��� ��ȣ
		int			m_nRoomIndex;			// ����ڰ� ���� Room�� Index
		int			m_nChannelIndex;		// ����ڰ� ���� channel�� Index

	public:
		/*
		VOID		SetPort( INT nPort )
		{
			m_nPort = nPort;
		}
		*/
		VOID		SetAddress( LPCTSTR strIP, short nPort )
		{
			m_strIP	= strIP;
			m_nPort	= nPort;
		}
		string&		GetIP()
		{
			return m_strIP;
		}
		SHORT		GetPort()
		{
			return m_nPort;
		}

		VOID		SetRoomInnerIndex( CHAR cIndex )
		{
			m_cRoomInnerIndex = cIndex;
		}
		CHAR		GetRoomInnerIndex()
		{
			return m_cRoomInnerIndex;
		}

		CPocket*	GetPocket()
		{
			return m_pPocket;
		}
		void		SetPocket( CPocket* pPocket )
		{
			m_pPocket = pPocket;
		}

		Common::DataStructure::CInnerObject<CUser>*
			GetInnerObjectForRoom()
		{
			return &m_InnerObjectForRoom;
		}

		Common::DataStructure::CInnerObject<CUser>*
			GetInnerObjectForChannel()
		{
			return &m_InnerObjectForChannel;
		}

		Common::DataStructure::CInnerObject<CUser>*
			GetInnerObjectForGame()
		{
			return &m_InnerObjectForGame;
		}

		/*
		string&		GetUserProperty()
		{
			return m_strUserProperty;
		}
		VOID		SetUserProperty( LPCTSTR strProperty )
		{
			m_strUserProperty = strProperty;
		}
		*/

		string&		GetUserNo()
		{
			return m_strUserNo;
		}

		string&		GetID()
		{
			return m_strID;
		}

		string&		GetNick()
		{
			return m_strNick;
		}

		void		SetUserInfo( LPCTSTR strUserNo, LPCTSTR strID, LPCTSTR strNick )
		{
			m_strUserNo		= strUserNo;
			m_strID			= strID;
			m_strNick		= strNick;
		}

		void		SetGrade( UINT nGrade )
		{
			m_nGrade = nGrade;
		}
		UINT		GetGrade()
		{
			return m_nGrade;
		}

		void		SetExp( __int64 nExp )
		{
			m_nExp = nExp;
		}
		__int64		GetExp()
		{
			return m_nExp;
		}

		void		SetMale( BOOL bMale )
		{
			m_bMale = bMale;
		}
		BOOL		IsMale()
		{
			return m_bMale;
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
		void		SetService( BOOL bService )	//logreplyʱ������Ϊtrue
		{
			m_bService = bService;
		}
		BOOL		IsMapped()
		{
			return m_bMapped;
		}
		VOID		SetMapped( BOOL bMapped )
		{
			m_bMapped = bMapped;
		}

		INT		GetRoomIndex()
		{
			return m_nRoomIndex;
		}
		void	SetRoomIndex( INT nRoomIndex )
		{
			m_nRoomIndex = nRoomIndex;
		}

		INT		GetChannelIndex()
		{
			return m_nChannelIndex;
		}
		void	SetChannelIndex( INT nChannelIndex )
		{
			m_nChannelIndex = nChannelIndex;
		}

		BOOL	IsCaptine()
		{
			return m_bCaptine;
		}
		void	SetCaptine( BOOL bCaptine )
		{
			m_bCaptine = bCaptine;
		}

		void	SetGameCaptine( BOOL bGameCaptine )
		{
			m_bGameCaptine = bGameCaptine;
		}
		void	SetP2PCaptine( BOOL bP2PCaptine )
		{
			m_bP2PCaptine = bP2PCaptine;
		}
		BOOL	CanBeGameCaptine()
		{
			return m_bGameCaptine;
		}
		BOOL	CanBeP2PCaptine()
		{
			return m_bP2PCaptine;
		}

	};// class CUser

}// namespace CasualGame
