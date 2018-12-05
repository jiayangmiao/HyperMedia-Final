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
	bool targetIsLoaded = false;
	bool originStartFrameIsChosen = false;
	bool originEndFrameIsChosen = false;
	bool targetFrameIsChosen = false;
	bool linkHasBeenEdited = false;

	std::list<HyperMediaLink *> tempLinks;
	
	std::string chosenLinkName;
	//std::string desiredLinkName;
	std::string chosenOriginFilename;
	std::string chosenTargetFilename;

	int chosenTargetFrame = 1;
	int chosenStartFrame = 1;
	int chosenEndFrame = 9000;
	int chosenX = -1;
	int chosenY = -1;
	int chosenWidth = -1;
	int chosenHeight = -1;

	HyperMediaLink *temporaryLink = NULL;

private:
	Ui::HyperMediaEditorClass ui;
	
	QFileDialog *originDialog;
	QFileDialog *targetDialog;

	QString frame2time(int iFrameNum, QString string);

	void initialFrames();
	void initialOriginFrame();
	void initialTargetFrame();

	void updateRectUI();

	void clearTempLinks();
	void loadTempLinkFromFrame();
	void setupComboBoxFromTemp();
	HyperMediaLink * tempLinkWithName(std::string name);

	void resetAllTempVariables();
	void resetOriginTempVariables();
	void resetTargetTempVariables();

	void addLinkToTemp(HyperMediaLink *newLink);
	void removeLinkFromTemp(std::string linkName);

	void saveTempLinksIntoFile();

	void enableRectUI(bool);
	
	bool checkNewRect(const QRect &rect);


