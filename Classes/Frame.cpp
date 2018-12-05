#include "Frame.h"
#include "HyperMediaLink.h"
#include "HyperMediaLinkFast.h"
#include <typeinfo>


void fulFillZero(std::string & sNum)
{
	//qDebug() << "sNum: " << sNum.data() << endl;
	if (sNum.size() == 1)
	{
		std::string sFill("000");
		sNum = sFill + sNum;
	}
	else if( sNum.size() == 2)
	{
		std::string sFill("00");
		sNum = sFill + sNum;
	}
	else if (sNum.size() == 3)
	{
		std::string sFill("0");
		sNum = sFill + sNum;
	}
	else if (sNum.size() == 4)
	{
		return;
	}

}

Frame::Frame( QWidget *parent) :
	QWidget(parent)
{
	timer.setTimerType(Qt::PreciseTimer);
	m_caliTimer.setTimerType(Qt::PreciseTimer);
	connect(&timer, SIGNAL(timeout()), this, SLOT(updateCurrentFrame()));
	connect(this, SIGNAL(currentFrameUpdated(int)), this, SLOT(repaint()));
	connect(&m_caliTimer, SIGNAL(timeout()), this, SLOT(calibrationTimer()));
	//connect(this, SIGNAL(rootFolderIsSet()), this, SLOT(LoadVideo()));  //signal: "rootFolderIsSet()" used to be emitted by setRootFolder()

	//connect(&loadTimer, SIGNAL(timeout()), this, SLOT(startThread()));
	loaderThread.setFrameObject(this);

	audioPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);
	connect(audioPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(updateAudioStatus(QMediaPlayer::MediaStatus)));
	m_iCurrentFrame = 1;

	m_mRectBeingEdited.first.first = 1;
	m_mRectBeingEdited.first.second = 9000;
}

void Frame::setBasic(int iMaxframe, int iFrameWidth, int iFrameHeight, int iFps,int iCacheSize, int iInitialLoadedFrameSize )
{
	m_iMaxFrame = iMaxframe;
	m_iFrameWidth = iFrameWidth;
	m_iFrameHeight = iFrameHeight;
	m_iInterval = 1000/iFps;
	m_iCacheSize = iCacheSize;
	m_iInitialLoadedFrameSize = iInitialLoadedFrameSize;
	//timer.start(m_iInterval);
	//m_caliTimer.start(1000);

}

void Frame::Init()
{
	initCacheSystem(m_iCacheSize);

}

void Frame::LoadVideo(int startFrame)
{
	if (m_sRootFolder.compare("") == 0)
	{
		QMessageBox::warning(this, "Error", "Please specify path to video");
		return;
	}

	m_sVideoName_old = m_sVideoName;
	if (m_bIsStopped == false)
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::critical(this, tr("Load Video Warning"), " Please Stop Video First!");
		return;
	}

	m_FrameCacheMap.clear();
	_clear();
	disablePaintRect();
	loadMetaData();
	m_bAudioLoaded = false;
	enablePaintRect();
	if (startFrame != -1)
	{
		m_iCurrentFrame = startFrame;
		loadInitialFrame(startFrame,m_iInitialLoadedFrameSize);
	}
	else
	{
		m_iCurrentFrame = 1;
		loadInitialFrame(m_iInitialLoadedFrameSize);
	}
	m_bVideoIsLoaded = true;
	LoadAudio();
	setCurrentLink();
	qDebug() << "setCurrentLink finished";
	qDebug() << "current Frame: " << m_iCurrentFrame;
	emit currentFrameUpdated(m_iCurrentFrame);
	qDebug() << "LoadVideo Finished";
	emit videoLoaded(true);
}


void Frame::reloadVideo()
{
	m_sVideoName_old = m_sVideoName;
	loadMetaData();
	emit videoLoaded(true);
}

Frame::~Frame()
{
	//free(pRData);
	//free(pGData);
	//free(pBData);
	//free(pFrame);
	//freeMemory();
	freeCacheSystemMemory();
	freeLinkSystemMemory();
	delete audioPlayer;
}

int Frame::initMemory(int iFrameNum, int width, int height)
{
	pFrames = (DWORD**)malloc(iFrameNum * sizeof(DWORD*));
	if (pFrames == NULL)
	{
		return -1;
	}

	DWORD uiFrameSize_pixel = width * height;

	for (int i = 0; i < iFrameNum; i++)
	{
		pFrames[i] = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	}

	/*pFramesLoadFlag = new bool[iFrameNum];

	memset(pFramesLoadFlag, false, iFrameNum );*/

	pFrame = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	pRData = (BYTE*)malloc(uiFrameSize_pixel);
	pGData = (BYTE*)malloc(uiFrameSize_pixel);
	pBData = (BYTE*)malloc(uiFrameSize_pixel);
}

int Frame::initMemory()
{
	pFrames = (DWORD**)malloc(m_iMaxFrame * sizeof(DWORD*));
	if (pFrames == NULL)
	{
		return -1;
	}

	DWORD uiFrameSize_pixel = m_iFrameWidth * m_iFrameHeight;

	for (int i = 0; i < m_iMaxFrame; i++)
	{
		pFrames[i] = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
		if (pFrames[i] == NULL)
		{
			return -1;
		}
	}

	//pFramesLoadFlag = new bool[m_iMaxFrame];
	/*if (pFramesLoadFlag == NULL)
	{
		return -1;
	}

	memset(pFramesLoadFlag, false, m_iMaxFrame);*/

	pFrame = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	pRData = (BYTE*)malloc(uiFrameSize_pixel);
	pGData = (BYTE*)malloc(uiFrameSize_pixel);
	pBData = (BYTE*)malloc(uiFrameSize_pixel);

	return 0;
}

int Frame::freeMemory()
{
	for (int i = 0; i < m_iMaxFrame; i++)
	{
		free(pFrames[i]);
	}

	free(pFrames);
	return 0;
}

void Frame:: LoadAllFrame()
{
	for (int i = 0; i < m_iMaxFrame; i++)
	{
		LoadFrame(i + 1);
		if (i % 100 == 0)
		{
			printf(">");
		}
		if (i % 1000 == 0)
		{
			printf("\n");
		}
	}
	printf("\nLOAD ALL FRAME SUCCEED!\n");
}


