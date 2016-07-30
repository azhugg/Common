//
// 2016-03-17, jjuiddong
//
// ARToolkit���� �ν��� QRCode�� �����Ѵ�.
// 
//

#pragma once


namespace ar
{

	class cQRCode
	{
	public:
		cQRCode();
		virtual ~cQRCode();

		void Init(graphic::cRenderer &renderer, graphic::cModel *model, graphic::cText *text);
		void Init2(graphic::cRenderer &renderer, graphic::cCube2 *cube, graphic::cText *text);
		void Render(graphic::cRenderer &renderer);


	public:
		int m_id; // QRCode ID 
		Matrix44 m_tm;	// QRCode�� �ν��� ��, 3���� �󿡼��� ī�޶� ���

		// display
		graphic::cCube2 *m_cube;
		graphic::cModel *m_model; // reference
		graphic::cText *m_text; // reference
	};

}
