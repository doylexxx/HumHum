#include "humhum.h"
#include <qstringlistmodel.h>
#include <qtextstream.h>
#include <qfile.h>

Humhum::Humhum(QWidget *parent)
	: QMainWindow(parent, Qt::WindowCloseButtonHint)
{
	ui.setupUi(this);
	
	// ÏÔÊ¾Çú¿â
	QFile dataFile("Song100.txt");
	if (dataFile.open(QFile::ReadOnly | QIODevice::Text))
	{
		QTextStream data(&dataFile);
		QStringList fonts;
		QString line;
		while (!data.atEnd())
		{
			line = data.readLine();
			line.remove('\n');
			fonts << line;
		}
		QStringListModel *model = new QStringListModel(fonts);
		ui.list_mids->setModel(model);
	}

	setFixedSize(this->width(), this->height());
}

void Humhum::on_button_rec_click()
{
}

void Humhum::on_button_hum_click()
{
}