void Frame::LoadFrame( int iFrameNum )
{
	FILE *pf = NULL;
	DWORD uiFrameSize_pixel = m_iFrameWidth * m_iFrameHeight;

	const std::string root = m_sRootFolder;// "D:\\Downloads\\London\\London\\LondonOne\\LondonOne";
	const std::string suffix = m_sVideoSuffix;// ".rgb";

	DWORD * pFrame = pFrames[iFrameNum - 1];

	std::string sFileNum = std::to_string(iFrameNum);

	fulFillZero(sFileNum);

	std::string sFullFilePath = root + "\\" + m_sVideoName + sFileNum + suffix;


	if ((pf = fopen(sFullFilePath.data(), "rb")) == NULL)
	{
		printf("File coulkd not be opened ");
		qDebug() << sFullFilePath.data() << "\n";
		return;
	}

	int n = fread(pRData, uiFrameSize_pixel, 1, pf);
	n = fread(pGData, uiFrameSize_pixel, 1, pf);
	n = fread(pBData, uiFrameSize_pixel, 1, pf);

	if (n == 1)
	{
		for (unsigned int i = 0; i < uiFrameSize_pixel; ++i)
		{
			BYTE *pb = (BYTE *)(pFrame + i);
			pFrame[i] = pBData[i];
			pb[1] = pGData[i];
			pb[2] = pRData[i];
			pb[3] = 255;
		}
	}
	fclose(pf);
}

void Frame::LoadAudio()
{
	QString audioFileName = QString::fromStdString(m_sRootFolder+"\\" + m_sVideoName + m_sAudioSuffix);
	audioPlayer->setMedia(QMediaContent());
	audioPlayer->setMedia(QUrl::fromLocalFile(audioFileName));
	audioPlayer->setVolume(0);
	audioPlayer->play();
	m_bAudioLoaded = false;
	qDebug() << "LoadAudio finished";
}


void Frame::paintEvent(QPaintEvent *e)
{
	if (m_bVideoIsLoaded == false)
	{
		return;
	}
	paint.begin(this);

	DWORD * pFrame = fetchFrameBlock(m_iCurrentFrame); //pFrames[m_iCurrentFrame-1];

	QImage image = QImage((uchar*)pFrame, m_iFrameWidth, m_iFrameHeight, QImage::Format_RGB32);
	QSize size = image.size().scaled(this->rect().size(), Qt::KeepAspectRatio);
	QRect topPortion = QRect(QPoint((this->rect().size().width() - size.width()) / 2, (this->rect().size().height() - size.height()) / 2), size);
	paint.setRenderHint(QPainter::SmoothPixmapTransform, 1);
	paint.drawImage(topPortion, image);


	if (isPaintRect())
	{
		paint.setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap));
		std::map<std::string, HyperLinkForFrame *>::iterator it;

		m_iImageOffset_x = topPortion.x();
		m_iImageOffset_y = topPortion.y();
		m_dImageScalor_x = (1.0 * size.width()) / m_iFrameWidth;
		m_dImageScalor_y = (1.0 * size.height()) / m_iFrameHeight;

		//qDebug()<<"fullMap Size "<<fullMap.size();
		for (it = m_mCurrentLink.begin(); it != m_mCurrentLink.end(); ++it) {
			//std::cout << it->first << " | ";
			int X = topPortion.x() + ((it->second)->X * size.width()) / m_iFrameWidth;
			int Y = topPortion.y() + ((it->second)->Y * size.height()) / m_iFrameHeight;
			int Width = ((it->second)->width * size.width()) / m_iFrameWidth;
			int Height = ((it->second)->height * size.height()) / m_iFrameHeight;
			paint.drawRect(QRect(X, Y, Width, Height));
		}
	}

	if (getEnableEditRect())
	{
		if ( currentFrameIsInRange( m_mRectBeingEdited.first, m_iCurrentFrame) )
		{
			QRect rect = m_mRectBeingEdited.second;
			//printf("paintEventedit %d %d %d %d %d %d ", m_iCurrentFrame, m_mRectBeingEdited.first, rect.x(), rect.y(), rect.width(), rect.height());
			int X = m_iImageOffset_x + (rect.x() * m_dImageScalor_x);
			int Y = m_iImageOffset_y + (rect.y() * m_dImageScalor_y);
			int Width = (rect.width() * m_dImageScalor_x);
			int Height = (rect.height() * m_dImageScalor_y);
			paint.setPen(QPen(Qt::yellow, 1, Qt::SolidLine, Qt::RoundCap));
			paint.drawRect(QRect(X, Y, Width, Height));
		}
	}

	paint.end();

}

