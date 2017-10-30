// �ΰ����� ��ü �������̽�
// �ΰ������� �����Ϸ��� ��ü�� �� �������̽��� ��ӹ޾ƾ� �Ѵ�.
#pragma once


namespace ai
{
	static Transform g_tempTransform;

	template<class T>
	class iActorInterface
	{
	public:
		iActorInterface(T *p)  : m_ptr(p) {}
		virtual ~iActorInterface() {}

		virtual Transform& aiGetTransform() { return g_tempTransform; }
		virtual void aiSetAnimation(const Str64 &animationName) {}


	public:
		T *m_ptr;
	};

}
