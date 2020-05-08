// ------------------------------------------------------------------ //
//
//  FILE      : ClientMgr.h
//
//  PURPOSE   : This is the Mgr for the client side DirectEngine
//              libraries.
//
//  CREATED   : November 7 1996
//
//  COPYRIGHT : LithTech Inc., 1996-2000
//
// ------------------------------------------------------------------ //

#ifndef __CLIENTMGR_H__
#define __CLIENTMGR_H__


// Includes....

// ESD Includes
#ifdef LITHTECH_ESD
#include "iltesd.h"
class CLTRealAudioMgr;
class CLTRealVideoMgr;
#endif // LITHTECH_ESD

//----------------------------------------------------------------------------
//Below here are headers that probably wont be needed after certain things
//are removed from the client mgr.
//----------------------------------------------------------------------------
#ifndef __OBJECTMGR_H__
#include "objectmgr.h"
#endif

#ifndef __MOTION_H__
#include "motion.h"
#endif

#ifndef __LINUX
#ifndef __SOUNDMGR_H__
#include "soundmgr.h"
#endif
#endif // !__LINUX

#ifndef __MUSICMGR_H__
#include "musicmgr.h"
#endif

#ifndef __CONCOMMAND_H__
#include "concommand.h"
#endif

#ifndef __PACKETDEFS_H__
#include "packetdefs.h"
#endif

#ifndef __NETMGR_H__
#include "netmgr.h"
#endif

#ifdef COMPILE_JUPITER_EVAL
#include "WaterMark.h"
#endif

#include <vector>

//----------------------------------------------------------------------------
//Above here are headers that probably wont be needed after certain things
//are removed from the client mgr.
//----------------------------------------------------------------------------

#ifndef __VERSION_INFO_H__
#include "version_info.h"
#endif

class CNetMgr;
class VideoMgr;
class CClientShell;
class CMoveAbstract;
class ILTClient;
class InternalObjectSetup;
class CSoundInstance;
class InputMgr;
class CLTBenchmarkMgr;
class CSoundMgr;
struct SMusicMgr;
class LThreadMessage;
class FileRef;
class WorldTree;
class ILTCursor;
class ILTDirectMusicMgr;
class CBindModuleType;



//CSoundMgr *GetSoundMgr();
SMusicMgr *GetMusicMgr();

#define OBJID_CLIENTCREATED_TYPE1	0xFFFD	//Client���� ���� , ���� ���Ӱ� ���� ������ remove
#define OBJID_CLIENTCREATED_TYPE2	0xFFFE	//Client���� ����, ������ �������� remove
#define OBJID_CLIENTCREATED			0xFFFF



#define MAX_DLLNAME_LEN         255
#define MAX_CLIENTNAME_LEN      200
#define MAX_RESTREES            30	//[MURSUM]


enum
{
	L_CONNECT_HOST = 16,
	L_CONNECT_HOST1,
	L_RECV_MY_ID,
	L_LOAD_WORLD,
	L_START_PRELOAD,
	L_END_PRELOAD,
	L_RECV_CLIENT_OBJ_ID,
};	

#define MAX_CLIENTERRORSTRING_LEN   300

// Maximum number of client commands
#define MAX_CLIENT_COMMANDS		255

// All the resource types used in the FileMgr.
#define PCX_TYPECODE        1


// Leeches.
extern LeechDef g_ModelInheritLeechDef;


//#ifndef _FINAL
// [NOVICE]
extern uint32	g_CV_SndCntPerSec;
extern uint32	g_CV_SndCntAtSameTime;
extern LTBOOL	g_CV_SndTestEnable;
extern uint32	g_CV_SndTestHeardOffset;
extern char*	g_CV_SndTestFileName;
//#endif // #ifndef _FINAL


class CClientMgr {
    // Main stuff.
    public:

        CClientMgr();
        ~CClientMgr();

        // Initialize the client.. the client will hang onto pResources, so
        // allocate it if necessary..
        LTRESULT                    Init(const char *resTrees[MAX_RESTREES], uint32 nResTrees, uint32 nNumConfigs, const char **ppConfigFileName);

        //shuts down everything
        void Term();

