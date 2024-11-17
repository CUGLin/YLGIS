#include "S3dmSymbolExportDlg.h"
#include "qgssymbol.h"
#include "qfiledialog.h"

using namespace std;
S3dmSymbolExportDlg::S3dmSymbolExportDlg(Type type, QWidget *parent)
	: mType(type), QDialog(parent)
{
	ui.setupUi(this);
	ui.spinWidth->setValue(150);
	ui.spinHeight->setValue(150);
	connect(ui.buttonPath, &QPushButton::clicked, this, &S3dmSymbolExportDlg::selectPath);
	connect(ui.buttonExport, &QPushButton::clicked, this, &S3dmSymbolExportDlg::accept);
}

S3dmSymbolExportDlg::~S3dmSymbolExportDlg()
{
}

void S3dmSymbolExportDlg::selectPath()
{
	QString filepath = "";
	if (mType == Type::SVG)
	{
		filepath = QFileDialog::getExistingDirectory(this, QStringLiteral("导出为SVG"), QDir::home().absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	}
	else if (mType == Type::PNG)
	{
		filepath = QFileDialog::getExistingDirectory(this, QStringLiteral("导出为Image"), QDir::home().absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	}
	ui.lineEdit->setText(filepath);
}

double S3dmSymbolExportDlg::height()
{
	return ui.spinHeight->value();
}

double S3dmSymbolExportDlg::width()
{
	return ui.spinWidth->value();
}

QString S3dmSymbolExportDlg::path()
{
	return ui.lineEdit->text();
}
