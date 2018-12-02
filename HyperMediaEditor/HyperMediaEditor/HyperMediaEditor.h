#ifndef HYPERMEDIAEDITOR_H
#define HYPERMEDIAEDITOR_H

#include <QtWidgets/QMainWindow>
#include "ui_HyperMediaEditor.h"

class HyperMediaEditor : public QMainWindow
{
	Q_OBJECT

public:
	HyperMediaEditor(QWidget *parent = 0);
	~HyperMediaEditor();

private:
	Ui::HyperMediaEditorClass ui;
};

#endif // HYPERMEDIAEDITOR_H
