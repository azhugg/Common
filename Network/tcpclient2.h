//
// 2016-07-05, jjuiddong
// 
// cTCPClient �� ��ü�ϱ����� ���������.
// Connection �� ������� ó���Ѵ�.
//
#pragma once

#include "packetqueue.h"

namespace network
{

	class cTCPClient2
	{
	public:
		cTCPClient2();
		virtual ~cTCPClient2();

		bool Init(const string &ip, const int port,
			const int packetSize = 512, const int maxPacketCount = 10, const int sleepMillis = 30,
			const bool isIgnoreHeader = false);
		void Send(BYTE *buff, const int len);
		void Close();
		bool IsConnect() const;
		bool IsFailConnection() const;


	public:
		enum STATE {
			READYCONNECT,  // Ŀ�ؼ� ��, ������ �����ϱ� ����.
			TRYCONNECT,  // ������ ������ ���� Ŀ�ؼ� ��û ����
			CONNECT,  // Ŀ�ؼ� ����
			DISCONNECT, // Ŀ�ؼ� ����
			CONNECT_ERROR, // Ŀ�ؼ� ����, �ַ� ����ȭ �������� �߻�
		};
		 
		STATE m_state;
		string m_ip;
		int m_port;
		SOCKET m_socket; // server socket
		int m_maxBuffLen;
		cPacketQueue m_sendQueue;
		cPacketQueue m_recvQueue;

		std::thread m_thread;
		int m_sleepMillis;
		int m_recvBytes; // debug
	};


	inline bool cTCPClient2::IsConnect() const { return m_state == CONNECT; }
	inline bool cTCPClient2::IsFailConnection() const {return m_state == DISCONNECT; }
}