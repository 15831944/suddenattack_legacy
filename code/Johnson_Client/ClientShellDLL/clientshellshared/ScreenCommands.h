// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenCommands.h
//
// PURPOSE : Enumerate interface screen commands
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef SCREENCOMMANDS_H
#define SCREENCOMMANDS_H

///////////////////////////////////////
// The command IDs for the screen options

enum eScreenCmds
{
	CMD_NONE,

	//MessageBox
	CMD_OK,
	CMD_CANCEL,
	CMD_MESSAGE_OK,
	CMD_MESSAGE_CANCEL,
	CMD_CLIENT_SHUTDOWN,
	

	//Channel =============================================
	CMD_JOIN_CHANNEL,
	CMD_REFRESH_CHANNEL,
	CMD_EXIT_CHANNEL,
	
	//Character ===========================================
	CMD_EDIT_NICKNAME,

	//Main ================================================
	CMD_ROOM_SCROLL,
	CMD_ROOM_SCROLL_UP,
	CMD_ROOM_SCROLL_DOWN,
	CMD_SHOW_PASSWORD_DLG,
	CMD_JOIN_PRIVATE_ROOM,
	CMD_USER_SCROLL,
	CMD_USER_SCROLL_UP,
	CMD_USER_SCROLL_DOWN,
	CMD_TEXT_SCROLL,
	CMD_TEXT_SCROLL_UP,
	CMD_TEXT_SCROLL_DOWN,
	CMD_CHAT_EDIT,

	// Top
	CMD_SHOP,
	CMD_CLAN,
	CMD_RANKING,
	CMD_CHANNEL,
	CMD_EXIT,

	// Left
	CMD_QUICK_JOIN,
	CMD_CREATE_ROOM,
	CMD_INVENTORY,
	CMD_FIND_WAIT_ROOM,
	CMD_OPTION,
	CMD_HELP, //[yaiin]

	//List Header
	CMD_LIST_HEADER_1,
	CMD_LIST_HEADER_2,
	CMD_LIST_HEADER_3,
	CMD_LIST_HEADER_4,
	CMD_LIST_HEADER_5,
	CMD_LIST_HEADER_6,
	CMD_LIST_HEADER_7,

	// Right Bottom
	CMD_USER,
	CMD_FRIEND,
	CMD_FIND,

	CMD_TEXT_WHELL,
	CMD_MAIN_ROOM_WHELL,
	CMD_MAIN_USER_WHELL,
	CMD_MAIN_FRIEND_WHELL,
	
	//Creat Room Popup
	CMD_CREATE_ROOM_TITLE,
	CMD_CREATE_ROOM_PASSWORD,
	CMD_CREATE_ROOM_COMBO,
	CMD_CREATE_ROOM_PLAYERS,
	CMD_CREATE_ROOM_CHECK,

	//Password
	CMD_EDIT_PASSWORD,
	
	//SearchResult
	CMD_ADD,
	CMD_JOIN,
	CMD_JOIN_ROOM,
	
	//UserInfo
	CMD_UI_RED_TEAM,
	CMD_UI_BLUE_TEAM,
	
	//Popup
	CMD_SHOW_POPUP,

	CMD_USER_INFO,
	CMD_WHISPER,
	CMD_KICK_PLAYER,
	CMD_ADD_FRIENT_LIST,
	CMD_CHANGE_CAPTAIN,
	
	//Room ================================================
	CMD_ROOM_CHAT_SCROLL_UP,
	CMD_ROOM_CHAT_SCROLL_DOWN,
	CMD_ROOM_CHAT_SCROLL,
	CMD_ROOM_CHAT_EDIT,

	CMD_ROOM_BTN_READY,
	CMD_ROOM_BTN_BACK,
	CMD_ROOM_BTN_CHARACTER,
	CMD_ROOM_BTN_OPTION,
	CMD_ROOM_BTN_INVITE,
	CMD_ROOM_BTN_HELP, //[yaiin]


	CMD_ROOM_BTN_TEAMLEFT,
	CMD_ROOM_BTN_TEAMRIGHT,
	CMD_ROOM_BTN_MAPLEFT,
	CMD_ROOM_BTN_MAPRIGHT,
	
	CMD_SELECT_MAP,
	CMD_MAP_DOWN,
	CMD_SCROLL_UP,
	CMD_SCROLL_DOWN,
	CMD_DRAG_UPDOWN,
	
	//Option
	CMD_PLAYERS_DOWN,
	CMD_TIME_DOWN,
	CMD_KILL_DOWN,
	CMD_ROUND_DOWN,

	CMD_INTRUSION_ON,
	CMD_INTRUSION_OFF,
	CMD_TEAMBAL_ON,
	CMD_TEAMBAL_OFF,
	CMD_FREESPEC_ON,
	CMD_FREESPEC_OFF,
	CMD_MAP_SCROLL,
	CMD_MAP_SCROLL_UP,
	CMD_MAP_SCROLL_DOWN,
	CMD_MAP_TYPE_SORT,
	CMD_ROOM_OPT_CHANGED,
	CMD_ROOM_MAP_CHANGED,
	CMD_EDIT_ROOM_TITLE,

	//Invite
	CMD_INVITE,
	CMD_INVITE_SCROLL,
	CMD_INVITE_SCROLL_UP,
	CMD_INVITE_SCROLL_DOWN,
	CMD_INVITE_RESULT,

	CMD_CUSTOM, //this needs to be the last cmd, so that custom commands may be defined after it
};

#endif // SCREENCOMMANDS_H