void Frame::mouseMoveEvent(QMouseEvent * event)
{
	setCursor(Qt::ArrowCursor);
	if (!isPaintRect())
	{
		qDebug() << " not enable PaintRect";
	}

	QPoint curvePos = event->pos();
	std::map<std::string, HyperLinkForFrame *> selectedLink;
	if (isPaintRect())
	{
		std::map<std::string, HyperLinkForFrame *>::iterator it;
		for (it = m_mCurrentLink.begin(); it != m_mCurrentLink.end(); ++it) {
			//std::cout << it->first << " | ";
			int X = m_iImageOffset_x + ((it->second)->X * m_dImageScalor_x);
			int Y = m_iImageOffset_y + ((it->second)->Y * m_dImageScalor_y);
			int Width = ((it->second)->width * m_dImageScalor_x);
			int Height = ((it->second)->height * m_dImageScalor_y);
			if (((curvePos.x() - X) <= Width) && ((curvePos.y() - Y) <= Height) && ((curvePos.x() - X) >= 0) && ((curvePos.y() - Y) >= 0))
			{
				selectedLink[it->first] = it->second;
				setCursor(Qt::OpenHandCursor);
			}
		}
	}
	
	emit linkSelected(selectedLink);

	if (getEnableEditRect())
	{
		int targetX = (curvePos.x() - m_iImageOffset_x) / m_dImageScalor_x;
		int targetY = (curvePos.y() - m_iImageOffset_y) / m_dImageScalor_y;
		QRect currentRect = m_mRectBeingEdited.second;
		if ((m_bEditStartPointSet == true) && (m_bEditEndPointSet == false))
		{
			//qDebug("first set");
			int Width = targetX - currentRect.x();
			int Height = targetY - currentRect.y();
			m_mRectBeingEdited.second.setWidth(Width);
			m_mRectBeingEdited.second.setHeight(Height);
			update();
		}
		else if ( currentFrameIsInRange(m_mRectBeingEdited.first,m_iCurrentFrame) && (m_bEditEndPointSet == true))
		{
			//if (currentRect.contains(QPoint(targetX, targetY)))
			if (m_bRectSelected == true)
			{
				//qDebug() << " ssss";
				//qDebug("move and drag rect");
				if (isCloseTo(targetX, currentRect.x()) && isCloseTo(targetY, currentRect.y())) // left top
				{
					setCursor(Qt::SizeFDiagCursor);
					m_mRectBeingEdited.second.setTopLeft(QPoint(targetX, targetY));
				}
				else if (isCloseTo(targetX, currentRect.x()) && isCloseTo(targetY, currentRect.right())) // right top
				{
					setCursor(Qt::SizeBDiagCursor);
					m_mRectBeingEdited.second.setTopRight(QPoint(targetX, targetY));
				}
				else if (isCloseTo(targetX, currentRect.x()) && isCloseTo(targetY, currentRect.bottom())) // left bottom
				{
					setCursor(Qt::SizeBDiagCursor);
					m_mRectBeingEdited.second.setBottomLeft(QPoint(targetX, targetY));
				}
				else if (isCloseTo(targetX, currentRect.right()) && isCloseTo(targetY, currentRect.bottom())) // right bottom
				{
					setCursor(Qt::SizeFDiagCursor);
					m_mRectBeingEdited.second.setBottomRight(QPoint(targetX, targetY));
				}
				else if ((targetY > currentRect.y()) && (targetY < currentRect.bottom()) && (isCloseTo(targetX, currentRect.right()) || isCloseTo(targetX, currentRect.x())))
				{
					if (isCloseTo(targetX, currentRect.right())) // click on right edge
					{
						setCursor(Qt::SizeHorCursor);
						m_mRectBeingEdited.second.setRight(targetX);
					}
					else if (isCloseTo(targetX, currentRect.x())) // click on left edge
					{
						setCursor(Qt::SizeHorCursor);
						m_mRectBeingEdited.second.setLeft(targetX);
					}
				}
				else if ((targetX > currentRect.x()) && (targetX < currentRect.right()) && (isCloseTo(targetY, currentRect.y()) || isCloseTo(targetY, currentRect.bottom())))
				{
					if (isCloseTo(targetY, currentRect.y())) // click on top edge
					{
						setCursor(Qt::SizeVerCursor);
						m_mRectBeingEdited.second.setTop(targetY);
					}
					else if (isCloseTo(targetY, currentRect.bottom())) // click on botton edge
					{
						setCursor(Qt::SizeVerCursor);
						m_mRectBeingEdited.second.setBottom(targetY);
					}
				}
				else if (isCloseTo(targetX, currentRect.center().x()) && isCloseTo(targetY, currentRect.center().y()))
				{
					setCursor(Qt::SizeAllCursor);
					m_mRectBeingEdited.second.moveCenter(QPoint(targetX, targetY));
				}
				else
				{
					;
				}
				update();
			}
		}
	}

}

void Frame::mousePressEvent(QMouseEvent *event)
{
	setCursor(Qt::ArrowCursor);
	if (!isPaintRect())
	{
		qDebug() << " not enable PaintRect";
	}

	QPoint curvePos = event->pos();

	if (isPaintRect())
	{
		if (Qt::RightButton == event->button())
		{
			std::map<std::string, HyperLinkForFrame *> selectedLink;
			std::map<std::string, HyperLinkForFrame *>::iterator it;
			for (it = m_mCurrentLink.begin(); it != m_mCurrentLink.end(); ++it) {
				//std::cout << it->first << " | ";
				int X = m_iImageOffset_x + ((it->second)->X * m_dImageScalor_x);
				int Y = m_iImageOffset_y + ((it->second)->Y * m_dImageScalor_y);
				int Width = ((it->second)->width * m_dImageScalor_x);
				int Height = ((it->second)->height * m_dImageScalor_y);
				if (((curvePos.x() - X) <= Width) && ((curvePos.y() - Y) <= Height) && ((curvePos.x() - X) >= 0) && ((curvePos.y() - Y) >= 0))
				{
					selectedLink[it->first] = it->second;
					setCursor(Qt::ClosedHandCursor);
				}
			}
		}
	}
	
	if (getEnableEditRect())
	{
		if (Qt::LeftButton == event->button())
		{
			if (currentFrameIsInRange(m_mRectBeingEdited.first, m_iCurrentFrame))
			{
				QRect currentRect = m_mRectBeingEdited.second;
				int targetX = (curvePos.x() - m_iImageOffset_x) / m_dImageScalor_x;
				int targetY = (curvePos.y() - m_iImageOffset_y) / m_dImageScalor_y;
				if ((m_bEditStartPointSet == true) && (m_bEditEndPointSet))
				{
					m_bRectSelected = true;
					//qDebug() << " ssss";
					if (isCloseTo(targetX, currentRect.x()) && isCloseTo(targetY, currentRect.y())) // left top
					{
						setCursor(Qt::SizeFDiagCursor);
						m_mRectBeingEdited.second.setTopLeft(QPoint(targetX, targetY));
					}
					else if (isCloseTo(targetX, currentRect.x()) && isCloseTo(targetY, currentRect.right())) // right top
					{
						setCursor(Qt::SizeBDiagCursor);
						m_mRectBeingEdited.second.setTopRight(QPoint(targetX, targetY));
					}
					else if (isCloseTo(targetX, currentRect.x()) && isCloseTo(targetY, currentRect.bottom())) // left bottom
					{
						setCursor(Qt::SizeBDiagCursor);
						m_mRectBeingEdited.second.setBottomLeft(QPoint(targetX, targetY));
					}
					else if (isCloseTo(targetX, currentRect.right()) && isCloseTo(targetY, currentRect.bottom())) // right bottom
					{
						setCursor(Qt::SizeFDiagCursor);
						m_mRectBeingEdited.second.setBottomRight(QPoint(targetX, targetY));
					}
					else if ((targetY > currentRect.y()) && (targetY < currentRect.bottom()) && (isCloseTo(targetX, currentRect.right()) || isCloseTo(targetX, currentRect.x())))
					{
						if (isCloseTo(targetX, currentRect.right())) // click on right edge
						{
							setCursor(Qt::SizeHorCursor);
							m_mRectBeingEdited.second.setRight(targetX);
						}
						else if (isCloseTo(targetX, currentRect.x())) // click on left edge
						{
							setCursor(Qt::SizeHorCursor);
							m_mRectBeingEdited.second.setLeft(targetX);
						}
					}
					else if ((targetX > currentRect.x()) && (targetX < currentRect.right()) && (isCloseTo(targetY, currentRect.y()) || isCloseTo(targetY, currentRect.bottom())))
					{
						if (isCloseTo(targetY, currentRect.y())) // click on top edge
						{
							setCursor(Qt::SizeVerCursor);
							m_mRectBeingEdited.second.setTop(targetY);
						}
						else if (isCloseTo(targetY, currentRect.bottom())) // click on botton edge
						{
							setCursor(Qt::SizeVerCursor);
							m_mRectBeingEdited.second.setBottom(targetY);
						}
					}
					else if (isCloseTo(targetX, currentRect.center().x()) && isCloseTo(targetY, currentRect.center().y()))
					{
						setCursor(Qt::SizeAllCursor);
						m_mRectBeingEdited.second.moveCenter(QPoint(targetX, targetY));
					}
					else
					{
						;
					}
				}
				else
				{
					if (m_bEditStartPointSet == false)
					{

						m_mRectBeingEdited.second.setX(targetX);
						m_mRectBeingEdited.second.setY(targetY);
						m_mRectBeingEdited.second.setWidth(1);
						m_mRectBeingEdited.second.setHeight(1);
						m_bEditStartPointSet = true;
						update();
					}
					else
					{
						//qDebug() << "drag ";
						int Width = targetX - currentRect.x();
						int Height = targetY - currentRect.y();
						m_mRectBeingEdited.second.setWidth(Width);
						m_mRectBeingEdited.second.setHeight(Height);
					}

				}
			}
		}
	}
}

