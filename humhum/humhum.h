#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_humhum.h"
#include <QAudioInput>
#include <qtimer.h>
#include <qfile.h>

struct WAVFILEHEADER
{
	// RIFF 头
	char RiffName[4];
	unsigned long nRiffLength;

	// 数据类型标识符
	char WavName[4];

	// 格式块中的块头
	char FmtName[4];
	unsigned long nFmtLength;

	// 格式块中的块数据
	unsigned short nAudioFormat;
	unsigned short nChannleNumber;
	unsigned long nSampleRate;
	unsigned long nBytesPerSecond;
	unsigned short nBytesPerSample;
	unsigned short nBitsPerSample;

	// 数据块中的块头
	char    DATANAME[4];
	unsigned long   nDataLength;
};



class Humhum : public QMainWindow
{
	Q_OBJECT

public:
	Humhum(QWidget *parent = Q_NULLPTR);

private:
	Ui::humhumClass ui;
	QAudioInput* audio;
	QFile outputFile;
	void recordAudio();
	qint64 addWavHeader(QString catheFileName, QString wavFileName);

private slots:
	void on_button_rec_click();

	void on_button_hum_click();

	void stopRecording();
};
