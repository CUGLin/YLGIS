#include "S3dmSymbolManager.h"
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "S3dmSymbolExportDlg.h"

#include "S3dmSymbolImportDlg.h"
#include "S3dmXmlGenerateDlg.h"

#include "qscrollbar.h"
#include "qgsgui.h"
#include "qgis.h"
#include "qstandarditemmodel.h"
#include "qgsstylemodel.h"
#include "qclipboard.h"
#include "qinputdialog.h"
#include "qmessagebox.h"
#include "qprogressdialog.h"
#include "qgssettings.h"

#include "S3dmSymbolSaveDlg.h"
#include "qgsguiutils.h"
#include "qfiledialog.h"
#include "qgssymbol.h"
#include "qgsapplication.h"

#include "qgsmarkersymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"

#include "S3dmStyleManager.h"

using namespace Smart3dMap;

S3dmSymbolManager::S3dmSymbolManager(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.stackedShow->setVisible(false);
	//setAttribute(Qt::WA_DeleteOnClose, true);//�رմ��ڼ�����
	QgsGui::instance()->enableAutoGeometryRestore(this);

	mActionCopyItem = new QAction(this);
	mActionPasteItem = new QAction(this);
	mAddTag = new QAction(this);
	mImport = new QAction(this);
	mImportXml = new QAction(this);
	mGenerateXml = new QAction(this);
	mImportMapGIS = new QAction(this);
	mDeleteTag = new QAction(this);
	mActionEditItem = new QAction(this);
	mActionCopyItem = new QAction(this);
	mActionPasteItem = new QAction(this);
	mActionAddToTag = new QAction(this);
	mActionExportSVG = new QAction(this);
	mActionExportImage = new QAction(this);
	mActionDelete = new QAction(this);
	mGroupTreeContextMenu = new QMenu(this);
	mGroupTreeFatherMenu = new QMenu(this);
	mGroupListMenu = new QMenu(this);
	mGroupListContextMenu = new QMenu(this);

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	//��ȡ����,�Ȳ����ݿ⣬û�оʹ����������ȡ�����ݿ�����Ϊ�����ȡ�̶�·����xml������û����򿪿����ݿ�
	//���ݿ��xml��û����Ҫ�Լ������ݿ��е���xml��Ҳ�ɰ�·�����봴��������xml��
	mStyle = S3dmStyleManager::getS3dmStyle();
	/*if (mStyle->symbolCount() < 1)
	{
		mStyle = QgsStyle::defaultStyle();
		QString path = OgAppResLoadingIterator::getSingletonPtr()->GetSysEvnPath().c_str();
		QStringList pathList = path.split("/");
		pathList.pop_back();
		pathList.pop_back();
		pathList.pop_back();
		path = "";
		for (int i = 0; i < pathList.size(); i++)
		{
			path += pathList[i] + "/";
		}
		path += "qgis-ltr/svg/symbols.xml";
		QFileInfo isExist(path);

		if (isExist.exists())
		{
			mStyle->importXml(path);
		}
	}*/

	//��ǩҳ�л�
	connect(ui.tabWidget, &QTabWidget::currentChanged, this, &S3dmSymbolManager::tabItemTypeChanged);

	//������ο����
	ui.treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.treeView->setHeaderHidden(true);
	double treeIconSize = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance('X') * 10;
	ui.treeView->setIconSize(QSize(static_cast<int>(treeIconSize), static_cast<int>(treeIconSize)));
	//������߿���м���model
	QStandardItemModel *groupModel = new QStandardItemModel(ui.treeView);
	ui.treeView->setModel(groupModel);
	ui.treeView->setHeaderHidden(true);
	populateGroups();
	ui.treeView->setCurrentIndex(ui.treeView->model()->index(0, 0));
	connect(ui.treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &S3dmSymbolManager::groupChanged);
	mModel = new QgsStyleProxyModel(mStyle);
	double iconSize = Qgis::UI_SCALE_FACTOR * fontMetrics().maxWidth() * 10;
	ui.listItems->setIconSize(QSize(static_cast<int>(iconSize*0.5), static_cast<int>(iconSize * 0.5)));
	ui.listItems->setGridSize(QSize(static_cast<int>(iconSize*0.55), static_cast<int>(iconSize * 0.85)));
	mModel->addDesiredIconSize(ui.listItems->iconSize());
	mModel->addDesiredIconSize(ui.treeView->iconSize());
	ui.listItems->setModel(mModel);
	ui.listItems->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui.listItems->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.treeView->setSelectionModel(ui.listItems->selectionModel());
	ui.treeView->setSelectionMode(ui.listItems->selectionMode());
	//�����߿���м����Ҽ��˵�
	ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	mAddTag->setText(QStringLiteral("��ӱ�ǩ"));
	mImport->setText(QStringLiteral("����SVG/Image"));
	mImportXml->setText(QStringLiteral("����Xml"));
	mGenerateXml->setText(QStringLiteral("����Xml"));
	mImportMapGIS->setText(QStringLiteral("����MapGIS��"));
	mDeleteTag->setText(QStringLiteral("ɾ����ǩ"));
	mGroupTreeFatherMenu->addAction(mGenerateXml);
	mGroupTreeFatherMenu->addAction(mImportXml);
	mGroupTreeFatherMenu->addAction(mImportMapGIS);
	mGroupTreeFatherMenu->addAction(mAddTag);
	mGroupTreeContextMenu->addAction(mImport);
	mGroupTreeContextMenu->addAction(mDeleteTag);
	connect(ui.treeView, &QWidget::customContextMenuRequested, this, &S3dmSymbolManager::grouptreeContextMenu);
	ui.listItems->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.listItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.listItems->setWordWrap(true);
	mActionEditItem->setText(QStringLiteral("�༭"));
	mActionCopyItem->setText(QStringLiteral("����"));
	mActionPasteItem->setText(QStringLiteral("ճ��Ϊ"));
	mActionAddToTag->setText(QStringLiteral("��ӵ���ǩ"));
	mActionExportSVG->setText(QStringLiteral("����ΪSVG"));
	mActionExportImage->setText(QStringLiteral("����ΪPNG"));
	mActionDelete->setText(QStringLiteral("�Ƴ�"));
	mActionAddToTag->setMenu(mGroupListContextMenu);
	mGroupListMenu->addAction(mActionEditItem);
	mGroupListMenu->addAction(mActionCopyItem);
	mGroupListMenu->addAction(mActionPasteItem);
	mGroupListMenu->addAction(mActionAddToTag);
	mGroupListMenu->addAction(mActionExportSVG);
	mGroupListMenu->addAction(mActionExportImage);
	mGroupListMenu->addAction(mActionDelete);
	connect(ui.listItems, &QWidget::customContextMenuRequested, this, &S3dmSymbolManager::listItemsContextMenu);
	connect(ui.listItems, &QAbstractItemView::clicked, this, &S3dmSymbolManager::selectItem);
	//�Ҽ��˵�����Ӧ
	connect(mAddTag, &QAction::triggered, this, &S3dmSymbolManager::addTag);
	connect(mImport, &QAction::triggered, this, &S3dmSymbolManager::importSymbol);
	connect(mGenerateXml, &QAction::triggered, this, &S3dmSymbolManager::generateXml);
	connect(mImportXml, &QAction::triggered, this, &S3dmSymbolManager::importSymbols);
	connect(mImportMapGIS, &QAction::triggered, this, &S3dmSymbolManager::importMapGIS);
	connect(mDeleteTag, &QAction::triggered, this, &S3dmSymbolManager::deleteTag);
	connect(mActionEditItem, &QAction::triggered, this, &S3dmSymbolManager::editItem);
	connect(mActionCopyItem, &QAction::triggered, this, &S3dmSymbolManager::copyItem);
	connect(mActionPasteItem, &QAction::triggered, this, &S3dmSymbolManager::pasteItem);
	connect(mActionAddToTag, &QAction::triggered, this, &S3dmSymbolManager::addTag);
	connect(mActionExportSVG, &QAction::triggered, this, &S3dmSymbolManager::exportItemToSVG);
	connect(mActionExportImage, &QAction::triggered, this, &S3dmSymbolManager::exportItemToImage);
	connect(mActionDelete, &QAction::triggered, this, &S3dmSymbolManager::deleteItem);
	//������һ�ε�ʱ����ǰ������Ԥ��ͼ�����棬��ֹ�л�����
	ui.tabWidget->setCurrentIndex(2);
	tabItemTypeChanged();
	for (int i = 0; i < ui.listItems->model()->rowCount(); i++)
	{
		mModel->data(ui.listItems->model()->index(i, 0), 1);
	}
	
	ui.tabWidget->setCurrentIndex(0);
	tabItemTypeChanged();
	ui.listItems->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
	connect(ui.pushButton_close, &QPushButton::clicked, this, &S3dmSymbolManager::hide);
	ui.searchBox->setPlaceholderText(QStringLiteral("���ˡ�"));
	connect(ui.searchBox, &QLineEdit::textChanged, this, &S3dmSymbolManager::filterSymbols);
	ui.treeView->resizeColumnToContents(0);
	
	QApplication::restoreOverrideCursor();

}

