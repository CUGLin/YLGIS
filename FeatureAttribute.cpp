#include "FeatureAttribute.h"

/*************得到当前选择图元的属性类实现**********/

QgsSelectFeatureAttribute::QgsSelectFeatureAttribute(void)
{
   pLayer=NULL;
}

QgsSelectFeatureAttribute::~QgsSelectFeatureAttribute(void)
{
}
//设置当前被选择(活动)的图层
void QgsSelectFeatureAttribute::SetSelectLayer(QgsVectorLayer *Layer)
{
   pLayer=Layer;
}
//得到选择特征指定字段的属性值
QStringList QgsSelectFeatureAttribute::GetSelectAttribute(QString FieldName)
{
    QStringList strAttribut;
	int i,nCount,indexfield;
	QgsFeature feature;
	QString Value;
	if(pLayer==NULL){
		return strAttribut;
	}
	//得到字段名所对应的字段索引号
	//indexfield=pLayer->fieldNameIndex(FieldName);
	//得到当前选择的特征列表
	QgsFeatureList featurelist=pLayer->selectedFeatures();
	nCount=featurelist.size();
	for(i=0;i<nCount;i++){
		//得到选择的特征
		feature=featurelist.at(i);
     //   const QgsAttributeMap& attributes=feature.attributeMap();
	    ////得到指定序号字段的属性值
     //   Value=attributes[indexfield].toString();
		strAttribut.append(Value);
	}
    return strAttribut;
}
/******************删除图层选择特征类实现**************/
//构造函数
QgsDeSelectFeature::QgsDeSelectFeature(void)
{
	pLayer=NULL;
}
QgsDeSelectFeature::QgsDeSelectFeature(QgsVectorLayer *Layer)
{
	pLayer=Layer;
}
//析构函数
QgsDeSelectFeature::~QgsDeSelectFeature(void)
{
	
}
//设置当前被选择(活动)的图层
void QgsDeSelectFeature::SetSelectLayer(QgsVectorLayer *Layer)
{
    pLayer=Layer;
}
//删除选择的图元
void QgsDeSelectFeature::RemoveSelectFeature()
{
    if(pLayer==NULL){
		return;
	}
	pLayer->removeSelection();
	//QApplication::setOverrideCursor(Qt::ArrowCursor);
}