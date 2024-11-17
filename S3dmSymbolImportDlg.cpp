#include "S3dmSymbolImportDlg.h"

#include "S3dmSymbolManager.h"

#include "qfiledialog.h"
#include "qgssymbol.h"
#include "qpushbutton.h"
#include "qgsmarkersymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qmessagebox.h"

//#include "shellapi.h"

using namespace std;
S3dmSymbolImportDlg::S3dmSymbolImportDlg(QgsStyle *style, QString tag, QWidget *parent)
	: mGetStyle(style)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);//关闭窗口即销毁
	mTempStyle = qgis::make_unique< QgsStyle >();
	mTempStyle->createMemoryDatabase();
	ui.checkboxFavorite->setChecked(true);
	ui.comboBox->addItem(QStringLiteral("点符号"));
	ui.comboBox->addItem(QStringLiteral("线符号"));
	ui.comboBox->addItem(QStringLiteral("面符号"));
	ui.comboBox->setCurrentIndex(0);
	ui.radioSelect->setChecked(true);
	ui.checkBoxAll->setChecked(false);
	ui.mSymbolTags->setEnabled(false);
	ui.mSymbolTags->setText(tag);
	ui.buttonBox->button(QDialogButtonBox::Ok)->setText(QStringLiteral("导入"));
	ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
	connect(ui.buttonOpen, &QPushButton::clicked, this, &S3dmSymbolImportDlg::openFiles);

	connect(ui.radioCreate, &QPushButton::clicked, this, &S3dmSymbolImportDlg::switchUse);
	connect(ui.radioSelect, &QPushButton::clicked, this, &S3dmSymbolImportDlg::switchUse);
	connect(ui.radioCreate, &QPushButton::clicked, this, &S3dmSymbolImportDlg::switchUse);
	connect(ui.comboBox, &QComboBox::currentTextChanged, this, &S3dmSymbolImportDlg::switchUse);

	double iconSize = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance('X') * 10;
	ui.listItems->setIconSize(QSize(static_cast<int>(iconSize), static_cast<int>(iconSize * 0.9)));
	ui.listItems->setGridSize(QSize(static_cast<int>(iconSize*1.1), static_cast<int>(iconSize * 1.2)));
	mShowStyle = mTempStyle.get();
	mModel = new QgsStyleProxyModel(mShowStyle);
	mModel->addDesiredIconSize(ui.listItems->iconSize());
	ui.listItems->setModel(mModel);
	ui.listItems->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui.listItems->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(ui.listItems->selectionModel(), &QItemSelectionModel::selectionChanged, this, &S3dmSymbolImportDlg::selectionChanged);

	connect(ui.checkBoxAll, &QPushButton::clicked, this, &S3dmSymbolImportDlg::selectAll);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &S3dmSymbolImportDlg::doImport);
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &S3dmSymbolImportDlg::closeDlg);
}

S3dmSymbolImportDlg::~S3dmSymbolImportDlg()
{
	if (mTempStyle!=nullptr)
	{
		mTempStyle.get()->clear();
		mTempStyle.release();
	}
	if (mShowStyle)
	{
		mShowStyle->clear();
		delete mShowStyle;
		mShowStyle = nullptr;
	}
	if (mModel)
	{
		delete mModel;
		mModel = nullptr;
	}
}

void S3dmSymbolImportDlg::switchUse()
{
	if (ui.radioCreate->isChecked())
	{
		ui.lineEditSelect->clear();
		ui.lineEditSelect->setEnabled(false);
		ui.buttonOpen->setEnabled(false);
		QString path = "";
		QStringList pathList = path.split("/");
		pathList.pop_back();
		pathList.pop_back();
		pathList.pop_back();
		path = "";
		for (int i = 0; i < pathList.size(); i++)
		{
			path += pathList[i] + "/";
		}
		//path += "inkscape/bin";
		//ShellExecute(NULL, "open", "inkscape.exe", NULL, path.toLocal8Bit().data(), SW_HIDE);
	}
	else if (ui.radioSelect->isChecked())
	{
		ui.lineEditSelect->setEnabled(true);
		ui.buttonOpen->setEnabled(true);
	}

	if (!ui.radioCreate->isChecked())
	{
		changeSymbolType();
	}
}

void S3dmSymbolImportDlg::selectAll()
{
	if (ui.checkBoxAll->isChecked())
	{
		ui.listItems->selectAll();
		ui.checkBoxAll->setChecked(true);
	}
	else
	{
		ui.listItems->clearSelection();
		ui.checkBoxAll->setChecked(false);
	}
}