S3dmSymbolManager::~S3dmSymbolManager()
{
	if (mModel)
	{
		delete mModel;
		mModel = nullptr;
	}
}

QgsStyle *S3dmSymbolManager::getStyle()
{
	return mStyle;
}

QgsStyleProxyModel *S3dmSymbolManager::getModel()
{
	return mModel;
}

void S3dmSymbolManager::groupChanged(const QModelIndex &index)
{
	QStringList groupSymbols;

	const QString category = index.data(Qt::UserRole + 1).toString();
	if (category == QLatin1String("all") || category == QLatin1String("tags"))
	{
		mModel->setTagId(-1);
		mModel->setSmartGroupId(-1);
		mModel->setFavoritesOnly(false);
	}
	else if (category == QLatin1String("favorite"))
	{
		mModel->setTagId(-1);
		mModel->setSmartGroupId(-1);
		mModel->setFavoritesOnly(true);
	} 
	else // ����
	{
		int tagId = index.data(Qt::UserRole + 1).toInt();
		mModel->setTagId(tagId);
		mModel->setSmartGroupId(-1);
		mModel->setFavoritesOnly(false);
	}
}

void S3dmSymbolManager::grouptreeContextMenu(QPoint point)
{
	QPoint globalPos = ui.treeView->viewport()->mapToGlobal(point);
	QModelIndex index = ui.treeView->indexAt(point);
	QString data = index.data(Qt::UserRole + 1).toString();
	if (data == QLatin1String("all") || data == QLatin1String("favorite"))
	{
		return;
	}
	if (index.isValid() && data != QLatin1String("all") && data != QLatin1String("favorite") && data != QLatin1String("tags"))
		mGroupTreeContextMenu->popup(globalPos);
	else
		mGroupTreeFatherMenu->popup(globalPos);
}