        // Loads the clientshell.dll stuff.  If pDirectory is NULL, it looks
        // for a console variable and in the registry.
        void                    TermClientShellDE();

        // Called by the client shell telling when you enter a world.
        void                    OnEnterWorld(CClientShell *pShell);
        void                    OnExitWorld(CClientShell *pShell);

        // Tries to start a new shell with the given parameters.
        // If it succeedes, it shuts down the currently running shell.
        LTRESULT                StartShell(StartGameRequest *pRequest);
        void                    EndShell();

		//[MURSUM] Cone
		LTRESULT				StartCheckConeType(const char* lpszIP1, uint16 nPort1,
													const char* lpszIP2, uint16 nPort2,
													const char* lpszID, uint8 nIDLen);
		void					StopCheckConeType();
		uint16					GetHostPort() { return ntohs( m_SockAddr.sin_port ); }
		void					SetDumpDir( const char* lpszDir );
		void					MakeScreenShot( const char* lpszFileName );
		LTRESULT				PlayAmbientSound();
		
		IDirect3DDevice8*		GetD3DDevice();
		uint8					GetNetState();
		LTRESULT				StartDummyHost( const char* lpszWorldName );

		long					InitFireGuard( const char* lpszGameName, const char* lpszUserID );
		long					StartFireGuard();
		void					CheckFireGuard();
		void					StopFireGuard();
		void					ShowFireGuardErr( long lResult, bool bStartErr );

		//[MURSUM] ���ϸ���Ʈ�� �̸� �ε�
		bool					PreLoadFileList( const char* lpszName, bool bDir );
		bool					PreLoadFile( const char* lpszFileName );
		bool					PreLoadDir( const char* lpszDirName );

		void					SendDummyPacket( const char* lpszIP, uint16 nPort );
		void					ToggleFullScreen();

		void					SetLoadingState( int32 nState );
		int32					GetLoadingState();
		
		void					SetUserNickName( const char* lpszNickName );
		void					SendAddedFiles();
		
        // Reads messages from the threads.
        void                    ProcessThreadMessage(LThreadMessage &msg);
        void                    CheckThreads();

        // DE_OK if everything's ok, otherwise, shutdown.
        LTRESULT                Update();

        int                     GetErrorCode();


    ///////////// Screen functions ///////////////////////
    public:

        bool                    BindClientShellWorlds();
        void                    UnbindClientShellWorlds();

        void                    BindSharedTextures();
        void                    UnbindSharedTextures(bool bUnLoad_EngineData);

        void                    InitConsole();
        void                    TermConsole();


    ///////////// Net functions ///////////////////////

        LTRESULT                ClearInput();
        void                    ProcessAllInput(bool bForceClear);
		void                    SendUpdate(CNetMgr *pNetMgr, CBaseConn *pConnID);



    ///////////// Utility functions ///////////////////////
    public:

        LTRESULT                PlaySound(PlaySoundInfo *pPlaySoundInfo, FileRef *pFile, float fStartTime);

    public:

        void                    KillSound(HLTSOUND hSound);
        void                    UpdateAllSounds();
        CSoundInstance*         FindSoundInstance(HLTSOUND hSound, bool bClientSound);

        //[MURSUM]
		LTFLOAT					GetFrameRateServer();
		LTFLOAT					GetFrameRateClient();
		LTFLOAT					GetFrameTimeServer();
		LTFLOAT					GetFrameTimeClient();

        void                    ForwardMessagesToScript();
        void                    ForwardCommandChanges(int32 *pChanges, int32 nChanges);

        LTRESULT                AppInitMusic(const char *pszMusicDLL);
        void                    AppTermMusic();

        void                    UpdateObjects( uint32 nFrameTimeMS );

        // MODEL INTERFACE FUNCTIONS
		// load file from rez.
		LTRESULT				LoadModel( const char *filename, Model *& );
		LTRESULT				LoadModel( const FileRef &file_ref , Model *&);

		// this will return true if the file has been cached on the client,
		// the file may or many not be cached on the server...
		bool					IsModelCached( const char *pFilename );
		bool					IsModelCached( const FileRef &file_ref );
		// load and cache file.
		LTRESULT				CacheModelFile( const char * filename );
		LTRESULT				CacheModelFile( const FileRef &file_ref  );
		// remove cache status, release model.
		LTRESULT				UncacheModelFile( Model *pModel );
		// removes cache status of all cached files, and releases them.
		void					UncacheModels();


