/**************************QGISʵ����*********************
*****************************************************************/
#ifndef QGSUTILITY_H
#define QGSUTILITY_H
#pragma once
#include "qgsvectorlayer.h"
#include <QApplication>
/*************�õ���ǰѡ��ͼԪ��������**********/

class QgsSelectFeatureAttribute
{
public:
   QgsSelectFeatureAttribute(void);
   ~QgsSelectFeatureAttribute(void);
public:
	//�趨ѡ���ͼ��
	void SetSelectLayer(QgsVectorLayer *);
    //�õ�ѡ������ָ���ֶε�����ֵ
	QStringList GetSelectAttribute(QString FieldName);
private:
    QgsVectorLayer *pLayer;
};

/******************ɾ��ͼ��ѡ��������**************/

class QgsDeSelectFeature
{
public:
	QgsDeSelectFeature(void);
	QgsDeSelectFeature(QgsVectorLayer *);
	~QgsDeSelectFeature(void);
public:
	//�趨ѡ���ͼ��
	void SetSelectLayer(QgsVectorLayer *);
	//ɾ��ѡ���ͼԪ
	void RemoveSelectFeature();
private:
    QgsVectorLayer *pLayer;
};
#endif