void Frame::mouseReleaseEvent(QMouseEvent *event)
{
	setCursor(Qt::ArrowCursor);

	QPoint curvePos = event->pos();

	if (ifJumpEnbale())
	{
		if (isPaintRect())
		{
			if (Qt::RightButton == event->button())
			{
				std::map<std::string, HyperLinkForFrame *> selectedLink;
				std::map<std::string, HyperLinkForFrame *>::iterator it;
				for (it = m_mCurrentLink.begin(); it != m_mCurrentLink.end(); ++it) {
					//std::cout << it->first << " | ";
					int X = m_iImageOffset_x + ((it->second)->X * m_dImageScalor_x);
					int Y = m_iImageOffset_y + ((it->second)->Y * m_dImageScalor_y);
					int Width = ((it->second)->width * m_dImageScalor_x);
					int Height = ((it->second)->height * m_dImageScalor_y);
					if (((curvePos.x() - X) <= Width) && ((curvePos.y() - Y) <= Height) && ((curvePos.x() - X) >= 0) && ((curvePos.y() - Y) >= 0))
					{
						selectedLink[it->first] = it->second;
						setCursor(Qt::OpenHandCursor);
						framePause();
						m_bIsStopped = true;
						loaderThread.interrupt();
						while (!loaderThread.isStoped())
						{
							//printf("!");
							loaderThread.exit();
						}
						emit requestJump((selectedLink.begin()->second)->targetFilename, (selectedLink.begin()->second)->targetFrame);
						break;
					}
				}
			}
		}
		
	}

	if (getEnableEditRect())
	{
		if (Qt::LeftButton == event->button())
		{
			if (currentFrameIsInRange(m_mRectBeingEdited.first, m_iCurrentFrame))
			{
				QRect currentRect = m_mRectBeingEdited.second;
				int targetX = (curvePos.x() - m_iImageOffset_x) / m_dImageScalor_x;
				int targetY = (curvePos.y() - m_iImageOffset_y) / m_dImageScalor_y;
				if ((m_bEditEndPointSet))
				{
					if (m_bRectSelected == true)
					{
						//qDebug() << " ssss";

						if (isCloseTo(targetX, currentRect.x()) && isCloseTo(targetY, currentRect.y())) // left top
						{
							setCursor(Qt::SizeFDiagCursor);
							m_mRectBeingEdited.second.setTopLeft(QPoint(targetX, targetY));
						}
						else if (isCloseTo(targetX, currentRect.x()) && isCloseTo(targetY, currentRect.right())) // right top
						{
							setCursor(Qt::SizeBDiagCursor);
							m_mRectBeingEdited.second.setTopRight(QPoint(targetX, targetY));
						}
						else if (isCloseTo(targetX, currentRect.x()) && isCloseTo(targetY, currentRect.bottom())) // left bottom
						{
							setCursor(Qt::SizeBDiagCursor);
							m_mRectBeingEdited.second.setBottomLeft(QPoint(targetX, targetY));
						}
						else if (isCloseTo(targetX, currentRect.right()) && isCloseTo(targetY, currentRect.bottom())) // right bottom
						{
							setCursor(Qt::SizeFDiagCursor);
							m_mRectBeingEdited.second.setBottomRight(QPoint(targetX, targetY));
						}
						else if ((targetY > currentRect.y()) && (targetY < currentRect.bottom()) && (isCloseTo(targetX, currentRect.right()) || isCloseTo(targetX, currentRect.x())))
						{
							if (isCloseTo(targetX, currentRect.right())) // click on right edge
							{
								setCursor(Qt::SizeHorCursor);
								m_mRectBeingEdited.second.setRight(targetX);
							}
							else if (isCloseTo(targetX, currentRect.x())) // click on left edge
							{
								setCursor(Qt::SizeHorCursor);
								m_mRectBeingEdited.second.setLeft(targetX);
							}
						}
						else if ((targetX > currentRect.x()) && (targetX < currentRect.right()) && (isCloseTo(targetY, currentRect.y()) || isCloseTo(targetY, currentRect.bottom())))
						{
							if (isCloseTo(targetY, currentRect.y())) // click on top edge
							{
								setCursor(Qt::SizeVerCursor);
								m_mRectBeingEdited.second.setTop(targetY);
							}
							else if (isCloseTo(targetY, currentRect.bottom())) // click on botton edge
							{
								setCursor(Qt::SizeVerCursor);
								m_mRectBeingEdited.second.setBottom(targetY);
							}
						}
						else if (isCloseTo(targetX, currentRect.center().x()) && isCloseTo(targetY, currentRect.center().y()))
						{
							setCursor(Qt::SizeAllCursor);
							m_mRectBeingEdited.second.moveCenter(QPoint(targetX, targetY));
						}
						else
						{
							;
						}
						update();
						setCursor(Qt::ArrowCursor);
						m_bRectSelected = false;
						emit newRectDrawn(m_mRectBeingEdited.second);
					}
				}
				else
				{
					if (m_bEditStartPointSet == true)
					{

						int Width = targetX - currentRect.x();
						int Height = targetY - currentRect.y();
						m_mRectBeingEdited.second.setWidth(Width);
						m_mRectBeingEdited.second.setHeight(Height);
						m_bEditEndPointSet = true;
						update();
						emit newRectDrawn(m_mRectBeingEdited.second);
						qDebug("first set finish");
					}
				}
			}	
		}
	}
}

