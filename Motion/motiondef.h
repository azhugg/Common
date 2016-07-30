#pragma once

namespace motion
{

	// ����� �̵�� Ÿ��
	namespace MOTION_MEDIA 
	{
		enum TYPE 
		{
			UDP = 0,
			TCP,
			SERIAL,
			JOYSTICK, // only input
			MWAVE, // only input
			SHARED_MEM,
		};
	}

	
	namespace MODULE_STATE
	{
		enum TYPE
		{
			STOP,
			START,
		};
	};

}
