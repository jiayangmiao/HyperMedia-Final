#include "HyperMediaEditor.h"
#include <qstandarditemmodel.h>
#include "../../Library/rapidxml/rapidxml.hpp"
#include "../../Library/rapidxml/rapidxml_iterators.hpp"
#include "../../Library/rapidxml/rapidxml_print.hpp"
#include "../../Library/rapidxml/rapidxml_utils.hpp"
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <QCloseEvent>

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

	connect(ui.selectLinkComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(chosenLinkChanged(QString)));

	connect(ui.setLinkButton, SIGNAL(clicked()), this, SLOT(setLinkButtonTapped()));
	connect(ui.removeLinkButton, SIGNAL(clicked()), this, SLOT(removeLinkButtonTapped()));
	connect(ui.saveFileButton, SIGNAL(clicked()), this, SLOT(saveButtonTapped()));
	enableLinkOperationUI(false);
	enableRectUI(false);
	initialFrames();
}

void HyperMediaEditor::initialFrames()
{
	enableOriginPlayerUI(false);
	enableTargetPlayerUI(false);
	enableOriginJumpToStartButton(false);
	enableOriginJumpToEndButton(false);
	enableTargetJumpButton(false);
	initialOriginFrame();
	initialTargetFrame();
}

void HyperMediaEditor::initialOriginFrame()
{
	// Signal for displayed values
	connect(ui.leftWidget, SIGNAL(videoLoaded(bool)), this, SLOT(updateOriginVideoInfo()));
	connect(ui.leftWidget, SIGNAL(currentFrameUpdated(int)), ui.leftSlider, SLOT(setValue(int)));
	connect(ui.leftWidget, SIGNAL(currentFrameUpdated(int)), this, SLOT(updateOriginTime(int)));

	connect(ui.originPathLineEdit, SIGNAL(textChanged(QString)), ui.leftWidget, SLOT(setRootFolderForEditor(QString)));
	connect(ui.originLoadButton, SIGNAL(clicked()), this, SLOT(needToLoadVideo()));

	ui.leftWidget->setBasic(m_iFrameNum, m_iWidth, m_iHeight, m_iFps, 800);
	ui.leftWidget->Init();

	// Signal for Playback related actions
	connect(ui.originPlayButton, SIGNAL(clicked()), this, SLOT(playTappedOnOrigin()));
	connect(ui.originStopButton, SIGNAL(clicked()), ui.leftWidget, SLOT(Stop()));
	connect(ui.leftSlider, SIGNAL(valueChanged(int)), ui.leftWidget, SLOT(setCurrentFrame(int)));

	// Jump to link target on the right pane 
	connect(ui.leftWidget , SIGNAL(requestJump(std::string, int)), this, SLOT(jumpToAnotherFrame(std::string, int)));

	// Signal for setting origin start and end frames
	connect(ui.originSetStartTimeButton, SIGNAL(clicked()), this, SLOT(setStartFrameButtonTapped()));
	connect(ui.originSetEndTimeButton, SIGNAL(clicked()), this, SLOT(setEndFrameButtonTapped()));
	connect(ui.originJumpToStartButton, SIGNAL(clicked()), this, SLOT(originJumpToStartTapped()));
	connect(ui.originJumpToEndButton, SIGNAL(clicked()), this, SLOT(originJumpToEndTapped()));

	connect(this, SIGNAL(temporaryRectUsable(bool, QRect)), ui.leftWidget, SLOT(rectChecked(bool, QRect)));
	connect(this, SIGNAL(startFrameUpdated(int)), ui.leftWidget, SLOT(setStartFrame(int)));
	connect(this, SIGNAL(endFrameUpdated(int)), ui.leftWidget, SLOT(setEndFrame(int)));
	connect(this, SIGNAL(successfullySetLink()), ui.leftWidget, SLOT(linkSet()));

	connect(ui.selectArea, SIGNAL(clicked()), this, SLOT(selectAreaButtonTapped()));
	connect(ui.resetAreaButton, SIGNAL(clicked()), this, SLOT(resetAreaButtonIsClicked()));

	connect(ui.leftWidget, SIGNAL(newRectDrawn(QRect)), this, SLOT(temporaryRectUpdated(QRect)));
	connect(this, SIGNAL(temporaryRectUpdated()), this, SLOT(printTemporaryRect()));
}

