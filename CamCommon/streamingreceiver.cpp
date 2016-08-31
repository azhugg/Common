
#include "stdafx.h"
#include "streamingreceiver.h"


using namespace cv;
using namespace cvproc;


cStreamingReceiver::cStreamingReceiver() 
	: m_rcvBuffer(NULL)
	, m_chunkSize(0)
	, m_currentChunkIdx(0)
	, m_isBeginDownload(false)
	, m_isLog(false)
{
}

cStreamingReceiver::~cStreamingReceiver()
{
	SAFE_DELETEA(m_rcvBuffer);
}


bool cStreamingReceiver::Init(const bool isUDP, const string &ip, const int port, const int networkCardIdx,
	const int gray, const int compressed, const int jpgComQuality, const int fps)
{
	m_isUDP = isUDP;

	m_udpServer.Close();
	m_tcpClient.Close();
	
	// �켱 TCP/IP�� ������ ��, udp/tcp ������ ���� �Ѵ�.
	if (!m_tcpClient.Init(ip, port, g_maxStreamSize, 512, 10))
		return false;

	// UDP�� ���� �ޱ�����, ���� ��ǻ���� IP�� ��Ʈ��ȣ�� �����Ѵ�.
	// ��Ʈ��ȣ�� tcp/ip��Ʈ�� +1 �� ���̴�.
	if (isUDP)
	{
		m_rcvUDPIp = common::GetHostIP(networkCardIdx);
		std::cout << "UDP Receive IP = " << m_rcvUDPIp << std::endl;

		sStreamingProtocol data;
		data.protocol = 100;
		data.type = 1;
		data.port = port + 1;
		data.uip = inet_addr(m_rcvUDPIp.c_str());
		m_tcpClient.Send((BYTE*)&data, sizeof(data));

		m_udpServer.SetMaxBufferLength(g_maxStreamSize);
		m_udpServer.m_sleepMillis = 10;
		if (!m_udpServer.Init(0, port + 1))
		{
			// udp�� ���ŵǴ� ���� �����ߴٸ�, tcp/ip�� �޴´�.
			m_isUDP = false;
		}
	}
	else
	{
		sStreamingProtocol data;
		data.protocol = 100;
		data.type = 0;
		m_tcpClient.Send((BYTE*)&data, sizeof(data));
	}


	{
		// gray, compressed, jpgCompQuality ���� ��������
		Sleep(500);
		sStreamingProtocol data;
		data.protocol = 101;
		data.gray = gray==1? true : false;
		data.compressed = compressed==1? true : false;
		data.compQuality = jpgComQuality;
		data.fps = fps;
		m_tcpClient.Send((BYTE*)&data, sizeof(data));
	}

	if (!m_rcvBuffer)
		m_rcvBuffer = new BYTE[g_maxStreamSize];

	return true;
}


