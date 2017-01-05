#include <WinSock2.h>
#include "../include/MinHook.h"
//#pragma comment(lib,"../lib/libMinHook.x86.lib")
#pragma comment(lib,"ws2_32.lib")
#include "MsgDef.h"
#include "SPIniReadWrite.h"

static String getModuleFilePathEx()  
{
	static String strModulePath;
	if (strModulePath.isEmpty())
	{
		wchar_t path[MAX_PATH];
		::GetModuleFileName(NULL, (LPWSTR)path, MAX_PATH);
		strModulePath = path;
		strModulePath = strModulePath.substring( 0, strModulePath.lastIndexOfChar(L'\\') + 1);
	}
	return strModulePath;
}

class ShareMem
{
private:
	ShareMem():clientCfgFilePath(File(getModuleFilePathEx()).getParentDirectory().getFullPathName()
		+ "/config/client_cfg.ini")
		, clientCfg(clientCfgFilePath)
	{}
public:
	static ShareMem* getInstance()
	{
		if (nullptr == Instance)
		{
			Instance = new ShareMem;
		}
		return Instance;
	}
	String getString(String strSetion, String strKey)
	{
		return clientCfg.getString(strSetion, strKey);
	}
	void setValue(String strSetion, String strKey, int nValue)
	{
		clientCfg.setValue(strSetion, strKey, nValue);
		clientCfg.writeFile(clientCfgFilePath);
	}
	void setValue(String strSetion, String strKey, String nValue)
	{
		clientCfg.setValue(strSetion, strKey, nValue);
		clientCfg.writeFile(clientCfgFilePath);
	}
protected:
private:
	static ShareMem* Instance;
	String clientCfgFilePath;
	CSPIniReadWrite clientCfg;	
};
class DataManager
{
private:
	DataManager()
	{
		war3UdpSock = 0;
		udpPort		= ShareMem::getInstance()->getString("CONFIG", "roomUdpPort").getIntValue();
		srvIp		= ShareMem::getInstance()->getString("CONFIG", "address");
		gSendBuf	= new char[UDP_SENDBUF_SIZE];
		myid		= ShareMem::getInstance()->getString("CONFIG", "myId").getIntValue();
		roomid		= ShareMem::getInstance()->getString("CONFIG", "roomId").getIntValue();
		isHost = myid == roomid;
		ZeroMemory(&addrSrv, sizeof(SOCKADDR_IN));
		addrSrv.sin_addr.S_un.S_addr = inet_addr(srvIp.getCharPointer());        //set the host IP  
		addrSrv.sin_family = AF_INET;     //set the protocol family  
		addrSrv.sin_port = htons(udpPort);  
		bigEndianMyId = htons(myid);
	}
public:
	static DataManager* getInstance()
	{
		if (nullptr == pInstance)
		{
			pInstance = new DataManager;
		}
		return pInstance;
	}
	void setUdpSock(SOCKET tempWar3UdpSock)
	{
		war3UdpSock = tempWar3UdpSock;
	}
	SOCKET getGameUdpSock()
	{
		return war3UdpSock;
	}
	int getUdpPort()
	{
		return udpPort;
	}
	int getMyId()
	{
		return myid;
	}
	bool getIsHost()
	{
		return isHost;
	}
	String& getIp()
	{
		return srvIp;
	}
	char* getBuff()
	{
		return gSendBuf;
	}
	SOCKADDR_IN& getAddrSrv()
	{
		return addrSrv;
	}
	unsigned short& getBigEndianMyid()
	{
		return bigEndianMyId;
	}
private:
	static DataManager* pInstance;

	SOCKET  war3UdpSock;
	int		udpPort;	
	String	srvIp;
	bool	isHost;
	char *	gSendBuf;
	int		myid;
	int     roomid;
	SOCKADDR_IN addrSrv;
	unsigned short bigEndianMyId;
};