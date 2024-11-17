#include "S3dmXmlGenerateDlg.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "qcombobox.h"
#include "qcheckbox.h"
#include "qgsmarkersymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproject.h"

S3dmXmlGenerateDlg::S3dmXmlGenerateDlg(QgsStyle *style, QWidget *parent)
	: mGetStyle(style)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);//关闭窗口即销毁
	ui.editPath->setEnabled(false);
	ui.lineEdit->setText("symbol");
	connect(ui.buttonGenerate, &QPushButton::clicked, this, &S3dmXmlGenerateDlg::generateXml);
	connect(ui.buttonCancel,&QPushButton::clicked,this,&S3dmXmlGenerateDlg::close);
	listPath();
}

S3dmXmlGenerateDlg::~S3dmXmlGenerateDlg()
{
}

void S3dmXmlGenerateDlg::listPath()
{
	m_subPaths.clear();
	QStringList defaultDirList = QgsApplication::applicationDirPath().split("/");
	QString defaultDir = "";
	for (int i = 0; i < defaultDirList.size() - 2; i++)
	{
		defaultDir += defaultDirList[i] + "/";
	}
	defaultDir += "qgis-ltr/svg";
	m_dir = defaultDir;
	if (m_dir.isEmpty())
	{
		return;
	}
	ui.editPath->setText(m_dir);

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QDir dir(m_dir);
	QStringList list = dir.entryList();
	for (int i = 2; i < list.size(); i++) //前两个为.和..需要跳过
	{
		m_subPaths.push_back(list[i].toLocal8Bit().data());
	}
	ui.tableWidget->clear();
	ui.tableWidget->setRowCount(m_subPaths.size());
	ui.tableWidget->setColumnCount(4);
	QStringList header;
	header << QStringLiteral("文件夹名称") << QStringLiteral("生成符号类型") << QStringLiteral("标签") << QStringLiteral("是否收藏");
	ui.tableWidget->setHorizontalHeaderLabels(header);
	
	for (int i = 0; i < m_subPaths.size(); i++)
	{
		ui.tableWidget->setRowHeight(i, 25);
		QFileInfo file(m_dir + "/" + QString::fromLocal8Bit(m_subPaths[i].c_str()));
		QLineEdit *name = new QLineEdit();
		name->setEnabled(false);
		name->setText(file.baseName());
		ui.tableWidget->setCellWidget(i, 0, name);
		QComboBox *type = new QComboBox();
		type->addItem(QStringLiteral("点符号"));
		type->addItem(QStringLiteral("线符号"));
		type->addItem(QStringLiteral("填充符号"));
		type->setCurrentIndex(0);
		ui.tableWidget->setCellWidget(i, 1, type);
		connect(type, &QComboBox::currentTextChanged, this, &S3dmXmlGenerateDlg::changeType);
		QLineEdit *label = new QLineEdit();
		label->setText(file.baseName());
		ui.tableWidget->setCellWidget(i, 2, label);
		QCheckBox *isfavorite = new QCheckBox();
		isfavorite->setChecked(true);
		ui.tableWidget->setCellWidget(i, 3, isfavorite);
		connect(isfavorite, &QCheckBox::clicked, this, &S3dmXmlGenerateDlg::changeFavorite);
	}
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);

	QApplication::restoreOverrideCursor();
}

