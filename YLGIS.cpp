#include "YLGIS.h"
#include "QFileDialog.h"
#include "QApplication.h"
#include "QgsProject.h"
#include <QgsMapCanvas.h>
#include "QgsVectorLayer.h"
#include "QMessageBox.h"
#include "QgsRasterLayer.h"
#include "QgsLayerTreeView.h"
#include "QgsLayerTreeModel.h"
#include "QgsLayerTreeViewDefaultActions.h"
#include "QgsApplication.h"
#include "QgsGui.h"
#include "QgsLayerTreeViewDefaultActions.h"
#include "QgsDockWidget.h"
#include "S3dmStyleManager.h"
#include "QgsStyleManagerDialog.h"
#include "QgsStyle.h"
#include "QgsVectorLayerCache.h"
#include "QgsAttributeTableView.h"
#include "QgsAttributeTableModel.h"
#include "QgsAttributeTableFilterModel.h"
#include "QgsSourceFieldsProperties.h"
#include "QgsAttributeTableDialog.h"
#include "QgsMapOverviewCanvas.h"
#include "QgsVectorFileWriter.h"
#include "QgsRasterFileWriter.h"
#include "QgsVectorLayerEditBuffer.h"



YLGIS::YLGIS(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	// 地图缩放工具
	m_zoomInTool = nullptr;
	m_zoomOutTool = nullptr;
	// 地图浏览工具
	m_panTool = nullptr;
	// 图层管理器
	m_layerTreeView = nullptr;
	// 桥连接器
	m_layerTreeCanvasBridge = nullptr;
	// 鹰眼图控件
	m_overviewCanvas = nullptr;
	// 连接Canvas到画布
	m_mapCanvas = new QgsMapCanvas();
	this->setCentralWidget(m_mapCanvas);

	// 初始化图层管理器
	m_layerTreeView = new QgsLayerTreeView();
	initLayerTreeView(); //应用图层管理器
	// 初始化鹰眼图控件
	m_overviewCanvas = new QgsMapOverviewCanvas(this, m_mapCanvas);
	initMapOverviewCanvas();

	// 初始化地图工具
	m_zoomInTool = new QgsMapToolZoom(m_mapCanvas, false);
	m_zoomOutTool = new QgsMapToolZoom(m_mapCanvas, true);
	m_panTool = new QgsMapToolPan(m_mapCanvas);


	//QgisApp* app = QgisApp::instance();
	//if (app == nullptr)
	//{
	//	app = new QgisApp();
	//	app->setObjectName(QStringLiteral("QgisApp"));
	//}
	setWindowTitle(QStringLiteral("YLGIS-地理信息系统设计与开发上机实习作业程序-114223袁林"));


	m_CurrentCoordinage = new QLabel("", this);
	this->statusBar()->addWidget(m_CurrentCoordinage); // 设置状态栏

	connect(m_mapCanvas, &QgsMapCanvas::xyCoordinates, this, &YLGIS::mouseCoordinateChanged);



	// 连接操作
	connect(ui.actionOpenAttrTable, &QAction::triggered, this, &YLGIS::on_actionOpenAttrTable_triggered);

	connect(ui.actionOpenFields, &QAction::triggered, this, &YLGIS::on_actionOpenFields_triggered);

}

YLGIS::~YLGIS()
{
	// 防止桥连接器连接到画布在实例化时出现未知的问题
	if (m_layerTreeCanvasBridge) {
		delete m_layerTreeCanvasBridge;
		m_layerTreeCanvasBridge = nullptr;
	}

	if (m_layerTreeView) {
		delete m_layerTreeView;
		m_layerTreeView = nullptr;
	}
}

