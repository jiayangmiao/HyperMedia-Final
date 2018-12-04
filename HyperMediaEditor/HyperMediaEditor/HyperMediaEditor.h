#ifndef HYPERMEDIAEDITOR_H
#define HYPERMEDIAEDITOR_H

#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include <QProxyStyle>
#include <QPushButton>
#include "ui_HyperMediaEditor.h"
#include "../../Classes/Frame.h"
#include "../../Classes/HyperMediaLink.h"
#include "../../Classes/HyperMediaLinkFast.h"

class HyperMediaEditor : public QMainWindow
{
	Q_OBJECT

public:
	HyperMediaEditor(QWidget *parent = 0);
	~HyperMediaEditor();

	const int m_iFrameNum = 9000;
	const int m_iWidth = 352;
	const int m_iHeight = 288;
	const int m_iFps = 30;

	// Link Editing
	// Constants for readability
	const int defaultOriginStartFrame = 1;
	const int defaultOriginEndFrame = 9000;
	const int defaultTargetFrame = 1;

	// Variables for temporary link (being edited)
	std::string chosenLinkName;
	std::string desiredLinkName;
	std::string chosenTargetFilename;
	int chosenTargetFrame;
	int chosenStartFrame;
	int chosenEndFrame;
	int chosenX;
	int chosenY;
	int chosenWidth;
	int chosenHeight;

	HyperMediaLink *temporaryLink;

private:
	Ui::HyperMediaEditorClass ui;
	
	QFileDialog *originDialog;
	QFileDialog *targetDialog;

	QString frame2time(int iFrameNum, QString string);

	void initialFrames();
	void initialOriginFrame();
	void initialTargetFrame();

signals:

public slots:

	void updateOriginTime(int i)
	{
		ui.originCurrentTimeLabel->setText(frame2time(i, ui.originCurrentTimeLabel->text()));
	}

	void updateTargetTime(int i)
	{
		ui.targetCurrentTimeLabel->setText(frame2time(i, ui.targetCurrentTimeLabel->text()));
	}

	void jumpToAnotherFrame(std::string targetVideoName, int targetFrame)
	{
		ui.rightWidget->Stop();
		std::cout << ui.leftWidget->m_sRootFolder << endl;
		std::size_t found = ui.leftWidget->m_sRootFolder.find_last_of("/\\");
		std::string parentDirectory = ui.leftWidget->m_sRootFolder.substr(0, found);
		parentDirectory = parentDirectory.append("/");
		std::cout << " path: " << parentDirectory << '\n';

		QString temp = QString::fromStdString(parentDirectory) + QString::fromStdString(targetVideoName); 

		ui.targetPathLineEdit->setText(temp);
		ui.rightWidget->setRootFolder(temp);
		ui.rightWidget->LoadVideo(targetFrame);
	}

	void enableOriginPlayerUI(bool enable)
	{
		ui.originSetStartTimeButton->setEnabled(enable);
		ui.originJumpToStartButton->setEnabled(enable);
		ui.selectArea->setEnabled(enable);
		ui.originSetEndTimeButton->setEnabled(enable);
		ui.originJumpToEndButton->setEnabled(enable);
		ui.originPlayButton->setEnabled(enable);
		ui.originStopButton->setEnabled(enable);
	}

	void enableTargetPlayerUI(bool enable)
	{
		ui.targetSetTimeButton->setEnabled(enable);
		ui.targetJumpToTargetButton->setEnabled(enable);
		ui.targetPlayButton->setEnabled(enable);
		ui.targetStopButton->setEnabled(enable);
	}

	void playTappedOnOrigin()
	{
		ui.rightWidget->framePause();
		ui.leftWidget->PlayOrPause();
	}

	void playTappedOnTarget()
	{
		ui.leftWidget->framePause();
		ui.rightWidget->PlayOrPause();
	}
};

class MyStyle : public QProxyStyle
{
public:
	using QProxyStyle::QProxyStyle;

	int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
	{
		if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
			return (Qt::LeftButton);// | Qt::MidButton | Qt::RightButton);
		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};


#endif // HYPERMEDIAEDITOR_H
