#include "S3dmApplySymbolDlg.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include "qgsstylemodel.h"
#include "qspinbox.h"
#include "qgsmarkersymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qmessagebox.h"
#include "qgsexpressioncontextutils.h"
#include "qcolordialog.h"
#include "qcheckbox.h"

S3dmApplySymbolDlg::S3dmApplySymbolDlg(QgsStyle *style, QgsStyleProxyModel *model, symbolType type, QWidget *parent)
	: mStyle(style), mModel(model), mType(type), QDialog(parent)
{
	ui.setupUi(this);
	//初始化控件
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	
	ui.spinInterval->setValue(3);
	ui.spinSize->setValue(4);
	ui.horizontalSlider->setRange(0, 100);
	ui.horizontalSlider->setValue(100);
	ui.spinRotate->setValue(0);
	ui.buttonColorSelect->setText("None");
	ui.buttonColorSelect->setEnabled(false);
	ui.checkBoxColor->setChecked(true);
	QStringList unitType;
	unitType.push_back(QStringLiteral("毫米"));
	unitType.push_back(QStringLiteral("点"));
	unitType.push_back(QStringLiteral("像素"));
	unitType.push_back(QStringLiteral("米"));
	unitType.push_back(QStringLiteral("地图单位"));
	unitType.push_back(QStringLiteral("英寸"));
	ui.comboIntervalUnit->addItems(unitType);
	ui.comboSizeUnit->addItems(unitType);
	ui.comboIntervalUnit->setCurrentIndex(0);
	ui.comboSizeUnit->setCurrentIndex(0);

	if (mType == symbolType::LINE)
	{
		ui.labelInterval->setVisible(true);
		ui.spinInterval->setVisible(true);
		ui.comboIntervalUnit->setVisible(true);
	}
	else
	{
		ui.labelInterval->setVisible(false);
		ui.spinInterval->setVisible(false);
		ui.comboIntervalUnit->setVisible(false);
	}
	changeComboTags();
	connect(ui.comboLabel, &QComboBox::currentTextChanged, this, &S3dmApplySymbolDlg::tagChanged);

	if (mModel == nullptr)
	{
		mModel = new QgsStyleProxyModel(mStyle, this);
	}
	double iconSize = Qgis::UI_SCALE_FACTOR * fontMetrics().maxWidth() * 10;
	
	ui.listView->setIconSize(QSize(static_cast<int>(iconSize*0.5), static_cast<int>(iconSize * 0.5)));
	ui.listView->setGridSize(QSize(static_cast<int>(iconSize*0.55), static_cast<int>(iconSize * 0.8)));
	mModel->addDesiredIconSize(ui.listView->iconSize());
	ui.listView->setModel(mModel);
	ui.listView->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui.listView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.listView->setViewMode(QListView::IconMode);
	QModelIndex index = mModel->index(0, 0);
	ui.listView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectionFlag::SelectCurrent);
	ui.listView->setToolTipDuration(-1);
	ui.listView->setWordWrap(true);
	mModel->setEntityFilter(QgsStyle::SymbolEntity);
	mModel->setEntityFilterEnabled(true);
	mModel->setSymbolTypeFilterEnabled(true);
	mModel->setSymbolType(static_cast<QgsSymbol::SymbolType>(type));
	mModel->setTagId(-1);
	mModel->setSmartGroupId(-1);
	mModel->setFavoritesOnly(false);

	connect(ui.listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &S3dmApplySymbolDlg::selectionChanged);

	mPreviewExpressionContext.appendScopes(QgsExpressionContextUtils::globalProjectLayerScopes(nullptr));
	QString symbolName = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();
	if (!symbolName.isEmpty())
	{
		QImage image = getImage(mStyle->symbol(symbolName), symbolName, &mPreviewExpressionContext);
		//QPixmap pixmap = QgsSymbolLayerUtils::symbolPreviewPixmap(mStyle->symbol(symbolName), QSize(50,50), iconSize * 0.45, nullptr, false, &mPreviewExpressionContext);
		//ui.labelPreview->setPixmap(pixmap);
		ui.labelPreview->setPixmap(QPixmap::fromImage(image));
		ui.labelPreview->setAcceptDrops(Qt::AlignCenter);
		ui.labelPreview->setAlignment(Qt::AlignCenter);
		mCurrentSymbol = mStyle->symbol(symbolName)->clone();
	}
	
	connect(ui.checkBoxColor, &QCheckBox::clicked, this, &S3dmApplySymbolDlg::changeToUseColor);
	connect(ui.buttonColorSelect, &QPushButton::clicked, this, &S3dmApplySymbolDlg::openColor);
	connect(ui.spinInterval, qgis::overload<double>::of(&QDoubleSpinBox::valueChanged), this, &S3dmApplySymbolDlg::changeToSymbol);
	connect(ui.spinSize, qgis::overload<double>::of(&QDoubleSpinBox::valueChanged), this, &S3dmApplySymbolDlg::changeToSymbol);
	connect(ui.comboIntervalUnit, &QComboBox::currentTextChanged, this, &S3dmApplySymbolDlg::changeToSymbol);
	connect(ui.comboSizeUnit, &QComboBox::currentTextChanged, this, &S3dmApplySymbolDlg::changeToSymbol);
	connect(ui.horizontalSlider, &QSlider::valueChanged, this, &S3dmApplySymbolDlg::changeToSymbol);
	connect(ui.spinRotate, qgis::overload<double>::of(&QDoubleSpinBox::valueChanged), this, &S3dmApplySymbolDlg::changeToSymbol);
	connect(ui.buttonYes, &QPushButton::clicked, this, &S3dmApplySymbolDlg::changeToLayer);

	ui.searchBox->setPlaceholderText(QStringLiteral("过滤…"));
	connect(ui.searchBox, &QLineEdit::textChanged, this, &S3dmApplySymbolDlg::filterSymbols);
	QApplication::restoreOverrideCursor();
}

