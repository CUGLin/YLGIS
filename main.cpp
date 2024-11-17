#include "YLGIS.h"
#include <QtWidgets/QApplication>
#include "qgsapplication.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QIcon icon(":/YLGIS/Resources/YLGIS.png");
    a.setWindowIcon(icon);  // ����Ӧ�ó����ȫ��ͼ��

	QgsApplication::setPrefixPath("E:/OSGeo4W/apps/qgis-ltr", true); // !!!���·��������Ҫ�����޸�
	QgsApplication::init();
	QgsApplication::initQgis();
    YLGIS w;
    w.showMaximized();
    return a.exec();
}