void S3dmSymbolImportDlg::openFiles()
{
	mSymbolNum = 0;
	mGetSymbolNames.clear();
	mGetSymbolNames = QFileDialog::getOpenFileNames(this, QStringLiteral("导入SVG/Image文件"), "", QStringLiteral("SVG文件(*.svg);; 图像文件(*.bmp *.cur *.gif *.icns *.ico *.jpeg *.jpg *.pbm *.pdf *.pgm *.png *.ppm *.tga *.tif *.tiff *.wbmp *.webp *.xbm *.xpm)"));
	if (mGetSymbolNames.size() < 1)
	{
		return;
	}
	QFileInfo file(mGetSymbolNames[0]);
	ui.lineEditSelect->setText(file.path());
	changeSymbolType();
}

void S3dmSymbolImportDlg::changeSymbolType()
{
	if (mGetSymbolNames.size() < 1)
	{
		return;
	}
	QFileInfo file(mGetSymbolNames[0]);
	mShowStyle->clear();
	for (int i = 0; i < mGetSymbolNames.size(); i++)
	{
		QFileInfo singlefile(mGetSymbolNames[i]);
		if (ui.comboBox->currentText() == QStringLiteral("点符号"))
		{
			QgsMarkerSymbol *symbol = new QgsMarkerSymbol();
			if (file.suffix() == "svg" || file.suffix() == "SVG")
			{
				QgsSvgMarkerSymbolLayer *symbolLayer = new QgsSvgMarkerSymbolLayer(mGetSymbolNames[i]);
				symbol->deleteSymbolLayer(0);
				symbol->appendSymbolLayer(symbolLayer);
			}
			else
			{
				QgsRasterMarkerSymbolLayer *symbolLayer = new QgsRasterMarkerSymbolLayer(mGetSymbolNames[i]);
				symbol->deleteSymbolLayer(0);
				symbol->appendSymbolLayer(symbolLayer);
			}
			mShowStyle->addSymbol(singlefile.baseName(), symbol->clone(), true);
		}
		else if (ui.comboBox->currentText() == QStringLiteral("线符号"))
		{
			QgsLineSymbol *symbol = new QgsLineSymbol();
			QgsMarkerLineSymbolLayer *symbolLayer = new QgsMarkerLineSymbolLayer();
			QgsMarkerSymbol *subSymbol = new QgsMarkerSymbol();
			if (file.suffix() == "svg" || file.suffix() == "SVG")
			{
				QgsSvgMarkerSymbolLayer *subSymbolLayer = new QgsSvgMarkerSymbolLayer(mGetSymbolNames[i]);
				subSymbol->deleteSymbolLayer(0);
				subSymbol->appendSymbolLayer(subSymbolLayer);
			}
			else
			{
				QgsRasterMarkerSymbolLayer *subSymbolLayer = new QgsRasterMarkerSymbolLayer(mGetSymbolNames[i]);
				subSymbol->deleteSymbolLayer(0);
				subSymbol->appendSymbolLayer(subSymbolLayer);
			}
			symbol->deleteSymbolLayer(0);
			symbol->appendSymbolLayer(symbolLayer);
			mShowStyle->addSymbol(singlefile.baseName(), symbol->clone(), true);
		}
		else if (ui.comboBox->currentText() == QStringLiteral("面符号"))
		{
			QgsFillSymbol *symbol = new QgsFillSymbol();
			if (file.suffix() == "svg" || file.suffix() == "SVG")
			{
				QgsSVGFillSymbolLayer *symbolLayer = new QgsSVGFillSymbolLayer(mGetSymbolNames[i]);
				symbol->deleteSymbolLayer(0);
				symbol->appendSymbolLayer(symbolLayer);
				((QgsSimpleLineSymbolLayer*)(symbolLayer->subSymbol()->symbolLayer(0)))->setPenStyle(Qt::PenStyle::NoPen);
			}
			else
			{
				QgsRasterFillSymbolLayer *symbolLayer = new QgsRasterFillSymbolLayer(mGetSymbolNames[i]);
				symbol->deleteSymbolLayer(0);
				symbol->appendSymbolLayer(symbolLayer);
			}
			mShowStyle->addSymbol(singlefile.baseName(), symbol->clone(), true);
		}
		mSymbolNum++;
	}

	ui.checkBoxAll->setChecked(true);
	selectAll();
}