void S3dmSymbolManager::listItemsContextMenu(QPoint point)
{
	//QPoint globalPos = ui.stackedShow->currentIndex() == 0 ? ui.listItems->viewport()->mapToGlobal(point) : ui.treeView->viewport()->mapToGlobal(point);
	mGroupListContextMenu->clear();
	const QModelIndexList indices = ui.listItems->selectionModel()->selectedIndexes();
	const QStringList currentTags = indices.count() == 1 ? indices.at(0).data(QgsStyleModel::TagRole).toStringList() : QStringList();
	QAction *a = nullptr;
	QStringList tags = mStyle->tags();
	tags.sort();
	for (const QString &tag : qgis::as_const(tags))
	{
		a = new QAction(tag, mGroupListContextMenu);
		a->setData(tag);
		if (indices.count() == 1)
		{
			a->setCheckable(true);
			a->setChecked(currentTags.contains(tag));
		}
		connect(a, &QAction::triggered, this, [=](bool) { tagSelectedSymbols(false); });
		mGroupListContextMenu->addAction(a);
	}
	if (tags.count() > 0)
	{
		mGroupListContextMenu->addSeparator();
	}
	a = new QAction(QStringLiteral("�����±�ǩ��"), mGroupListContextMenu);
	connect(a, &QAction::triggered, this, [=](bool) { tagSelectedSymbols(true); });
	mGroupListContextMenu->addAction(a);
	const QList< ItemDetails > items = selectedItems();
	mActionCopyItem->setEnabled(!items.isEmpty() && (items.at(0).entityType != QgsStyle::ColorrampEntity));

	bool enablePaste = false;
	std::unique_ptr< QgsSymbol > tempSymbol(QgsSymbolLayerUtils::symbolFromMimeData(QApplication::clipboard()->mimeData()));
	if (tempSymbol)
		enablePaste = true;
	else
	{
		(void)QgsTextFormat::fromMimeData(QApplication::clipboard()->mimeData(), &enablePaste);
	}
	mActionPasteItem->setEnabled(enablePaste);

	bool enableEdit = false;
	if (ui.tabWidget->currentIndex() != 3)
	{
		enableEdit = true;
	}
	mActionEditItem->setEnabled(enableEdit);
	mGroupListMenu->popup(QCursor::pos());
}

void S3dmSymbolManager::tagSelectedSymbols(bool newTag)
{
	QAction *selectedItem = qobject_cast<QAction *>(sender());
	if (selectedItem)
	{
		const QList< ItemDetails > items = selectedItems();
		QString tag;
		if (newTag)
		{
			int id = addTag();
			if (id == 0)
			{
				return;
			}

			tag = mStyle->tag(id);
		}
		else
		{
			tag = selectedItem->data().toString();
		}

		for (const ItemDetails &details : items)
		{
			mStyle->tagSymbol(details.entityType, details.name, QStringList(tag));
		}
	}
}

void S3dmSymbolManager::tabItemTypeChanged()
{
	int type = 0;
	if (ui.tabWidget->currentIndex() == 0)
	{
		type = QgsSymbol::Marker;
	}
	else if (ui.tabWidget->currentIndex() == 1)
	{
		type = QgsSymbol::Line;
	}
	else if (ui.tabWidget->currentIndex() == 2)
	{
		type = QgsSymbol::Fill;
	}
	else if (ui.tabWidget->currentIndex() == 3)
	{
		type = 3;
	}
	const bool isSymbol = type != 3;
	mModel->setEntityFilter(isSymbol ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity);
	if (type != 3)
	{
		mModel->setEntityFilterEnabled(true);
		mModel->setSymbolTypeFilterEnabled(true);
		mModel->setSymbolType(static_cast<QgsSymbol::SymbolType>(type));
		mActionExportSVG->setEnabled(true);
		mActionExportImage->setEnabled(true);
	}
	else
	{
		mModel->setEntityFilterEnabled(true);
		mModel->setSymbolTypeFilterEnabled(false);
		mActionExportSVG->setEnabled(false);
		mActionExportImage->setEnabled(false);
	}

	QModelIndex index = ui.treeView->selectionModel()->currentIndex();
	groupChanged(index);
}

int S3dmSymbolManager::addTag()
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui.treeView->model());
	QModelIndex index;
	for (int i = 0; i < ui.treeView->model()->rowCount(); i++)
	{
		index = ui.treeView->model()->index(i, 0);
		QString data = index.data(Qt::UserRole + 1).toString();
		if (data == QLatin1String("��ǩ"))
		{
			break;
		}
	}

	QString itemName;
	int id;
	bool ok;
	itemName = QInputDialog::getText(this, QStringLiteral("��ӱ�ǩ"),
		QStringLiteral("�������±�ǩ������:"), QLineEdit::Normal, QStringLiteral("�±�ǩ"), &ok).trimmed();
	if (!ok || itemName.isEmpty())
		return 0;

	int check = mStyle->tagId(itemName);
	if (check > 0)
	{
		QMessageBox::warning(this, QStringLiteral("��ӱ�ǩ"), QStringLiteral("��ǩ ��%1�� �Ѵ���.").arg(itemName));
		return 0;
	}

	id = mStyle->addTag(itemName);

	if (!id)
	{
		QMessageBox::warning(this, QStringLiteral("��ӱ�ǩ"), QStringLiteral("�޷������±�ǩ �� �������ݿ����."));
		return 0;
	}
	//����ӽڵ�
	QStandardItem *parentItem = model->itemFromIndex(index);
	QStandardItem *childItem = new QStandardItem(itemName);
	childItem->setData(id);
	parentItem->appendRow(childItem);
	ui.treeView->resizeColumnToContents(0);

	return id;
}

