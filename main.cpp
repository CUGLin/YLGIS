#include "YLGIS.h"
#include <QtWidgets/QApplication>
#include "qgsapplication.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QIcon icon(":/YLGIS/Resources/YLGIS.png");
    a.setWindowIcon(icon);  // 设置应用程序的全局图标

	QgsApplication::setPrefixPath("E:/OSGeo4W/apps/qgis-ltr", true); // !!!这个路径参数需要自行修改
	QgsApplication::init();
	QgsApplication::initQgis();
    YLGIS w;
    w.showMaximized();
    return a.exec();
}

