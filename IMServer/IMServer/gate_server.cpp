#include <iostream>
#include <string.h>

#include "stdafx.h"
#include "gate_server.h"
#include "login_server.h"

CGateServer::CGateServer()
{
	// ����������ַ
	m_strIPAddress = "";
	m_strPort = "8888";

	// ��ʼ����������ַ�Ͷ˿�
	m_ServerAddr.sin_family = PF_INET;
	m_ServerAddr.sin_port = htons(std::stoi(m_strPort));
	m_ServerAddr.sin_addr.s_addr = inet_addr(m_strIPAddress.c_str());

	// ��ʼ��socket
	m_uListener = 0;

	m_uEpollFd = 0;
}


CGateServer::~CGateServer()
{
}

bool CGateServer::InitServer()
{
	// ��������socket
	m_uListener = socket(PF_INET, SOCK_STREAM, 0);
	if (m_uListener < 0)
	{
		perror("listener");
		exit(-1);
	}

	// �󶨵�ַ
	if (bind(m_uListener, (struct sockaddr *)&m_ServerAddr, sizeof(m_ServerAddr)) < 0)
	{
		perror("bind error");
		exit(-1);
	}

	// ����
	int ret = listen(m_uListener, 5);
	if (ret < 0)
	{
		perror("listen error");
		exit(-1);
	}

	// ���ں��д����¼���
	m_uEpollFd = epoll_create(EPOLL_SIZE);

	if (m_uEpollFd < 0)
	{
		perror("epoll fd error");
		exit(-1);
	}

	// ���¼���������Ӽ����¼�
	if (!_AddFd(m_uEpollFd, m_uListener, true))
	{
		return false;
	}
	return true;
}

bool CGateServer::CloseServer()
{
	// �ر�socket
	close(m_uListener);

	// �ر�epoll����
	close(m_uEpollFd);
	return true;
}

bool CGateServer::_AddFd(int epollfd, int fd, bool enable_et)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	if (enable_et)
	{
		ev.events = EPOLLIN | EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
	return true;
}

bool CGateServer::Run()
{
	// epoll �¼�����
	static struct epoll_event events[EPOLL_SIZE];

	while (true)
	{
		// epoll_events_count��ʾ�����¼�����Ŀ
		int epoll_events_count = epoll_wait(m_uEpollFd, events, EPOLL_SIZE, -1);
		if (epoll_events_count < 0)
		{
			perror("epoll failure");
			break;
		}

		// ������epoll_events_count�������¼�
		for (int i = 0; i < epoll_events_count; ++i)
		{
			int sockfd = events[i].data.fd;
			// ���û�����
			if (sockfd == m_uListener)
			{
				struct sockaddr_in client_address;
				socklen_t client_addrLength = sizeof(struct sockaddr_in);
				int clientfd = accept(m_uListener, (struct sockaddr*)&client_address, &client_addrLength);
				std::cout << "client connection from: " << inet_ntoa(client_address.sin_addr) << ":"
					<< ntohs(client_address.sin_port) << ", clientfd = " << clientfd << std::endl;

				_AddFd(m_uEpollFd, clientfd, true);

				// �������list�����û�����
				m_lClientList.push_back(clientfd);
			}
			// �����û���������Ϣ
			else
			{
				RecvUserInfo(sockfd);
			}
		}

	}
	return true;
}

bool CGateServer::RecvUserInfo(int uClientFd)
{
	int message_id = c2s_message_begin;
	ssize_t size = recv(uClientFd, (void*)&message_id, sizeof(message_id), 0);

	int head_size = sizeof(message_id);
	int body_size = 0;
	char *pPackage = nullptr;

	switch (message_id)
	{
	case c2s_login:
		body_size = sizeof(C2S_LOGIN);
		pPackage = (char*)malloc(sizeof(message_id) + body_size);
		if (!_GetPackage(uClientFd, body_size, message_id, pPackage))
		{
			return false;
		}
		if (!g_LoginServer.RecvLoginInfo(uClientFd, pPackage))
		{
			return false;
		}
		break;
	default:
		break;
	}

}

bool CGateServer::_GetPackage(int uClientFd, int ubody_size, int uhead, char* pPackage)
{
	void *p = malloc(ubody_size);
	ssize_t size = recv(uClientFd, p, ubody_size, 0);

	if (-1 == size || 0 == size)
	{
		return false;
	}

	memcpy(pPackage, (void*)&uhead, sizeof(uhead));
	memcpy(pPackage + sizeof(uhead), p, ubody_size);

	return true;
}