S3dmApplySymbolDlg::~S3dmApplySymbolDlg()
{
	if (mCurrentSymbol)
	{
		delete mCurrentSymbol;
		mCurrentSymbol = nullptr;
	}
}

void S3dmApplySymbolDlg::setBackGroundColor(QColor &color)
{
	ui.buttonColorSelect->setEnabled(true);
	ui.checkBoxColor->setChecked(false);
	m_backgroundColor = color;
	QString text = "rgb:(" + QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue()) + ")";
	m_colorButtonText = text;
	ui.buttonColorSelect->setText(text);
	ui.buttonColorSelect->setStyleSheet("background-color:rgb(" + QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue()) + ")");
	//给图层
	changeToSymbol();
}

void S3dmApplySymbolDlg::setStyle(QgsStyle *style)
{
	mStyle = style;
	changeComboTags();
	ui.spinInterval->setValue(3);
	ui.spinSize->setValue(4);
	ui.horizontalSlider->setValue(100);
	ui.spinRotate->setValue(0);
}

void S3dmApplySymbolDlg::setCurrentSymbolName(QString name)
{
	if (name == QStringLiteral("全部"))
	{
		ui.comboLabel->setCurrentText(name);
		tagChanged();
		ui.searchBox->clear();
		filterSymbols("");
	}
	else
	{
		ui.searchBox->setText(name);
		filterSymbols(name);
		QModelIndex qindex = ui.listView->model()->index(0, 0);   //默认选第一个
		ui.listView->setCurrentIndex(qindex);
		changeSelect();
	}
	
}

void S3dmApplySymbolDlg::setCurrentColor(QColor color)
{
	m_backgroundColor = color;
	ui.buttonColorSelect->setEnabled(true);
	ui.checkBoxColor->setChecked(false);
	changeToUseColor();
}

void S3dmApplySymbolDlg::setCurrentType(symbolType type)
{
	mType = type;
	mModel->setSymbolType(static_cast<QgsSymbol::SymbolType>(type));
	ui.labelPreview->clear();
	changeComboTags();
}

QgsSymbol *S3dmApplySymbolDlg::getSymbol()
{
	return mCurrentSymbol->clone();
}

QString S3dmApplySymbolDlg::getCurrentSymbolName()
{
	QModelIndex index = ui.listView->selectionModel()->currentIndex();
	QString symbolName = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();
	if (symbolName.isEmpty())
	{
		return  QString();
	}
	return symbolName;
}

