
#include "dllmain.h"
DataManager* DataManager::pInstance = nullptr;
ShareMem* ShareMem::Instance = nullptr;
bool gIsGetGameUdpSock = false;

typedef int (WINAPI *MESSAGEBOXW)(HWND, LPCWSTR, LPCWSTR, UINT);
typedef int (PASCAL  *RECVFROM)( SOCKET s, char * buf, int len, int flags, struct sockaddr * from, int * fromlen);
typedef int (PASCAL  *SENDTO)( SOCKET s, const char * buf, int len, int flags, const struct sockaddr * to, int tolen);
typedef int (*CONNECT)(SOCKET s, const struct sockaddr  * name, int namelen);
MESSAGEBOXW fpMessageBoxW	= NULL;
RECVFROM	fpRecvfrom		= NULL;
SENDTO		fpSendto		= NULL;
CONNECT		fpConnect		= NULL;

int WINAPI DetourMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	return fpMessageBoxW(hWnd, L"Hooked!", lpCaption, uType);
}
int PASCAL DetourRecvfrom( SOCKET s, char * buf, int len, int flags, struct sockaddr * from, int * fromlen)
{

	fpRecvfrom(s, buf + 4, len - 4, flags, from, fromlen);
	return 0;
}
int PASCAL DetourSendto( SOCKET s, const char * buf, int len, int flags, const struct sockaddr * to, int tolen)
{
	if (!gIsGetGameUdpSock)
	{
		gIsGetGameUdpSock = true;
		DataManager::getInstance()->setUdpSock(s);
	}
	if (buf[WAR3_HEADER_POS] != WARCRAFT || len + 4 > UDP_SENDBUF_SIZE)
	{
		return 0;
	}
	char* gSendBuf = DataManager::getInstance()->getBuff();
	memcpy(gSendBuf + USER_ID_MSB_POS, &(DataManager::getInstance()->getBigEndianMyid()), 2);
	memcpy(gSendBuf + UDP_HEADER_LENGTH, buf, len);
	if (DataManager::getInstance()->getIsHost())
	{
		gSendBuf[FLAG_ISHOST_POS] = HOST;
		if (buf[WAR3_UDP_CMDID_POS] == BATTLE_CREATE)
		{
			gSendBuf[CMDID_POS]       = UC_CREATE_MAP;
		}
		else
		{
			gSendBuf[CMDID_POS]       = UC_DATA_SYNC;
		}
	}
	else
	{
		gSendBuf[FLAG_ISHOST_POS] = CLIENT;
	}

	fpSendto(s, gSendBuf, len + 4, 0, (SOCKADDR*) &(DataManager::getInstance()->getAddrSrv()), sizeof(SOCKADDR));  
	return 0;
}
int  DetourConnect(SOCKET s, const struct sockaddr  * name, int namelen)
{
	//MessageBoxW(NULL, L"connect hooked...", L"MH", MB_OK);
	if (!(DataManager::getInstance()->getIsHost()))
	{
		char* gSendBuf = DataManager::getInstance()->getBuff();
		gSendBuf[CMDID_POS]       = UC_JOIN_MAP_REQ;
		gSendBuf[FLAG_ISHOST_POS] = CLIENT;
		memcpy(gSendBuf + USER_ID_MSB_POS, &(DataManager::getInstance()->getBigEndianMyid()), 2);
		memcpy(gSendBuf + UDP_HEADER_LENGTH, buf, len);
		fpSendto(DataManager::getInstance()->getGameUdpSock(), gSendBuf, UDP_HEADER_LENGTH, 0, (SOCKADDR*) &(DataManager::getInstance()->getAddrSrv()), sizeof(SOCKADDR));  
	}
	return 0;
}
int doHook()
{
	// Initialize MinHook.
	if (MH_Initialize() != MH_OK)
	{
		return 1;
	}
	// Create a hook for MessageBoxW, in disabled state.
	if (MH_CreateHook(&recvfrom, &DetourRecvfrom, (LPVOID*)&fpRecvfrom) != MH_OK)
	{
		MessageBoxW(NULL, L"hook recvfrom failed...", L"MH", MB_OK);
		return 1;
	}
	if (MH_CreateHook(&sendto, &DetourSendto, (LPVOID*)&fpSendto) != MH_OK)
	{
		MessageBoxW(NULL, L"hook sendto failed...", L"MH", MB_OK);
		return 1;
	}
	if (MH_CreateHook(&connect, &DetourConnect, (LPVOID*)&fpConnect) != MH_OK)
	{
		MessageBoxW(NULL, L"hook connect failed...", L"MH", MB_OK);
		return 1;
	}

	// Enable the hook for MessageBoxW.
	if (MH_EnableHook(&recvfrom) != MH_OK)
	{
		return 1;
	}
	if (MH_EnableHook(&sendto) != MH_OK)
	{
		return 1;
	}
	if (MH_EnableHook(&connect) != MH_OK)
	{
		return 1;
	}

	return 0;
}
BOOL WINAPI DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			int ret = doHook();
			if (ret)
			{
				MessageBoxW(NULL, L"hook failed...", L"MH", MB_OK);
			}
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
