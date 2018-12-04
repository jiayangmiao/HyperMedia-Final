#pragma once
#include <QtWidgets/QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinbox>
#include <QObject>
#include <QDialog>
#include <QString>
#include <QCheckBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLayout>
#include <QPainter>
#include <QSizePolicy>
#include <QList>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <cstring>
#include <string>
#include <QTimer>
#include <QEventLoop>
#include <time.h> 
#include <QMediaPlayer>
#include <QBuffer>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainterPath>

#include "multiThread.h"
#include "HyperLinkForFrame.h"


#include <iostream>
#include "../Library/rapidxml/rapidxml.hpp"
#include "../Library/rapidxml/rapidxml_iterators.hpp"
#include "../Library/rapidxml/rapidxml_utils.hpp"
#include "../Library/rapidxml/rapidxml_print.hpp"

#include "HyperMediaLink.h"
#include "HyperMediaLinkFast.h"

typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int DWORD;
const int iFrameWidth = 352;
const int iFrameHeight = 288;

class Frame : public QWidget
{
	Q_OBJECT
private:
	
	int m_iLastLoadedFrame = 0;
	int m_iMaxFrame = 0;
	int m_iFrameWidth = 0;
	int m_iFrameHeight = 0;
	int m_iInterval = 33;
	const int m_iCaliInterval = 1000;
	int m_iInitialLoadedFrameSize;

	bool m_bVideoIsLoaded = false;
	

	
	std::string m_sVideoName;
	std::string m_sVideoSuffix = ".rgb";// ".rgb";
	std::string m_sAudioSuffix = ".wav";
	std::string m_sMetaDataSuffix = ".xml";

	bool m_bAudioLoaded = false;
	
	QTimer timer;
	QTimer m_caliTimer;
	QPainter paint;
	QMediaPlayer *audioPlayer;

	BYTE *pRData;
	BYTE *pGData;
	BYTE *pBData;

public:
	Frame(QWidget *parent = Q_NULLPTR);
	Frame(int iMaxframe, int iFrameWidth, int iFrameHeight,QWidget *parent = Q_NULLPTR);
	~Frame();
	void paintEvent(QPaintEvent *e);
	QList<QFileInfo> fileList;
	QDir dirPath;
	DWORD ** pFrames;
	DWORD * pFrame;
	std::string m_sRootFolder = "";// "D:\\Downloads\\London\\London\\LondonOne\\LondonOne";
	int m_iCurrentFrame = 0;
	//bool * pFramesLoadFlag;// is not used
	bool m_bIsStopped = true;

	void setFileList(QString path);
	void LoadAudio();
	void LoadFrame(int FrameNum);
	void LoadAllFrame();
	int initMemory(int iFrameNum, int width, int height);
	int initMemory();
	int freeMemory();
	void Init();
	void setBasic(int iMaxframe, int iFrameWidth, int iFrameHeight, int iFps, int iCacheSize, int iInitialLoadedFrameSize = 100);
	void setVideoName(QString videoName);

//// For Editor /////////////////////////////////////////////////////////
// write back to m_mCurrentLink ?

private:
	bool m_bEnableEditRect = true;

public:
	void enableEditRect()
	{
		m_bEnableEditRect = true;
	}

	void disableEditRect()
	{
		m_bEnableEditRect = false;
	}

	bool getEnableEditRect()
	{
		return m_bEnableEditRect;
	}