QgsVectorLayer* createTempLayer(QgsWkbTypes::GeometryType geomType, QString crs, QgsFields fields, QgsFeatureList features)
{
	QgsVectorLayer* newLayerPtr = nullptr;

	if (geomType == QgsWkbTypes::GeometryType::PointGeometry)
	{
		QString layerDef = "Point?crs=" + crs;
		newLayerPtr = new QgsVectorLayer(layerDef, QStringLiteral("PointLayer"), QStringLiteral("memory"));
	}
	else if (geomType == QgsWkbTypes::GeometryType::LineGeometry)
	{
		QString layerDef = "LineString?crs=" + crs;
		newLayerPtr = new QgsVectorLayer(layerDef, QStringLiteral("LineLayer"), QStringLiteral("memory"));
	}
	else if (geomType == QgsWkbTypes::GeometryType::PolygonGeometry)
	{
		QString layerDef = "MultiPolygon?crs=" + crs;
		newLayerPtr = new QgsVectorLayer(layerDef, QStringLiteral("PolyLayer"), QStringLiteral("memory"));
	}

	if (newLayerPtr == nullptr && !newLayerPtr->isValid())
	{
		return newLayerPtr;
	}

	newLayerPtr->commitChanges();
	if (!newLayerPtr->isEditable())
	{
		newLayerPtr->startEditing();
	}
	QgsFields sourceFlds = fields;
	for (int i = 0; i < sourceFlds.size(); i++)
	{
		newLayerPtr->editBuffer()->addAttribute(sourceFlds[i]);
	}
	newLayerPtr->commitChanges();

	if (!newLayerPtr->isEditable())
	{
		newLayerPtr->startEditing();
	}
	newLayerPtr->editBuffer()->addFeatures(features);
	bool flag = newLayerPtr->commitChanges();//保存
	newLayerPtr->editingStopped();
	return newLayerPtr;
}

void YLGIS::on_actionOpenProject_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, QStringLiteral("选择工程文件"), "", "QGIS project (*.qgs)");
	QFileInfo fi(filename);
	if (!fi.exists())
	{
		return;
	}

	QgsProject::instance()->read(filename);

	m_curMapLayer = QgsProject::instance()->mapLayer(0);

	// 删除已有的m_layerTreeCanvasBridge以防止重复实例化
	if (m_layerTreeCanvasBridge) {
		delete m_layerTreeCanvasBridge;
		m_layerTreeCanvasBridge = nullptr;
	}

	m_layerTreeCanvasBridge = new QgsLayerTreeMapCanvasBridge(QgsProject::instance()->layerTreeRoot(), m_mapCanvas, this);
}

void YLGIS::on_actionSaveProject_triggered()
{
	QString filename = QFileDialog::getSaveFileName(this, QStringLiteral("工程保存为"), "", "QGIS project (*.qgs)");
	QFileInfo file(filename);
	QgsProject::instance()->write(filename);
}

void YLGIS::on_actionSaveAsProject_triggered()
{

}

void YLGIS::on_actionExit_triggered() 
{
	this->close();
}

void YLGIS::on_actionLayerTreeControl_triggered()
{
	//// 判断视图情况打开图层管理器
	//if (ui.LayerTreeControl->isVisible())
	//{
	//	ui.LayerTreeControl->setFocus();
	//}
	//else
	//{
	//	ui.LayerTreeControl->show();
	//}

	ui.LayerTreeControl->show();  //简单的打开图层管理器
}

void YLGIS::on_actionOverviewMap_triggered()
{
	// 打开鹰眼图
	if (ui.OverviewMap->isVisible())
	{
		ui.OverviewMap->setFocus();
	}
	else
	{
		ui.OverviewMap->show();
	}

	//点击进行一次刷新
	m_overviewCanvas->setLayers(m_mapCanvas->layers());

}


void YLGIS::on_actionAddVectorData_triggered()
{
	QStringList layerPathList = QFileDialog::getOpenFileNames(this, QStringLiteral("选择矢量数据"), "", "shapefile (*.shp)");
	QList<QgsMapLayer*> layerList;
	for (QString layerPath : layerPathList)
	{
		QFileInfo fi(layerPath);
		if (!fi.exists()) { return; }
		QString layerBaseName = fi.baseName(); // 图层名称

		QgsVectorLayer* vecLayer = new QgsVectorLayer(layerPath, layerBaseName);
		if (!vecLayer) { return; }
		if (!vecLayer->isValid())
		{
			QMessageBox::information(0, "", "layer is invalid");
			return;
		}
		layerList << vecLayer;
	}
	if (layerList.size() < 1)
		return;
	//选择图层
	connect(m_layerTreeView, &QgsLayerTreeView::currentLayerChanged, this, &YLGIS::onCurrentLayerChanged);

	m_curMapLayer = layerList[0];
	QgsProject::instance()->addMapLayers(layerList);
	m_mapCanvas->refresh();

}