void Frame::setFileList(QString path)
{
	dirPath.setPath(path);
	fileList.append(dirPath.entryInfoList());
	/*for (int i = 0; i < fileList.size(); i++)
	{
		qDebug() << "Filepath" << fileList[i].path()<< "Filename" << fileList[i].fileName()<<endl;
	}*/
}

void Frame::setRootFolder(QString rootFolderPath)
{
	m_sRootFolder = rootFolderPath.toStdString();
	QDir dir;
	dir.setPath(rootFolderPath);
	m_sVideoName = dir.dirName().toStdString();
	printf(" rootfolder: %s, VideoName: %s\n", m_sRootFolder.data(), m_sVideoName.data());

	//emit rootFolderIsSet();
}

void Frame::setRootFolderForEditor(QString rootFolderPath)
{
	m_sVideoName_old = m_sVideoName;
	
	setRootFolder(rootFolderPath);
}

void Frame::setVideoName(QString videoName )
{
	m_sVideoName = videoName.toStdString();
}

//  Dynamic Loading
int Frame::initCacheSystem(int iCacheSize)
{

	m_FrameCacheMap.setCapacity(m_iCacheSize);

	m_pFrameCache = (DWORD**)malloc(m_iCacheSize * sizeof(DWORD*));
	if (m_pFrameCache == NULL)
	{
		return -1;
	}

	DWORD uiFrameSize_pixel = m_iFrameWidth * m_iFrameHeight;

	for (int i = 0; i < m_iCacheSize; i++)
	{
		m_pFrameCache[i] = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	}

	m_pFrameIndexOfCacheBlock = new int[m_iCacheSize];

	//m_pFrameStateFlag = new bool[m_iMaxFrame];

	//memset(m_pFrameStateFlag, false, m_iMaxFrame);

	m_iHead = 0;
	m_iTail = 0;
	m_bIsFull = false;
	m_bIsAlmostFull = false;

	pFrame = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	pRData = (BYTE*)malloc(uiFrameSize_pixel);
	pGData = (BYTE*)malloc(uiFrameSize_pixel);
	pBData = (BYTE*)malloc(uiFrameSize_pixel);

	return 0;
}

//  Dynamic Loading
void Frame:: freeCacheSystemMemory()
{
	for (int i = 0; i < m_iCacheSize; i++)
	{
		free(m_pFrameCache[i]);
	}

	free(m_pFrameCache);

	free(pFrame);
	free(pRData);
	free(pGData);
	free(pBData);

	delete m_pFrameIndexOfCacheBlock;
}


//  Dynamic Loading
DWORD * Frame::fetchFrameBlock(int iCurrentFrame)
{
	//loaderThread.interrupt();
	//while (!loaderThread.isStoped())
	//{
	//	qDebug() << "still Running";
	//	//loaderThread.exit();
	//}
	int iCurrentFrameId = iCurrentFrame - 1;

	if (m_FrameCacheMap.containsIndex(iCurrentFrameId))//&& (m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)] == iCurrentFrame))
	{
		if (iCurrentFrameId - m_FrameCacheMap.firstIndex() >= m_iStorageThreshold)
		{
			/*qDebug() << "Map remove: " << m_FrameCacheMap.first();
			qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail << " |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to " << m_pFrameIndexOfCacheBlock[m_iTail - 1];*/
			m_FrameCacheMap.takeFirst();
			_prepop();

			//loaderThread.start(QThread::HighPriority);
			//startThread();
			return m_pFrameCache[m_FrameCacheMap.at(iCurrentFrameId)];
		}
		else
		{
			//loaderThread.start(QThread::HighPriority);
			//startThread();
			return m_pFrameCache[m_FrameCacheMap.at(iCurrentFrameId)];
		}

	}
	else
	{
		printf("fetchFrameBlock: fatal error! iCurrentFrameId: %d\n", iCurrentFrameId);
		return NULL;
	}

}

//  Dynamic Loading
int Frame::checkFrameExisted(int iCurrentFrame)
{
	int iCurrentFrameId = iCurrentFrame - 1;

	if (m_FrameCacheMap.containsIndex(iCurrentFrameId))
	{
		if (m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)] == iCurrentFrame)
		{
			;
		}
		else
		{
			printf("\n existedFrameID confliction: conflicted block! iCurrentFrame: %d, FrameIndexOfCacheBlock  %d\n", iCurrentFrame, m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)]);
			loaderThread.interrupt();
			while (!loaderThread.isStoped())
			{
				//printf("!");
				loaderThread.exit();
			}
			_clear();
			m_FrameCacheMap.clear();
			return -2;
		}
		return 0;
	}
	else if (!m_FrameCacheMap.containsIndex(iCurrentFrameId))
	{
		printf("\n \n");
		qDebug() << "checkFrameId: " << iCurrentFrameId << "  doesn't hit!  ";
		if (iCurrentFrameId == m_FrameCacheMap.firstIndex() - 1) // ��ǰ��������Ԫ�� add new item at the head
		{
			return 1;
		}
		else if (iCurrentFrameId == m_FrameCacheMap.lastIndex() + 1)
		{
			return 2;
		}
		else
		{
			loaderThread.interrupt();
			while (!loaderThread.isStoped())
			{
				//printf("!");
				loaderThread.exit();
			}
			_clear();
			m_FrameCacheMap.clear();
			return -1;
		}
	}
	else
	{
		printf("checkFrameExisted: fatal error! \n");
		return NULL;
	}
}