	bool isCloseTo(int a, int b)
	{
		if (abs(a - b) <= 5)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	std::pair<int, QRect> m_mRectBeingEdited;
	bool m_bEditStartPointSet = false;
	bool m_bEditEndPointSet = false;
	bool m_bRectSelected = false;

public slots:
	void resetRectBeingEdited()
	{
		m_mRectBeingEdited.first = -1;
		m_bEditStartPointSet = false;
		m_bEditEndPointSet = false;
		m_bRectSelected = false;
		m_mRectBeingEdited.second.setWidth(0);
		m_mRectBeingEdited.second.setHeight(0);
		m_mRectBeingEdited.second.setX(0);
		m_mRectBeingEdited.second.setY(0);
		update();
	}

signals:
	void newRectDrawn(QRect);

/////End for Editor////////////////////////////////////////////////////////////


//// HyperLink///////////////////////////////////////////////////////////
private:
	bool m_bIsPaintRect = false;
	int m_iImageOffset_x;
	int m_iImageOffset_y;
	double m_dImageScalor_x;
	double m_dImageScalor_y;
	double m_bEnableJump = true;

signals:
	void linkSelected(std::map<std::string, HyperLinkForFrame *>);
	void requestJump(std::string, int);
	void canEnablePlayerUI(bool);

public:
	void loadMetaData();
	void generateListAndMaps(std::list<HyperMediaLink *>);
	std::map<std::string, HyperLinkForFrame *> m_mCurrentLink;
	std::map<int, std::list<HyperMediaLink *>> fullMap;
	std::map<int, std::list<HyperMediaLinkFast *>> fastMap;

	std::list<HyperMediaLink *> links;// is used for store the pointer of links and is also used to delete the meomory in the end
	std::list<HyperMediaLinkFast *> fastLinks; // is used for store the pointer of links to delete in the end

	void freeLinkSystemMemory();
	void clearLinksList();
	void clearFastLinksList();
	void setCurrentLink();
	void updateCurrentLink();
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enableJump()
	{
		m_bEnableJump = true;
	}
	void disableJump()
	{
		m_bEnableJump = false;
	}
	bool ifJumpEnbale()
	{
		return m_bEnableJump;
	}

	void enablePaintRect()
	{
		m_bIsPaintRect = true;
	}

	void disablePaintRect()
	{
		m_bIsPaintRect = false;
	}

	bool isPaintRect()
	{
		return m_bIsPaintRect;
	}



//// END OF HyperLink//////////////////////////////////////////////////////////

//// START OF Dynamic Loading /////////////////////////////////////////////////////
private:
	
	DWORD ** m_pFrameCache;
	int * m_pFrameIndexOfCacheBlock;
	QContiguousCache<int> m_FrameCacheMap;
	int m_iCacheSize;
	int m_iStorageThreshold = 8; // 保留一部分使用过的图像
	int m_iLoadTimerInterval = 300;
	bool * m_pFrameStateFlag;

	CFramesLoaderThread loaderThread;
	QTimer loadTimer;

	int m_iHead;
	int m_iTail;
	bool m_bIsFull;
	bool m_bIsAlmostFull;

	int initCacheSystem(int );
	void freeCacheSystemMemory();

	DWORD * fetchFrameBlock(int iCurrentFrame);
	void _loadFrames(int startFrame, int iInitialNum);
	int checkAndLoadFrame(int iCurrentFrame);
	int checkFrameExisted(int iCurrentFrame);
	int _setFrameBlock(int, int);
	int _resetFrameBlock(int iTargetBlock);
	int _loadFrame(int iCurrentFrameNum, int iTargetBlock);
	int _setFrameIndexOfCacheBlock(int, int);
	int _prepop();
	int _backpop();
	int _prepend(int);
	int _append(int);
	bool _isFull();
	int _clear();

public:
	void loadInitialFrame(int iInitialNum);
	void loadInitialFrame(int, int);
	int forwardLoadFrameSeq();
	bool isFull();
	bool isAlmostFull();

////END OF Dynamic Loading////////////////////////////////////////////////////////////////

signals:
	void endPlay();
	void currentFrameUpdated(int);
	void rootFolderIsSet();

public slots:
	void signal()
	{
		qDebug() << "run "  << endl;
	}

	void signal2()
	{
		qDebug() << "wait " ;
	}

	void updateCurrentFrame()
	{
		//qDebug() << "updateCurrentFrame";
		if (m_iCurrentFrame < m_iMaxFrame)
		{
			m_iCurrentFrame = m_iCurrentFrame + 1;
		}
		else
		{
			m_iCurrentFrame = 1;
			timer.stop();
			m_caliTimer.stop();
			audioPlayer->stop();
			loadTimer.stop();
			_clear();
		}
		/*if ((checkFrameExisted(m_iCurrentFrame) == -2))
		{
			qDebug() << "-2 ";
			_loadFrames(m_iCurrentFrame, 10);
		}*/
		updateCurrentLink();
		//printf("update CurrentFrame: %d \n", m_iCurrentFrame);
		checkAndLoadFrame(m_iCurrentFrame);
		qint64 audiopos = (m_iCurrentFrame * 1000) / 30;
		if (abs(audioPlayer->position() - audiopos) >= 1500)
		{
			audioPlayer->setPosition(audiopos);
		}
		emit currentFrameUpdated(m_iCurrentFrame);
		m_bIsStopped = false;
		//emit currentFrameUpdated();
	}

	void setCurrentFrame(int i)
	{
		//qDebug("setCurrentFrame");

		
		if (m_bVideoIsLoaded == false)
		{
			return;
		}
		if (i == m_iCurrentFrame)
		{
			return;
		}
		else
		{
			m_iCurrentFrame = i;
		}
		

		qint64 audiopos = (m_iCurrentFrame * 1000) / 30;
		if (abs(audioPlayer->position() - audiopos) >= 1500)
		{
			audioPlayer->setPosition(audiopos);
		}
		if ( (checkFrameExisted(m_iCurrentFrame) == -1) || (checkFrameExisted(m_iCurrentFrame) == -2) )
		{
			qDebug() << "return -1 or -2 ";
			_loadFrames(m_iCurrentFrame, 10);
		}
		checkAndLoadFrame(m_iCurrentFrame);
		setCurrentLink();

		emit currentFrameUpdated(m_iCurrentFrame);
	}

	void PlayOrPause()
	{
		if (m_bVideoIsLoaded == false)
		{
		return;
		}

		if ((audioPlayer->state() == QMediaPlayer::StoppedState) || (audioPlayer->state() == QMediaPlayer::PausedState))
		{
			audioPlayer->play();
		}
		else
		{
			audioPlayer->pause();
		}

		if (timer.isActive())
		{
			timer.stop();
			m_caliTimer.stop();
			loadTimer.stop();
			audioPlayer->pause();
		}
		else
		{
			timer.start(m_iInterval);
			m_caliTimer.start(m_iCaliInterval);
			loadTimer.start(m_iLoadTimerInterval);
			audioPlayer->play();
		}
		m_bIsStopped = false;
	}

	void framePause()
	{
		audioPlayer->pause();
		timer.stop();
		m_caliTimer.stop();
		loadTimer.stop();
	}

	void Stop()
	{
		if (timer.isActive())
		{
			timer.stop();
			m_caliTimer.stop();	
		}
		loadTimer.stop();
		loaderThread.exit();
		m_iCurrentFrame = 1;
		checkAndLoadFrame(m_iCurrentFrame);
		emit currentFrameUpdated(m_iCurrentFrame);
		audioPlayer->stop();
		_clear();
		m_bIsStopped = true;
	}

	void calibrationTimer()
	{
		printf("calibration execute ");
		time_t rawtime;
		struct tm * timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		printf("The current date/time is: %s \n", asctime(timeinfo));

		timer.stop();
		timer.start(m_iInterval);
		
	}

	void updateAudioStatus(QMediaPlayer::MediaStatus status)
	{
		if ((status == QMediaPlayer::BufferedMedia)&&(m_bAudioLoaded==false))
		{
			printf("Audio Truely Loaded\n");
			m_bAudioLoaded = true;
			audioPlayer->stop();
			audioPlayer->setVolume(100);
		}
	}

	void startThread()
	{
		if (!(isFull()) && !(isAlmostFull()))
		{
			loaderThread.start();
		}
	}

	void setRootFolder(QString rootFolderName); 

	void LoadVideo(int startFrame = -1);
};