QColor S3dmApplySymbolDlg::getCurrentColor()
{
	if (m_colorButtonText != "None")
	{
		return m_backgroundColor;
	}
	return QColor();
}

QgsSymbol * S3dmApplySymbolDlg::getSymbolByName(QString name)
{
	if (mStyle->symbol(name) == nullptr)
	{
		return nullptr;
	}
	return mStyle->symbol(name)->clone();
}

QgsSymbol * S3dmApplySymbolDlg::getSymbolByName(QString name, QColor color)
{
	if (mStyle->symbol(name) == nullptr)
	{
		return nullptr;
	}
	QgsSymbol *symbol = mStyle->symbol(name)->clone();
	for (int i = 0; i < symbol->symbolLayerCount(); i++)
	{
		QString type = symbol->symbolLayer(i)->layerType();
		if (mType != symbolType::LINE)
		{
			symbol->setOutputUnit(matchSizeUnit());
			if (mType == symbolType::POINT)
			{
				if (type == QStringLiteral("SvgMarker") || type == QStringLiteral("RasterMarker"))
				{
					if (i + 1 == symbol->symbolLayerCount())
					{
						QgsSimpleMarkerSymbolLayer *simplelayer = new QgsSimpleMarkerSymbolLayer();
						simplelayer->setColor(color);
						simplelayer->setStrokeStyle(Qt::PenStyle::NoPen);
						symbol->insertSymbolLayer(i, simplelayer);
						break;
					}					
				}
			}
			else if (mType == symbolType::POLYGON)
			{
				if (type == QStringLiteral("SVGFill") || type == QStringLiteral("RasterFill"))
				{
					if (i + 1 == symbol->symbolLayerCount())
					{
						QgsSimpleFillSymbolLayer *simplelayer = new QgsSimpleFillSymbolLayer();
						simplelayer->setColor(color);
						simplelayer->setStrokeStyle(Qt::PenStyle::NoPen);
						symbol->insertSymbolLayer(i, simplelayer);
						break;
					}
				}
			}
		}
		else
		{
			if (type == QStringLiteral("MarkerLine"))
			{
				for (int j = 0; j < symbol->symbolLayer(i)->subSymbol()->symbolLayerCount(); j++)
				{
					QString subType = symbol->symbolLayer(i)->subSymbol()->symbolLayer(j)->layerType();
					if (subType == QStringLiteral("SvgMarker") || subType == QStringLiteral("RasterMarker"))
					{
						if (j + 1 == symbol->symbolLayer(i)->subSymbol()->symbolLayerCount())
						{
							QgsSimpleMarkerSymbolLayer *simplelayer = new QgsSimpleMarkerSymbolLayer();
							simplelayer->setColor(color);
							simplelayer->setStrokeStyle(Qt::PenStyle::NoPen);
							symbol->symbolLayer(i)->subSymbol()->insertSymbolLayer(j, simplelayer);
							break;
						}
					}
				}
			}
		}
	}
	return symbol;
}

void S3dmApplySymbolDlg::tagChanged()
{
	if (ui.comboLabel->currentText() == QStringLiteral("全部"))
	{
		mModel->setTagId(-1);
		mModel->setSmartGroupId(-1);
		mModel->setFavoritesOnly(false);
	}
	else
	{
		mModel->setTagId(mStyle->tagId(ui.comboLabel->currentText()));
		mModel->setSmartGroupId(-1);
		mModel->setFavoritesOnly(false);
	}
	QModelIndex qindex = ui.listView->model()->index(0, 0);   //默认选第一个
	ui.listView->setCurrentIndex(qindex);
	changeSelect();
}

void S3dmApplySymbolDlg::changeToUseColor()
{
	if (ui.checkBoxColor->isChecked())
	{
		ui.buttonColorSelect->setEnabled(false);
		ui.buttonColorSelect->setText("None");
		m_colorButtonText = "None";
		ui.buttonColorSelect->setStyleSheet("");
		//删除图层
		deleteSimpleLayers();
	}
	else
	{
		ui.buttonColorSelect->setEnabled(true);
		QString text = "rgb:(" + QString::number(m_backgroundColor.red()) + "," + QString::number(m_backgroundColor.green()) + "," + QString::number(m_backgroundColor.blue()) + ")";
		m_colorButtonText = text;
		ui.buttonColorSelect->setText(text);
		ui.buttonColorSelect->setStyleSheet("background-color:rgb(" + QString::number(m_backgroundColor.red()) + "," + QString::number(m_backgroundColor.green()) + "," + QString::number(m_backgroundColor.blue()) + ")");
		changeToSymbol();
	}
}

