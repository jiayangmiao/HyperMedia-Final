#include "HyperMediaPlayer.h"
QString FillZero2(QString  sNum)
{
	//qDebug() << "sNum: " << sNum.data() << endl;
	if (sNum.size() == 1)
	{
		QString sFill("0");
		sNum = sFill + sNum;
		return sNum;
	}
	else if (sNum.size() == 2)
	{
		return sNum;
	}

}


HyperMediaPlayer::HyperMediaPlayer(int iNum, int iWidth, int iHeight,int iFps, QWidget * parent) :QWidget(parent)
{
	ui.setupUi(this);

	m_iFrameNum = iNum;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iFps = iFps;

	// set folder
	videoFolderDialog = new QFileDialog(this);
	videoFolderDialog->setFileMode(QFileDialog::Directory);
	connect(videoFolderDialog, SIGNAL(fileSelected(QString)), ui.lineEdit, SLOT(setText(QString)));
	connect(ui.pushButton_3, SIGNAL(clicked()), videoFolderDialog, SLOT(exec()));
	// set folder end

	connect(ui.pushButton_5, SIGNAL(clicked()), this, SLOT(backwardToHistoryFrame()));

	ui.horizontalSlider->setMinimum(1);
	ui.horizontalSlider->setMaximum(m_iFrameNum);
	ui.horizontalSlider->setStyle(new MyStyle(ui.horizontalSlider->style()));

	ui.label_3->setText(frame2time(m_iFrameNum, ui.label_3->text()));
	
	ui.lineEdit_2->setEnabled(false);
	ui.lineEdit_2->setFocusPolicy(Qt::NoFocus);

	enablePlayerUI(false);

	initialFrame();

}

void HyperMediaPlayer::initialFrame()
{
	connect(ui.widget, SIGNAL(videoLoaded(bool)), this, SLOT(enablePlayerUI(bool)));
	connect(ui.widget, SIGNAL(currentFrameUpdated(int)), ui.horizontalSlider, SLOT(setValue(int)));
	connect(ui.widget, SIGNAL(currentFrameUpdated(int)), this, SLOT(updateTime(int)));

	connect(ui.lineEdit, SIGNAL(textChanged(QString)), ui.widget, SLOT(setRootFolder(QString)));
	connect(ui.pushButton_4, SIGNAL(clicked()), ui.widget, SLOT(LoadVideo()));

	ui.widget->setBasic(m_iFrameNum, m_iWidth, m_iHeight, m_iFps, 300);
	ui.widget->Init();
	ui.widget->enableJump();
	ui.widget->disableEditRect();

	connect(ui.pushButton, SIGNAL(clicked()), ui.widget, SLOT(PlayOrPause()));
	connect(ui.pushButton_2, SIGNAL(clicked()), ui.widget, SLOT(Stop()));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), ui.widget, SLOT(setCurrentFrame(int)));

	connect(ui.widget, SIGNAL(linkSelected(std::map<std::string, HyperLinkForFrame *>)), this, SLOT(updateSelectedLink(std::map<std::string, HyperLinkForFrame *>)));
	connect(this, SIGNAL(linkOutputUpdated(QString)), ui.lineEdit_2, SLOT(setText(QString)));

	// jump to target Video 
	connect(ui.widget, SIGNAL(requestJump(std::string, int)), this, SLOT(jumpToAnotherFrame(std::string, int)));
}

QString HyperMediaPlayer::frame2time(int iFrameNum, QString  string)
{
	int iSecondTotal = iFrameNum / m_iFps;

	int iHour = iSecondTotal / 3600;

	int iMinute = (iSecondTotal / 60) % 60;

	int iSecond = iSecondTotal % 60;

	string.replace(0, 2, FillZero2( QString::number(iHour)));
	string.replace(3, 2, FillZero2(QString::number(iMinute)));
	string.replace(6, 2, FillZero2(QString::number(iSecond)));

	//qDebug() << string;
	return string;
}


HyperMediaPlayer::~HyperMediaPlayer()
{
	;
}
