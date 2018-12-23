#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_humhum.h"

class Humhum : public QMainWindow
{
	Q_OBJECT

public:
	Humhum(QWidget *parent = Q_NULLPTR);

private:
	Ui::humhumClass ui;

private slots:
	void on_button_rec_click();

	void on_button_hum_click();
};