void S3dmApplySymbolDlg::openColor()
{
	QPalette patte = ui.buttonColorSelect->palette();
	QColor oldClr = patte.color(QPalette::Background);
	QColor clr = QColorDialog::getColor(clr);
	if (!clr.isValid())
		return;
	patte.setColor(QPalette::Background, clr);
	ui.buttonColorSelect->setPalette(patte);
	QString text = "rgb:(" + QString::number(clr.red()) + "," + QString::number(clr.green()) + "," + QString::number(clr.blue()) + ")";
	m_colorButtonText = text;
	m_backgroundColor = clr;
	ui.buttonColorSelect->setText(text);
	ui.buttonColorSelect->setStyleSheet("background-color:rgb(" + QString::number(clr.red()) + "," + QString::number(clr.green()) + "," + QString::number(clr.blue()) + ")");
	if (oldClr != clr)
	{
		changeToSymbol();
	}
}

void S3dmApplySymbolDlg::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(selected)
	Q_UNUSED(deselected)
	changeSelect();
}

void S3dmApplySymbolDlg::changeToSymbol()
{
	m_backgroundColor = ui.buttonColorSelect->palette().color(QPalette::Background);
	QModelIndex index = ui.listView->selectionModel()->currentIndex();
	QString symbolName = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();
	if (symbolName.isEmpty())
	{
		return;
	}
	QgsSymbol *tmpSymbol = mStyle->symbol(symbolName)->clone();
	double opacity = ui.horizontalSlider->value() / 100.00;
	tmpSymbol->setOpacity(opacity);
	for (int i = 0; i < tmpSymbol->symbolLayerCount(); i++)
	{
		QString type = tmpSymbol->symbolLayer(i)->layerType();
		if (mType != symbolType::LINE)
		{
			tmpSymbol->setOutputUnit(matchSizeUnit());
			if (mType == symbolType::POINT)
			{
				if (type == QStringLiteral("SvgMarker"))
				{
					QgsSvgMarkerSymbolLayer *layer = (QgsSvgMarkerSymbolLayer*)(tmpSymbol->symbolLayer(i));
					layer->setSize(ui.spinSize->value());
					layer->setSizeUnit(matchSizeUnit());
					layer->setAngle(ui.spinRotate->value());
				}
				else if (type == QStringLiteral("RasterMarker"))
				{
					QgsRasterMarkerSymbolLayer *layer = (QgsRasterMarkerSymbolLayer*)(tmpSymbol->symbolLayer(i));
					layer->setSize(ui.spinSize->value());
					layer->setSizeUnit(matchSizeUnit());
					layer->setAngle(ui.spinRotate->value());
					layer->setOpacity(opacity);
				}
				else if (type == QStringLiteral("SimpleMarker") && m_colorButtonText != "None" &&  i + 1 == tmpSymbol->symbolLayerCount())
				{
					QgsSimpleMarkerSymbolLayer *simplelayer = (QgsSimpleMarkerSymbolLayer*)(tmpSymbol->symbolLayer(i));
					simplelayer->setColor(m_backgroundColor);
					break;
				}
				else
				{
					continue;
				}
				if (m_colorButtonText != "None" && i + 1 == tmpSymbol->symbolLayerCount())
				{
					QgsSimpleMarkerSymbolLayer *simplelayer = new QgsSimpleMarkerSymbolLayer();
					simplelayer->setColor(m_backgroundColor);
					simplelayer->setStrokeStyle(Qt::PenStyle::NoPen);
					tmpSymbol->insertSymbolLayer(i, simplelayer);
					break;
				}
			}
			else if (mType == symbolType::POLYGON)
			{
				if (type == QStringLiteral("SVGFill"))
				{
					QgsSVGFillSymbolLayer *layer = (QgsSVGFillSymbolLayer*)(tmpSymbol->symbolLayer(i));
					layer->setPatternWidth(ui.spinSize->value());
					layer->setPatternWidthUnit(matchSizeUnit());
					layer->setAngle(ui.spinRotate->value());
				}
				else if (type == QStringLiteral("RasterFill"))
				{
					QgsRasterFillSymbolLayer *layer = (QgsRasterFillSymbolLayer*)(tmpSymbol->symbolLayer(i));
					layer->setWidth(ui.spinSize->value());
					layer->setWidthUnit(matchSizeUnit());
					layer->setAngle(ui.spinRotate->value());
					layer->setOpacity(opacity);
				}
				else if (type == QStringLiteral("SimpleFill") && m_colorButtonText != "None" &&  i + 1 == tmpSymbol->symbolLayerCount())
				{
					QgsSimpleFillSymbolLayer *simplelayer = (QgsSimpleFillSymbolLayer*)(tmpSymbol->symbolLayer(i));
					simplelayer->setColor(m_backgroundColor);
					simplelayer->setStrokeStyle(Qt::PenStyle::NoPen);
					break;
				}
				else
				{
					continue;
				}
				if (m_colorButtonText != "None"&& i + 1 == tmpSymbol->symbolLayerCount())
				{
					QgsSimpleFillSymbolLayer *simplelayer = new QgsSimpleFillSymbolLayer();
					simplelayer->setColor(m_backgroundColor);
					tmpSymbol->insertSymbolLayer(i, simplelayer);
					break;
				}
			}
		}
		else
		{
			if (type == QStringLiteral("MarkerLine"))
			{
				((QgsMarkerLineSymbolLayer*)tmpSymbol->symbolLayer(i))->setInterval(ui.spinInterval->value());
				((QgsMarkerLineSymbolLayer*)tmpSymbol->symbolLayer(i))->setIntervalUnit(matchIntervalUnit());
				for (int j = 0; j < tmpSymbol->symbolLayer(i)->subSymbol()->symbolLayerCount(); j++)
				{
					QString subType = tmpSymbol->symbolLayer(i)->subSymbol()->symbolLayer(j)->layerType();
					if (subType == QStringLiteral("SvgMarker"))
					{
						QgsSvgMarkerSymbolLayer *layer = (QgsSvgMarkerSymbolLayer*)(tmpSymbol->symbolLayer(i)->subSymbol()->symbolLayer(j));
						layer->setSize(ui.spinSize->value());
						layer->setSizeUnit(matchSizeUnit());
						layer->setAngle(ui.spinRotate->value());
					}
					else if (subType == QStringLiteral("RasterMarker"))
					{
						QgsRasterMarkerSymbolLayer *layer = (QgsRasterMarkerSymbolLayer*)(tmpSymbol->symbolLayer(i)->subSymbol()->symbolLayer(j));
						layer->setSize(ui.spinSize->value());
						layer->setSizeUnit(matchSizeUnit());
						layer->setAngle(ui.spinRotate->value());
						layer->setOpacity(opacity);
					}
					else if (type == QStringLiteral("SimpleMarker") && m_colorButtonText != "None" && j + 1 == tmpSymbol->symbolLayer(i)->subSymbol()->symbolLayerCount())
					{
						QgsSimpleMarkerSymbolLayer *simplelayer = (QgsSimpleMarkerSymbolLayer*)(tmpSymbol->symbolLayer(i)->subSymbol()->symbolLayer(j));
						simplelayer->setColor(m_backgroundColor);
						simplelayer->setStrokeStyle(Qt::PenStyle::NoPen);
						break;
					}
					else
					{
						continue;
					}
					if (m_colorButtonText != "None" && j + 1 == tmpSymbol->symbolLayer(i)->subSymbol()->symbolLayerCount())
					{
						QgsSimpleMarkerSymbolLayer *simplelayer = new QgsSimpleMarkerSymbolLayer();
						simplelayer->setColor(m_backgroundColor);
						tmpSymbol->symbolLayer(i)->subSymbol()->insertSymbolLayer(j, simplelayer);
						break;
					}
				}
			}
		}
	}
	QImage image = getImage(tmpSymbol, symbolName, &mPreviewExpressionContext);
	ui.labelPreview->setPixmap(QPixmap::fromImage(image));
	ui.labelPreview->setAcceptDrops(Qt::AlignCenter);
	mCurrentSymbol = tmpSymbol;
}