void S3dmSymbolManager::importSymbol()
{
	QModelIndex index = ui.treeView->selectionModel()->currentIndex();
	S3dmSymbolImportDlg *importDlg = new S3dmSymbolImportDlg(mStyle, index.data().toString());
	importDlg->raise();
	importDlg->show();
	int status = importDlg->exec();
	if (status == QDialog::Accepted)
	{
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, QStringLiteral("�������"), QStringLiteral("������ɡ�"));
	}
}

void S3dmSymbolManager::generateXml()
{
	S3dmXmlGenerateDlg *dlg = new S3dmXmlGenerateDlg(mStyle);
	dlg->raise();
	dlg->show();
	int status = dlg->exec();
	if (status == QDialog::Accepted)
	{
		populateGroups();
		tabItemTypeChanged();
		ui.treeView->resizeColumnToContents(0);
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, QStringLiteral("������ſ�"), QStringLiteral("������ɣ�"));
	}
}

void S3dmSymbolManager::importSymbols()
{
	bool isDelete = false; 
	if (mStyle->symbolCount() > 0)
	{
		int res = QMessageBox::warning(this, QStringLiteral("������ſ�"), QStringLiteral("�Ƿ���Ҫ�����ǰ���ſ��ٵ��룿"), QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);
		if (res == QMessageBox::StandardButton::Yes)
		{
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
			isDelete = true;
			deleteSymbols(QgsStyle::StyleEntity::SymbolEntity, mStyle);
			deleteSymbols(QgsStyle::StyleEntity::TagEntity, mStyle);
			mStyle->clear();
			populateGroups();
			tabItemTypeChanged();
			QApplication::restoreOverrideCursor();
			/*QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui.treeView->model());
			QModelIndexList indexList = ui.treeView->selectionModel()->selectedRows(0);
			int num = 0;
			for (auto it = indexList.begin(); it != indexList.end(); it++)
			{
				QString data = (*it).data(Qt::UserRole + 1).toString();
				if (data == QLatin1String("all") || data == QLatin1String("favorite") || data == QLatin1String("tags"))
				{
					if (model->hasChildren(*it))
					{
						model->removeColumn(0, *it);
					}
				}
			}*/
			/*QProgressDialog progress;
			progress.setWindowTitle(QString::fromLocal8Bit("��ʾ"));
			progress.setLabelText(QString::fromLocal8Bit("ɾ����..."));
			progress.setCancelButtonText(QString::fromLocal8Bit("ȡ��"));
			progress.setModal(true);
			progress.show();
			QStringList symbolNames = mStyle->allNames(QgsStyle::StyleEntity::SymbolEntity);
			QStringList tagNames = mStyle->allNames(QgsStyle::StyleEntity::TagEntity);
			progress.setRange(0, symbolNames.size() + tagNames.size());*/
			////ɾ��symbol
			//for (int i = 0; i < symbolNames.size(); i++)
			//{
			//	progress.setValue(i);
			//	if (progress.wasCanceled())
			//	{
			//		QMessageBox::warning(this, QStringLiteral("������ſ�"), QStringLiteral("ɾ���жϣ�������ѡ��"));
			//		return;
			//	}
			//	
			//	mStyle->removeSymbol(symbolNames[i]);
			//}
			////ɾ����ǩ
			//for (int i = 0; i < tagNames.size(); i++)
			//{
			//	progress.setValue(symbolNames.size() + i);
			//	if (progress.wasCanceled())
			//	{
			//		QMessageBox::warning(this, QStringLiteral("������ſ�"), QStringLiteral("ɾ���жϣ�������ѡ��"));
			//		return;
			//	}
			//	int id = mStyle->tagId(tagNames[i]);
			//	mStyle->remove(QgsStyle::StyleEntity::TagEntity, id);
			//}
			////�ݲ�ɾ��ɫ����TODO...
			//progress.close();
		}
	}

	QStringList fileNames = QFileDialog::getOpenFileNames(this, QStringLiteral("������ſ�"), "", QStringLiteral("xml�ļ�(*.xml)"));
	if (fileNames.size() < 1)
	{
		return;
	}
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	for (int i = 0; i < fileNames.size(); i++)
	{
		bool status = mStyle->importXml(fileNames[i]);
		if (!status)
		{
			QApplication::restoreOverrideCursor();
			QFileInfo file(fileNames[i]);
			QMessageBox::warning(this, QStringLiteral("������ſ�"), QStringLiteral("���롰%1�����������Ի��߼���ļ���").arg(file.baseName()));
		}
		if (isDelete)
		{
			if (mModel != nullptr)
			{
				delete mModel;
				mModel = nullptr;
			}
			mModel = new QgsStyleProxyModel(mStyle);
			mModel->addDesiredIconSize(ui.listItems->iconSize());
			mModel->addDesiredIconSize(ui.treeView->iconSize());
			ui.listItems->setModel(mModel);
		}
		
	}
	populateGroups();
	//���ؼ���ʱ��
	ui.tabWidget->setCurrentIndex(2);
	ui.treeView->setCurrentIndex(ui.treeView->model()->index(0, 0));
	int count = ((QStandardItemModel*)ui.listItems->model())->rowCount();
	for (int i = 0; i < count; i++)
	{
		mModel->data(((QStandardItemModel*)ui.listItems->model())->index(i, 0), 1);
	}
	ui.tabWidget->setCurrentIndex(0);
	tabItemTypeChanged();

	ui.listItems->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	QApplication::restoreOverrideCursor();
	ui.treeView->resizeColumnToContents(0);
	QMessageBox::warning(this, QStringLiteral("������ſ�"), QStringLiteral("������ɣ�"));
}