//  Dynamic Loading
int Frame::checkAndLoadFrame(int iCurrentFrame)
{
	//qDebug() << "checkAndLoadFrame";
	loaderThread.interrupt();
	/*while (!loaderThread.isStoped())
	{
		printf("!");
		loaderThread.exit();
	}*/
	int iCurrentFrameId = iCurrentFrame - 1;

	if (m_FrameCacheMap.containsIndex(iCurrentFrameId))
	{
		//printf("hit!\n");
		if (m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)] == iCurrentFrame)
		{
			//printf("hit! \n");
		}
		else
		{
			printf("\n existedFrameID confliction: conflicted block! iCurrentFrame: %d, FrameIndexOfCacheBlock  %d\n", iCurrentFrame, m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)]);
			//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			//qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail << " |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to " << m_pFrameIndexOfCacheBlock[m_iTail - 1]<<'\n';
			while (!loaderThread.isStoped())
			{
				//printf("!");
				loaderThread.exit();
			}
			_clear();
			m_FrameCacheMap.clear();
			m_FrameCacheMap.insert(iCurrentFrameId, _append(iCurrentFrame));
		}
	}
	else if (!m_FrameCacheMap.containsIndex(iCurrentFrameId))
	{

		qDebug() << "checkAndLoadFrame: FrameId: " << iCurrentFrameId << "  doesn't hit!  ";
		while (!loaderThread.isStoped())
		{
		//printf("!");
		loaderThread.exit();
		}
		if (iCurrentFrameId == m_FrameCacheMap.firstIndex() - 1) // ��ǰ��������Ԫ�� add new item at the head
		{
			//printf("insert next front!!!\n");
			if (_isFull())
			{
				if (!m_FrameCacheMap.isFull())
				{
					printf("checkAndLoadFrame: fatal error: _isFull() conflicts! \n");
				}
				int iBlockIndex = m_FrameCacheMap.takeLast();//remove the head item
				if (iBlockIndex != m_iTail-1)
				{
					printf("checkAndLoadFrame: fatal error: m_iTail coflicts! \n");
				}
				_backpop();
			}
			m_FrameCacheMap.prepend(_prepend(iCurrentFrame));
		}
		else if (iCurrentFrameId == m_FrameCacheMap.lastIndex() + 1)
		{
			//printf("insert next back!!!\n");
			if (_isFull())
			{
				if (!m_FrameCacheMap.isFull())
				{
					printf("checkAndLoadFrame: fatal error: _isFull() conflicts! \n");
				}
				int iBlockIndex = m_FrameCacheMap.takeFirst();//remove the head item
				if (iBlockIndex != m_iHead)
				{
					printf("checkAndLoadFrame: fatal error: m_iHead coflicts! iBlockIndex: %d, m_iHead: %d \n", iBlockIndex, m_iHead);
				}
				_prepop();
			}
			m_FrameCacheMap.append(_append(iCurrentFrame));
			//qDebug() << "append: ";
			//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			//qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail << " |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to " << m_pFrameIndexOfCacheBlock[m_iTail - 1] << '\n';
		}
		else
		{
			_clear();
			m_FrameCacheMap.insert(iCurrentFrameId, _append(iCurrentFrame));
			//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			//qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail<< "  |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to "  << m_pFrameIndexOfCacheBlock[m_iTail - 1]<<'\n';
		}
	}
	else
	{
		printf("checkAndLoadFrame: fatal error! \n");
		return NULL;
	}
	//loaderThread.start(QThread::HighPriority);
	startThread();
	return 0;
}

//  Dynamic Loading
int Frame::_clear()
{
	//printf("clear\n");
	m_iHead = 0;
	m_iTail = 0;
	m_bIsFull = false;
	m_bIsAlmostFull = false;
	return 0;
}

//  Dynamic Loading
bool Frame::_isFull()
{
	return m_bIsFull;
}

//  Dynamic Loading
bool Frame::isFull()
{
	return m_bIsFull;
}

//  Dynamic Loading
bool Frame::isAlmostFull()
{
	return m_bIsAlmostFull;
}
//  Dynamic Loading
int Frame::_prepop()
{
	if ((m_iHead == m_iTail) && (!m_bIsFull))
	{
		printf("error: Queue is empty! \n");
		return -1;
	}

	_resetFrameBlock(m_iHead);
	m_iHead = (m_iHead + 1) % m_iCacheSize;

	m_bIsFull = false;

	if (((m_iTail + 2) % m_iCacheSize != m_iHead) && (m_bIsFull == false))
	{
		m_bIsAlmostFull = false;
	}


	return 0;

}
//  Dynamic Loading
int Frame::_backpop()
{
	if ((m_iHead == m_iTail) && (!m_bIsFull))
	{
		printf("error: Queue is empty! \n");
		return -1;
	}
	m_iTail = (m_iTail - 1 + m_iCacheSize) % m_iCacheSize;
	_resetFrameBlock(m_iTail);

	m_bIsFull = false;

	if (((m_iTail + 2) % m_iCacheSize != m_iHead) && (m_bIsFull == false))
	{
		m_bIsAlmostFull = false;
	}

	return 0;
}

//  Dynamic Loading
int Frame::_prepend(int iFrameNum)  // return 1: queue is full
{

	if ((m_iHead == m_iTail) && (m_bIsFull))
	{
		printf("_prepend: Queue is full! \n");
		return 1;
	}

	int iNextHead = (m_iHead - 1 + m_iCacheSize) % m_iCacheSize;

	if (iNextHead == m_iTail)
	{
		m_bIsFull = true;
	}

	if (iNextHead == (m_iTail + 2) % m_iCacheSize)
	{
		m_bIsAlmostFull = true;
	}

	m_iHead = iNextHead;
	_setFrameBlock(iFrameNum, m_iHead);

	return m_iHead;
}

//  Dynamic Loading
int Frame::_append(int iFrameNum)
{
	if ((m_iTail == m_iHead) && (m_bIsFull))
	{
		printf("_append: Queue is full! \n");
		return 1;
	}

	//qDebug() << "head:  "<< m_iHead  << ' '<<"tail:  "<< (m_iTail + 1) % m_iCacheSize ;
	if ((m_iTail + 1) % m_iCacheSize == m_iHead)
	{
		m_bIsFull = true;
	}

	if ((m_iTail + 3) % m_iCacheSize == m_iHead)
	{
		m_bIsAlmostFull = true;
	}

	_setFrameBlock(iFrameNum, m_iTail);

	int iPastTail = m_iTail;

	m_iTail = (m_iTail + 1) % m_iCacheSize;

	return iPastTail;

}