void S3dmApplySymbolDlg::changeToLayer()
{
	if (mCurrentSymbol)
	{
		this->accept();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("应用于图层"), QStringLiteral("未能获取符号"));
	}
}

void S3dmApplySymbolDlg::filterSymbols(const QString &qword)
{
	if (mModel != nullptr)
	{
		mModel->setFilterString(qword);
	}
	
}

QgsUnitTypes::RenderUnit S3dmApplySymbolDlg::matchSizeUnit()
{
	if (ui.comboSizeUnit->currentText() == QStringLiteral("毫米"))
	{
		return QgsUnitTypes::RenderUnit::RenderMillimeters;
	}
	else if (ui.comboSizeUnit->currentText() == QStringLiteral("点"))
	{
		return QgsUnitTypes::RenderUnit::RenderPoints;
	}
	else if (ui.comboSizeUnit->currentText() == QStringLiteral("像素"))
	{
		return QgsUnitTypes::RenderUnit::RenderPixels;
	}
	else if (ui.comboSizeUnit->currentText() == QStringLiteral("米"))
	{
		return QgsUnitTypes::RenderUnit::RenderMetersInMapUnits;
	}
	else if (ui.comboSizeUnit->currentText() == QStringLiteral("地图单位"))
	{
		return QgsUnitTypes::RenderUnit::RenderMapUnits;
	}
	else if (ui.comboSizeUnit->currentText() == QStringLiteral("英尺"))
	{
		return QgsUnitTypes::RenderUnit::RenderInches;
	}
	return QgsUnitTypes::RenderUnit::RenderMillimeters;
}