void S3dmSymbolManager::importMapGIS()
{
	//QString path = QFileDialog::getExistingDirectory(this, QStringLiteral("��ѡ��MapGIS���ſ������ļ���(Slib)"));
	//if (path.isEmpty())
	//{
	//	return;
	//}
	//s3dmSvgConvert convert;
	//int status = convert.convertLibToSVG(path.toLocal8Bit().data(), 150, path.toLocal8Bit().data());
	//if (!status)
	//{
	//	QMessageBox::warning(this, QStringLiteral("����MapGIS���ſ�"), QStringLiteral("ת��ʧ�ܣ�������ѡ��·��"));
	//	return;
	//}
	//QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	////Ѱ��ת������ļ��У��ӵ㿪ʼ����
	//QDir pointPath(path + "/point");
	//if (pointPath.exists())
	//{
	//	QStringList points = pointPath.entryList();
	//	if (points.size() > 0)
	//	{
	//		for (int i = 2; i < points.size(); i++)
	//		{
	//			QFileInfo file(path + "/" + points[i]);
	//			QgsMarkerSymbol *symbol = new QgsMarkerSymbol();
	//			QgsSvgMarkerSymbolLayer *layer = new QgsSvgMarkerSymbolLayer(path + "/point/" + points[i]);
	//			symbol->deleteSymbolLayer(0);
	//			symbol->appendSymbolLayer(layer);
	//			mStyle->addSymbol(file.baseName(), symbol, true);
	//			QStringList tags;
	//			tags.push_back("MapGIS_point");
	//			mStyle->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, file.baseName(), tags);
	//			mStyle->addFavorite(QgsStyle::StyleEntity::SymbolEntity, file.baseName());
	//		}
	//	}
	//}
	//QDir linePath(path + "/line");
	//if (linePath.exists())
	//{
	//	QStringList lines = linePath.entryList();
	//	if (lines.size() > 0)
	//	{
	//		for (int i = 2; i < lines.size(); i++)
	//		{
	//			QFileInfo file(path + "/" + lines[i]);
	//			QgsLineSymbol *symbol = new QgsLineSymbol();
	//			QgsMarkerLineSymbolLayer *symbolLayer = new QgsMarkerLineSymbolLayer();
	//			QgsMarkerSymbol *subSymbol = new QgsMarkerSymbol();
	//			QgsSvgMarkerSymbolLayer *subSymbolLayer = new QgsSvgMarkerSymbolLayer(path + "/line/" + lines[i]);
	//			subSymbol->deleteSymbolLayer(0);
	//			subSymbol->appendSymbolLayer(subSymbolLayer);
	//			symbolLayer->setSubSymbol(subSymbol);
	//			symbol->deleteSymbolLayer(0);
	//			symbol->appendSymbolLayer(symbolLayer);
	//			mStyle->addSymbol(file.baseName(), symbol, true);
	//			QStringList tags;
	//			tags.push_back("MapGIS_line");
	//			mStyle->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, file.baseName(), tags);
	//			mStyle->addFavorite(QgsStyle::StyleEntity::SymbolEntity, file.baseName());
	//		}
	//	}
	//}
	//QDir polyPath(path + "/polygon");
	//if (polyPath.exists())
	//{
	//	QStringList polys = polyPath.entryList();
	//	if (polys.size() > 0)
	//	{
	//		for (int i = 2; i < polys.size(); i++)
	//		{
	//			QFileInfo file(path + "/" + polys[i]);
	//			QgsFillSymbol *symbol = new QgsFillSymbol();
	//			QgsSVGFillSymbolLayer *layer = new QgsSVGFillSymbolLayer(path + "/polygon/" + polys[i]);
	//			symbol->deleteSymbolLayer(0);
	//			symbol->appendSymbolLayer(layer);
	//			mStyle->addSymbol(file.baseName(), symbol, true);
	//			QStringList tags;
	//			tags.push_back("MapGIS_fill");
	//			mStyle->tagSymbol(QgsStyle::StyleEntity::SymbolEntity, file.baseName(), tags);
	//			mStyle->addFavorite(QgsStyle::StyleEntity::SymbolEntity, file.baseName());
	//		}
	//	}
	//}
	//populateGroups();
	//tabItemTypeChanged();
	//QApplication::restoreOverrideCursor();
	//ui.treeView->resizeColumnToContents(0);
	//QMessageBox::warning(this, QStringLiteral("����MapGIS���ſ�"), QStringLiteral("������ɣ�"));
}

void S3dmSymbolManager::deleteTag()
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui.treeView->model());
	QModelIndexList indexList = ui.treeView->selectionModel()->selectedRows(0);
	int num = 0;
	for (auto it = indexList.begin(); it != indexList.end(); it++)
	{
		QString data = (*it).data(Qt::UserRole + 1).toString();
		if (data == QLatin1String("all") || data == QLatin1String("favorite") || data == QLatin1String("tags"))
		{
			QMessageBox::warning(this, QStringLiteral("�Ƴ���"), QStringLiteral("�޷�ɾ��ϵͳ����ı�ǩ�飬��ѡ�����ڱ�ǩ"));
			continue;
		}
		QStandardItem *parentItem = model->itemFromIndex((*it).parent());
		mStyle->remove(QgsStyle::TagEntity, (*it).data(Qt::UserRole + 1).toInt());
		parentItem->removeRow((*it).row()-num);
		num++;
	}
}