void YLGIS::on_actionAddRasterData_triggered()
{
	QStringList layerPathList = QFileDialog::getOpenFileNames(this, QStringLiteral("选择栅格数据"), "", "Image (*.img *.tif *.tiff)");
	QList<QgsMapLayer*> layerList;
	for (QString layerPath : layerPathList)
	{
		QFileInfo fi(layerPath);
		if (!fi.exists()) { return; }
		QString layerBaseName = fi.baseName(); // 图层名称

		QgsRasterLayer* rasterLayer = new QgsRasterLayer(layerPath, layerBaseName);
		if (!rasterLayer) { return; }
		if (!rasterLayer->isValid())
		{
			QMessageBox::information(0, "", "layer is invalid");
			return;
		}
		layerList << rasterLayer;
	}

	QgsProject::instance()->addMapLayers(layerList);
	m_mapCanvas->refresh();
}

void YLGIS::removeLayer()
{
	if (!m_layerTreeView) { return; }

	QModelIndexList indexes;
	while ((indexes = m_layerTreeView->selectionModel()->selectedRows()).size()) {
		m_layerTreeView->model()->removeRow(indexes.first().row());
	}
}

void YLGIS::onCurrentLayerChanged(QgsMapLayer* layer)
{
	QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>(layer);
	if (vectorLayer)
	{
		//判断是否为你想要处理的图层类型，例如shapefile
		if (vectorLayer->dataProvider()->name() == "ogr" && vectorLayer->source().endsWith(".shp"))
		{
			//获取矢量图层的路径
			m_curMapLayer = vectorLayer;
		}
	}
	QgsRasterLayer* rasterLayer = qobject_cast<QgsRasterLayer*>(layer);
	m_curMapLayer = rasterLayer;
}