    public:

//      CSoundMgr*              GetSoundMgr()       { return &m_SoundMgr; }
        SMusicMgr*              GetMusicMgr()       { return &m_MusicMgr; }

        //not in this class anymore.  see IClientDebugGraphMgr.
        //CDebugGraphMgr*           GetGraphMgr()       { return &m_DebugGraphMgr; }

//      ILTCursor*              GetCursorMgr()      { return m_pCursorMgr; }

    //-------------------------------------------------------------------------
    //Below here is stuff will eventually be split out of this class and put
    //in their own standalone modules.
    //-------------------------------------------------------------------------

    public:

        // The object manager.  This holds the lists of all the objects (which aren't
        // necessarily always in the world tree).
        ObjectMgr               m_ObjectMgr;

        SMusicMgr               m_MusicMgr;

		CNetMgr                 m_ClientNetMgr;

		const char*				GetCpCookie() { return m_szCpCookie; }
		LTBOOL					GetCookieFromClipboard( HWND hWnd );
		char					m_szCpCookie[1024*2];
	
		//[MURSUM] Cone
		bool					InitNetworking( const char* lpszBindIP, unsigned short uPort, bool bTermShell=true );
		void					TermNetworking();
		SOCKET					m_Socket;
		bool					m_bWSAInitted;
		sockaddr_in				m_SockAddr;
		
        // The server mirror console state.
        ConsoleState            m_ServerConsoleMirror;

		// Returns false if the command is off, or bigger than MAX_CLIENT_COMMANDS
        bool					IsCommandOn(uint32 nCommand) const;

	protected:
        // Commands for the current and previous frames.
        uint8                   m_Commands[2][MAX_CLIENT_COMMANDS];
        uint32                  m_iCurInputSlot; // Alternates between the two slots.

    //-------------------------------------------------------------------------
    //Above here is stuff will eventually be split out of this class and put
    //in their own standalone modules.
    //-------------------------------------------------------------------------

    public:


        //this thing is an interface now, dont store pointers to it like this.
//      ILTClient               *m_pClientDE;

        // The ClientShellDE we're using.
        CBindModuleType         *m_hClientResourceModule;
        CBindModuleType         *m_hLocalizedClientResourceModule;
        CBindModuleType         *m_hShellModule;

        char                    m_ErrorString[MAX_CLIENTERRORSTRING_LEN+1];

    // Screen stuff.
    public:

        void                    ShowDrawSurface(uint flags = NULL);

        // Global lighting (values 0-1).
        LTVector                m_GlobalLightScale;
		// ����
		LTVector				m_GlobalLightDir;
		LTVector				m_GlobalLightColor;
		float					m_GlobalLightAmbient;


    // Sound stuff...
    public:

        char                    m_MusicDLLName[MAX_DLLNAME_LEN+1];

    // Data collections.
    public:

        LTList                  m_Sprites;
        LTList                  m_SharedTextures;

        StructBank              m_FileIDInfoBank;
        ObjectBank<SharedTexture>   m_SharedTextureBank;


        // Sky stuff.
        uint16                  m_SkyObjects[MAX_SKYOBJECTS];
        uint16                  m_nSkyObjects;
        SkyDef                  m_SkyDef;

    // Other.
    public:

        // Executable version info.
        LTVersionInfo           m_VersionInfo;

        // Maps object IDs to object pointers.
        LTRecord                *m_ObjectMap;
        uint32                  m_ObjectMapSize;

        // These tell how long the current frame took.

		//the integer ms time of the last update
		uint32					m_nLastUpdateTimeMS;

		//the integer ms time of this update
		uint32					m_nCurUpdateTimeMS;

		//the frame time stored in milliseconds
		uint32					m_nFrameTimeMS;

		//the floating point delta between the last and current update in seconds
		float                   m_FrameTime;

		//the floating point current update time in seconds
		float					m_CurTime;

        // cm_Ping needs to keep track of the last time it got called
        float                   m_fLastPingTime;

