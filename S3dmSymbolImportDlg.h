#pragma once

#include <QDialog>
#include "s3d_stylemanagerlib_global.h"
#include "ui_S3dmSymbolImportDlg.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgshelp.h"
#include "qgsstylemodel.h"
#include <memory>

class QgsStyle;
class QgsStyleGroupSelectionDialog;
class QgsTemporaryCursorOverride;
class QgsStyleProxyModel;
class QTemporaryFile;

class S3D_STYLEMANAGERLIB_EXPORT S3dmSymbolImportDlg : public QDialog
{
	Q_OBJECT
public:
	S3dmSymbolImportDlg(QgsStyle *style, QString tag, QWidget *parent = Q_NULLPTR);
	~S3dmSymbolImportDlg();

private:
	void switchUse();         //�л�����������Ӧ��ѡ���ļ�/�½��ļ���Ӧ
	void selectAll();         //ѡ��ȫ���г�����
	void openFiles();         //ѡ��svg/image�ļ�
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected); //�л�ѡ��״̬��Ӧ
	void doImport();          //������Ӧ
	void closeDlg();          //�رս�����Ӧ
private:
	void changeSymbolType();  //��switchUse�йأ��л����ͺ���ʾ��symbolҲҪ��֮�޸�
private:
	Ui::S3dmSymbolImportDlg ui;
	QStringList mGetSymbolNames;
	QgsStyle *mGetStyle;
	std::unique_ptr< QgsStyle > mTempStyle;
	QgsStyle *mShowStyle;
	QgsStyleProxyModel *mModel;
	int mSymbolNum;           //�����ж�ȫѡ״̬
};
