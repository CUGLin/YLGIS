/**************************QGIS实用类*********************
*****************************************************************/
#ifndef QGSUTILITY_H
#define QGSUTILITY_H
#pragma once
#include "qgsvectorlayer.h"
#include <QApplication>
/*************得到当前选择图元的属性类**********/

class QgsSelectFeatureAttribute
{
public:
   QgsSelectFeatureAttribute(void);
   ~QgsSelectFeatureAttribute(void);
public:
	//设定选择的图层
	void SetSelectLayer(QgsVectorLayer *);
    //得到选择特征指定字段的属性值
	QStringList GetSelectAttribute(QString FieldName);
private:
    QgsVectorLayer *pLayer;
};

/******************删除图层选择特征类**************/

class QgsDeSelectFeature
{
public:
	QgsDeSelectFeature(void);
	QgsDeSelectFeature(QgsVectorLayer *);
	~QgsDeSelectFeature(void);
public:
	//设定选择的图层
	void SetSelectLayer(QgsVectorLayer *);
	//删除选择的图元
	void RemoveSelectFeature();
private:
    QgsVectorLayer *pLayer;
};
#endif