void S3dmSymbolManager::editItem()
{
	////��ȡ���̣����inkscape����ر�
	//HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	//if (SnapShot == INVALID_HANDLE_VALUE)
	//	return;
	//PROCESSENTRY32 ProcessInfo;
	//ProcessInfo.dwSize = sizeof(ProcessInfo);

	//BOOL Status = Process32First(SnapShot, &ProcessInfo);

	//while (Status)
	//{
	//	if (ProcessInfo.szExeFile == "inkscape.exe"|| ProcessInfo.szExeFile=="gdbus.exe")
	//	{
	//		QMessageBox::warning(this, QStringLiteral("���ű༭"), QStringLiteral("��һ��༭��δ����������һ�α༭һ�����ţ�"));
	//		return;
	//	}
	//	Status = Process32Next(SnapShot, &ProcessInfo);
	//}
	//CloseHandle(SnapShot);
	if (ui.tabWidget->currentIndex() != 3)
	{
		QModelIndex index = ui.listItems->selectionModel()->currentIndex();
		QString symbolName = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();
		QgsSymbol *symbol = mStyle->symbol(symbolName);
		QString filepath = "";
		QString type = "";
		QString subType = "";
		int layerIndex = -1;
		int lineSubLayerIndex = -1;
		for (int i = 0; i < symbol->symbolLayerCount(); i++)
		{
			type = symbol->symbolLayer(i)->layerType();
			if (type == QStringLiteral("SvgMarker"))
			{
				layerIndex = i;
				QgsSvgMarkerSymbolLayer *layer = (QgsSvgMarkerSymbolLayer*)(symbol->symbolLayer(i));
				filepath = layer->path();
				break;
			}
			else if (type == QStringLiteral("SVGFill"))
			{
				layerIndex = i;
				QgsSVGFillSymbolLayer *layer = (QgsSVGFillSymbolLayer*)(symbol->symbolLayer(i));
				filepath = layer->svgFilePath();
				break;
			}
			else if (type == QStringLiteral("RasterMarker"))
			{
				layerIndex = i;
				QgsRasterMarkerSymbolLayer *layer = (QgsRasterMarkerSymbolLayer*)(symbol->symbolLayer(i));
				filepath = layer->path();
				break;
			}
			else if (type == QStringLiteral("RasterFill"))
			{
				layerIndex = i;
				QgsRasterFillSymbolLayer *layer = (QgsRasterFillSymbolLayer*)(symbol->symbolLayer(i));
				filepath = layer->imageFilePath();
				break;
			}
			else if (type == QStringLiteral("MarkerLine"))
			{
				for (int j = 0; j < symbol->symbolLayer(i)->subSymbol()->symbolLayerCount(); j++)
				{
					subType = symbol->symbolLayer(i)->subSymbol()->symbolLayer(j)->layerType();
					if (subType == QStringLiteral("SvgMarker"))
					{
						layerIndex = i;
						lineSubLayerIndex = j;
						QgsSvgMarkerSymbolLayer *layer = (QgsSvgMarkerSymbolLayer*)(symbol->symbolLayer(i)->subSymbol()->symbolLayer(j));
						filepath = layer->path();
						break;
					}
					else if (subType == QStringLiteral("RasterMarker"))
					{
						layerIndex = i;
						lineSubLayerIndex = j;
						QgsRasterMarkerSymbolLayer *layer = (QgsRasterMarkerSymbolLayer*)(symbol->symbolLayer(i)->subSymbol()->symbolLayer(j));
						filepath = layer->path();
						break;
					}
				}
			}
		}
		if (filepath.isEmpty())
		{
			QMessageBox::warning(this, QStringLiteral("���ű༭"), QStringLiteral("��ǰ���Ų���svg/ͼƬ�ļ�������༭��"));
			return;
		}
		QString path = "";
		QStringList pathList = path.split("/");

		path.clear();
		for (int i = 0; i < pathList.size() - 3; i++)
		{
			path += pathList[i] + "/";
		}
		path += "inkscape/bin/inkscape.exe";
		QProcess *process = new QProcess(this);
		QStringList list;
		list.push_back(filepath);
		process->start(path, list, QIODevice::OpenModeFlag::ReadWrite);
		process->waitForFinished(-1);
		if (QProcess::NotRunning == process->state())
		{
			mStyle->addSymbol(symbolName, mStyle->symbol(symbolName), true);
			//emit mModel->dataChanged(index, index, QVector< int >() << Qt::DecorationRole); 
			//mStyle->addSymbol(symbolName, symbol->clone(), true);
		}
	}
	else
	{
		/*QgsGradientColorRamp *gradRamp = static_cast<QgsGradientColorRamp *>(ramp.get());
		QgsGradientColorRampDialog dlg(*gradRamp, this);
		if (mReadOnly)
			dlg.buttonBox()->button(QDialogButtonBox::Ok)->setEnabled(false);

		if (!dlg.exec())
		{
			return false;
		}
		ramp.reset(dlg.ramp().clone());*/
	}
}

void S3dmSymbolManager::copyItem()
{
	const QList< ItemDetails > items = selectedItems();
	if (items.empty())
		return;
	ItemDetails details = items.at(0);
	if(details.entityType == QgsStyle::SymbolEntity)
	{
		std::unique_ptr< QgsSymbol > symbol(mStyle->symbol(details.name));
		if (!symbol)
			return;
		QApplication::clipboard()->setMimeData(QgsSymbolLayerUtils::symbolToMimeData(symbol.get()));
	}
	mActionPasteItem->setEnabled(true);
}

