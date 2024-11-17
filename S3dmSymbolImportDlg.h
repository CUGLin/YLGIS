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
	void switchUse();         //切换符号类型响应和选择文件/新建文件响应
	void selectAll();         //选择全部列出的项
	void openFiles();         //选择svg/image文件
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected); //切换选择状态响应
	void doImport();          //导入响应
	void closeDlg();          //关闭界面响应
private:
	void changeSymbolType();  //和switchUse有关，切换类型后显示的symbol也要随之修改
private:
	Ui::S3dmSymbolImportDlg ui;
	QStringList mGetSymbolNames;
	QgsStyle *mGetStyle;
	std::unique_ptr< QgsStyle > mTempStyle;
	QgsStyle *mShowStyle;
	QgsStyleProxyModel *mModel;
	int mSymbolNum;           //用于判断全选状态
};
