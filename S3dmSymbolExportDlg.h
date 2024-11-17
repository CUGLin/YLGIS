#pragma once

#include <QDialog>
#include "s3d_stylemanagerlib_global.h"
#include "ui_S3dmSymbolExportDlg.h"

class QgsSymbol;
class S3D_STYLEMANAGERLIB_EXPORT S3dmSymbolExportDlg : public QDialog
{
	Q_OBJECT

public:
	enum Type
	{
		SVG = 0,
		PNG = 1
	};

	S3dmSymbolExportDlg(Type type, QWidget *parent = Q_NULLPTR);
	~S3dmSymbolExportDlg();

public:
	double height();
	double width();
	QString path();
private:
	void selectPath();
	
private:
	Ui::S3dmSymbolExportDlg ui;
	Type mType;
};
