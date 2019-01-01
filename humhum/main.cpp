#include "humhum.h"
#include <QtWidgets/QApplication>
#include "client.h"
#include <QDebug>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Humhum w;
	w.show();

	qDebug() << "Hello";

	///*训练模型  从train_mid_list.txt读取所有midi歌曲的文件名，训练成模型输出OutputModel的文件夹中*/
	//const char * filename = "train_mid_list.txt";
	//const char * foldername = "OutputModel";
	//std::cout << "---------------------Traning Model---------------------" << endl;
	//BuildSDHummingModel myMidi((char *)filename, (char *)foldername);
	//if (!myMidi.GenFilelist()) {
	//	std::cout << "ERROR: Could Not Loading Label File!" << endl;
	//}
	//
	//if (myMidi.Write2Model() != 0) {
	//	std::cout << "ERROR: Could Not Write New Model!" << endl;
	//}

	
	



	return a.exec();
}
