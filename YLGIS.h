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

    // ����ͼ�������
    void initLayerTreeView();

    // ��ʼ��ӥ��ͼ�ؼ�
    void initMapOverviewCanvas();

private:
    Ui::YLGISClass ui;

public slots:
    //�򿪹���
    void on_actionOpenProject_triggered();
    // ���湤��
    void on_actionSaveProject_triggered();
    // ��湤��
    void on_actionSaveAsProject_triggered();
    //�˳�
    void on_actionExit_triggered();

    // ��ͼ�������
    void on_actionLayerTreeControl_triggered();
    // ��ӥ��ͼ
    void on_actionOverviewMap_triggered();
    // ʹ�õ�ͼ�Ŵ󹤾�
    void on_actionZoomIn_triggered();
    // ʹ�õ�ͼ��С����
    void on_actionZoomOut_triggered();
    // ʹ�õ�ͼ���ι���
    void on_actionPan_triggered();
    // ���ŵ�ͼ��ʵ�ʱ���
    void on_actionZoomActual_triggered();
    // ���ŵ�ͼ��ȫͼ��Χ
    void on_actionFullExtent_triggered();
    
    // ���ʸ������
    void on_actionAddVectorData_triggered();
    // ���դ������
    void on_actionAddRasterData_triggered();
    //ͼ��
    void onCurrentLayerChanged(QgsMapLayer* layer);
    // �Ƴ�ͼ��
    void removeLayer();
    // ���WMSͼ��...
    void on_actionAddWmsLayer_triggered();
    // ���WFSͼ��...
    void on_actionAddWfsLayer_triggered();
    // ���WCSͼ��...
    void on_actionAddWcsLayer_triggered();


    //���ſ����
    void on_actionSelfStylelibMng_triggered();
    void on_actionQgsStylelibMng_triggered();
    //�鿴���Ա�
    void on_actionOpenAttrTable_triggered();
    //�鿴�ֶ�
    void on_actionOpenFields_triggered();


    void mouseCoordinateChanged(const QgsPointXY& newCoordinate);
    // ��ī��������ϵ�µĵ�����תΪWGS84����ϵ�µ�����
    void convertMercatorToWGS84(const QgsPoint& curPoint, QgsPoint& newPoint);

    //����ת��
    void on_actionCoordTrans_triggered();
    //shp�ļ�����ת��
    void on_actionCoordTransShp_triggered();
    void on_actionCoordTransqgis_triggered();
    void on_actionCoordTranstif_triggered();
    void on_actionCoordTranspoint_triggered();

private:
    QAction* actionOpenAttrTable;
    QAction* actionOpenFields;



private:
    QgsMapLayer* m_curMapLayer; //��ǰѡ�е�ͼ��

    QgsLayerTreeMapCanvasBridge* m_layerTreeCanvasBridge; //�����Ž������ӻ�����ͼ��

    QgsMapCanvas* m_mapCanvas = nullptr; //����

    QgsLayerTreeView* m_layerTreeView = nullptr; // ͼ�������
        
    QgsMapOverviewCanvas* m_overviewCanvas;  // ӥ��ͼ�ؼ�

    QLabel* m_CurrentCoordinage; //����ƶ���ʾ��γ��

    // ��ͼ���Ź���
    QgsMapTool* m_zoomInTool;
    QgsMapTool* m_zoomOutTool;
    
    QgsMapTool* m_panTool;  // ��ͼ�������
};