void HyperMediaEditor::initialTargetFrame()
{
	ui.rightWidget->disableJump();
	// Signal for displayed values
	connect(ui.rightWidget, SIGNAL(videoLoaded(bool)), this, SLOT(updateTargetVideoInfo()));
	connect(ui.rightWidget, SIGNAL(currentFrameUpdated(int)), ui.rightSlider, SLOT(setValue(int)));
	connect(ui.rightWidget, SIGNAL(currentFrameUpdated(int)), this, SLOT(updateTargetTime(int)));

	connect(ui.targetPathLineEdit, SIGNAL(textChanged(QString)), ui.rightWidget, SLOT(setRootFolder(QString)));
	connect(ui.targetLoadButton, SIGNAL(clicked()), ui.rightWidget, SLOT(LoadVideo()));

	ui.rightWidget->setBasic(m_iFrameNum, m_iWidth, m_iHeight, m_iFps, 800);
	ui.rightWidget->Init();

	// Signal for Playback related actions
	connect(ui.targetPlayButton, SIGNAL(clicked()), this, SLOT(playTappedOnTarget()));
	connect(ui.targetStopButton, SIGNAL(clicked()), ui.rightWidget, SLOT(Stop()));
	connect(ui.rightSlider, SIGNAL(valueChanged(int)), ui.rightWidget, SLOT(setCurrentFrame(int)));

	// Signal for setting target frame
	connect(ui.targetSetTimeButton, SIGNAL(clicked()), this, SLOT(setTargetFrameButtonTapped()));
	connect(ui.targetJumpToTargetButton, SIGNAL(clicked()), this, SLOT(targetJumpToTargetTapped()));

}

void HyperMediaEditor::clearTempLinks()
{
	while (!tempLinks.empty())
	{
		HyperMediaLink * temp = tempLinks.back();
		delete temp;
		tempLinks.pop_back();
	}
}

void HyperMediaEditor::loadTempLinkFromFrame()
{
	clearTempLinks();
	std::list<HyperMediaLink *>::iterator it;
	for (it = ui.leftWidget->links.begin(); it != ui.leftWidget->links.end(); ++it) {
		(*it)->beautifullyPrint();
		// Completely re-generating a new link
		HyperMediaLink *tempLink = new HyperMediaLink((*it)->linkName, (*it)->startFrame, (*it)->endFrame, 
			(*it)->targetFilename, (*it)->targetFrame, (*it)->X, (*it)->Y, (*it)->height, (*it)->width);
		tempLinks.push_back(tempLink);
	}
	// Since this is directly loaded from the frame the fullmap and fastmap wouldn't need to be regenerated
	setupComboBoxFromTemp();
}

void HyperMediaEditor::updateRectUI() {
	if (originIsLoaded && targetIsLoaded) {
		enableRectUI(true);
	}
	else {
		enableRectUI(false);
	}
}

void HyperMediaEditor::enableRectUI(bool enable)
{
	ui.selectArea->setEnabled(enable);
	ui.resetAreaButton->setEnabled(enable);
}

void HyperMediaEditor::setupComboBoxFromTemp()
{
	std::cout << "Temp link size " << tempLinks.size() << "\n";

	// Clear all combo box items
	for (int i = ui.selectLinkComboBox->count() - 1; i >= 0; i--) {
		ui.selectLinkComboBox->removeItem(i);
	}

	if (tempLinks.size() == 0) {
		ui.selectLinkComboBox->addItem("- No Links -");

		QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.selectLinkComboBox->model());
		QModelIndex firstIndex = model->index(0, ui.selectLinkComboBox->modelColumn(),
			ui.selectLinkComboBox->rootModelIndex());
		QStandardItem* firstItem = model->itemFromIndex(firstIndex);
		firstItem->setSelectable(false);
	}
	else {
		ui.selectLinkComboBox->addItem("- Select Link -");
		std::list<HyperMediaLink *>::iterator it;
		for (it = tempLinks.begin(); it != tempLinks.end(); ++it) {
			ui.selectLinkComboBox->addItem(QString::fromStdString((*it)->linkName));
		}

		QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.selectLinkComboBox->model());
		QModelIndex firstIndex = model->index(0, ui.selectLinkComboBox->modelColumn(),
			ui.selectLinkComboBox->rootModelIndex());
		QStandardItem* firstItem = model->itemFromIndex(firstIndex);
		firstItem->setSelectable(false);
	}
}

