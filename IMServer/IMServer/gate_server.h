#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <list>
#include <string>

#define EPOLL_SIZE 5000

class CGateServer
{
public:
	CGateServer();
	~CGateServer();
	bool InitServer();
	bool CloseServer();

	bool Run();

	//�����û���������Ϣ
	bool RecvUserInfo(int uClientFd);

private:
	bool _AddFd(int epollfd, int fd, bool enable_et);

	bool _GetPackage(int uClientFd, int ubody_size, int uhead, char* pPackage);

private:
	struct sockaddr_in m_ServerAddr;

	// ����socket
	int m_uListener;

	// epoll_create������ķ���ֵ
	int m_uEpollFd;

	std::string m_strIPAddress;
	std::string m_strPort;

	// �ͻ����б�
	std::list<int> m_lClientList;
};

static CGateServer g_GateServer;
