#pragma once

#include <string>

#include "..\NJean\Application\Peer.h"
#include "..\DataStructure\InnerObject.h"
#include "CasualPacket.h"	// ID의 길이를 가져오기 위해 사용
#include "Pocket.h"

namespace CasualGame
{
	using namespace CasualPacket;
	using namespace std;

	class CUser : public Application::CPeer //실�セ必않轍�
	{
	public:
		CUser();
		virtual ~CUser();

	private:
		Common::DataStructure::CInnerObject<CUser>
				m_InnerObjectForRoom;		// Room 안에 속하기 위한 Inner Object

		Common::DataStructure::CInnerObject<CUser>
				m_InnerObjectForChannel;	// Channel 안에 속하기 위한 Inner Object

		Common::DataStructure::CInnerObject<CUser>
				m_InnerObjectForGame;		// Game 안에 속하기 위한 Inner Object

		CPocket*	m_pPocket;				// 사용자의 게임별 특화된 클래스

	protected:
		string		m_strUserNo;				// 사용자 아이디 인덱스
		string		m_strID;				// 사용자 아이디
		string		m_strNick;				// 사용자 닉

		UINT		m_nGrade;				// 사용자 등급
		__int64		m_nExp;					// 사용자 경험치
		BOOL		m_bMale;				// 성별

		BOOL		m_bLogined;				// 인증 성공 여부
		BOOL		m_bService;				// 서비스 가능 상태 여부
		BOOL		m_bMapped;				// 맵에 등록되었는지 여부

//			string		m_strUserProperty;		// 사용자 정보
		string		m_strIP;				// p2p에서의 IP
		INT			m_nPort;				// p2p에서의 Port

		BOOL		m_bCaptine;				// 방장 여부
		BOOL		m_bP2PCaptine;			// p2p에서 방장이 될 수 있는지 여부 : 시스템 상황
		BOOL		m_bGameCaptine;			// 방장이 될 수 있는지 여부 : 게임 룰 상황

		CHAR		m_cRoomInnerIndex;		// 방 안에서의 자신의 번호
		int			m_nRoomIndex;			// 사용자가 속한 Room의 Index
		int			m_nChannelIndex;		// 사용자가 속한 channel의 Index

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
		void		SetService( BOOL bService )	//logreply珂꼽�阮촘챫rue
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
