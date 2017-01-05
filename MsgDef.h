#pragma  once

#define SRV_TCP_IP  "127.0.0.1"
#define SRV_TCP_PORT 12345
#define BUFSIZE 1024
#define POOL_INCREASE_SIZE 10
#define POOL_INIT_SIZE 20
#define MAX_ID 10000
#define UDP_PORT_INIT 7000
#define DEFAULT 0
#define MAX_ROOMPLAYER_NUM 10
#define UDP_SENDBUF_SIZE 8192
#define WAR3_TCP_PORT    6112

// 释放指针宏
#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}



enum TCP_MSG_
{
	MSG_HEADER = 0xFA,
	MSG_HEARER2 = 0XFB,

	MSG_HEADER_POS = 0,
	MSG_HEARER2_POS,
	MSG_TOTAL_LEN_POS,
	MSG_TOTAL_LEN_POS_LSB,
	MSG_CMDID_POS,
	MSG_USERID_POS,
	MSG_USERID_LSB_POS,
	MSG_ROOMID_POS,
	MSG_ROOMID_LSB_POS,
	MSG_CLIENTID_POS,
	MSG_CLIENTID_LSB_POS,
	MSG_START_POS,
};

enum TCP_CMD_ID
{
	CMD_SYNC_DATA			= 0xC1,
	CMD_JOIN_ROOM			= 0xC2,
	CMD_JOIN_ROOM_RSP		= 0x2C,
	CMD_CREATE_ROOM			= 0xC3,
	CMD_CREATE_ROOM_RSP		= 0xC4,
	CMD_QUIT_ROOM			= 0xC5,
	CMD_QUIT_ROOM_RSP		= 0xC6,// 1退出房间，0或者其他不变
	CMD_HEARTBEAT			= 0xA0,
	CMD_HEARTBEAT_RSP		= 0xA1,
	CMD_USER_LOGIN			= 0xB0,
	CMD_USER_LOGIN_RSP		= 0xB1, // 消息体 1个字节， 0 成功，1失败； 成功携带USERID
	CMD_USER_REGISTER		= 0xD0,
	CMD_USER_REGISTER_RSP	= 0xD1,// 消息体 1个字节， 0 成功，1失败；成功携带USERID
	CMD_START_GAME_REQ		= 0xD2,
	CMD_START_GAME_RSP		= 0xD3,
};

enum SYNC_DATA_TAG
{
	TAG_GAMES	= 0xB0,
	TAG_ROOMS	= 0xB1,
	TAG_FRIENDS = 0xB2,
	TAG_NAME = 0xB3,
	TAG_PASSWORD = 0xB4,
	TAG_RESULT = 0xBB, //0 succed, 1 failed
	TAG_PLAYERS = 0xB5,
};
enum RESULT_
{
	SUCCEED = 0,
	FAILED  = 1,
};
enum ROOM_ACTION_
{
	KEEP_EXSIT = 0,
	DESTROY  = 1,
};
enum GAME_TYPE
{
	TYPE_WARCRAFT = 0x01,
};


//**************************UDP protocol*************************
// | 1 BYTE| 1 BYTE|   2 BYTE |    ...   |
//   isHost   cmdId    userId    msgBody
enum udp_msg_pos
{
	FLAG_ISHOST_POS		= 0,
	CMDID_POS			= 1,
	USER_ID_MSB_POS		= 2,
	USER_ID_LSB_POS		= 3,
	UDP_HEADER_LENGTH   = 4,
};
enum msg_flag
{
	HOST   = 0,
	CLIENT = 1,
};
enum udp_cmd
{
	UC_JOIN_MAP_REQ = 0x01,
	UC_JOIN_MAP_RES = 0x02,
	UC_CHECK_OK_REQ = 0x10,
	UC_CHECK_OK_RES = 0x11,
	UC_CREATE_MAP	= 0x20,
	UC_DATA_SYNC	= 0x30,
};
//******* war3 udp protocl*******
//    1     1        2      N
// | F7 | cmdId | len    | msg
enum warcraft_udp_cmd
{
	WARCRAFT		= 0xF7,
	FIND_BATTLE		= 0x2F,
	MAP_PREVIEW		= 0x30, //查找应答
	BATTLE_CREATE	= 0x31,
	BATTLE_CANCLE	= 0x33,
};
enum warcraft_udp_msg_pos
{
	WAR3_HEADER_POS			= 0,
	WAR3_UDP_CMDID_POS		= 1,
	WAR3_LENGTH_MSB_POS		= 2,
};