QgsUnitTypes::RenderUnit S3dmApplySymbolDlg::matchIntervalUnit()
{
	if (ui.comboIntervalUnit->currentText() == QStringLiteral("毫米"))
	{
		return QgsUnitTypes::RenderUnit::RenderMillimeters;
	}
	else if (ui.comboIntervalUnit->currentText() == QStringLiteral("点"))
	{
		return QgsUnitTypes::RenderUnit::RenderPoints;
	}
	else if (ui.comboIntervalUnit->currentText() == QStringLiteral("像素"))
	{
		return QgsUnitTypes::RenderUnit::RenderPixels;
	}
	else if (ui.comboIntervalUnit->currentText() == QStringLiteral("米"))
	{
		return QgsUnitTypes::RenderUnit::RenderMetersInMapUnits;
	}
	else if (ui.comboIntervalUnit->currentText() == QStringLiteral("地图单位"))
	{
		return QgsUnitTypes::RenderUnit::RenderMapUnits;
	}
	else if (ui.comboIntervalUnit->currentText() == QStringLiteral("英尺"))
	{
		return QgsUnitTypes::RenderUnit::RenderInches;
	}
	return QgsUnitTypes::RenderUnit::RenderMillimeters;
}

void S3dmApplySymbolDlg::changeComboTags()
{
	QgsSymbol::SymbolType symbolType;
	if (mType == POINT)
	{
		symbolType = QgsSymbol::Marker;
	}
	else if (mType == LINE)
	{
		symbolType = QgsSymbol::Line;
	}
	else if (mType == POLYGON)
	{
		symbolType = QgsSymbol::Fill;
	}
	QStringList tags;
	for (int i = 0; i < mStyle->tags().size(); i++)
	{
		QStringList list = mStyle->symbolsWithTag(QgsStyle::StyleEntity::SymbolEntity, mStyle->tagId(mStyle->tags()[i]));
		for (int j = 0; j < list.size(); j++)
		{
			if (mStyle->symbol(list[j]) != nullptr)
			{
				if (mStyle->symbol(list[j])->type() == symbolType)
				{
					tags.push_back(mStyle->tags()[i]);
					break;
				}
			}
			
		}
	}
	ui.comboLabel->clear();
	ui.comboLabel->addItem(QStringLiteral("全部"));
	ui.comboLabel->addItems(tags);
	ui.comboLabel->setCurrentIndex(0);
}

