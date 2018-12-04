#include "HyperMediaEditor.h"
#include <qstandarditemmodel.h>

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
	initialOriginFrame();
	initialTargetFrame();
}

void HyperMediaEditor::initialOriginFrame()
{
	// Signal for displayed values
	connect(ui.leftWidget, SIGNAL(videoLoaded(bool)), this, SLOT(updateOriginVideoInfo()));
	connect(ui.leftWidget, SIGNAL(currentFrameUpdated(int)), ui.leftSlider, SLOT(setValue(int)));
	connect(ui.leftWidget, SIGNAL(currentFrameUpdated(int)), this, SLOT(updateOriginTime(int)));

	connect(ui.originPathLineEdit, SIGNAL(textChanged(QString)), ui.leftWidget, SLOT(setRootFolder(QString)));
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
	// Add code to enable/disable rect selection UI here

}

void HyperMediaEditor::setupComboBoxFromTemp()
{
	std::cout << "Temp link size " << tempLinks.size() << endl;

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

}

void HyperMediaEditor::removeLinkFromTemp(std::string name)
{
	std::cout << "Temp link now size " << tempLinks.size() << endl;

	std::list<HyperMediaLink *>::iterator it = tempLinks.begin();
	while (it != tempLinks.end())
	{
		if ((*it)->linkName.compare(name) == 0) {
			std::cout << "Erasing link with " << (*it)->linkName << endl;
			it = tempLinks.erase(it);
		}
		else
		{
			++it;
		}
	}
	std::cout << "Temp link now size " << tempLinks.size() << endl;
}

void HyperMediaEditor::resetTempVariables()
{
	originStartFrameIsChosen = false;
	originEndFrameIsChosen = false;
	targetFrameIsChosen = false;

	chosenLinkName = "";

	chosenTargetFrame = 1;
	chosenStartFrame = 1;
	chosenEndFrame = 9000;
	chosenX = 0;
	chosenY = 0;
	chosenWidth = 0;
	chosenHeight = 0;
}

void HyperMediaEditor::saveTempLinksIntoFile()
{
	std::cout << "save requested" << endl;
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