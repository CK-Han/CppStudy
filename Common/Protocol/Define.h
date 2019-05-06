#pragma once

#define	PORT		8888

enum class USER_STATE
{
	NOT_LOGIN = 0,
	LOGIN_WAIT,
	LOBBY,
	ROOM,
};

enum class ROOM_STATE
{
	NONE = 0,
	CREATED_WAIT,
	FULL,
	STARTED,
};