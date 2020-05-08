/*****************************************************************************************************************
CLog

�ۼ��� : ������
�ۼ��� : 2004�� 3�� 2��

��  �� :
�α׸� �����.
��� �޽��� Static���� �̷������ ��� Ŭ���������� ���� �α׸� ����ϵ��� �Ѵ�.
�α� ����� ���Ͽ� ���, consoleâ�� ���, ����� â�� ��� �̷��� �������� �ִ�.
�α״� ����, ���, ����, ������, ��� �̷��� 5������ ����Ѵ�.
- ��  �� : �α� �ð�
- ��� : Ŭ����&�޽�� �̸�
- ��  �� : �α׸� ����� Object
- ������ : �α� ����
- ��  �� : ����ϰ��� �ϴ� �α��� ���
�α״� 5�ܰ��� ����� �ξ� �� ��޺��� �α��� ��� ������ �޸� �Ѵ�.
- Always	: �ݵ�� �ʿ��� ���
- Important	: �α׸� �ʿ������� �𸣴� �߿��� ���
- Command	: ��� ������ �ùٸ� ������ �귯������ �˱����� ���� ���
- Debug		: ����� �뵵�ν� Command���� �������� ������ ����ϱ� ���� ���
- Resource	: �޸� �������� üũ�ϱ� ���� ���
��  �� :
1. SetOutputLevel, OpenFile���� ����Ͽ�, �α׸� ���� �⺻ ���� �����Ѵ�.
2. WriteLog�� ���ؼ� �α׸� ����Ѵ�.
3. Flush�� ���ؼ� ���Ͽ� ����Ѵ�.
4. �α� ��ü�� �� ����� �������� CloseFile�� ���ؼ� ���� �ڵ��� �ݴ´�.
*****************************************************************************************************************/


#pragma once
#include <Windows.h>

namespace Common
{
	namespace Debug
	{
		class CLog
		{
			// Attributes
		public:
			enum	LogLevel
			{
				Always		= 0,
				Important	= 1,
				Command		= 2,
				Debug		= 3,
				Resource	= 4,
				None		= 5
			};

			// ���� / �Ҹ���
		public:
			CLog( int nIndex, const char* strModule, LogLevel logLevel );
			~CLog(VOID);

		private:
			struct	TagOutputLevel
			{
				LogLevel	File;
				LogLevel	Console;
				LogLevel	Debug;
			};

			struct	TagModuleDebug
			{
				char			strModule[100];
				long			lPerf;
				BOOL			bUse;
			};

			static TagModuleDebug		g_ModuleDebug[100];
			LogLevel					m_ModuleLogLevel;
			LARGE_INTEGER				m_iPerfStart;
			int							m_nIndex;

			static TagOutputLevel	g_OutputLevel;
			static FILE*			g_pFileLog;
			static LogLevel			g_LogLevel;

			static BOOL				g_bFileWrite;

			static CRITICAL_SECTION	g_cs;

			static int				g_nIndex;
			static LARGE_INTEGER	g_iPerfStart;

			// Operations
		public:
			static void SpecialStart( int nIndex, const char* strModule, LogLevel logLevel );
			static void SpecialEnd();

			// ��� ���⺰ �α׸� ����� ������ �����Ѵ�.
			static VOID	SetOutputLevel( LogLevel FileLevel, LogLevel ConsoleLevel, LogLevel DebugLevel )
			{
				g_OutputLevel.File		= FileLevel;
				g_OutputLevel.Console	= ConsoleLevel;
				g_OutputLevel.Debug		= DebugLevel;
			}

			// �α׸� ����� ������ Open�Ѵ�.
			static BOOL OpenFile( const char* strFileName );

			// �α׸� ����� ������ Close�Ѵ�.
			static VOID CloseFile();

			// �α׸� ����Ѵ�.
			static VOID WriteLog( const char* strWhere, const char* strWho, const char* strWhat, CLog::LogLevel logLevel );

			// �α׸� ����� ���Ͽ� ���������� Write�ϵ��� Flush �Ѵ�.
			static VOID Flush();

			static VOID	WriteProfile();

		private:
			// ���Ͽ� ����.
			static VOID WriteToFile		( const char* strWhen, const char* strWhere, const char* strWho, const char* strWhat, CLog::LogLevel logLevel );
			// �ֿܼ� ����.
			static VOID WriteToConsole	( const char* strWhen, const char* strWhere, const char* strWho, const char* strWhat, CLog::LogLevel logLevel );
			// �����â�� ����.
			static VOID WriteToDebug		( const char* strWhen, const char* strWhere, const char* strWho, const char* strWhat, CLog::LogLevel logLevel );

		};// class CLog

	}// namespace Debug
}// namespace Common
