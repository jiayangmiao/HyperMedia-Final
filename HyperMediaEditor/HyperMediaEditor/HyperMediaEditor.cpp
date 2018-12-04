#include "HyperMediaEditor.h"

HyperMediaEditor::HyperMediaEditor(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// Path stuff for origin and target sides
	originDialog = new QFileDialog(this);
	originDialog->setFileMode(QFileDialog::Directory);
	connect(originDialog, SIGNAL(fileSelected(QString)), ui.originPathLineEdit, SLOT(setText(QString)));
	connect(ui.originPathButton, SIGNAL(clicked()), originDialog, SLOT(exec()));

	targetDialog = new QFileDialog(this);
	targetDialog->setFileMode(QFileDialog::Directory);
	connect(targetDialog, SIGNAL(fileSelected(QString)), ui.targetPathLineEdit, SLOT(setText(QString)));
	connect(ui.targetPathButton, SIGNAL(clicked()), targetDialog, SLOT(exec()));

	ui.leftSlider->setMinimum(1);
	ui.leftSlider->setMaximum(m_iFrameNum);
	ui.leftSlider->setStyle(new MyStyle(ui.leftSlider->style()));
	ui.originVideoLengthLabel->setText(frame2time(m_iFrameNum, ui.originVideoLengthLabel->text()));

	ui.rightSlider->setMinimum(1);
	ui.rightSlider->setMaximum(m_iFrameNum);
	ui.rightSlider->setStyle(new MyStyle(ui.rightSlider->style()));
	ui.targetVideoLengthLabel->setText(frame2time(m_iFrameNum, ui.targetVideoLengthLabel->text()));

	initialFrames();
}

void HyperMediaEditor::initialFrames()
{
	enableOriginPlayerUI(false);
	enableTargetPlayerUI(false);
	initialOriginFrame();
	initialTargetFrame();
}

void HyperMediaEditor::initialOriginFrame()
{
	// Frame itself
	connect(ui.leftWidget, SIGNAL(canEnablePlayerUI(bool)), this, SLOT(enableOriginPlayerUI(bool)));
	connect(ui.leftWidget, SIGNAL(currentFrameUpdated(int)), ui.leftSlider, SLOT(setValue(int)));
	connect(ui.leftWidget, SIGNAL(currentFrameUpdated(int)), this, SLOT(updateOriginTime(int)));

	connect(ui.originPathLineEdit, SIGNAL(textChanged(QString)), ui.leftWidget, SLOT(setRootFolder(QString)));
	connect(ui.originLoadButton, SIGNAL(clicked()), ui.leftWidget, SLOT(LoadVideo()));

	ui.leftWidget->setBasic(m_iFrameNum, m_iWidth, m_iHeight, m_iFps, 800);
	ui.leftWidget->Init();

	// Play Stop and shit
	connect(ui.originPlayButton, SIGNAL(clicked()), this, SLOT(playTappedOnOrigin()));
	connect(ui.originStopButton, SIGNAL(clicked()), ui.leftWidget, SLOT(Stop()));
	connect(ui.leftSlider, SIGNAL(valueChanged(int)), ui.leftWidget, SLOT(setCurrentFrame(int)));

	// jump to target Video 
	connect(ui.leftWidget , SIGNAL(requestJump(std::string, int)), this, SLOT(jumpToAnotherFrame(std::string, int)));
}

void HyperMediaEditor::initialTargetFrame()
{
	connect(ui.rightWidget, SIGNAL(canEnablePlayerUI(bool)), this, SLOT(enableTargetPlayerUI(bool)));
	connect(ui.rightWidget, SIGNAL(currentFrameUpdated(int)), ui.rightSlider, SLOT(setValue(int)));
	connect(ui.rightWidget, SIGNAL(currentFrameUpdated(int)), this, SLOT(updateTargetTime(int)));

	connect(ui.targetPathLineEdit, SIGNAL(textChanged(QString)), ui.rightWidget, SLOT(setRootFolder(QString)));
	connect(ui.targetLoadButton, SIGNAL(clicked()), ui.rightWidget, SLOT(LoadVideo()));

	ui.rightWidget->setBasic(m_iFrameNum, m_iWidth, m_iHeight, m_iFps, 800);
	ui.rightWidget->Init();

	// Play Stop and shit
	connect(ui.targetPlayButton, SIGNAL(clicked()), this, SLOT(playTappedOnTarget()));
	connect(ui.targetStopButton, SIGNAL(clicked()), ui.rightWidget, SLOT(Stop()));
	connect(ui.rightSlider, SIGNAL(valueChanged(int)), ui.rightWidget, SLOT(setCurrentFrame(int)));

}

HyperMediaEditor::~HyperMediaEditor()
{

}

// Helper functions for conversion between frame number and time

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

QString HyperMediaEditor::frame2time(int iFrameNum, QString  string)
{
	int iSecondTotal = iFrameNum / m_iFps;

	int iHour = iSecondTotal / 3600;

	int iMinute = (iSecondTotal / 60) % 60;

	int iSecond = iSecondTotal % 60;

	string.replace(0, 2, FillZero2(QString::number(iHour)));
	string.replace(3, 2, FillZero2(QString::number(iMinute)));
	string.replace(6, 2, FillZero2(QString::number(iSecond)));

	//qDebug() << string;
	return string;
}