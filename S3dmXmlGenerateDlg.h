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
	void generateImportXml();  //����xml�����ʱ������Ӧ
	void listPath();           //����xml��ѡ��·�����б��г���
	void generateXml();        //����xml��·�������ɱ���ָ������symbol��xml
	void changeType();         //����xml���ı��б�������combo��Ӧ
	void changeFavorite();     //����xml���ı��б����ղ�check��Ӧ
private:
	Ui::S3dmXmlGenerateDlg ui;
	QgsStyle *mGetStyle;
	QString m_dir;             //����xml��ѡ������xml���ļ���Ŀ¼
	vector<string> m_subPaths; //����xml����Ŀ¼�µ������ļ�������
};
