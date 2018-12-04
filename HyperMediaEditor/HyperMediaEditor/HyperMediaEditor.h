#ifndef HYPERMEDIAEDITOR_H
#define HYPERMEDIAEDITOR_H

#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include <QProxyStyle>
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

	// Link Editing

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

	const int m_iFrameNum = 9000;
	const int m_iFps = 30;
	QString frame2time(int iFrameNum, QString string);

	void initialFrame();

public slots:

	
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
