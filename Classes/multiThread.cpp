#include "multiThread.h"
#include "Frame.h"

void CFramesLoaderThread:: setFrameObject(Frame * input)
{
	objFrame = input;
}

void CFramesLoaderThread:: run() 
{
	//printf("thread start \n");
	jobForFrame();
}

void CFramesLoaderThread::interrupt()
{
	requestInterruption();
}
void CFramesLoaderThread:: jobForFrame()
{
	int iLoop = 10;
	
	for (int i = 0; i < iLoop; i++)
	{
		if (isInterruptionRequested())
		{
			m_isStopped = true;
			printf("thread interruped ");
			exit();
			return;
		}
		if (!(objFrame->isAlmostFull()))
		{
			if (!isInterruptionRequested())
			{
				m_isStopped = false;
			}
			else
			{
				m_isStopped = true;
				printf("thread interruped ");
				exit();
				return;
			}
			printf(">");
			objFrame->forwardLoadFrameSeq();
			if (!isInterruptionRequested())
			{
				m_isStopped = false;
			}
			else
			{
				m_isStopped = true;
				printf("thread interruped ");
				exit();
				return;
			}
		}
		else
		{
			printf("thread stopped ");
			m_isStopped = true;
			exit();
			return;
		}
	}
	m_isStopped = true;
	exit();
	return;
}