void YLGIS::initLayerTreeView()
{

	QgsLayerTreeModel* model = new QgsLayerTreeModel(QgsProject::instance()->layerTreeRoot(), this);
	qDebug() << QgsProject::instance()->layerTreeRoot();

	// 创建 MODEL 类型建立右键选项
	model->setFlag(QgsLayerTreeModel::AllowNodeReorder);
	model->setFlag(QgsLayerTreeModel::AllowNodeRename);
	model->setFlag(QgsLayerTreeModel::AllowNodeChangeVisibility);
	model->setFlag(QgsLayerTreeModel::ShowLegendAsTree);
	model->setFlag(QgsLayerTreeModel::UseEmbeddedWidgets);
	model->setAutoCollapseLegendNodes(10);

	//创建View
	m_layerTreeView = new QgsLayerTreeView;

	 //将 MODEL 实例化到树结构的图层上
	m_layerTreeView->setModel(model);

	// 创建连接
	m_layerTreeCanvasBridge = new QgsLayerTreeMapCanvasBridge(QgsProject::instance()->layerTreeRoot(), m_mapCanvas, this);
	connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument&)), m_layerTreeCanvasBridge, SLOT(writeProject(QDomDocument&)));
	connect(QgsProject::instance(), SIGNAL(readProject(QDomDocument)), m_layerTreeCanvasBridge, SLOT(readProject(QDomDocument)));

	// 添加组命令
	QAction* actionAddGroup = new QAction(QStringLiteral("添加组"), this); 
	actionAddGroup->setIcon(QIcon(QStringLiteral(":/YLGIS/Resources/mActionAddGroup.png")));
	actionAddGroup->setToolTip(QStringLiteral("添加组"));
	connect(actionAddGroup, &QAction::triggered, m_layerTreeView->defaultActions(), &QgsLayerTreeViewDefaultActions::addGroup);

	// 扩展和收缩图层树
	QAction* actionExpandAll = new QAction(QStringLiteral("展开所有组"), this);
	actionExpandAll->setIcon(QIcon(QStringLiteral(":/YLGIS/Resources/mActionExpandTree.png")));
	actionExpandAll->setToolTip(QStringLiteral("展开所有组"));
	connect(actionExpandAll, &QAction::triggered, m_layerTreeView, &QgsLayerTreeView::expandAllNodes);
	QAction* actionCollapseAll = new QAction(QStringLiteral("折叠所有组"), this);
	actionCollapseAll->setIcon(QIcon(QStringLiteral(":/YLGIS/Resources/mActionCollapseTree.png")));
	actionCollapseAll->setToolTip(QStringLiteral("折叠所有组"));
	connect(actionCollapseAll, &QAction::triggered, m_layerTreeView, &QgsLayerTreeView::collapseAllNodes);

	// 移除图层
	QAction* actionRemoveLayer = new QAction(QStringLiteral("移除图层/组"));
	actionRemoveLayer->setIcon(QIcon(QStringLiteral(":/YLGIS/Resources/mActionRemoveLayer.png")));
	actionRemoveLayer->setToolTip(QStringLiteral("移除图层/组"));
	connect(actionRemoveLayer, &QAction::triggered, this, &YLGIS::removeLayer);

	QToolBar* toolbar = new QToolBar();
	toolbar->setIconSize(QSize(25, 25));
	toolbar->addAction(actionAddGroup);
	toolbar->addAction(actionExpandAll);
	toolbar->addAction(actionCollapseAll);
	toolbar->addAction(actionRemoveLayer);

	QVBoxLayout* vBoxLayout = new QVBoxLayout();
	vBoxLayout->setMargin(0);
	vBoxLayout->setContentsMargins(0, 0, 0, 0);
	vBoxLayout->setSpacing(0);
	vBoxLayout->addWidget(toolbar);
	vBoxLayout->addWidget(m_layerTreeView);

	QWidget* widget = new QWidget;
	widget->setLayout(vBoxLayout);
	this->ui.LayerTreeControl->setWidget(widget);

}

void YLGIS::initMapOverviewCanvas() {
	// 设置背景颜色
	m_overviewCanvas->setBackgroundColor(QColor(180, 180, 180));

	// 创建布局并添加 overview canvas
	QVBoxLayout* vBoxLayout = new QVBoxLayout();
	vBoxLayout->setMargin(0);
	vBoxLayout->setContentsMargins(0, 0, 0, 0);
	vBoxLayout->setSpacing(0);
	vBoxLayout->addWidget(m_overviewCanvas);

	QWidget* w = new QWidget;
	w->setLayout(vBoxLayout);
	w->setMinimumHeight(100);

	// 将布局设置到主界面
	this->ui.OverviewMap->setWidget(w);

	// 设置地图图层同步
	m_overviewCanvas->setLayers(m_mapCanvas->layers());

	// 获取主地图视图的范围并转换为 QRectF
	QgsRectangle extent = m_mapCanvas->extent();
	QRectF reducedExtent(
		extent.xMinimum() - extent.width() * 0.5,
		extent.yMinimum() - extent.height() * 0.5,
		extent.width() * 2,
		extent.height() * 2
	);



	// 连接地图刷新信号以保持同步
	connect(m_mapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, [this]() {
		QgsRectangle extent = m_mapCanvas->extent();
		QRectF reducedExtent(
			extent.xMinimum() - extent.width() * 0.5,
			extent.yMinimum() - extent.height() * 0.5,
			extent.width() * 2,
			extent.height() * 2
		);

		m_overviewCanvas->refresh();
		});

	// 绑定 layer tree
	m_layerTreeCanvasBridge->setOverviewCanvas(m_overviewCanvas);


}



void YLGIS::on_actionAddWmsLayer_triggered()
{

}

void YLGIS::on_actionAddWfsLayer_triggered()
{

}

