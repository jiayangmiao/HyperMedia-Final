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

	ui.rightSlider->setMinimum(1);
	ui.rightSlider->setMaximum(m_iFrameNum);
	ui.rightSlider->setStyle(new MyStyle(ui.rightSlider->style()));

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