signals:
	void temporaryRectUpdated();
	void temporaryRectUsable(bool, QRect);
	void startFrameUpdated(int);
	void endFrameUpdated(int);
	void successfullySetLink();
	 
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
		std::cout << ui.leftWidget->m_sRootFolder << "\n";
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
		ui.originSetEndTimeButton->setEnabled(enable);
		ui.originPlayButton->setEnabled(enable);
		ui.originStopButton->setEnabled(enable);
	}

	void enableOriginJumpToStartButton(bool enable)
	{
		ui.originJumpToStartButton->setEnabled(enable);
	}

	void enableOriginJumpToEndButton(bool enable)
	{
		ui.originJumpToEndButton->setEnabled(enable);
	}
	
	void enableTargetPlayerUI(bool enable)
	{
		ui.targetSetTimeButton->setEnabled(enable);
		ui.targetPlayButton->setEnabled(enable);
		ui.targetStopButton->setEnabled(enable);
	}

	void enableTargetJumpButton(bool enable)
	{
		ui.targetJumpToTargetButton->setEnabled(enable);
	}

	void enableLinkOperationUI(bool enable)
	{
		ui.selectLinkComboBox->setEnabled(enable);
		ui.linkNameLineEdit->setEnabled(enable);
		ui.saveFileButton->setEnabled(enable);
		ui.setLinkButton->setEnabled(enable);
		ui.removeLinkButton->setEnabled(enable);
	}

	void setStartFrameButtonTapped()
	{
		// CHECK START < END
		int desiredStartFrame = ui.leftWidget->m_iCurrentFrame;
		if (desiredStartFrame >= chosenEndFrame) {
			QMessageBox::warning(this, "Error", "You cannot set the start frame to after the end frame.");
			ui.leftWidget->setCurrentFrame(chosenStartFrame);
		}
		else {
			originStartFrameIsChosen = true;
			chosenStartFrame = desiredStartFrame;
			enableOriginJumpToStartButton(true);
			emit startFrameUpdated(chosenStartFrame);
		}
		ui.originSelectedStartTimeLabel->setText(frame2time(chosenStartFrame, ui.originSelectedStartTimeLabel->text()));
	}

	void setEndFrameButtonTapped()
	{
		// CHECK END > START
		int desiredEndFrame = ui.leftWidget->m_iCurrentFrame;
		if (desiredEndFrame <= chosenStartFrame) {
			QMessageBox::warning(this, "Error", "You cannot set the end frame to before the start frame.");
			ui.leftWidget->setCurrentFrame(chosenEndFrame);
		}
		else {
			originEndFrameIsChosen = true;
			chosenEndFrame = desiredEndFrame;
			enableOriginJumpToEndButton(true);
			emit endFrameUpdated(chosenEndFrame);
		}
		ui.originSelectedEndTimeLabel->setText(frame2time(chosenEndFrame, ui.originSelectedEndTimeLabel->text()));
	}

	void setTargetFrameButtonTapped()
	{
		// Get and set the frame value
		targetFrameIsChosen = true;
		chosenTargetFrame = ui.rightWidget->m_iCurrentFrame;
		enableTargetJumpButton(true);
		ui.targetSelectedTimeLabel->setText(frame2time(chosenTargetFrame, ui.targetSelectedTimeLabel->text()));
	}

	void originJumpToStartTapped()
	{
		ui.leftWidget->setCurrentFrame(chosenStartFrame);
	}

	void originJumpToEndTapped()
	{
		ui.leftWidget->setCurrentFrame(chosenEndFrame);
	}

	void targetJumpToTargetTapped()
	{
		ui.rightWidget->setCurrentFrame(chosenTargetFrame);
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
		originIsLoaded = true;
		linkHasBeenEdited = false;

		chosenOriginFilename = ui.leftWidget->m_sVideoName;
		std::cout << "Origin: " << ui.leftWidget->m_sVideoName << "\n";

		resetOriginTempVariables();
		ui.leftWidget->setCurrentFrame(chosenStartFrame);

		enableOriginPlayerUI(true);
		enableLinkOperationUI(true);
		updateRectUI();

		loadTempLinkFromFrame();
	}

	void updateTargetVideoInfo()
	{
		targetIsLoaded = true;

		chosenTargetFilename = ui.rightWidget->m_sVideoName;
		std::cout << "Target: " << ui.rightWidget->m_sVideoName << "\n";

		resetTargetTempVariables();
		//ui.rightWidget->setCurrentFrame(chosenTargetFrame);

		enableTargetPlayerUI(true);
		updateRectUI();
	}

	void needToLoadVideo()
	{
		clearTempLinks();
		ui.selectLinkComboBox->setCurrentIndex(0);

		// if UI leftwidget filename (from path) is same to whats in the widget now, dont let it load.
		std::string currentFileName = ui.leftWidget->m_sVideoName_old;

		std::string designatedPath = ui.originPathLineEdit->text().toStdString();
		std::size_t found = designatedPath.find_last_of("/\\");
		std::string designatedFileName = designatedPath.substr(found + 1);

		std::cout << "current file name: " << currentFileName << " Designated file name: " << designatedFileName << "\n";
		
		if (currentFileName.compare(designatedFileName) == 0)
		{
			std::cout << "its the same video" << "\n";
			ui.leftWidget->reloadVideo();
		}
		else {
			ui.leftWidget->LoadVideo();
		}
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
				std::cout << "Selected link name: " << chosenLinkName << "\n";
			}
		}
	}

	void selectAreaButtonTapped()
	{
		
		/*
		if !originStartFrameIsChosen warning
		if !originEndFrameIsChosen warning
		if !targetFrameIsChosen warning
		*/
		if (ui.selectArea->text().contains("Off"))
		{
			ui.leftWidget->enableEditRect();
			ui.selectArea->setText("Select Area On");
		}
		else
		{
			ui.leftWidget->disableEditRect();
			ui.selectArea->setText("Select Area Off");
		}
	}

	void setLinkButtonTapped()
	{
		// TODO: emit successfullySetLink();
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

			resetAllTempVariables();
		}
	}

	void saveButtonTapped()
	{
		saveTempLinksIntoFile();
	}

	void temporaryRectUpdated(QRect  rect)
	{
		// Check if rect is valid
		// 遍历links，找他们的rect，看intersection
		// 有intersection，不通过
		//		发Warning
		//		创建一个新的oldRect(chosenX, chosenY, chosenWidth, chosenHeight)
		//		注意：如果是新创建的，并且无效，那么就是 -1 -1 -1 -1
		//		emit temporaryRectUsable(false, oldRect)
		// 通过！
		//		赋值

		//		创建一个新的newRect(chosenX, chosenY, chosenWidth, chosenHeight)
		//		emit temporaryRectUsable(true, newRect)
				//printTemporaryRect();

		if (checkNewRect(rect))
		{
			chosenX = rect.x();
			chosenY = rect.y();
			chosenWidth = rect.width();
			chosenHeight = rect.height();
			emit temporaryRectUsable(true, rect);
		}
		else
		{
			QRect oldRect(chosenX, chosenY, chosenWidth, chosenHeight);
			emit temporaryRectUsable(false, oldRect);
		}

	}


	void resetAreaButtonIsClicked()
	{
		ui.leftWidget->resetRectBeingEdited();
		chosenX = -1;
		chosenY = -1;
		chosenWidth = -1;
		chosenHeight = -1;
	}

	void printTemporaryRect()
	{
		qDebug() << "temporary rect: " << chosenX << " " << chosenY << " " << chosenWidth << " " << chosenHeight;
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
