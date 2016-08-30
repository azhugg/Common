//
// ��Ʈ���ֿ� ���õ� ������ �����Ѵ�.
//
#pragma once


namespace cvproc
{
	struct sStreamingData
	{
		unsigned char id;			// streaming id (���� ���̵𳢸� ����Ÿ�� ��ģ �Ŀ� ����Ѵ�.)
		unsigned short chunkSize;	// chunk size (�̹��� �뷮 ������ ������ ����Ÿ ûũ�� ����)
		unsigned short chunkIndex;	// chunk index
		unsigned short width;			// image width
		unsigned short height;			// image height
		unsigned int flag;				// Mat flag
		unsigned char isGray;		// gray 0 : 1
		unsigned char isCompressed;	// jpeg compressed 0 : 1
		int imageBytes;				// image size (byte unit)
		BYTE *data;					// image data
	};


	// ���ù��� �������� ���� ��� ��������
	// TCP/IP �θ� ���۵ȴ�.
	struct sStreamingProtocol
	{
		BYTE protocol;		// �������� Ÿ��
									// protocol = 100, ���� �������� ���� (TCP/IP) 
									// Receiver�� IP�� Port, ���� Ÿ�� �۽�

									// protocol = 101, gray, compressed, compressed quality, fps ����
									// gray, compressed, compQuality �� ���.
									//


		BYTE type;			// udp=1, tcp=0
		unsigned int uip; // ip address, type long
		int port;
		bool gray;
		bool compressed;
		int compQuality;
		int fps;					// �ʴ� ���� ������
	};


	// http://stackoverflow.com/questions/1098897/what-is-the-largest-safe-udp-packet-size-on-the-internet
	const static int g_maxStreamSize = 65507;
}