void YLGIS::on_actionAddWcsLayer_triggered()
{


}

void YLGIS::on_actionSelfStylelibMng_triggered()
{
	Smart3dMap::S3d_StyleManagerLib::getSingletonPtr()->excuteSysStyleManager();
}

void YLGIS::on_actionQgsStylelibMng_triggered()
{

	//显示属性表
	static QgsStyle style;

	if (style.symbolCount() == 0)
	{
		sqlite3_initialize();
		style.load(QgsApplication::userStylePath());
	}
	QgsStyleManagerDialog* dlg = new QgsStyleManagerDialog(&style);
	dlg->show();
}

void YLGIS::on_actionOpenAttrTable_triggered()
{
	// 检查当前图层是否有效
	QgsVectorLayer* layer = (QgsVectorLayer*)m_curMapLayer;
	if (layer == nullptr || !layer->isValid() || QgsMapLayerType::VectorLayer != m_curMapLayer->type())
	{
		qDebug() << QStringLiteral("当前图层无效或不是矢量图层。");
		return;
	}

	// 检查图层是否有要素，若没有则提示
	if (layer->featureCount() == 0)
	{
		qDebug() << QStringLiteral("图层中没有要素数据。");
		return;
	}

	// 创建一个属性表缓存（缓存大小可根据图层要素数量调整）
	QgsVectorLayerCache* lc = new QgsVectorLayerCache(layer, 5000);

	// 创建属性表模型并加载图层数据
	QgsAttributeTableModel* tm = new QgsAttributeTableModel(lc);
	//if (!tm->loadLayer())  // 确保加载成功
	//{
	//	qDebug() << "属性表模型加载失败。";
	//	delete lc;
	//	delete tm;
	//	return;
	//}

	// 创建过滤模型，并设置为显示所有数据
	QgsAttributeTableFilterModel* tfm = new QgsAttributeTableFilterModel(m_mapCanvas, tm, tm);
	tfm->setFilterMode(QgsAttributeTableFilterModel::ShowAll);

	// 检查是否已经存在一个打开的属性表视图，避免重复打开
	static QgsAttributeTableView* tv = nullptr;
	if (tv)  // 如果已存在，则直接显示
	{
		tv->raise();
		tv->activateWindow();
		return;
	}

	// 创建并设置属性表视图
	tv = new QgsAttributeTableView();
	tv->setModel(tfm);

	// 显示属性表视图
	tv->show();
	tv->raise();
	tv->activateWindow();  // 将窗口置于前台

	// 在窗口关闭时，重置 tv 指针，确保下次可以重新创建
	connect(tv, &QgsAttributeTableView::destroyed, [=]() {
		tv = nullptr;
		});
}


void YLGIS::on_actionOpenFields_triggered()
{
	// 检查当前图层是否有效
	QgsVectorLayer* layer = (QgsVectorLayer*)m_curMapLayer;
	if (layer == nullptr || !layer->isValid() || QgsMapLayerType::VectorLayer != m_curMapLayer->type())
	{
		qDebug() << QStringLiteral("当前图层无效或不是矢量图层。");
		return;
	}

	// 启用编辑模式（如果未启动）
	if (!layer->isEditable())
	{
		layer->startEditing();
	}

	// 静态指针，确保只打开一个字段结构窗口
	static QgsSourceFieldsProperties* pWidget = nullptr;
	if (pWidget)  // 如果窗口已存在，则直接激活
	{
		pWidget->raise();
		pWidget->activateWindow();
		return;
	}

	// 创建属性结构窗口并加载字段数据
	pWidget = new QgsSourceFieldsProperties(layer, nullptr);
	pWidget->loadRows();

	// 显示窗口
	pWidget->show();
	pWidget->raise();
	pWidget->activateWindow();

	// 连接窗口销毁事件，窗口关闭时重置 pWidget 指针
	connect(pWidget, &QgsSourceFieldsProperties::destroyed, [=]() {
		pWidget = nullptr;
		});
}


