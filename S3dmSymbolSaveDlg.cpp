#include "s3dmsymbolsavedlg.h"
#include "qgsstyle.h"
#include "qpushbutton.h"

S3dmSymbolSaveDlg::S3dmSymbolSaveDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList defaultTags = QgsStyle::defaultStyle()->tags();
	defaultTags.sort(Qt::CaseInsensitive);
	ui.mTags->addItems(defaultTags);
	
	connect(ui.buttonSave, &QPushButton::clicked, this, &S3dmSymbolSaveDlg::closeDlg);
}

QString S3dmSymbolSaveDlg::name() const
{
	return ui.mName->text();
}

void S3dmSymbolSaveDlg::setDefaultTags(const QString &tags)
{
	ui.mTags->setCurrentText(tags);
}

QString S3dmSymbolSaveDlg::tags() const
{
	return ui.mTags->currentText();
}

bool S3dmSymbolSaveDlg::isFavorite() const
{
	return ui.mFavorite->isChecked();
}

void S3dmSymbolSaveDlg::closeDlg()
{
	this->accept();
}