// return value : 0 = �ֱٿ� ���� �̹��� ������ ����.
//						   1 = ������ ������Ʈ ��
int cStreamingReceiver::Update()
{
	RETV(!IsConnect(), 0);

	const static int sizePerChunk = g_maxStreamSize - sizeof(sStreamingData) - sizeof(network::cPacketQueue::sHeader); // size per chunk

	int len = 0;
	if (m_isUDP)
	{
		len = m_udpServer.GetRecvData(m_rcvBuffer, g_maxStreamSize);
	}
	else
	{
		network::sSockBuffer packet;
		if (m_tcpClient.m_recvQueue.Front(packet))
		{
			len = packet.actualLen;
			memcpy(m_rcvBuffer, packet.buffer, packet.actualLen);
			m_tcpClient.m_recvQueue.Pop();
		}
	}

	if (len <= 0)
		return 0;

	sStreamingData *packet = (sStreamingData*)m_rcvBuffer;
	if (m_oldCompressed != (int)packet->isCompressed)
	{
		m_oldCompressed = (int)packet->isCompressed;
		std::cout << "compressed = " << (int)packet->isCompressed << std::endl;
	}

	// ���� ������ �ٲ��� ��.
	if ((m_oldGray != (int)packet->isGray) ||
		(m_oldRows != packet->height) ||
		(m_oldCols != packet->width) ||
		(m_oldFlags != packet->flag))
	{
		m_oldGray = (int)packet->isGray;
		m_oldRows = packet->height;
		m_oldCols = packet->width;
		m_oldFlags = packet->flag;

		std::cout << "gray = " << (int)packet->isGray << std::endl;

		if (packet->isGray)
		{
 			m_finalImage = Mat(packet->height, packet->width, CV_8UC1);
			m_cloneImage = Mat(packet->height, packet->width, CV_8UC1);
		}
		else
		{
			m_finalImage = Mat(packet->height, packet->width, packet->flag);
			m_cloneImage = Mat(packet->height, packet->width, packet->flag);
		}
	}

	int reVal = 0;
	if (packet->chunkSize == 1)
	{
		m_tempBuffer.resize(packet->imageBytes);
		packet->data = m_rcvBuffer + sizeof(sStreamingData);

		if (packet->isCompressed)
		{
			if (packet->imageBytes <= len)
			{
				memcpy((char*)&m_tempBuffer[0], packet->data, packet->imageBytes);
				cv::imdecode(m_tempBuffer, IMREAD_UNCHANGED, &m_finalImage);
			}
		}
		else
		{
			// �ٷ� decode �� �����ؼ� �����Ѵ�.
			memcpy((char*)m_finalImage.data, packet->data, packet->imageBytes);
		}

		reVal = 1;
		m_isBeginDownload = false;
		m_chunkSize = 1;
		m_checkRcv[0] = true;
		if ((m_cloneImage.step[0] * m_cloneImage.rows) == (m_finalImage.step[0] * m_finalImage.rows))
			memcpy(m_cloneImage.data, m_finalImage.data, m_finalImage.step[0]* m_finalImage.rows);

		if (m_isLog)
			dbg::Log("recv packet id=%d, chunksize=1, chunk=%d, copyLen=%d\n", 
				packet->id, packet->chunkIndex, packet->imageBytes);
	}
	else if (packet->chunkSize > 1)
	{
		m_chunkSize = packet->chunkSize;
		m_currentChunkIdx = packet->chunkIndex;

		// ������ �ٲ�� �� ��, ���� �̹��� ù�� ° ûũ�� �޾��� ��..
		if (m_oldId != packet->id)
		{
			m_tempBuffer.resize(packet->imageBytes);
			ZeroMemory(m_checkRcv, sizeof(m_checkRcv));
			m_oldId = packet->id;
			m_isBeginDownload = true;
		}

		packet->data = m_rcvBuffer + sizeof(sStreamingData);
		char *dst = (char*)&m_tempBuffer[0] + packet->chunkIndex * sizePerChunk;
		const int copyLen = max(0, (len - (int)sizeof(sStreamingData)));
		memcpy(dst, packet->data, copyLen);

		if (m_isLog)
			dbg::Log("recv packet id=%d, chunkSize=%d, chunk=%d, copyLen=%d\n",
				packet->id, packet->chunkSize, packet->chunkIndex, copyLen);

		if (packet->chunkIndex < 512)
			m_checkRcv[packet->chunkIndex] = true;

		// ��� ûũ�� �޾Ҵٸ�~
		if ((packet->chunkSize - 1) == packet->chunkIndex)
		{
			if (packet->isCompressed)
			{
				if (!m_tempBuffer.empty())
					cv::imdecode(m_tempBuffer, IMREAD_UNCHANGED, &m_finalImage);
			}
			else
			{
				// �ٷ� final image �� �����ؼ� �����Ѵ�.
				if (!m_tempBuffer.empty())
					memcpy((char*)m_finalImage.data, (char*)&m_tempBuffer[0], m_tempBuffer.size());
			}

			reVal = 1;
			m_isBeginDownload = false;
			m_chunkSize = packet->chunkSize;
			m_currentChunkIdx = packet->chunkSize;
			if ((m_cloneImage.step[0] * m_cloneImage.rows) == (m_finalImage.step[0] * m_finalImage.rows))
				memcpy(m_cloneImage.data, m_finalImage.data, m_finalImage.step[0] * m_finalImage.rows);
		}
	}

	return reVal;
}


// �̹����� �ް� �ִ� ���̶��, true �� �����Ѵ�.
// maxLen = chunkSize
// readLen = chunkIndex
bool cStreamingReceiver::DownLoadState(OUT int &maxLen, OUT int &readLen)
{
	maxLen = m_chunkSize;
	readLen = m_currentChunkIdx;
	return m_chunkSize != m_currentChunkIdx;
}


bool cStreamingReceiver::IsConnect()
{
	if (m_isUDP)
		return m_udpServer.IsConnect();
	return m_tcpClient.IsConnect();
}


void cStreamingReceiver::Close()
{
	m_udpServer.Close();
	m_tcpClient.Close();
}


// ������ ���� ������ ��Ŷ�� �����Ѵ�.
string cStreamingReceiver::CheckError()
{
	stringstream ss;
	for (int i = 0; i < m_chunkSize; ++i)
	{
		if (!m_checkRcv[i])
			ss << "[" << i << "]";
	}

	string reval = ss.str();
	return reval.empty() ? "all receive" : "missing >> " + reval;
}