void YLGIS::mouseCoordinateChanged(const QgsPointXY& newCoordinate)
{
	QgsPoint point(newCoordinate.x(), newCoordinate.y()), newPoint;
	convertMercatorToWGS84(point, newPoint);

	auto qsCurCoordinate = QString(QString::fromLocal8Bit("lon:%1, lat:%2")).arg(QString::number(newPoint.x())).arg(QString::number(newPoint.y()));
	m_CurrentCoordinage->setText(qsCurCoordinate);
}
   

// 将墨卡托坐标系下的点坐标转为WGS84坐标系下的坐标
void YLGIS::convertMercatorToWGS84(const QgsPoint& curPoint, QgsPoint& newPoint)
{
	QgsCoordinateReferenceSystem dstCrs("EPSG:4326");
	QgsCoordinateReferenceSystem srcCrs("EPSG:3857");
	const QgsCoordinateTransformContext context;
	QgsCoordinateTransform crsTras(srcCrs, dstCrs, context);

	auto tmpPt = crsTras.transform(curPoint);
	newPoint.setX(tmpPt.x());
	newPoint.setY(tmpPt.y());

}


void YLGIS::on_actionCoordTrans_triggered()
{
	//shp文件转换
	QgsVectorLayer* layer = (QgsVectorLayer*)m_curMapLayer;
	QString outputFile = QStringLiteral("E:/output.shp");;
	if (layer == nullptr || !layer->isValid() || QgsMapLayerType::VectorLayer != m_curMapLayer->type())
	{
		return;
	}

	//确定原始图层的参考系和目标图层的参考系
	QString targetCrsCode = "EPSG:4326";
	QgsCoordinateReferenceSystem shpCrs = layer->crs();//原始图层参考系

	QgsCoordinateReferenceSystem targetCrs = QgsCoordinateReferenceSystem();
	targetCrs.createFromProj("+proj=utm +zone=49 +datum=WGS84 +units=m +no_defs");//目标图层参考系

	if (!targetCrs.isValid()) {
		qDebug() << "目标图层参考系无效:" << targetCrs.authid();
		return;
	}
	//构造坐标转换对象
	QString srsDbName = QgsApplication::srsDatabaseFilePath();
	QgsCoordinateTransform* pTransform = new QgsCoordinateTransform(shpCrs, targetCrs, QgsProject::instance());
	if (pTransform == nullptr || !pTransform->isValid())
	{
		return;
	}
	//对图层中每一个要素进行转换，将转换后的结果放入postRansfeatureLIst中
	QgsFeatureList postTransfeatureList;
	QgsFeatureIterator iter = layer->getFeatures();
	QgsFeature feature;
	while ((iter.nextFeature(feature)))
	{
		QgsGeometry g = feature.geometry();
		QgsAttributes f = feature.attributes();
		//transform函数就是转换函数，将每一个geometry进行转换
		//QgsGeometry是基类，transform函数是个虚函数
		//根据具体的类型调用各自Geometry中的transfrom
		//具体的转换可以参考QgsGeometry的子类的transform
		if (g.transform(*pTransform) == 0)
		{
			feature.setGeometry(g);
		}
		else
		{
			feature.clearGeometry();
		}
		postTransfeatureList << feature;
	}

	//创建一个新shp,将转换后的postTransfeatureList存入新的shp中
	QString errorMessage;
	const QString fileFormat = "ESRI Shapefile";
	const QString enc = "System";
	QgsFields fields = layer->fields();
	QgsVectorLayer* targetLayer = createTempLayer(layer->geometryType(), targetCrs.authid(), fields, postTransfeatureList);
	QString errMsg;
	QgsVectorFileWriter::SaveVectorOptions saveOptions;
	saveOptions.fileEncoding = targetLayer->dataProvider()->encoding();
	saveOptions.driverName = "ESRI Shapefile";
	targetLayer->setCrs(targetCrs);//再设置一次！

	QgsVectorFileWriter::WriterError err = QgsVectorFileWriter::writeAsVectorFormatV2(targetLayer, outputFile, targetLayer->transformContext(), saveOptions, nullptr, nullptr, &errMsg);
	if (err != QgsVectorFileWriter::WriterError::NoError)
	{
		return;
	}
	QgsProject::instance()->addMapLayer(targetLayer);

	return;

	//qgs工程转换
	//QgsCoordinateReferenceSystem targetCrs = QgsCoordinateReferenceSystem();
	//targetCrs.createFromProj("+proj=utm +zone=49 +datum=WGS84 +units=m +no_defs");//目标图层参考系
	//QgsProject* project = QgsProject::instance();
	//project->setCrs(targetCrs);
	//return;

	//栅格数据转换

}

