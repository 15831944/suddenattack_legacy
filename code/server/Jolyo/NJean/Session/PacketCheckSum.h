#pragma once

// 2004�� 3�� 22��
#include <Windows.h>

namespace Session
{
	namespace Message
	{
		class CPacketCheckSum
		{
		public:
			CPacketCheckSum(void){}
			~CPacketCheckSum(void){}

		public:
			enum	CheckSumType
			{
				None			= 0,	// ý���� ������� �ʴ´�.
				TwoByteZoroOne	= 1,	// ý�������� 010101�� 2Byte�� ����Ѵ�.
			};

			static void	SetCheckSum( CheckSumType checkSumType, char* pPacket, int nPacketSize )
			{
				switch( checkSumType )
				{
					// None�� ��쿡�� �ƹ��͵� ���� �ʴ´�.
				case CheckSumType::None :
					int a;
					a = 3;
					break;

				case CheckSumType::TwoByteZoroOne :
					CheckSumTwoByteZeroOne	( pPacket, nPacketSize );
					break;

				default :
					int b;
					b = 3;
					break;
				}
			}

			static BOOL	IsCorrectCheckSum( CheckSumType checkSumType, const char* pPacket, int nPacketSize )
			{
				switch( checkSumType )
				{
					// None�� ��쿡�� �ƹ��͵� ���� �ʴ´�.
				case CheckSumType::None :
					return TRUE;

				case CheckSumType::TwoByteZoroOne :
					return IsCorrectCheckSumTwoByteZeroOne	( pPacket, nPacketSize );

				default :
					return FALSE;
				}
			}

			static int	GetCheckSumSize( CheckSumType checkSumType )
			{
				switch( checkSumType )
				{
					// None�� ��쿡�� �ƹ��͵� ���� �ʴ´�.
				case CheckSumType::None :
					return 0;

				case CheckSumType::TwoByteZoroOne :
					return 2;

				default :
					return -1;
				}
			}

		private:
			////////////////////////////////////////////////////////////////////////////////////////////////////
			// check sum�� ��Ŷ�� �ڿ� ä���
			//

			// TwoByteZeroOne�� ��쿡�� �ڿ� 2byte�� 010101�� ä���.
			static void	CheckSumTwoByteZeroOne	( char* pPacket, int nPacketSize )
			{
				short nCheckSum = 0x1010;

				if( pPacket == NULL || nPacketSize > 300 )
				{
					int a = 3;
					return;
				}

				memcpy( &pPacket[nPacketSize], &nCheckSum, 2 );
			}


			////////////////////////////////////////////////////////////////////////////////////////////////////
			// check�� �´����� �˻��Ѵ�.
			//

			// ���� 2byte�� 01010������ �˻��Ѵ�.
			static bool	IsCorrectCheckSumTwoByteZeroOne	( const char* pPacket, int nPacketSize )
			{
				short nCheckSum = 0x1010;

				if( pPacket == NULL || nPacketSize > 300 )
				{
					int a = 3;
					return false;
				}

				return( memcmp( &pPacket[nPacketSize], &nCheckSum, 2 ) == 0 );
			}

		};// class CPacketCheckSum

	}// namespace Message

}// namespace Session
