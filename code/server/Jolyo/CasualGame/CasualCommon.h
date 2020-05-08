#pragma once

#include "..\NJean\Application\Packet.h"
// ��Ŷ Ŭ������ ����� virtual �Լ��� ����ؼ��� �ȵȴ�!!
#define __TEST__
#define __TEST_USERNO__	TEXT("1234")

namespace CasualGame { namespace Define
{
	class CCasualCommon
	{
	public:
		enum	UserMessage
		{
			UserSignal	= 1000,
			UserQuery	= 1000,
			UserPacket	= 1000,
		};

		enum	CasualQueryMessage
		{
			LoginQuery					= 100,
		};// enum CasualQueryMessage

		enum	CasualSignalMessage
		{
			ChangeCaptineSignal			= 100,		// ���� ���� �ñ׳�
			CompulseExitSignal			= 101,		// ���� ����
			//CompulseDisconnectSignal	= 102,		// ���� ���� ����
		};// enum CasualSignalMessage

		enum	CasualPacketCommand
		{
			///////////////////////////////////
			// ���� ����
			
			LoginRequest			= 101,		// �α��� ��û
			//LoginCompulsionRequest	= 102,		// ���� �α��� ��û
			SecLoginRequest			= 103,		// ��ȣȭ �α���
			JoinChannelRequest		= 104,		// ä�ο� ����
			LeaveChannelRequest		= 105,		// ä�ο��� ����

			LoginReply				= 106,		// �α��� ���
			//CompulsionDisconnect	= 107,		// ���� ����
			JoinChannelReply		= 108,		// ä�ο� ����
			LeaveChannelReply		= 109,		// ä�ο��� ����


			///////////////////////////////////
			// Room ����
			
			OpenRoomRequest			= 201,		// �� ���� ��û
			JoinRoomRequest			= 202,		// �濡 ���� ��û
			AutoJoinRoomRequest		= 203,		// �濡 �ڵ� ���� ��û
			LeaveRoomRequest		= 204,		// �濡�� ���� ��û

			OpenRoomReply			= 205,		// �� ���� ���
			JoinRoomReply			= 206,		// �濡 ���� ���
			AutoJoinRoomReply		= 207,		// �濡 �ڵ� ���� ���
			LeaveRoomReply			= 208,		// �濡�� ���� ��û ���


			///////////////////////////////////
			// Room ����

			//RoomUserInfoRequest		= 301,		// �� ����� ���� ��û
			//RoomWaitUserInfoRequest	= 302,		// ���� ����� ���� ��û

			//RoomUserInfoReply		= 303,		// �� ����� ���� ���
			//RoomWaitUserInfoReply	= 304,		// ���� ����� ���� ���
			RoomInfo				= 305,		// ������ ��
			RemoveRoom				= 306,		// ���� ��
			RoomInfoChange			= 307,		// ����� �� : �ɼ�, �ο�, ��� ����
			//RoomUserInfo			= 308,		// ���� �����
			//RoomWaitUserInfo		= 309,		// ������ �����
			RoomCaptineChange		= 310,		// ���� ����


			///////////////////////////////////
			// User ����
			
			FindRequest				= 401,		// ����� ã��
			CompulseExitRequest		= 402,		// �������� ��û
			//WaitUserPropertyRequest	= 403,		// ���ǿ� �ִ� ����� ������û

			UserInfo				= 404,		// ���ǿ� ������ ������� �⺻ ����
			RemoveUser				= 405,		// ���ǿ��� ������ �����
			CompulseExitReply		= 412,		// �������� ��û ���
			CompulseExited			= 406,		// ���� ���� ����
			FindReply				= 407,		// ����� ã��
			//RoomUserProperty		= 408,		// �濡 �ִ� ����� ����(�濡 �����ϴ� �����)
			RemoveRoomUser			= 409,		// �濡�� ������ �����
			//WaitUserPropertyReply	= 410,		// ���ǿ� �ִ� ����� �������
			//MyProperty				= 411,		// �ڱ� �ڽ��� ���� : �α��ν� ����


			///////////////////////////////////
			// Packet �߰�

			Chat					= 501,		// ä��
			Memo					= 502,		// ����
			Invite					= 503,		// �ʴ��ϱ�

			MemoReply				= 504,		// ���� ������ ���
			InviteReply				= 505,		// �ʴ��ϱ� ���
		};// enum CasualPacketCommand

		enum	CasualStringLength
		{
			SecLogin		= 1024,		// ��ȣȭ �� �α��� ������ ����
			IpAddr			= 16,		// IP Address
			UserNo			= 20,		// User Id Index
			UserID			= 40,		// User ID
			UserNick		= 36,		// User Nick
			RoomName		= 36,		// �� �̸�
			RoomOption		= 16,		// �� �ɼ�
			RoomPass		= 16,		// �� �н�����
			FindUser		= 40,		// ����� �˻��� �ʿ��� ��Ʈ���� �� ����Ÿ (���̵� �Ǵ� ��)
			ChatMsg			= 200,		// ä�� �޽���
			MemoMsg			= 200,		// �޸� �޽���
			AssembleBody	= 220,		// �ӽ�
			UserProperty	= 2000,		// ����� ���� ������ ũ��
			SessionKey		= 128,		// ����Ű
		};// enum CasualStringLength
	};// class CCasualCommon

}/*namespace Common*/ }/*namespace CasualGame*/
using namespace CasualGame::Define;
#define LIBCOMMAND CCasualCommon::CasualPacketCommand
#define LIBPACKETLENGTH CCasualCommon::CasualStringLength