void S3dmXmlGenerateDlg::generateXml()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	std::unique_ptr< QgsStyle > tmpStyle = qgis::make_unique< QgsStyle >();
	tmpStyle->createMemoryDatabase();
	for (int i = 0; i < ui.tableWidget->rowCount(); i++)
	{
		QLineEdit *line1 = qobject_cast<QLineEdit*>(ui.tableWidget->cellWidget(i, 0));
		QString name = line1->text();
		QComboBox *combo = qobject_cast<QComboBox*>(ui.tableWidget->cellWidget(i, 1));
		QString type = combo->currentText();
		QLineEdit *line = qobject_cast<QLineEdit*>(ui.tableWidget->cellWidget(i, 2));
		QString label = line->text();
		QCheckBox *check = qobject_cast<QCheckBox*>(ui.tableWidget->cellWidget(i, 3));
		bool isfavorite = check->isChecked();
		QStringList tags;
		tags.push_back(label);
		QString folder = m_dir + "/" + QString::fromLocal8Bit(m_subPaths[i].c_str());
		QDir dir(folder);
		dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
		QStringList subPaths = dir.entryList();
		QFileInfo file(folder);
		if (subPaths.size() < 1 && file.suffix().isEmpty())
		{
			QApplication::restoreOverrideCursor();
			QMessageBox::warning(this, QStringLiteral("生成符号库"), QString::fromLocal8Bit("提示：路径“%1”获取内容为空").arg(folder));
			continue;
		}
		//TODO...做到读取不同级文件夹的层级
		for (int j = 0; j < subPaths.size(); j++)
		{
			QString totalPath = m_dir + "/" + name + "/" + subPaths[j];
			QString relativePath = name + "/" + subPaths[j];

			QFileInfo file(subPaths[j]);
			if (file.suffix() == "svg" || file.suffix() == "SVG")
			{
				if (type == QStringLiteral("点符号"))
				{
					QgsMarkerSymbol *symbol = new QgsMarkerSymbol();
					QgsSvgMarkerSymbolLayer *symbolLayer = new QgsSvgMarkerSymbolLayer(relativePath);
					symbol->deleteSymbolLayer(0);
					symbol->appendSymbolLayer(symbolLayer);
					tmpStyle.get()->addSymbol(file.baseName(),symbol,true);
					tmpStyle.get()->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, file.baseName(), tags);
					if (isfavorite)
					{
						tmpStyle.get()->addFavorite(QgsStyle::StyleEntity::SymbolEntity, file.baseName());
					}
				}
				else if (type == QStringLiteral("线符号"))
				{
					QgsLineSymbol *symbol = new QgsLineSymbol();
					QgsMarkerLineSymbolLayer *symbolLayer = new QgsMarkerLineSymbolLayer();
					QgsMarkerSymbol *subSymbol = new QgsMarkerSymbol();
					QgsSvgMarkerSymbolLayer *subSymbolLayer = new QgsSvgMarkerSymbolLayer(relativePath);
					subSymbol->deleteSymbolLayer(0);
					subSymbol->appendSymbolLayer(subSymbolLayer);
					symbolLayer->setSubSymbol(subSymbol);
					symbol->deleteSymbolLayer(0);
					symbol->appendSymbolLayer(symbolLayer);
					tmpStyle.get()->addSymbol(file.baseName(), symbol, true);
					tmpStyle.get()->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, file.baseName(), tags);
					if (isfavorite)
					{
						tmpStyle.get()->addFavorite(QgsStyle::StyleEntity::SymbolEntity, file.baseName());
					}
				}
				else
				{
					QgsFillSymbol *symbol = new QgsFillSymbol();
					QgsSVGFillSymbolLayer *symbolLayer = new QgsSVGFillSymbolLayer(totalPath);
					symbol->deleteSymbolLayer(0);
					symbol->appendSymbolLayer(symbolLayer);
					((QgsSimpleLineSymbolLayer*)(symbolLayer->subSymbol()->symbolLayer(0)))->setPenStyle(Qt::PenStyle::NoPen);
					tmpStyle.get()->addSymbol(file.baseName(), symbol, true);
					tmpStyle.get()->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, file.baseName(), tags);
					if (isfavorite)
					{
						tmpStyle.get()->addFavorite(QgsStyle::StyleEntity::SymbolEntity, file.baseName());
					}
				}
			}
			else
			{
				if (type == QStringLiteral("点符号"))
				{
					QgsMarkerSymbol *symbol = new QgsMarkerSymbol();
					QgsRasterMarkerSymbolLayer *symbolLayer = new QgsRasterMarkerSymbolLayer(totalPath);
					symbol->deleteSymbolLayer(0);
					symbol->appendSymbolLayer(symbolLayer);
					tmpStyle.get()->addSymbol(file.baseName(), symbol, true);
					tmpStyle.get()->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, file.baseName(), tags);
					if (isfavorite)
					{
						tmpStyle.get()->addFavorite(QgsStyle::StyleEntity::SymbolEntity, file.baseName());
					}
				}
				else if (type == QStringLiteral("线符号"))
				{
					QgsLineSymbol *symbol = new QgsLineSymbol();
					QgsMarkerLineSymbolLayer *symbolLayer = new QgsMarkerLineSymbolLayer();
					QgsMarkerSymbol *subSymbol = new QgsMarkerSymbol();
					QgsRasterMarkerSymbolLayer *subSymbolLayer = new QgsRasterMarkerSymbolLayer(totalPath);
					subSymbol->deleteSymbolLayer(0);
					subSymbol->appendSymbolLayer(subSymbolLayer);
					tmpStyle.get()->addSymbol(file.baseName(), symbol, true);
					tmpStyle.get()->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, file.baseName(), tags);
					if (isfavorite)
					{
						tmpStyle.get()->addFavorite(QgsStyle::StyleEntity::SymbolEntity, file.baseName());
					}
				}
				else
				{
					QgsFillSymbol *symbol = new QgsFillSymbol();
					QgsRasterFillSymbolLayer *symbolLayer = new QgsRasterFillSymbolLayer(totalPath);
					symbol->deleteSymbolLayer(0);
					symbol->appendSymbolLayer(symbolLayer);
					tmpStyle.get()->addSymbol(file.baseName(), symbol, true);
					tmpStyle.get()->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, file.baseName(), tags);
					if (isfavorite)
					{
						tmpStyle.get()->addFavorite(QgsStyle::StyleEntity::SymbolEntity, file.baseName());
					}
				}
			}
		}
	}
	bool status = tmpStyle.get()->exportXml(m_dir + "/" + ui.lineEdit->text() + ".xml");
	if (status)
	{
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, QStringLiteral("生成xml"), QStringLiteral("生成成功！"));
	}
	else
	{
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, QStringLiteral("生成xml"), QStringLiteral("生成失败！"));
	}
	tmpStyle.get()->clear();
	tmpStyle.release();
}