        // The last bandwidth requirement we sent to the server.
		uint16					m_LastReceiveBandwidth;

        float                   m_AxisOffsets[NUM_AXIS_OFFSETS];
        InputMgr                *m_InputMgr; // Gotten right at startup.

        // The current client shell.  This is used for error handling only!  This is set
        // before calling any client shell functions that could throw exceptions.  If the
        // shell throws an exception that says it wants the shell shutdown, it'll know which one to shutdown.
        CClientShell            *m_pCurShell;

        int                     m_LastErrorCode;
        uint32                  m_ExceptionFlags;   // Flags from the last exception thrown.

        bool                    m_bCanSaveConfigFile; // Set to true when it's ok to save the config file.

        // If this is FALSE, the client will tell the server to ignore
        // its input in the CMSG_UPDATE packet.
        bool                    m_bInputState;

        // If this is true, we're tracking potentially all device input
        // and ReadInput will not be called.
        bool                    m_bTrackingInputDevices;

        // Model hook stuff.
        ModelHookFn             m_ModelHookFn;
        void                    *m_ModelHookUser;

        // Used so we don't render multiple times at once.
        bool                    m_bRendering;

        // Don't do CF_NOTIFYREMOVEs when shutting down.
        bool                    m_bNotifyRemoves;


        // File manager..
//      ClientFileMgr           *m_hFileMgr;

        const char              *m_ResTrees[MAX_RESTREES];
        uint32                  m_nResTrees;

        VideoMgr                *m_pVideoMgr;

//      ILTCursor               *m_pCursorMgr;

        // DirectMusic manager.
//      ILTDirectMusicMgr       *m_pDirectMusicMgr;

        // Benchmark manager.
//      CLTBenchmarkMgr         *m_pBenchmarkMgr;

        // ESD Member Variables
        #ifdef LITHTECH_ESD
        CLTRealAudioMgr         *m_pRealAudioMgr;
        ILTRealConsoleMgr       *m_pRealConsoleMgr;
        CLTRealVideoMgr         *m_pRealVideoMgr;
        #endif // LITHTECH_ESD

    // ------------------------------------------------------------------
    //
    //
    //  Functions that used to be called cm_*, now they are member functions.
    //
    //
    // ------------------------------------------------------------------

    public:

        // ------------------------------------------------------------------ //
        // clientmgr.cpp
        // ------------------------------------------------------------------ //

        // Called when a connection to a server is established
        // (m_pCurShell) and when it is broken.
        void OnEnterServer();
        void OnExitServer(CClientShell *pShell);

        void RunCommandLine(uint32 flags);
        void RunAutoConfig(const char *pFilename, uint32 flags);
        void SaveAutoConfig(const char *pFilename);

        // Start the renderer from the global options.
        LTRESULT StartRenderFromGlobals();

        // Render through a camera.
        bool Render(CameraInstance *pCamera, int drawMode, LTObject **pObjects, int nObjects, float fFrameTime);

		//renders a cubic environement map to disk from the given location
		bool MakeCubicEnvMap(CameraInstance *pCamera, uint32 nTextureSize, const char* pszFilePrefix);

        void RebindTextures();

		//generates a log to the specified file detailing the texture memory breakdown
		void LogTextureMemory(const char* pszFilename);

        // Normal add.. fully sets up the object.
        // Pass OBJID_CLIENTCREATED for id if the object doesn't come from the server.
        LTRESULT AddObjectToClientWorld(uint16 id, InternalObjectSetup *pSetup, LTObject **ppObject, bool bMove, bool bRotate);

        LTRESULT RemoveObjectFromClientWorld(LTObject *pObject);

        void RemoveObjectsInList(LTList *pList, bool bServerOnly);

		// Remove the object from our internal client object list
		void RemoveClientObject(LTObject *pObj);

        void UpdateModels( uint32 nFrameTimeMS );
        void UpdateLineSystems( uint32 nFrameTimeMS );
        void UpdatePolyGrids( uint32 nFrameTimeMS );
        void UpdateAnimations( uint32 nFrameTimeMS );
        void UpdateParticleSystems( uint32 nFrameTimeMS );


