#include "stdafx.h"
#include "Thread.h"
#include "Task.h"
#include <boost/bind.hpp>

using namespace common;

namespace common
{
	unsigned __stdcall ThreadProcess(void *pThreadPtr )
	{
		cThread *pThread = (cThread*)pThreadPtr;
		pThread->Run();
		pThread->Exit();
		return 0;
	}

	template<class T>
	static bool IsSameId(T *p, int id)
	{
		if (!p) return false;
		return p->m_Id == id;
	}
}


cThread::cThread(const std::string &name) :
	m_state(eState::WAIT)
,	m_hThread(NULL)
,	m_name(name)
{
}

cThread::~cThread()
{
	Clear();
}


//------------------------------------------------------------------------
//  ������ ����
//------------------------------------------------------------------------
void cThread::Start()
{
	if (eState::RUN != m_state)
	{
		m_state = eState::RUN;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProcess, this, 0, NULL);
	}
}


//------------------------------------------------------------------------
// ������ ����
//------------------------------------------------------------------------
void cThread::Terminate(const int milliSeconds) //milliSeconds = -1
{
	m_state = eState::END;
	DWORD timeOutTime = (milliSeconds>=0)? milliSeconds : INFINITE;
	WaitForSingleObject(m_hThread, timeOutTime);
	CloseHandle(m_hThread);
	m_hThread = NULL;
}


//------------------------------------------------------------------------
// �����忡�� �޼����� ������.
// rcvTaskId : ���� �½�ũ ���̵� ('0' �̶�� �����尡 �޴´�.)
//			   -1 : �ܺη� ���� �޼����� ����
//------------------------------------------------------------------------
void cThread::Send2ThreadMessage( threadmsg::MSG msg, WPARAM wParam, LPARAM lParam, LPARAM added)
{
	AutoCSLock cs(m_msgCS);
	m_threadMsgs.push_back( SExternalMsg(-1, (int)msg, wParam, lParam, added) );
}


//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
void cThread::Send2ExternalMessage( int msg, WPARAM wParam, LPARAM lParam, LPARAM added )
{
	AutoCSLock cs(m_msgCS);
	m_externalMsgs.push_back( SExternalMsg(-1, msg, wParam, lParam, added) );
}


//------------------------------------------------------------------------
// �����尡 ���� �޼����� �����Ѵ�.
// �޼����� ���ٸ� false�� �����Ѵ�.
//------------------------------------------------------------------------
bool cThread::GetThreadMsg( OUT SExternalMsg *out
	, eMessageOption::Enum opt  // = eMessageOption::REMOVE
)
{
	if (!out)
		return false;

	bool reval;
	{
		AutoCSLock cs(m_msgCS);
		if (m_threadMsgs.empty())
		{
			reval = false;
		}
		else
		{
			*out = m_threadMsgs.front();
			if (eMessageOption::REMOVE == opt)
				m_threadMsgs.pop_front();
			reval = true;
		}
	}
	return reval;
}


//------------------------------------------------------------------------
// �����忡�� �ܺη� ���ϴ� �޼����� �����Ѵ�.
// �޼����� ���ٸ� false�� �����Ѵ�.
//------------------------------------------------------------------------
bool cThread::GetExternalMsg( OUT SExternalMsg *out
	, eMessageOption::Enum opt // = eMessageOption::REMOVE
) 
{
	if (!out)
		return false;

	bool reval;
	{
		AutoCSLock cs(m_msgCS);
		if (m_externalMsgs.empty())
		{
			reval = false;
		}
		else
		{
			*out = m_externalMsgs.front();
			if (eMessageOption::REMOVE == opt)
				m_externalMsgs.pop_front();
			reval = true;
		}
	}
	return reval;
}


//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
bool cThread::AddTask(cTask *task)
{
	RETV(!task, false);

	AutoCSLock cs(m_containerCS);
	task->m_pThread = this;
	m_addTasks.push_back( task );
	return true;
}


//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
bool cThread::RemoveTask(const int id)
{		
	AutoCSLock cs(m_containerCS);
	m_removeTasks.push_back(id);
	return true;
}


int cThread::GetTaskCount()
{
	AutoCSLock cs(m_taskCS);
	return m_tasks.size();
}


/**
@brief  taskId�� �ش��ϴ� task�� ����.
*/
cTask*	cThread::GetTask(const int taskId)
{
	//AutoCSLock cs(m_taskCS); ����ȭ ���� �ʴ´�.
	auto it = find_if(m_tasks.begin(), m_tasks.end(), IsTask(taskId));
	if (m_tasks.end() == it)
		return NULL; // ���ٸ� ����
	return *it;
}


