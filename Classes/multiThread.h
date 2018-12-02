#pragma once
#include <QThread>
extern class Frame;

// Dynamic Loading
class CFramesLoaderThread : public QThread
{
	Q_OBJECT
public:
	friend class Frame;

	Frame * objFrame;

	void setFrameObject(Frame * input);

	void run() override;

	void interrupt();

	void jobForFrame();

	bool isStoped()
	{
		return m_isStopped;
	}

private:
	bool m_isStopped = true;

public slots:
	void gotoWork()
	{
		printf("start worker \n");
		start();
	}
};
