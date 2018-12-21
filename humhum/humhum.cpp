#include "humhum.h"

humhum::humhum(QWidget *parent)
	: QMainWindow(parent, Qt::WindowCloseButtonHint)
{
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());
}

void humhum::on_button_rec_click()
{
}

void humhum::on_button_hum_click()
{
}