//  Dynamic Loading
int Frame::_setFrameBlock(int iCurrentFrame, int iTargetBlock)
{
	_loadFrame(iCurrentFrame, iTargetBlock);
	_setFrameIndexOfCacheBlock(iCurrentFrame, iTargetBlock);

	return 0;
}

int Frame::_resetFrameBlock(int iTargetBlock)
{
	//qDebug() << "cache Index remove: " << iTargetBlock<<'\n';
	_setFrameIndexOfCacheBlock(-1, iTargetBlock);

	return 0;
}

//  Dynamic Loading
int Frame::_setFrameIndexOfCacheBlock(int iCurrentFrame, int iTargetBlock)
{
	m_pFrameIndexOfCacheBlock[iTargetBlock] = iCurrentFrame;
	return 0;
}

//  Dynamic Loading
int Frame::_loadFrame(int iCurrentFrameNum, int iTargetBlock)
{
	//qDebug() << "_loadFrame";
	FILE *pf = NULL;
	DWORD uiFrameSize_pixel = m_iFrameWidth * m_iFrameHeight;

	const std::string root = m_sRootFolder;// "D:\\Downloads\\London\\London\\LondonOne\\LondonOne";
	const std::string suffix = m_sVideoSuffix;// ".rgb";

	DWORD * pFrame = m_pFrameCache[iTargetBlock];

	std::string sFileNum = std::to_string(iCurrentFrameNum);

	fulFillZero(sFileNum);

	std::string sFullFilePath = root +"\\"+ m_sVideoName+ sFileNum + suffix;


	if ((pf = fopen(sFullFilePath.data(), "rb")) == NULL)
	{
		printf("File coulkd not be opened ");
		qDebug() << sFullFilePath.data() << "\n";
		return -1;
	}

	int n = fread(pRData, uiFrameSize_pixel, 1, pf);
	n = fread(pGData, uiFrameSize_pixel, 1, pf);
	n = fread(pBData, uiFrameSize_pixel, 1, pf);

	if (n == 1)
	{
		for (unsigned int i = 0; i < uiFrameSize_pixel; ++i)
		{
			BYTE *pb = (BYTE *)(pFrame + i);
			pFrame[i] = pBData[i];
			pb[1] = pGData[i];
			pb[2] = pRData[i];
			pb[3] = 255;
		}
	}
	else
	{
		printf("_loadFrame: fread failed!\n");
		return -1;
	}
	fclose(pf);

	//qDebug() << "targetBlock: "<< iTargetBlock << ' ' << "currentFrameNum:  "<<iCurrentFrameNum;
	return 0;
}


int Frame::forwardLoadFrameSeq()
{
	int iNextFrameNum = (m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize] + 1) % (m_iMaxFrame + 1);


	if (iNextFrameNum == 0) //&& (m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize] != -1))
	{
		if (m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize] == -1)
		{
			if (m_iTail == (m_iHead + 1)%m_iCacheSize)
			{
				printf("it is true, ");
			}
			printf("forwardLoadFrame race condition \n");
			qDebug() << "last Frame in Map is: " << m_FrameCacheMap.lastIndex() + 1;
			iNextFrameNum = (m_FrameCacheMap.lastIndex() + 1) + 1;
		}
		else
		{
			printf("forwardLoadFrame end \n");
			iNextFrameNum += 1;
			return 0;
		}
	}
	//printf("current Frame: %d, current head: %d, current tail: %d, last loaded frame: %d, loading Frame: %d | ", m_iCurrentFrame, m_iHead, m_iTail, m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize], iNextFrameNum);
	//printf("current Frame: %d, current head frame: %d, current tail frame: %d, loading Frame: %d \n ", m_iCurrentFrame, m_pFrameIndexOfCacheBlock[m_iHead], m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize],iNextFrameNum);

	if ((iNextFrameNum - 1) != m_FrameCacheMap.lastIndex() + 1) // ID + 1 = true frame number
	{
		qDebug() << "forward function error!\n: " << "iNextFrameNum - 1: " << iNextFrameNum - 1 << " m_FrameCacheMap.lastIndex() + 1" << m_FrameCacheMap.lastIndex() + 1;
		return -2;
		//_clear();
	}
	m_FrameCacheMap.append(_append(iNextFrameNum));

	//qDebug() << "forward: map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
	return 0;
}

//Dynamic Loading
void Frame::loadInitialFrame(int iInitialNum)
{
	for (int i = 0; i < iInitialNum; i++)
	{
		m_FrameCacheMap.append(_append(i + 1));
		if (i % 100 == 0)
		{
			printf(">");
		}
		if (i % 1000 == 0)
		{
			printf("\n");
		}
	}
	printf("\nLOAD Initial FRAME SUCCEED!\n");
}

void Frame::loadInitialFrame(int startFrame, int iInitialNum)
{
	qDebug() << "loadFrames";
	for (int i = startFrame; i <= startFrame + iInitialNum; i++)
	{
		//qDebug() << startFrame;
		m_FrameCacheMap.insert(i - 1, _append(i));// !!!!! i-1 cause the index in QContiguous starts from 0!!!
		if (i % 100 == 0)
		{
			printf(">");
		}
		if (i % 1000 == 0)
		{
			printf("\n");
		}
	}
	//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
	printf("LOAD FRAMES SUCCEED!\n");
}

//Dynamic Loading
void Frame::_loadFrames(int startFrame, int iInitialNum)
{
	qDebug() << "loadFrames";
	for (int i = startFrame ; i <= startFrame + iInitialNum; i++)
	{
		//qDebug() << startFrame;
		m_FrameCacheMap.insert(i-1 , _append(i ));// !!!!! i-1 cause the index in QContiguous starts from 0!!!
		if (i % 100 == 0)
		{
			printf(">");
		}
		if (i % 1000 == 0)
		{
			printf("\n");
		}
	}
	//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
	printf("LOAD FRAMES SUCCEED!\n");
}

void Frame::setCurrentLink()
{
	/*if (!isPaintRect())
	{
		return;
	}*/
	std::map<std::string, HyperLinkForFrame *>::iterator it;
	for (it = m_mCurrentLink.begin(); it != m_mCurrentLink.end(); ++it)
	{
		delete (it->second);
	}
	m_mCurrentLink.clear();
	std::map<int, std::list<HyperMediaLink *>>::iterator itmap = fullMap.find(m_iCurrentFrame);
	if (itmap == fullMap.end())
	{
		return;
	}
	//std::cout << "Frame " << m_iCurrentFrame << " has " << itmap->second.size() << " links\n";
	std::list<HyperMediaLink *>::iterator it2;
	for (it2 = itmap->second.begin(); it2 != itmap->second.end(); ++it2) {
		//std::cout << (*it2)->linkName << " | ";
		m_mCurrentLink[(*it2)->linkName] = new HyperLinkForFrame(*it2);
	}
	enablePaintRect();
}

