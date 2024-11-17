#ifndef S3DMSYMBOLSAVEDLG_H
#define S3DMSYMBOLSAVEDLG_H

#include <QDialog>
#include "s3d_stylemanagerlib_global.h"
#include "ui_s3dmsymbolsavedlg.h"

class S3D_STYLEMANAGERLIB_EXPORT S3dmSymbolSaveDlg : public QDialog
{
	Q_OBJECT

public:
	S3dmSymbolSaveDlg(QWidget *parent = nullptr);

	QString name() const;
	void setDefaultTags(const QString &tags);
	QString tags() const;
	bool isFavorite() const;
private:
	void closeDlg();
private:
	Ui::S3dmSymbolSaveDlg ui;
};

#endif // S3DMSYMBOLSAVEDLG_H