void S3dmSymbolManager::pasteItem()
{
	const QString defaultTag = ui.treeView->currentIndex().isValid() ? ui.treeView->currentIndex().data().toString() : QString();
	std::unique_ptr< QgsSymbol > tempSymbol(QgsSymbolLayerUtils::symbolFromMimeData(QApplication::clipboard()->mimeData()));
	if (tempSymbol)
	{
		S3dmSymbolSaveDlg saveDlg(this);
		saveDlg.setDefaultTags(defaultTag);
		int res = saveDlg.exec();
		if (res != QDialog::Accepted || saveDlg.name().isEmpty())
			return;
		QStringList saveTags = saveDlg.tags().split(",");
		for (int i = 0; i < saveTags.size(); i++)
		{
			if (!mStyle->tags().contains(saveTags[i]))
			{
				QMessageBox::warning(this, QStringLiteral("ճ������"), QStringLiteral("��ѡ�����б�ǩ��"));
				return;
			}
		}
		
		if (mStyle->symbolNames().contains(saveDlg.name()))
		{
			int res = QMessageBox::warning(this, QStringLiteral("ճ������"),
				QStringLiteral("��Ϊ '%1' �ķ����Ѵ���. �Ƿ񸲸�?")
				.arg(saveDlg.name()),
				QMessageBox::Ok | QMessageBox::Cancel);
			if (res != QMessageBox::Ok)
			{
				return;
			}
			mStyle->removeSymbol(saveDlg.name());
		}

		QStringList symbolTags = saveDlg.tags().split(',');
		mStyle->addSymbol(saveDlg.name(), tempSymbol->clone());
		mStyle->saveSymbol(saveDlg.name(), tempSymbol->clone(), saveDlg.isFavorite(), symbolTags);
		return;
	}
}

void S3dmSymbolManager::selectItem()
{
	QModelIndex index = ui.listItems->selectionModel()->currentIndex();
	QString symbolName = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();
	if (symbolName.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("ѡ����"), QStringLiteral("��ȡ����ʧ��"));
		return;
	}
	/*std::unique_ptr< QgsSymbol > symbol(mStyle->symbol(symbolName));
	bool hasSVG = false;
	for (int i = 0; i < symbol.get()->symbolLayerCount(); i++)
	{
		QString type = symbol.get()->symbolLayer(i)->layerType();
		if (type == QStringLiteral("SvgMarker") || type == QStringLiteral("SvgFill"))
		{
			hasSVG = true;
			break;
		}
		if (type == QStringLiteral("MarkerLine"))
		{
			for (int j = 0; j < symbol.get()->symbolLayer(i)->subSymbol()->symbolLayerCount(); j++)
			{
				type = symbol.get()->symbolLayer(i)->subSymbol()->symbolLayer(j)->layerType();
				if (type == QStringLiteral("SvgMarker"))
				{
					hasSVG = true;
					break;
				}
			}
		}
	}*/
	/*if (hasSVG)
	{
		ui.stackedWidget->setCurrentIndex(0);
	}
	else
	{
		ui.stackedWidget->setCurrentIndex(1);
	}*/
	/*QImage getImage = symbol.get()->asImage(QSize(100, 94));
	ui.labelPreview->setPixmap(QPixmap::fromImage(getImage));
	ui.labelPreview->setAcceptDrops(Qt::AlignCenter);*/
}

void S3dmSymbolManager::exportItemToSVG()
{
	S3dmSymbolExportDlg exportDlg(S3dmSymbolExportDlg::Type::SVG, this);
	if (exportDlg.exec() == QDialog::Accepted)
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		QString filepath = exportDlg.path();
		const QList< ItemDetails > items = selectedItems();
		for (const ItemDetails &details : items)
		{
			if (details.entityType != QgsStyle::SymbolEntity)
				continue;

			QString path = filepath + '/' + details.name + ".svg";
			std::unique_ptr< QgsSymbol > sym(mStyle->symbol(details.name));
			if (sym)
				sym->exportImage(path, "svg", QSize(exportDlg.width(), exportDlg.height()));
		}
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, QStringLiteral("����SVG"), QStringLiteral("������ɣ�"));
	}
}

void S3dmSymbolManager::exportItemToImage()
{
	S3dmSymbolExportDlg exportDlg(S3dmSymbolExportDlg::Type::PNG, this);
	if (exportDlg.exec() == QDialog::Accepted)
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		QString filepath = exportDlg.path();
		const QList< ItemDetails > items = selectedItems();
		for (const ItemDetails &details : items)
		{
			if (details.entityType != QgsStyle::SymbolEntity)
				continue;

			QString path = filepath + '/' + details.name + ".png";
			std::unique_ptr< QgsSymbol > sym(mStyle->symbol(details.name));
			if (sym)
				sym->exportImage(path, "png", QSize(exportDlg.width(), exportDlg.height()));
		}
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, QStringLiteral("����PNG"), QStringLiteral("������ɣ�"));
	}
}

