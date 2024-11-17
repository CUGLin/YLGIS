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
	setAttribute(Qt::WA_DeleteOnClose, true);//�رմ��ڼ�����
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
	for (int i = 2; i < list.size(); i++) //ǰ����Ϊ.��..��Ҫ����
	{
		m_subPaths.push_back(list[i].toLocal8Bit().data());
	}
	ui.tableWidget->clear();
	ui.tableWidget->setRowCount(m_subPaths.size());
	ui.tableWidget->setColumnCount(4);
	QStringList header;
	header << QStringLiteral("�ļ�������") << QStringLiteral("���ɷ�������") << QStringLiteral("��ǩ") << QStringLiteral("�Ƿ��ղ�");
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
		type->addItem(QStringLiteral("�����"));
		type->addItem(QStringLiteral("�߷���"));
		type->addItem(QStringLiteral("������"));
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
			QMessageBox::warning(this, QStringLiteral("���ɷ��ſ�"), QString::fromLocal8Bit("��ʾ��·����%1����ȡ����Ϊ��").arg(folder));
			continue;
		}
		//TODO...������ȡ��ͬ���ļ��еĲ㼶
		for (int j = 0; j < subPaths.size(); j++)
		{
			QString totalPath = m_dir + "/" + name + "/" + subPaths[j];
			QString relativePath = name + "/" + subPaths[j];

			QFileInfo file(subPaths[j]);
			if (file.suffix() == "svg" || file.suffix() == "SVG")
			{
				if (type == QStringLiteral("�����"))
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
				else if (type == QStringLiteral("�߷���"))
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
				if (type == QStringLiteral("�����"))
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
				else if (type == QStringLiteral("�߷���"))
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
		QMessageBox::warning(this, QStringLiteral("����xml"), QStringLiteral("���ɳɹ���"));
	}
	else
	{
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, QStringLiteral("����xml"), QStringLiteral("����ʧ�ܣ�"));
	}
	tmpStyle.get()->clear();
	tmpStyle.release();
}

void S3dmXmlGenerateDlg::changeType()
{
	//�Ȼ�ȡ�����combo�鿴���ɵ�����
	QComboBox *getCombo = qobject_cast<QComboBox*>(this->sender());
	int typeIndex = getCombo->currentIndex();
	// ��ȡ��ť��x�����y����
	ui.tableWidget->setFocusPolicy(Qt::NoFocus);
	// ���ݰ�ť��x��y��������λ��Ӧ�ĵ�Ԫ��
	QModelIndex index = ui.tableWidget->indexAt(getCombo->pos());
	// ��ȡ�ð�ť���ڱ����кź��к�
	int row = index.row();
	//��ȡ��ѡ���У�ͳһ�ı�combo
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
	//�Ȼ�ȡ�����check�鿴�Ƿ�ѡ
	QCheckBox *getCheck = qobject_cast<QCheckBox*>(this->sender());
	bool isCheck = getCheck->isChecked();
	// ��ȡ��ť��x�����y����
	ui.tableWidget->setFocusPolicy(Qt::NoFocus);
	// ���ݰ�ť��x��y��������λ��Ӧ�ĵ�Ԫ��
	QModelIndex index = ui.tableWidget->indexAt(getCheck->pos());
	// ��ȡ�ð�ť���ڱ����кź��к�
	int row = index.row();
	//��ȡ��ѡ���У�ͳһ�ı乴ѡ״̬
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
