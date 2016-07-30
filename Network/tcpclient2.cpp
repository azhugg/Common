
#include "stdafx.h"
#include "tcpclient2.h"
#include <iostream>
#include <process.h> 

using namespace std;
using namespace network;

void TCPClient2ThreadFunction(network::cTCPClient2 *client);


cTCPClient2::cTCPClient2()
	: m_sleepMillis(30)
	, m_maxBuffLen(BUFFER_LENGTH)
	, m_recvBytes(0)
	, m_state(DISCONNECT)
{
}

cTCPClient2::~cTCPClient2()
{
	Close();
}


bool cTCPClient2::Init(const string &ip, const int port,
	const int packetSize, const int maxPacketCount, const int sleepMillis,
	const bool isIgnoreHeader)
	// packetSize = 512, maxPacketCount = 10, int sleepMillis = 30, isIgnoreHeader=false
{
	Close();

	m_ip = ip;
	m_port = port;
	m_sleepMillis = sleepMillis;
	m_maxBuffLen = packetSize;

	m_state = READYCONNECT;
	m_thread = std::thread(TCPClient2ThreadFunction, this);

	if (!m_recvQueue.Init(packetSize, maxPacketCount, isIgnoreHeader))
	{
		Close();
		return false;
	}

	if (!m_sendQueue.Init(packetSize, maxPacketCount, isIgnoreHeader))
	{
		Close();
		return false;
	}

	//cout << "Error!! Connect TCP/IP Client ip=" << ip << ", port=" << port << endl;
 	//common::dbg::ErrLog("Error!! Connect TCP/IP Client ip= %s, port=%d \n", ip.c_str(), port);
	return true;
}


void cTCPClient2::Send(BYTE *buff, const int len)
{
	RET(!IsConnect());
	m_sendQueue.Push(m_socket, buff, len);
}


void cTCPClient2::Close()
{
	m_state = DISCONNECT;
	if (m_thread.joinable()) // ������ ����.
		m_thread.join();

	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
	m_state = DISCONNECT;
}


void TCPClient2ThreadFunction(network::cTCPClient2 *client)
{
	if (client->m_state != cTCPClient2::READYCONNECT)
	{
		client->m_state = cTCPClient2::CONNECT_ERROR;
		return;
	}

	client->m_state = cTCPClient2::TRYCONNECT;

	if (!network::LaunchTCPClient(client->m_ip, client->m_port, client->m_socket))
	{
		client->m_state = cTCPClient2::DISCONNECT;
		return;
	}

	client->m_state = cTCPClient2::CONNECT;

	char *buff = new char[client->m_maxBuffLen];
	const int maxBuffLen = client->m_maxBuffLen;

	while (cTCPClient2::CONNECT == client->m_state)
	{
		const timeval t = { 0, client->m_sleepMillis };

		//-----------------------------------------------------------------------------------
		// Receive Packet
		fd_set readSockets;
		FD_ZERO(&readSockets);
		FD_SET(client->m_socket, &readSockets);
		const int ret = select(readSockets.fd_count, &readSockets, NULL, NULL, &t);
		if (ret != 0 && ret != SOCKET_ERROR)
		{
			const int result = recv(readSockets.fd_array[0], buff, maxBuffLen, 0);
			if (result == SOCKET_ERROR || result == 0) // ���� ��Ŷ����� 0�̸� ������ ������ ����ٴ� �ǹ̴�.
			{
				// error occur
				client->m_state = cTCPClient2::DISCONNECT;
				//cout << "cTCPClient2 socket close " << endl;
			}
			else
			{
				//cout << "recv packet size = " << result << endl;
				client->m_recvBytes += result;
				client->m_recvQueue.Push(readSockets.fd_array[0], (BYTE*)buff, result, true);
			}
		}
		//-----------------------------------------------------------------------------------


		//-----------------------------------------------------------------------------------
		// Send Packet
		client->m_sendQueue.SendAll();
		//-----------------------------------------------------------------------------------
	}

	delete[] buff;
}