void S3dmApplySymbolDlg::deleteSimpleLayers()
{
	QModelIndex index = ui.listView->selectionModel()->currentIndex();
	if (mCurrentSymbol == nullptr)
	{
		return;
	}
	QString symbolName = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();
	if (symbolName.isEmpty())
	{
		return;
	}
	for (int i = 0; i < mCurrentSymbol->symbolLayerCount(); i++)
	{
		QString type = mCurrentSymbol->symbolLayer(i)->layerType();
		if (mType != symbolType::LINE)
		{
			if (mType == symbolType::POINT)
			{
				if (type == QStringLiteral("SimpleMarker"))
				{
					mCurrentSymbol->deleteSymbolLayer(i);
				}
			}
			else if (mType == symbolType::POLYGON)
			{
				if (type == QStringLiteral("SimpleFill"))
				{
					mCurrentSymbol->deleteSymbolLayer(i);
				}
			}
		}
		else
		{
			if (type == QStringLiteral("MarkerLine"))
			{
				for (int j = 0; j < mCurrentSymbol->symbolLayer(i)->subSymbol()->symbolLayerCount(); j++)
				{
					QString subType = mCurrentSymbol->symbolLayer(i)->subSymbol()->symbolLayer(j)->layerType();
					if (subType == QStringLiteral("SimpleMarker"))
					{
						mCurrentSymbol->symbolLayer(i)->subSymbol()->deleteSymbolLayer(j);
					}
				}
			}
		}
	}
	QImage image = getImage(mCurrentSymbol, symbolName, &mPreviewExpressionContext);
	ui.labelPreview->setPixmap(QPixmap::fromImage(image));
	ui.labelPreview->setAcceptDrops(Qt::AlignCenter);
}

void S3dmApplySymbolDlg::changeSelect()
{
	QModelIndex index = ui.listView->selectionModel()->currentIndex();
	QString symbolName = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();
	if (!symbolName.isEmpty())
	{
		QImage image = getImage(mStyle->symbol(symbolName), symbolName, &mPreviewExpressionContext);
		ui.labelPreview->setPixmap(QPixmap::fromImage(image));
		ui.labelPreview->setAcceptDrops(Qt::AlignCenter);
		ui.spinInterval->setValue(3);
		ui.spinSize->setValue(10);
		ui.horizontalSlider->setValue(100);
		ui.checkBoxColor->setChecked(false);
		ui.buttonColorSelect->setEnabled(true);
		ui.spinRotate->setValue(0);
		/*if (m_colorButtonText != "None")
		{
			m_backgroundColor = ui.buttonColorSelect->palette().color(QPalette::Background);
		}*/
		changeToSymbol();
		//mCurrentSymbol = mStyle->symbol(symbolName)->clone();
	}
}

QImage S3dmApplySymbolDlg::getImage(QgsSymbol *symbol, QString &symbolName, QgsExpressionContext *expressionContext)
{
	
	QImage preview(QSize(200, 200), QImage::Format_ARGB32_Premultiplied);
	preview.fill(0);
	if (symbol == nullptr)
	{
		return preview;
	}

	QPainter p(&preview);
	p.setRenderHint(QPainter::Antialiasing);
	p.translate(0.5, 0.5); // shift by half a pixel to avoid blurring due antialising

	if (mType == QgsSymbol::Marker)
	{
		p.setPen(QPen(Qt::gray));
		p.drawLine(0, 100, 200, 100);
		p.drawLine(100, 0, 100, 200);
	}

	QgsRenderContext context = QgsRenderContext::fromQPainter(&p);
	if (expressionContext)
		context.setExpressionContext(*expressionContext);

	symbol->startRender(context);

	if (mType == QgsSymbol::Line)
	{
		QPolygonF poly;
		poly << QPointF(0, 100) << QPointF(199, 100);
		static_cast<QgsLineSymbol *>(symbol)->renderPolyline(poly, nullptr, context);
	}
	else if (mType == QgsSymbol::Fill)
	{
		QPolygonF polygon;
		polygon << QPointF(40, 40) << QPointF(160, 40) << QPointF(160, 160) << QPointF(40, 160) << QPointF(40, 40);
		static_cast<QgsFillSymbol *>(symbol)->renderPolygon(polygon, nullptr, nullptr, context);
	}
	else // marker
	{
		static_cast<QgsMarkerSymbol *>(symbol)->renderPoint(QPointF(100, 100), nullptr, context);
	}

	symbol->stopRender(context);
	return preview;
}
