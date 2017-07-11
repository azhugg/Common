//
// 2017-07-11, jjuiddong
// ȭ�鿡 ��µǴ� ��� ��ü�� �� Ŭ�����κ��� ��ӹ޴´�.
//
#pragma once


namespace graphic
{
	class cRenderer;

	struct eNodeType {
		enum Enum { NONE, BONE, MESH, MODEL, TERRAIN };
	};

	class cNode2
	{
	public:
		cNode2(const int id, const StrId &name = "none", const eNodeType::Enum type=eNodeType::NONE);
		virtual ~cNode2();

		virtual bool Update(cRenderer &renderer, const float deltaSeconds);
		virtual bool Render(cRenderer &renderer, const Matrix44 &parentTm = Matrix44::Identity, const int flags = 1);

		bool AddChild(cNode2 *node);
		const cNode2* FindNode(const int id) const;
		const cNode2* FindNode(const StrId &name) const;
		bool RemoveChild(const int id);
		bool RemoveChild(cNode2 *rmNode);
		virtual void LostDevice() {}
		virtual void ResetDevice(cRenderer &renderer) {}
		virtual void Clear();


	public:
		int m_id;
		StrId m_name;
		bool m_isEnable; // if false, didn't show
		bool m_isShow;
		int m_flags; // default:1
		eNodeType::Enum m_nodeType;
		cNode2 *m_parent;
		vector<cNode2*> m_children;
		Transform m_transform;
	};

}