void YLGIS::on_actionCoordTransShp_triggered()
{
	//shp文件转换
	QgsVectorLayer* layer = (QgsVectorLayer*)m_curMapLayer;
	QString outputFile = QStringLiteral("E:/output.shp");;
	if (layer == nullptr || !layer->isValid() || QgsMapLayerType::VectorLayer != m_curMapLayer->type())
	{
		return;
	}

	//确定原始图层的参考系和目标图层的参考系
	QString targetCrsCode = "EPSG:4326";
	QgsCoordinateReferenceSystem shpCrs = layer->crs();//原始图层参考系

	QgsCoordinateReferenceSystem targetCrs = QgsCoordinateReferenceSystem();
	targetCrs.createFromProj("+proj=utm +zone=49 +datum=WGS84 +units=m +no_defs");//目标图层参考系

	if (!targetCrs.isValid()) {
		qDebug() << "目标图层参考系无效:" << targetCrs.authid();
		return;
	}
	//构造坐标转换对象
	QString srsDbName = QgsApplication::srsDatabaseFilePath();
	QgsCoordinateTransform* pTransform = new QgsCoordinateTransform(shpCrs, targetCrs, QgsProject::instance());
	if (pTransform == nullptr || !pTransform->isValid())
	{
		return;
	}
	//对图层中每一个要素进行转换，将转换后的结果放入postRansfeatureLIst中
	QgsFeatureList postTransfeatureList;
	QgsFeatureIterator iter = layer->getFeatures();
	QgsFeature feature;
	while ((iter.nextFeature(feature)))
	{
		QgsGeometry g = feature.geometry();
		QgsAttributes f = feature.attributes();
		//transform函数就是转换函数，将每一个geometry进行转换
		//QgsGeometry是基类，transform函数是个虚函数
		//根据具体的类型调用各自Geometry中的transfrom
		//具体的转换可以参考QgsGeometry的子类的transform
		if (g.transform(*pTransform) == 0)
		{
			feature.setGeometry(g);
		}
		else
		{
			feature.clearGeometry();
		}
		postTransfeatureList << feature;
	}

	//创建一个新shp,将转换后的postTransfeatureList存入新的shp中
	QString errorMessage;
	const QString fileFormat = "ESRI Shapefile";
	const QString enc = "System";
	QgsFields fields = layer->fields();
	QgsVectorLayer* targetLayer = createTempLayer(layer->geometryType(), targetCrs.authid(), fields, postTransfeatureList);
	QString errMsg;
	QgsVectorFileWriter::SaveVectorOptions saveOptions;
	saveOptions.fileEncoding = targetLayer->dataProvider()->encoding();
	saveOptions.driverName = "ESRI Shapefile";
	targetLayer->setCrs(targetCrs);//再设置一次！

	QgsVectorFileWriter::WriterError err = QgsVectorFileWriter::writeAsVectorFormatV2(targetLayer, outputFile, targetLayer->transformContext(), saveOptions, nullptr, nullptr, &errMsg);
	if (err != QgsVectorFileWriter::WriterError::NoError)
	{
		return;
	}
	QgsProject::instance()->addMapLayer(targetLayer);

	return;
}

void YLGIS::on_actionCoordTransqgis_triggered()
{
	//qgs工程转换
	QgsCoordinateReferenceSystem targetCrs = QgsCoordinateReferenceSystem();
	targetCrs.createFromProj("+proj=utm +zone=49 +datum=WGS84 +units=m +no_defs");//目标图层参考系
	QgsProject* project = QgsProject::instance();
	project->setCrs(targetCrs);
	return;
}