        // ------------------------------------------------------------------ //
        // c_util.cpp
        // ------------------------------------------------------------------ //

        // Sets up the error string for the error.
        LTRESULT SetupError(LTRESULT theError, ...);

        // Processes an error (disconnects, prints error message, etc).  This will
        // usually return LT_OK, but may return LT_ERROR if the error was fatal
        // and it needs to shutdown.
        LTRESULT ProcessError(LTRESULT theError);

        // Sets up a CSharedTexture for the texture (if one doesn't exist already).
        SharedTexture* AddSharedTexture(FileRef *pRef);
        LTRESULT AddSharedTexture2(FileRef *pRef, SharedTexture* &pTexture);
        LTRESULT AddSharedTexture3(FileIdentifier *pIdent, SharedTexture* &pTexture);

        void FreeSharedTexture(SharedTexture *pTexture);

        // Frees all SharedTextures.
        void FreeSharedTextures();

        void UntagAllTextures();
        void TagUsedTextures();
        void BindUnboundTextures();
        void UnbindUnusedSharedTextures();

        // This function gets rid of as many sprites as it can.
        void TagAndFreeSprites();

        // This function gets rid of as many textures as it can.
        void TagAndFreeTextures();

        // Free untagged textures.
        void FreeUnusedSharedTextures();
        // Free the textures associated with a model if nobody else is using them
        void FreeUnusedModelTextures(LTObject *pObject);


        void AddToObjectMap(uint16 id);
        void ClearObjectMapEntry(uint16 id);
        LTObject* FindObject(uint16 id);
        LTRecord* FindRecord(uint16 id);

        void ScaleObject(LTObject *pObject, const LTVector *pNewScale);
        void UpdateModelDims(ModelInstance *pInstance);
        void RelocateObject(LTObject *pObject);
        void MoveObject(LTObject *pObject, const LTVector *pNewPos, bool bForce);
        void RotateObject(LTObject *pObject, const LTRotation *pNewRot);
        void MoveAndRotateObject(LTObject *pObject, const LTVector *pNewPos, const LTRotation *pNewRot);

        MotionInfo              m_MotionInfo;
        CollisionInfo *         m_pCollisionInfo; // Info on the last collision.
        CMoveAbstract           *m_MoveAbstract;

        //----------------------------------------------------
        //  setupobject.cpp
        //----------------------------------------------------

			/*
	\return the frame code

  The frame code is a counter that is updated after all the objects in the scene are updated.
  This frame code is used by the engine to make sure things are synced. The frame code number
  is not going to match between client and server.
	*/

	uint32 GetFrameCode()				{ return m_FrameCode ; }

/*
	Set the current frame code. Don't do this unless you mean it.

  */

	uint32 SetFrameCode(uint32 fc )		{ return m_FrameCode = fc ; }
	/*
	increment the framecode by one.
	*/
	uint32 IncrementFrameCode()        { return m_FrameCode++ ; }

	// ����
	bool			m_bConnected;

	//[MURSUM]
	int32			m_nLoadingState;


private :
	uint32 m_FrameCode ;
private:

	// The actual file loading.
    LTRESULT LoadModelData(FileIdentifier *pIdent, Model* &pModel);

	// Default net handling for when we don't have a client shell
	class CDefaultNetHandler : public CNetHandler
	{
		virtual bool	NewConnectionNotify(CBaseConn *id, bool bIsLocal) { return false; }
		virtual void	DisconnectNotify(CBaseConn *id, EDisconnectReason eDisconnectReason ) { return; }

		virtual void	HandleUnknownPacket(const CPacket_Read &cPacket, uint8 senderAddr[4], uint16 senderPort);
	};

	CDefaultNetHandler m_cDefaultNetHandler;

#ifdef COMPILE_JUPITER_EVAL
	WaterMark				m_WaterMark;
#endif // COMPILE_JUPITER_EVAL

};

extern CClientMgr *g_pClientMgr;

// ------------------------------------------------------------------ //
// C routines.
// ------------------------------------------------------------------ //


// ------------------------------------------------------------------ //
// clientmgr.cpp
// ------------------------------------------------------------------ //

void cm_Init();

#endif  // __CLIENTMGR_H__






