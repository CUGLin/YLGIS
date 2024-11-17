#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_YLGIS.h"
#include "qgsmaplayer.h"
#include "qgslayertreemapcanvasbridge.h"
#include "QgsLayerTreeView.h"
#include "QLabel.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptoolpan.h"

class YLGIS : public QMainWindow
{
    Q_OBJECT


public:
    YLGIS(QWidget *parent = nullptr);
    ~YLGIS();

    // 声明图层管理器
    void initLayerTreeView();

    // 初始化鹰眼图控件
    void initMapOverviewCanvas();

private:
    Ui::YLGISClass ui;

public slots:
    //打开工程
    void on_actionOpenProject_triggered();
    // 保存工程
    void on_actionSaveProject_triggered();
    // 另存工程
    void on_actionSaveAsProject_triggered();
    //退出
    void on_actionExit_triggered();

    // 打开图层管理器
    void on_actionLayerTreeControl_triggered();
    // 打开鹰眼图
    void on_actionOverviewMap_triggered();
    // 使用地图放大工具
    void on_actionZoomIn_triggered();
    // 使用地图缩小工具
    void on_actionZoomOut_triggered();
    // 使用地图漫游工具
    void on_actionPan_triggered();
    // 缩放地图到实际比例
    void on_actionZoomActual_triggered();
    // 缩放地图到全图范围
    void on_actionFullExtent_triggered();
    
    // 添加矢量数据
    void on_actionAddVectorData_triggered();
    // 添加栅格数据
    void on_actionAddRasterData_triggered();
    //图层
    void onCurrentLayerChanged(QgsMapLayer* layer);
    // 移除图层
    void removeLayer();
    // 添加WMS图层...
    void on_actionAddWmsLayer_triggered();
    // 添加WFS图层...
    void on_actionAddWfsLayer_triggered();
    // 添加WCS图层...
    void on_actionAddWcsLayer_triggered();


    //符号库管理
    void on_actionSelfStylelibMng_triggered();
    void on_actionQgsStylelibMng_triggered();
    //查看属性表
    void on_actionOpenAttrTable_triggered();
    //查看字段
    void on_actionOpenFields_triggered();


    void mouseCoordinateChanged(const QgsPointXY& newCoordinate);
    // 将墨卡托坐标系下的点坐标转为WGS84坐标系下的坐标
    void convertMercatorToWGS84(const QgsPoint& curPoint, QgsPoint& newPoint);

    //坐标转换
    void on_actionCoordTrans_triggered();
    //shp文件坐标转换
    void on_actionCoordTransShp_triggered();
    void on_actionCoordTransqgis_triggered();
    void on_actionCoordTranstif_triggered();
    void on_actionCoordTranspoint_triggered();

private:
    QAction* actionOpenAttrTable;
    QAction* actionOpenFields;



private:
    QgsMapLayer* m_curMapLayer; //当前选中的图层

    QgsLayerTreeMapCanvasBridge* m_layerTreeCanvasBridge; //创建桥接器连接画布与图层

    QgsMapCanvas* m_mapCanvas = nullptr; //画布

    QgsLayerTreeView* m_layerTreeView = nullptr; // 图层管理器
        
    QgsMapOverviewCanvas* m_overviewCanvas;  // 鹰眼图控件

    QLabel* m_CurrentCoordinage; //鼠标移动显示经纬度

    // 地图缩放工具
    QgsMapTool* m_zoomInTool;
    QgsMapTool* m_zoomOutTool;
    
    QgsMapTool* m_panTool;  // 地图浏览工具
};
