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

	/*ѵ��ģ�ͣ���ζ�����ɾ������mid_list.txt��ȡ����midi�������ļ�����ѵ����ģ�����OutputModel���ļ�����*/
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

	
	const char * modelFileName = "ShengdaModel/QBH.Model";//ģ������ʢ���
	const char * modelInfoName = "ShengdaModel/QBHModel.info";
	const char * wavFileName = "test.wav"; //�����wav�Ǳ��صģ���Ҫ��UI��һ����¼�ƣ�
	const char * resultFileName = "result.txt"; //ƥ���Ľ��
	/*��ȡģ��ת����midi���ݽṹ����ȡwav����ת����midi���ݽṹ������ƥ��õ����������ƥ�䲿�ֱ�ע�͵���*/
	SClientTester((char *)modelFileName, (char *)modelInfoName, (char *)wavFileName, (char *)resultFileName);



	return a.exec();
}