void S3dmXmlGenerateDlg::changeType()
{
	//先获取点击的combo查看换成的类型
	QComboBox *getCombo = qobject_cast<QComboBox*>(this->sender());
	int typeIndex = getCombo->currentIndex();
	// 获取按钮的x坐标和y坐标
	ui.tableWidget->setFocusPolicy(Qt::NoFocus);
	// 根据按钮的x和y坐标来定位对应的单元格
	QModelIndex index = ui.tableWidget->indexAt(getCombo->pos());
	// 获取该按钮所在表格的行号和列号
	int row = index.row();
	//获取多选的行，统一改变combo
	QModelIndexList indexList = ui.tableWidget->selectionModel()->selectedRows();
	for (int i = 0; i < indexList.count(); i++)
	{
		if (indexList[i].row() == row)
		{
			continue;
		}
		QComboBox *changeCombo = qobject_cast<QComboBox*>(ui.tableWidget->cellWidget(indexList[i].row(), 1));
		changeCombo->setCurrentIndex(typeIndex);
	}
}

void S3dmXmlGenerateDlg::changeFavorite()
{
	//先获取点击的check查看是否勾选
	QCheckBox *getCheck = qobject_cast<QCheckBox*>(this->sender());
	bool isCheck = getCheck->isChecked();
	// 获取按钮的x坐标和y坐标
	ui.tableWidget->setFocusPolicy(Qt::NoFocus);
	// 根据按钮的x和y坐标来定位对应的单元格
	QModelIndex index = ui.tableWidget->indexAt(getCheck->pos());
	// 获取该按钮所在表格的行号和列号
	int row = index.row();
	//获取多选的行，统一改变勾选状态
	QModelIndexList indexList = ui.tableWidget->selectionModel()->selectedRows();
	for (int i = 0; i < indexList.count(); i++)
	{
		if (indexList[i].row() == row)
		{
			continue;
		}
		QCheckBox *changeCheck = qobject_cast<QCheckBox*>(ui.tableWidget->cellWidget(indexList[i].row(), 3));
		changeCheck->setChecked(isCheck);
	}
}