HyperMediaLink * HyperMediaEditor::tempLinkWithName(std::string name)
{
	std::list<HyperMediaLink *>::iterator it;
	for (it = tempLinks.begin(); it != tempLinks.end(); ++it) {
		if ((*it)->linkName.compare(name) == 0) {
			return *it;
		}
	}
	return NULL;
}

void HyperMediaEditor::addLinkToTemp(HyperMediaLink *newLink)
{
	linkHasBeenEdited = true;
	tempLinks.push_back(newLink);

	ui.leftWidget->generateListAndMaps(tempLinks);
	setupComboBoxFromTemp();

	resetAllTempVariables();
	resetLinkRelatedUI();

	ui.leftWidget->setCurrentFrame(1);
	ui.rightWidget->setCurrentFrame(1);

	emit successfullySetLink();
}

void HyperMediaEditor::removeLinkFromTemp(std::string name)
{
	linkHasBeenEdited = true;

	std::cout << "Temp link now size " << tempLinks.size() << "\n";

	std::list<HyperMediaLink *>::iterator it = tempLinks.begin();
	while (it != tempLinks.end())
	{
		if ((*it)->linkName.compare(name) == 0) {
			std::cout << "Erasing link with " << (*it)->linkName << "\n";
			HyperMediaLink *toDelete = *it;
			it = tempLinks.erase(it);
			delete toDelete;
		}
		else
		{
			++it;
		}
	}
	std::cout << "Temp link now size " << tempLinks.size() << "\n";
}

void HyperMediaEditor::resetAllTempVariables()
{
	chosenLinkName = "";
	resetOriginTempVariables();
	resetTargetTempVariables();
}

void HyperMediaEditor::resetLinkRelatedUI()
{
	ui.leftWidget->setCurrentLink();
	ui.linkNameLineEdit->setText("");
	ui.selectLinkComboBox->setCurrentIndex(0);
}

void HyperMediaEditor::resetOriginTempVariables()
{
	originStartFrameIsChosen = false;
	originEndFrameIsChosen = false;
	chosenStartFrame = 1;
	chosenEndFrame = 9000;
	chosenX = -1;
	chosenY = -1;
	chosenWidth = -1;
	chosenHeight = -1;
	enableOriginJumpToStartButton(false);
	enableOriginJumpToEndButton(false);
	ui.originSelectedStartTimeLabel->setText(frame2time(chosenStartFrame, ui.originSelectedStartTimeLabel->text()));
	ui.originSelectedEndTimeLabel->setText(frame2time(chosenEndFrame, ui.originSelectedEndTimeLabel->text()));
}

void HyperMediaEditor::resetTargetTempVariables()
{
	targetFrameIsChosen = false;
	chosenTargetFrame = 1;
	enableTargetJumpButton(false);
	ui.targetSelectedTimeLabel->setText(frame2time(chosenTargetFrame, ui.targetSelectedTimeLabel->text()));
}