void Frame::updateCurrentLink()
{
	/*if (!isPaintRect())
	{
		return;
	}*/
	std::map<int, std::list<HyperMediaLinkFast *>>::iterator itmapfast = fastMap.find(m_iCurrentFrame);
	if (itmapfast == fastMap.end())
	{
		return;
	}
	//std::cout << "Frame " << m_iCurrentFrame << " has " << itmapfast->second.size() << " fast links\n";
	std::list<HyperMediaLinkFast *>::iterator it2fast;
	for (it2fast = itmapfast->second.begin(); it2fast != itmapfast->second.end(); ++it2fast) {
		//std::cout << (*it2fast)->linkName << " | ";
		//std::cout << (*it2fast)->type << " | ";
		if ((*it2fast)->type == ChangeType::Add)
		{
			m_mCurrentLink[(*it2fast)->linkName] = new HyperLinkForFrame((*it2fast));
		}
		else if ((*it2fast)->type == ChangeType::Remove)
		{
			delete m_mCurrentLink[(*it2fast)->linkName];
			m_mCurrentLink.erase((*it2fast)->linkName);
		}
	}
	enablePaintRect();
	//std::cout << "\n";
}


void Frame::loadMetaData()
{
	// First things first, remove everything
	freeLinkSystemMemory();
	try {
		std::string sFullFilePath = m_sRootFolder + "\\" + m_sVideoName + m_sMetaDataSuffix;
		rapidxml::file<> metadataFile(sFullFilePath.data());
		rapidxml::xml_document<> doc;
		doc.parse<0>(metadataFile.data());

		rapidxml::xml_node<> *linkNode = doc.first_node("links");
		for (rapidxml::xml_node<> *thisNode = linkNode->first_node("link"); thisNode; thisNode = thisNode->next_sibling())
		{
			HyperMediaLink *thisLink = new HyperMediaLink(thisNode);
			links.push_back(thisLink);
		}

		generateListAndMaps(links);
		enablePaintRect();
		setMouseTracking(true);
	}
	catch (...) {
		std::cout << "Cannot open metadata file. Playing video without links." << "\n";
	}
}

// Design on the fullmap and fastmap:
// The list of links itself is a dynamically allocated list of links. 
// whose content should not be updated unless its being saved in editor
// However fullmap is just a map from frame number (int) to pointers of links 
// and fastmap similarily is a map from frame number (int) to pointer of fastlinks
// Both of which can be cleared

// Generate the full map and fast map from given listOfLinks
// in loadMetaData it is called with freshly generated links list read from the file
// in other case (used for updating the previews in the editor after some operations
// The generateListAndMaps should receive a updated deep-copied list from outside
// generated with the copy constructor.
void Frame::generateListAndMaps(std::list<HyperMediaLink *> listOfLinks)
{
	clearFullMap();
	clearFastMap();

	// Second pass
	std::list<HyperMediaLink *>::iterator it;
	for (it = listOfLinks.begin(); it != listOfLinks.end(); ++it) {
		(*it)->beautifullyPrint();
		for (int i = (*it)->startFrame; i < (*it)->endFrame; i++) {

			std::map<int, std::list<HyperMediaLink *>>::iterator itmap = fullMap.find(i);
			if (itmap != fullMap.end())
			{ // Found it
				fullMap.at(i).push_back(*it);
			}
			else {
				std::list<HyperMediaLink *> thisList;
				thisList.push_back(*it);
				fullMap.insert(std::pair<int, std::list<HyperMediaLink *>>(i, thisList));
			}
		}
	}

	// Third pass
	for (it = listOfLinks.begin(); it != listOfLinks.end(); ++it) {
		// Start => Add operation
		int startFrame = (*it)->startFrame;
		int endFrame = (*it)->endFrame;

		HyperMediaLinkFast *addLink = new HyperMediaLinkFast(*it, Add);
		HyperMediaLinkFast *removeLink = new HyperMediaLinkFast(*it, Remove);

		fastLinks.push_back(addLink);
		fastLinks.push_back(removeLink);

		std::map<int, std::list<HyperMediaLinkFast *>>::iterator itmapfast = fastMap.find(startFrame);
		if (itmapfast != fastMap.end())
		{ // Found it
			fastMap.at(startFrame).push_back(addLink);
		}
		else {
			std::list<HyperMediaLinkFast *> thisList;
			thisList.push_back(addLink);
			fastMap.insert(std::pair<int, std::list<HyperMediaLinkFast *>>(startFrame, thisList));
		}

		// End => Remove operation
		itmapfast = fastMap.find(endFrame);
		if (itmapfast != fastMap.end())
		{ // Found it
			fastMap.at(endFrame).push_back(removeLink);
		}
		else {
			std::list<HyperMediaLinkFast *> thisList;
			thisList.push_back(removeLink);
			fastMap.insert(std::pair<int, std::list<HyperMediaLinkFast *>>(endFrame, thisList));
		}
	}
}

void Frame::freeLinkSystemMemory()
{
	std::map<std::string, HyperLinkForFrame *>::iterator it;
	for (it = m_mCurrentLink.begin(); it != m_mCurrentLink.end(); ++it)
	{
		delete (it->second);
	}
	m_mCurrentLink.clear();
	qDebug() << "\n HyperLinkForFrame Free";

	clearLinksList();
	clearFullMap();
	clearFastMap();
}

void Frame::clearLinksList()
{
	while (!links.empty())
	{
		HyperMediaLink * temp = links.back();
		delete temp;
		links.pop_back();
	}
	qDebug() << "\n List of Links Freed";
}

void Frame::clearFullMap()
{
	fullMap.clear();
	qDebug() << "\n Full Map Freed";
}

void Frame::clearFastMap()
{
	while (!fastLinks.empty())
	{
		HyperMediaLinkFast * temp = fastLinks.back();
		delete temp;
		fastLinks.pop_back();
	}
	fastMap.clear();//!!!!! don't forget
	qDebug() << "\n Fast Link Map Freed";
}