//------------------------------------------------------------------------
// �������� ������ Ŭ���� ����
//------------------------------------------------------------------------
void cThread::Clear()
{
	{
		AutoCSLock cs(m_taskCS);
		auto it = m_tasks.begin();
		while (m_tasks.end() != it)
		{
			cTask *p = *it++;
			delete p;
		}
		m_tasks.clear();
	}

	{
		AutoCSLock cs(m_containerCS);
		for (auto &p : m_addTasks)
			delete p;
		m_addTasks.clear();
	}

	{
		AutoCSLock cs2(m_msgCS);
		m_threadMsgs.clear();
		m_externalMsgs.clear();
	}

	CloseHandle(m_hThread);
}


//------------------------------------------------------------------------
// ������ ����
// Task�� �����Ų��.
//------------------------------------------------------------------------
void cThread::Run()
{
	while (eState::RUN == m_state)
	{
		//1. Add & Remove Task
		UpdateTask();

		if (m_tasks.empty()) // break no task
			break;

		//2. Task Process
		{
			AutoCSLock cs(m_taskCS);
			auto it = m_tasks.begin();
			while (m_tasks.end() != it)
			{
				cTask *task = *it;
				if (cTask::eRunResult::END == task->Run())
				{
					// finish task , remove taks
					it = m_tasks.erase(it);
					delete task;
				}
				else
				{
					++it;
				}
			}
		}

		//3. Message Process
		DispatchMessage();

		Sleep(1);
	}

	// ���������� �� �޼����� ���������� ó���Ѵ�.
	DispatchMessage();
}


//------------------------------------------------------------------------
// call exit thread
//------------------------------------------------------------------------
void	cThread::Exit()
{
	m_state = eState::END;
}


void cThread::UpdateTask()
{
	AutoCSLock cs(m_containerCS);
	for (auto &p : m_addTasks)
	{
		auto it = find_if(m_tasks.begin(), m_tasks.end(), IsTask(p->m_Id));
		if (m_tasks.end() == it) // not exist
			m_tasks.push_back(p);
		else
			assert(0); // already exist
	}
	m_addTasks.clear();

	for (auto &id : m_removeTasks)
	{
		auto it = find_if(m_tasks.begin(), m_tasks.end(), IsTask(id));
		if (m_tasks.end() != it)
		{
			cTask *p = *it;
			m_tasks.remove_if(IsTask(id));
			delete p;
		}
	}
	m_removeTasks.clear();
}


//------------------------------------------------------------------------
// ����� �޼������� �½�ũ�� ������.
//------------------------------------------------------------------------
void cThread::DispatchMessage()
{
	AutoCSLock cs(m_msgCS);
	auto it = m_threadMsgs.begin();
	while (m_threadMsgs.end() != it)
	{
		if (threadmsg::TASK_MSG == it->msg) // task message
		{
			{
				AutoCSLock cs(m_taskCS);
				auto t = find_if(m_tasks.begin(), m_tasks.end(), 
					boost::bind( &IsSameId<cTask>, _1, it->wParam) );
				if (m_tasks.end() != t)
				{
					(*t)->MessageProc((threadmsg::MSG)it->msg, it->wParam, it->lParam, it->added);
				}
				else
				{
					dbg::ErrLog("cThread::DispatchMessage Not Find Target Task\n");
				}
			}
		}
		else // Thread���� �� �޼���
		{
			MessageProc((threadmsg::MSG)it->msg, it->wParam, it->lParam, it->added);
		}
		++it;
	}
	m_threadMsgs.clear();
}


//------------------------------------------------------------------------
// Message Process
//------------------------------------------------------------------------
void	cThread::MessageProc( threadmsg::MSG msg, WPARAM wParam, LPARAM lParam, LPARAM added )
{
	switch (msg)
	{
	case threadmsg::TERMINATE_TASK:
		{
			// terminate task of id wParam
			{
				AutoCSLock cs(m_taskCS);
				auto it = std::find_if( m_tasks.begin(), m_tasks.end(), 
					bind( &IsSameId<common::cTask>, _1, (int)wParam) );
				if (m_tasks.end() != it)
				{
					delete *it;
					m_tasks.erase(it);
				}
			}
		}
		break;
	}
}


bool cThread::IsRun()
{
	return m_state == eState::RUN;
}
