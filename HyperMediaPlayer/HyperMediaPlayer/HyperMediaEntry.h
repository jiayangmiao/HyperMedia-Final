#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HyperMediaEntry.h"

class HyperMediaEntry : public QMainWindow
{
	Q_OBJECT

public:
	HyperMediaEntry(QWidget *parent = Q_NULLPTR);

private:
	Ui::HyperMediaEntryClass ui;
};