void S3dmSymbolImportDlg::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(selected)
	Q_UNUSED(deselected)
	bool nothingSelected = ui.listItems->selectionModel()->selectedIndexes().empty();
	bool allSelected = ui.listItems->selectionModel()->selectedIndexes().size() == mSymbolNum;
	if (nothingSelected)
	{
		ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(nothingSelected);
		ui.checkBoxAll->setChecked(false);
	}
	else if(allSelected)
	{
		ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
		ui.checkBoxAll->setChecked(true);
	}
	else
	{
		ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
		ui.checkBoxAll->setChecked(false);
	}
}

void S3dmSymbolImportDlg::doImport()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QStringList tags = ui.mSymbolTags->text().split(',');
	QModelIndexList allselection = ui.listItems->selectionModel()->selection().indexes();
	QModelIndexList selection;
	for (int i = 0; i < allselection.size(); i++)
	{
		if (allselection[i].column() > 0)
		{
			continue;
		}
		selection.push_back(allselection[i]);
	}
	QList< S3dmSymbolManager::ItemDetails > items;
	items.reserve(selection.size());
	QFileInfo file(mGetSymbolNames[0]);
	if (selection.size() == 0)
	{
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, QStringLiteral("导入符号"), QStringLiteral("当前选择为空，请选择符号导入"));
		return;
	}
	
	bool isCover = false;
	for (int i = 0; i < selection.size(); i++)
	{
		QModelIndex index = selection.at(i);

		QString name = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();
		bool hasName = mGetStyle->symbolNames().contains(name);
		if (hasName)
		{
			if (ui.comboBox->currentText() == QStringLiteral("点符号") && mGetStyle->symbol(name)->type() == QgsSymbol::SymbolType::Marker || ui.comboBox->currentText() == QStringLiteral("线符号") && mGetStyle->symbol(name)->type() == QgsSymbol::SymbolType::Line || ui.comboBox->currentText() == QStringLiteral("面符号") && mGetStyle->symbol(name)->type() == QgsSymbol::SymbolType::Fill)
			{
				QApplication::restoreOverrideCursor();
				int res = QMessageBox::warning(this, QStringLiteral("导入符号"), QStringLiteral("导入的符号和已有符号存在相同类型下同名的情况，是否覆盖?（点击否将在名称后添加编号，点击取消将取消导入）").arg(name), QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No, QMessageBox::StandardButton::Cancel);
				if (res == QMessageBox::No)
				{
					isCover = false;
					break;
				} 
				else if (res == QMessageBox::Cancel)
				{
					QApplication::restoreOverrideCursor();
					return;
				}
				else
				{
					isCover = true;
					break;
				}
			}
		}
	}
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	for (int i = 0; i < selection.size(); i++)
	{
		QModelIndex index = selection.at(i);
		QString name = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();
		QString newName = name;
		const bool hasDuplicateName = mGetStyle->symbolNames().contains(name);
		if (ui.comboBox->currentText() == QStringLiteral("点符号"))
		{
			if (hasDuplicateName)
			{
				if (mGetStyle->symbol(name)->type() == QgsSymbol::SymbolType::Marker)
				{
					if (!isCover)
					{
						bool stillDup = false;
						for (int i = 1; i < 1000000; i++)
						{
							newName = name + "_" + QString::number(i);
							stillDup = mGetStyle->symbolNames().contains(newName);
							if (!stillDup)
							{
								break;
							}
						}
					}
				}
			}
			QgsMarkerSymbol *symbol = new QgsMarkerSymbol();
			if (file.suffix() == "svg" || file.suffix() == "SVG")
			{
				QgsSvgMarkerSymbolLayer *symbolLayer = new QgsSvgMarkerSymbolLayer(ui.lineEditSelect->text() + "/" + name + "." + file.suffix());
				symbol->deleteSymbolLayer(0);
				symbol->appendSymbolLayer(symbolLayer);
			}
			else
			{
				QgsRasterMarkerSymbolLayer *symbolLayer = new QgsRasterMarkerSymbolLayer(ui.lineEditSelect->text() + "/" + name + "." + file.suffix());
				symbol->deleteSymbolLayer(0);
				symbol->appendSymbolLayer(symbolLayer);
			}
			mGetStyle->addSymbol(newName, symbol->clone(), true);
			mGetStyle->saveSymbol(newName, symbol->clone(), ui.checkboxFavorite->isChecked(), tags);
			mGetStyle->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, newName, tags);
			if (ui.checkboxFavorite->isChecked())
			{
				mGetStyle->addFavorite(QgsStyle::StyleEntity::SymbolEntity, newName);
			}
			if (symbol != nullptr)
			{
				delete symbol;
				symbol = nullptr;
			}
		}
		else if (ui.comboBox->currentText() == QStringLiteral("线符号"))
		{
			if (hasDuplicateName)
			{
				if (mGetStyle->symbol(name)->type() == QgsSymbol::SymbolType::Line)
				{
					if (!isCover)
					{
						bool stillDup = false;
						for (int i = 1; i < 1000000; i++)
						{
							newName = name + "_" + QString::number(i);
							stillDup = mGetStyle->symbolNames().contains(newName);
							if (!stillDup)
							{
								break;
							}
						}
					}
				}
			}
			QgsLineSymbol *symbol = new QgsLineSymbol();
			QgsMarkerLineSymbolLayer *symbolLayer = new QgsMarkerLineSymbolLayer();
			QgsMarkerSymbol *subSymbol = new QgsMarkerSymbol();
			if (file.suffix() == "svg" || file.suffix() == "SVG")
			{
				QgsSvgMarkerSymbolLayer *subSymbolLayer = new QgsSvgMarkerSymbolLayer(ui.lineEditSelect->text() + "/" + name + "." + file.suffix());
				subSymbol->deleteSymbolLayer(0);
				subSymbol->appendSymbolLayer(subSymbolLayer);
			}
			else
			{
				QgsRasterMarkerSymbolLayer *subSymbolLayer = new QgsRasterMarkerSymbolLayer(ui.lineEditSelect->text() + "/" + name + "." + file.suffix());
				subSymbol->deleteSymbolLayer(0);
				subSymbol->appendSymbolLayer(subSymbolLayer);
			}
			symbolLayer->setSubSymbol(subSymbol);
			symbol->deleteSymbolLayer(0);
			symbol->appendSymbolLayer(symbolLayer);
			mGetStyle->addSymbol(newName, symbol->clone(), true);
			mGetStyle->saveSymbol(newName, symbol->clone(), ui.checkboxFavorite->isChecked(), tags);
			mGetStyle->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, newName, tags);
			if (ui.checkboxFavorite->isChecked())
			{
				mGetStyle->addFavorite(QgsStyle::StyleEntity::SymbolEntity, newName);
			}
			if (symbol != nullptr)
			{
				delete symbol;
				symbol = nullptr;
			}
		}
		else if (ui.comboBox->currentText() == QStringLiteral("面符号"))
		{
			if (hasDuplicateName)
			{
				if (mGetStyle->symbol(name)->type() == QgsSymbol::SymbolType::Fill)
				{
					if (!isCover)
					{
						bool stillDup = false;
						for (int i = 1; i < 1000000; i++)
						{
							newName = name + "_" + QString::number(i);
							stillDup = mGetStyle->symbolNames().contains(newName);
							if (!stillDup)
							{
								break;
							}
						}
					}
				}
			}
			QgsFillSymbol *symbol = new QgsFillSymbol();
			if (file.suffix() == "svg" || file.suffix() == "SVG")
			{
				QgsSVGFillSymbolLayer *symbolLayer = new QgsSVGFillSymbolLayer(ui.lineEditSelect->text() + "/" + name + "." + file.suffix());
				symbol->deleteSymbolLayer(0);
				symbol->appendSymbolLayer(symbolLayer);
				((QgsSimpleLineSymbolLayer*)(symbolLayer->subSymbol()->symbolLayer(0)))->setPenStyle(Qt::PenStyle::NoPen);
			}
			else
			{
				QgsRasterFillSymbolLayer *symbolLayer = new QgsRasterFillSymbolLayer(ui.lineEditSelect->text() + "/" + name + "." + file.suffix());
				symbol->deleteSymbolLayer(0);
				symbol->appendSymbolLayer(symbolLayer);
			}
			mGetStyle->addSymbol(newName, symbol->clone(), true);
			mGetStyle->saveSymbol(newName, symbol->clone(), ui.checkboxFavorite->isChecked(), tags);
			mGetStyle->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, newName, tags);
			if (ui.checkboxFavorite->isChecked())
			{
				mGetStyle->addFavorite(QgsStyle::StyleEntity::SymbolEntity, newName);
			}
			if (symbol != nullptr)
			{
				delete symbol;
				symbol = nullptr;
			}
		}
	}
	QApplication::restoreOverrideCursor();
	this->accept();
}

void S3dmSymbolImportDlg::closeDlg()
{
	mTempStyle.get()->clear();
	mTempStyle.release();
	mShowStyle->clear();
	if (mModel)
	{
		delete mModel;
		mModel = nullptr;
	}
	
	this->close();
}
