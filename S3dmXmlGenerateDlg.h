#pragma once

#include <QDialog>
#include "s3d_stylemanagerlib_global.h"
#include "ui_S3dmXmlGenerateDlg.h"
#include "qgsstyle.h"

using namespace std;
class S3D_STYLEMANAGERLIB_EXPORT S3dmXmlGenerateDlg : public QDialog
{
	Q_OBJECT

public:
	S3dmXmlGenerateDlg(QgsStyle *style, QWidget *parent = Q_NULLPTR);
	~S3dmXmlGenerateDlg();
private:
	void generateImportXml();  //生成xml：点击时界面响应
	void listPath();           //生成xml：选择路径后列表列出项
	void generateXml();        //生成xml：路径下生成保存指定参数symbol的xml
	void changeType();         //生成xml：改变列表中类型combo响应
	void changeFavorite();     //生成xml：改变列表中收藏check响应
private:
	Ui::S3dmXmlGenerateDlg ui;
	QgsStyle *mGetStyle;
	QString m_dir;             //生成xml：选择生成xml的文件总目录
	vector<string> m_subPaths; //生成xml：总目录下的所有文件夹名称
};