void S3dmSymbolManager::deleteItem()
{
	const QList< ItemDetails > items = selectedItems();

	if (ui.tabWidget->currentIndex() == 0)
	{
		if (QMessageBox::Yes != QMessageBox::warning(this, QStringLiteral("�Ƴ���"),
			QStringLiteral("��ȷ��Ҫ�Ƴ� %1 ������?").arg(items.count()),
			QMessageBox::Yes, QMessageBox::No))
			return;
	}
	else
	{
		if (ui.tabWidget->currentIndex() <= 3)
		{
			if (QMessageBox::Yes != QMessageBox::warning(this, QStringLiteral("�Ƴ�����"),
				QStringLiteral("��ȷ��Ҫ�Ƴ� %1 ��������?").arg(items.count()),
				QMessageBox::Yes, QMessageBox::No))
				return;
		}
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	for (const ItemDetails &details : items)
	{
		if (details.name.isEmpty())
			continue;
		if(details.entityType == QgsStyle::SymbolEntity)
		{
			mStyle->removeSymbol(details.name);
			continue;
		}
	}
	mStyle->load(QgsApplication::userStylePath());
	QApplication::restoreOverrideCursor();
}

void S3dmSymbolManager::filterSymbols(const QString &qword)
{
	mModel->setFilterString(qword);
}

void S3dmSymbolManager::populateGroups()
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui.treeView->model());
	model->clear();

	QStandardItem *allSymbols = new QStandardItem(QStringLiteral("ȫ��"));
	allSymbols->setData("all");
	allSymbols->setEditable(false);
	QFont font = allSymbols->font();
	font.setBold(true);
	allSymbols->setFont(font);
	model->appendRow(allSymbols);

	QStandardItem *favoriteSymbols = new QStandardItem(QStringLiteral("�ղ�"));
	favoriteSymbols->setData("favorite");
	favoriteSymbols->setEditable(false);
	font = favoriteSymbols->font();
	font.setBold(true);
	favoriteSymbols->setFont(font);
	model->appendRow(favoriteSymbols);
	
	QStandardItem *taggroup = new QStandardItem(QString()); //���ÿ�
	taggroup->setData("tags");
	taggroup->setEditable(false);
	QStringList tags = mStyle->tags();
	tags.sort();
	for (const QString &tag : qgis::as_const(tags))
	{
		QStandardItem *item = new QStandardItem(tag);
		item->setData(mStyle->tagId(tag));
		item->setEditable(true);
		taggroup->appendRow(item);
	}
	taggroup->setText(QStringLiteral("��ǩ"));
	font = taggroup->font();
	font.setBold(true);
	taggroup->setFont(font);
	model->appendRow(taggroup);

	//��չ��С
	int rows = model->rowCount(model->indexFromItem(model->invisibleRootItem()));
	for (int i = 0; i < rows; i++)
	{
		ui.treeView->setExpanded(model->indexFromItem(model->item(i)), true);
	}
}

QList<S3dmSymbolManager::ItemDetails> S3dmSymbolManager::selectedItems()
{
	QList<ItemDetails > res;
	QModelIndexList indices = ui.listItems->selectionModel()->selectedIndexes();
	for (const QModelIndex &index : indices)
	{
		if (!index.isValid())
			continue;

		ItemDetails details;
		details.entityType = static_cast<QgsStyle::StyleEntity>(mModel->data(index, QgsStyleModel::TypeRole).toInt());
		if (details.entityType == QgsStyle::SymbolEntity)
			details.symbolType = static_cast<QgsSymbol::SymbolType>(mModel->data(index, QgsStyleModel::SymbolTypeRole).toInt());
		details.name = mModel->data(mModel->index(index.row(), QgsStyleModel::Name, index.parent()), Qt::DisplayRole).toString();

		res << details;
	}
	return res;
}

bool S3dmSymbolManager::deleteSymbols(QgsStyle::StyleEntity type, QgsStyle *style)
{
	QStringList names = style->allNames(type);
	bool groupRemoved = false;
	QString query;
	switch (type)
	{
	case QgsStyle::StyleEntity::SymbolEntity:
		query = QgsSqlite3Mprintf("DELETE FROM symbol; DELETE FROM tagmap");
		break;
	case QgsStyle::StyleEntity::ColorrampEntity:
		query = QgsSqlite3Mprintf("DELETE FROM colorramp");
		break;
	case QgsStyle::StyleEntity::TextFormatEntity:
		query = QgsSqlite3Mprintf("DELETE FROM textformat");
		break;
	case QgsStyle::StyleEntity::LabelSettingsEntity:
		query = QgsSqlite3Mprintf("DELETE FROM labelsettings");
		break;
	case QgsStyle::StyleEntity::TagEntity:
		query = QgsSqlite3Mprintf("DELETE FROM tag");
		groupRemoved = true;
		break;
	case QgsStyle::StyleEntity::SmartgroupEntity:
		query = QgsSqlite3Mprintf("DELETE FROM smartgroup");
		groupRemoved = true;
		break;
	}

	bool result = false;

	bool noError = true;
	sqlite3_database_unique_ptr mCurrentDB;
	mCurrentDB.open(QgsApplication::userStylePath());
	if (!mCurrentDB)
		noError = false;

	char *zErr = nullptr;
	int nErr = sqlite3_exec(mCurrentDB.get(), query.toUtf8().constData(), nullptr, nullptr, &zErr);

	if (nErr != SQLITE_OK)
	{
		sqlite3_free(zErr);
		noError = false;
	}

	if (!noError)
	{
		return false;
	}
	else
	{

		if (groupRemoved)
		{
			QgsSettings settings;
			settings.setValue(QStringLiteral("qgis/symbolsListGroupsIndex"), 0);
			if (type == QgsStyle::StyleEntity::SymbolEntity)
			{
				for (int i = 0; i < names.size(); i++)
				{

					emit style->symbolRemoved(names[i]);
				}
			}
			emit style->groupsModified();
		}
		result = true;
	}
	return result;
}

