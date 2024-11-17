#include "FeatureAttribute.h"

/*************�õ���ǰѡ��ͼԪ��������ʵ��**********/

QgsSelectFeatureAttribute::QgsSelectFeatureAttribute(void)
{
   pLayer=NULL;
}

QgsSelectFeatureAttribute::~QgsSelectFeatureAttribute(void)
{
}
//���õ�ǰ��ѡ��(�)��ͼ��
void QgsSelectFeatureAttribute::SetSelectLayer(QgsVectorLayer *Layer)
{
   pLayer=Layer;
}
//�õ�ѡ������ָ���ֶε�����ֵ
QStringList QgsSelectFeatureAttribute::GetSelectAttribute(QString FieldName)
{
    QStringList strAttribut;
	int i,nCount,indexfield;
	QgsFeature feature;
	QString Value;
	if(pLayer==NULL){
		return strAttribut;
	}
	//�õ��ֶ�������Ӧ���ֶ�������
	//indexfield=pLayer->fieldNameIndex(FieldName);
	//�õ���ǰѡ��������б�
	QgsFeatureList featurelist=pLayer->selectedFeatures();
	nCount=featurelist.size();
	for(i=0;i<nCount;i++){
		//�õ�ѡ�������
		feature=featurelist.at(i);
     //   const QgsAttributeMap& attributes=feature.attributeMap();
	    ////�õ�ָ������ֶε�����ֵ
     //   Value=attributes[indexfield].toString();
		strAttribut.append(Value);
	}
    return strAttribut;
}
/******************ɾ��ͼ��ѡ��������ʵ��**************/
//���캯��
QgsDeSelectFeature::QgsDeSelectFeature(void)
{
	pLayer=NULL;
}
QgsDeSelectFeature::QgsDeSelectFeature(QgsVectorLayer *Layer)
{
	pLayer=Layer;
}
//��������
QgsDeSelectFeature::~QgsDeSelectFeature(void)
{
	
}
//���õ�ǰ��ѡ��(�)��ͼ��
void QgsDeSelectFeature::SetSelectLayer(QgsVectorLayer *Layer)
{
    pLayer=Layer;
}
//ɾ��ѡ���ͼԪ
void QgsDeSelectFeature::RemoveSelectFeature()
{
    if(pLayer==NULL){
		return;
	}
	pLayer->removeSelection();
	//QApplication::setOverrideCursor(Qt::ArrowCursor);
}