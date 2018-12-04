#ifndef HYPERMEDIAEDITOR_H
#define HYPERMEDIAEDITOR_H

#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include <QString>
#include <QProxyStyle>
#include <QPushButton>
#include <QMessageBox>
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
	
	// Constants for readability
	const int m_iFrameNum = 9000;
	const int m_iWidth = 352;
	const int m_iHeight = 288;
	const int m_iFps = 30;

	const int defaultOriginStartFrame = 1;
	const int defaultOriginEndFrame = 9000;
	const int defaultTargetFrame = 1;

	// Variables for temporary link (being edited)
	bool originIsLoaded = false;
	bool originStartFrameIsChosen = false;
	bool originEndFrameIsChosen = false;
	bool targetIsLoaded = false;
	bool targetFrameIsChosen = false;

	std::list<HyperMediaLink *> tempLinks;
	
	std::string chosenLinkName;
	std::string desiredLinkName;
	std::string chosenOriginFilename;
	std::string chosenTargetFilename;

	int chosenTargetFrame = 1;
	int chosenStartFrame = 1;
	int chosenEndFrame = 9000;
	int chosenX = 0;
	int chosenY = 0;
	int chosenWidth = 0;
	int chosenHeight = 0;

	HyperMediaLink *temporaryLink;

private:
	Ui::HyperMediaEditorClass ui;
	
	QFileDialog *originDialog;
	QFileDialog *targetDialog;

	QString frame2time(int iFrameNum, QString string);

	void initialFrames();
	void initialOriginFrame();
	void initialTargetFrame();

	void clearTempLinks();
	void loadTempLinkFromFrame();
	void setupComboBoxFromTemp();
	HyperMediaLink * tempLinkWithName(std::string name);

	void resetTempVariables();

	void addLinkToTemp(HyperMediaLink *newLink);
	void removeLinkFromTemp(std::string linkName);

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

	void enableLinkOperationUI(bool enable)
	{
		ui.selectLinkComboBox->setEnabled(enable);
		ui.linkNameLineEdit->setEnabled(enable);
		ui.saveFileButton->setEnabled(enable);
		ui.setLinkButton->setEnabled(enable);
		ui.removeLinkButton->setEnabled(enable);
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

	void updateOriginVideoInfo()
	{
		resetTempVariables();
		enableOriginPlayerUI(true);
		enableLinkOperationUI(true);
		originIsLoaded = true;
		chosenOriginFilename = ui.leftWidget->m_sVideoName;
		std::cout << "Origin: " << ui.leftWidget->m_sVideoName << endl;
		resetTempVariables();
		loadTempLinkFromFrame();
	}

	void updateTargetVideoInfo()
	{
		enableTargetPlayerUI(true);
		targetIsLoaded = true;
		chosenTargetFilename = ui.rightWidget->m_sVideoName;
		std::cout << "Target: " << ui.rightWidget->m_sVideoName << endl;
	}

	void needToLoadVideo()
	{
		// if UI leftwidget filename (from path) is same to whats in the widget now, dont let it load.

		ui.selectLinkComboBox->setCurrentIndex(0);
		clearTempLinks();
		ui.leftWidget->LoadVideo();
	}

	void chosenLinkChanged(const QString &text)
	{
		if ((text.compare("- No Links -") == 0) || (text.compare("- Select Link -") == 0)) {
			return;
		}
		else {
			std::string name = text.toStdString();
			HyperMediaLink *chosenLink = tempLinkWithName(name);
			if (chosenLink != NULL) { // Sometimes it fucks up so protecting
				ui.leftWidget->Stop();
				ui.leftWidget->setCurrentFrame(chosenLink->startFrame);
				ui.linkNameLineEdit->setText(text);
				chosenLinkName = name;
				std::cout << "Selected link name: " << chosenLinkName << endl;
			}
		}
	}

	void setLinkButtonTapped()
	{

	}

	void removeLinkButtonTapped()
	{
		if (ui.linkNameLineEdit->text().size() == 0) {
			QMessageBox::warning(this, "Error", "No link name provided. Please pick from the menu on the left or type in manually.");
		}
		else if (tempLinkWithName(ui.linkNameLineEdit->text().toStdString()) == NULL) {
			QMessageBox::warning(this, "Error", "Cannot find link with provided name.");
		}
		else {
			removeLinkFromTemp(ui.linkNameLineEdit->text().toStdString());
			ui.leftWidget->generateListAndMaps(tempLinks);
			setupComboBoxFromTemp();

			resetTempVariables();
		}
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