void HyperMediaEditor::saveTempLinksIntoFile()
{
	std::cout << "Save requested" << "\n";
	std::string filePath = ui.leftWidget->m_sRootFolder;
	filePath +=  "/" + ui.leftWidget->m_sVideoName + ".xml";
	QString qfilePath = QString::fromStdString(filePath);
	qfilePath.replace("/", "//");
	filePath = qfilePath.toStdString();

	FILE * of;
	of = fopen(filePath.c_str(), "w");

	if (of != NULL) {

		rapidxml::xml_document<> outputDoc;

		rapidxml::xml_node<>* filenameNode = outputDoc.allocate_node(rapidxml::node_element, "filename");
		filenameNode->value(ui.leftWidget->m_sVideoName.c_str());
		outputDoc.append_node(filenameNode);

		rapidxml::xml_node<>* linksNode = outputDoc.allocate_node(rapidxml::node_element, "links");

		std::list<HyperMediaLink *>::iterator it;
		for (it = tempLinks.begin(); it != tempLinks.end(); ++it) {
			if ((*it)->linkName.compare("temporary_link") != 0) { // Skipping the temp link

				rapidxml::xml_node<>* thisLinkNode = outputDoc.allocate_node(rapidxml::node_element, "link");

				rapidxml::xml_node<>* linkNameNode = outputDoc.allocate_node(rapidxml::node_element, "name");
				linkNameNode->value(outputDoc.allocate_string((*it)->linkName.c_str()));
				thisLinkNode->append_node(linkNameNode);

				int temp = (*it)->startFrame;
				char buffer[20];
				itoa(temp, buffer, 10);
				rapidxml::xml_node<>* linkStartFrameNode = outputDoc.allocate_node(rapidxml::node_element, "startFrame");
				linkStartFrameNode->value(outputDoc.allocate_string(buffer));
				thisLinkNode->append_node(linkStartFrameNode);

				temp = (*it)->endFrame;
				itoa(temp, buffer, 10);
				rapidxml::xml_node<>* linkEndFrameNode = outputDoc.allocate_node(rapidxml::node_element, "endFrame");
				linkEndFrameNode->value(outputDoc.allocate_string(buffer));
				thisLinkNode->append_node(linkEndFrameNode);

				rapidxml::xml_node<>* linkTargetFilenameNode = outputDoc.allocate_node(rapidxml::node_element, "targetFilename");
				linkTargetFilenameNode->value(outputDoc.allocate_string((*it)->targetFilename.c_str()));
				thisLinkNode->append_node(linkTargetFilenameNode);

				temp = (*it)->targetFrame;
				itoa(temp, buffer, 10);
				rapidxml::xml_node<>* linkTargetFrameNode = outputDoc.allocate_node(rapidxml::node_element, "targetFrame");
				linkTargetFrameNode->value(outputDoc.allocate_string(buffer));
				thisLinkNode->append_node(linkTargetFrameNode);

				temp = (*it)->X;
				itoa(temp, buffer, 10);
				rapidxml::xml_node<>* linkXNode = outputDoc.allocate_node(rapidxml::node_element, "originX");
				linkXNode->value(outputDoc.allocate_string(buffer));
				thisLinkNode->append_node(linkXNode);

				temp = (*it)->Y;
				itoa(temp, buffer, 10);
				rapidxml::xml_node<>* linkYNode = outputDoc.allocate_node(rapidxml::node_element, "originY");
				linkYNode->value(outputDoc.allocate_string(buffer));
				thisLinkNode->append_node(linkYNode);

				temp = (*it)->height;
				itoa(temp, buffer, 10);
				rapidxml::xml_node<>* linkHeightNode = outputDoc.allocate_node(rapidxml::node_element, "height");
				linkHeightNode->value(outputDoc.allocate_string(buffer));
				thisLinkNode->append_node(linkHeightNode);

				temp = (*it)->width;
				itoa(temp, buffer, 10);
				rapidxml::xml_node<>* linkWidthNode = outputDoc.allocate_node(rapidxml::node_element, "width");
				linkWidthNode->value(outputDoc.allocate_string(buffer));
				thisLinkNode->append_node(linkWidthNode);

				linksNode->append_node(thisLinkNode);
			}
		}
		outputDoc.append_node(linksNode);

		std::stringstream ss;
		ss.str("");
		ss << outputDoc;
		std::string result_xml = ss.str();
		std::cout << result_xml << "\n";
		fwrite(result_xml.c_str(), sizeof(char), result_xml.size(), of);
		fflush(of);
		fclose(of);
		outputDoc.clear();

		ui.leftWidget->loadMetaData();
	}
	else {
		// CANNOT open file
		printf("err %d \n", errno);
	}
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

bool HyperMediaEditor::checkNewRect(const QRect &rect)
{
	std::list<HyperMediaLink *>::iterator it;
	for (it = tempLinks.begin(); it != tempLinks.end(); ++it)
	{
		if ((chosenStartFrame > (*it)->endFrame) || (chosenEndFrame < (*it)->startFrame) )
		{
			;
		}
		else
		{
			QRect temp = QRect((*it)->X, (*it)->Y, (*it)->width, (*it)->height);
			if (rect.intersects(temp))
			{
				QRect intersecRect = rect.intersected(temp);
				if (( intersecRect == rect) || ( intersecRect == temp))
				{
					QMessageBox::warning(this, "Error", "The currently chosen area and time period is covering or was covered completely by other areas of links. Please try again ");
					return false;
				}
			}
			else
			{
				;
			}
		}
	}
	return true;
}

void HyperMediaEditor::closeEvent(QCloseEvent *event)
{
	if (linkHasBeenEdited) 
	{
		QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Unsaved changes",
			tr("You have changes that are not saved\nAre you sure you want to quit?"),
			QMessageBox::No | QMessageBox::Yes,
			QMessageBox::Yes);
		if (resBtn != QMessageBox::Yes) {
			event->ignore();
		}
		else {
			event->accept();
		}
	}
}