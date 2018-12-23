#include "humhum.h"

Humhum::Humhum(QWidget *parent)
	: QMainWindow(parent, Qt::WindowCloseButtonHint)
{
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());
}

void Humhum::on_button_rec_click()
{
}

void Humhum::on_button_hum_click()
{
}
