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

	/*训练模型（这段都可以删掉）从mid_list.txt读取所有midi歌曲的文件名，训练成模型输出OutputModel的文件夹中*/
	const char * filename = "mid_list.txt";
	const char * foldername = "OutputModel";
	std::cout << "---------------------Traning Model---------------------" << endl;
	BuildSDHummingModel myMidi((char *)filename, (char *)foldername);
	if (!myMidi.GenFilelist()) {
		std::cout << "ERROR: Could Not Loading Label File!" << endl;
	}
	
	if (myMidi.Write2Model() != 0) {
		std::cout << "ERROR: Could Not Write New Model!" << endl;
	}

	
	const char * modelFileName = "ShengdaModel/QBH.Model";//模型先用盛大的
	const char * modelInfoName = "ShengdaModel/QBHModel.info";
	const char * wavFileName = "test.wav"; //输入的wav是本地的，需要在UI加一步先录制？
	const char * resultFileName = "result.txt"; //匹配后的结果
	/*读取模型转化成midi数据结构，读取wav音乐转化成midi数据结构，进行匹配得到结果，但是匹配部分被注释掉了*/
	SClientTester((char *)modelFileName, (char *)modelInfoName, (char *)wavFileName, (char *)resultFileName);



	return a.exec();
}