void YLGIS::on_actionCoordTranstif_triggered()
{
	//tif转换
	QgsRasterLayer* layer = (QgsRasterLayer*)m_curMapLayer;
	QString outputFile = QStringLiteral("E:/output.tif");;
	if (layer == nullptr || !layer->isValid())
	{
		return;
	}

	//确定原始图层的参考系和目标图层的参考系
	QString targetCrsCode = "EPSG:4326";
	QgsCoordinateReferenceSystem shpCrs = layer->crs();//原始图层参考系

	QgsCoordinateReferenceSystem targetCrs = QgsCoordinateReferenceSystem();
	targetCrs.createFromProj("+proj=utm +zone=49 +datum=WGS84 +units=m +no_defs");//目标图层参考系
	layer->setCrs(targetCrs);

	if (!targetCrs.isValid()) {
		qDebug() << "目标图层参考系无效:" << targetCrs.authid();
		return;
	}

	QgsRasterFileWriter writer(outputFile);

	// 创建一个QgsRasterPipe对象并设置渲染器
	QgsRasterPipe pipe;
	pipe.set(layer->dataProvider()->clone());
	// 创建一个QgsRasterBlockFeedback对象  
	QgsRasterBlockFeedback feedback;
	// 写入新的tif文件  
	//writer.writeRaster(&pipe, layer->width(), layer->height(), layer->extent(), layer->crs(), &feedback);

	QgsProject::instance()->addMapLayer(layer);

	return;
}
void YLGIS::on_actionCoordTranspoint_triggered()
{
	QgsPointXY curPoint(11472426, 4197692);
	QgsCoordinateReferenceSystem srcCrs = QgsCoordinateReferenceSystem(); // 墨卡托坐标
	srcCrs.createFromProj("+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs");
	QgsCoordinateReferenceSystem dstCrs = QgsCoordinateReferenceSystem();
	dstCrs.createFromProj("+proj=utm +zone=49 +datum=WGS84 +units=m +no_defs");// WSG84坐标

	QgsCoordinateTransform* crsTras = new QgsCoordinateTransform(srcCrs, dstCrs, QgsProject::instance());
	if (crsTras == nullptr || !crsTras->isValid())
	{
		return;
	}
	auto tmpPt = crsTras->transform(curPoint);
	QgsPoint* newPoint = new QgsPoint();
	newPoint->setX(tmpPt.x());
	newPoint->setY(tmpPt.y());

	QList<QgsField> fields;
	fields.append(QgsField("id", QVariant::Int)); // 添加一个字段

	QgsVectorLayer* pointLayer = new QgsVectorLayer("Point?crs=EPSG:4326", "PointLayer", "memory");
	if (!pointLayer) {
		qWarning() << "Layer not created!";
		return;
	}

	// 设置字段
	pointLayer->dataProvider()->addAttributes(fields);
	pointLayer->updateFields();

	// 创建要素
	QgsFeature feature;
	feature.setGeometry(QgsGeometry::fromPointXY(*newPoint)); // 使用 newPoint 设置几何体
	feature.setAttributes(QgsAttributes() << 1); // 设置属性，这里示例为 ID 为 1

	// 将要素添加到图层
	pointLayer->dataProvider()->addFeature(feature);
	pointLayer->updateExtents();

	// 将图层添加到地图画布
	QgsProject::instance()->addMapLayer(pointLayer);
}

void YLGIS::on_actionZoomIn_triggered()
{
	m_mapCanvas->setMapTool(m_zoomInTool);
}
void YLGIS::on_actionZoomOut_triggered()
{
	m_mapCanvas->setMapTool(m_zoomOutTool);
}
void YLGIS::on_actionPan_triggered()
{
	m_mapCanvas->setMapTool(m_panTool);
}
void YLGIS::on_actionZoomActual_triggered()
{

}
void YLGIS::on_actionFullExtent_triggered()
{
	m_mapCanvas->zoomToFullExtent();
}

