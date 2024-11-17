#include "S3dmStyleManager.h"
#include "qgsapplication.h"
#include "qgsmarkersymbollayer.h"
#include "qgsfillsymbollayer.h"



#include "qfiledialog.h"
#include "QgsSymbolLayerRegistry.h"
#include "QgsApplication.h"
#include "qgsfillsymbollayer.h"
#include "qmessagebox.h"
#include "qgsvectorlayer.h"
#include "S3dmSymbolManager.h"
#include "S3dmApplySymbolDlg.h"

QgsStyle Smart3dMap::S3dmStyleManager::m_style;

Smart3dMap::S3dmStyleManager::S3dmStyleManager()
{
}

Smart3dMap::S3dmStyleManager::~S3dmStyleManager()
{
}

bool Smart3dMap::S3dmStyleManager::initStyle()
{
	sqlite3_initialize();
	return m_style.load(QgsApplication::userStylePath());
}

QgsStyle * Smart3dMap::S3dmStyleManager::getS3dmStyle()
{
	if (m_style.symbolCount() == 0)
	{
		initStyle();
	}

	return &m_style;
}

QgsSymbol * Smart3dMap::S3dmStyleManager::getSymbol(std::string name)
{
	if (m_style.symbolCount() == 0)
	{
		initStyle();
	}
	return m_style.symbol(QString::fromLocal8Bit(name.c_str()));
}

QgsSymbol * Smart3dMap::S3dmStyleManager::getSymbol(std::string name, const QColor & color)
{
	if (m_style.symbolCount() == 0)
	{
		initStyle();
	}
	QgsSymbol* oriSymbol = m_style.symbol(QString::fromLocal8Bit(name.c_str()));
	if (oriSymbol == nullptr)
	{
		return nullptr;
	}
	QgsSymbol* symbol = oriSymbol->clone();
	//添加一个背景图层


	QgsSymbol::SymbolType qgsType =symbol->type();

	if (qgsType != QgsSymbol::SymbolType::Line)
	{
		if (qgsType == QgsSymbol::SymbolType::Marker)
		{
			QgsSimpleMarkerSymbolLayer *simplelayer = new QgsSimpleMarkerSymbolLayer();
			simplelayer->setColor(color);
			simplelayer->setStrokeStyle(Qt::PenStyle::NoPen);
			symbol->insertSymbolLayer(0,simplelayer);
		}
		else if (qgsType == QgsSymbol::SymbolType::Fill)
		{
			QgsSimpleFillSymbolLayer *simplelayer = new QgsSimpleFillSymbolLayer();
			simplelayer->setColor(color);
			simplelayer->setStrokeStyle(Qt::PenStyle::NoPen);
			symbol->insertSymbolLayer(0, simplelayer);
		}
	}
	return symbol;
}





// -------------------------------- 材质管理对象 --------------- //
Smart3dMap::S3d_StyleManagerLib * Smart3dMap::S3d_StyleManagerLib::getSingletonPtr()
{
	static S3d_StyleManagerLib instance;
	return &instance;
}

void Smart3dMap::S3d_StyleManagerLib::excuteSysStyleManager()
{
	S3dmSymbolManager* geo3dStyleManager = new S3dmSymbolManager();
	if (geo3dStyleManager == nullptr)
	{
		geo3dStyleManager = new S3dmSymbolManager(nullptr);
		geo3dStyleManager->setObjectName("excuteSysStyleManager");
		geo3dStyleManager->setWindowFlags(geo3dStyleManager->windowFlags() | Qt::Dialog);
	}
	geo3dStyleManager->show();

}

__int64 Smart3dMap::S3d_StyleManagerLib::excuteSysStyleSelector()
{
	S3dmSymbolManager* geo3dStyleManager = new S3dmSymbolManager();
	QgsStyle* qStyle = QgsStyle::defaultStyle();
	if (geo3dStyleManager == nullptr)
	{
		QString dbpath = QgsApplication::userStylePath();
		qStyle->load(dbpath);
	}
	else
	{
		qStyle = geo3dStyleManager->getStyle();
		if (qStyle == nullptr)
		{
			return 0;
		}
	}

	S3dmApplySymbolDlg dlg(qStyle, nullptr, S3dmApplySymbolDlg::symbolType::POLYGON, NULL);
	int res = dlg.exec();
	if (res == QDialog::Accepted)
	{
		return (__int64)dlg.getSymbol();
	}
	else
	{
		return 0;
	}
	return 0;
}


Smart3dMap::S3d_StyleManagerLib::S3d_StyleManagerLib()
{
}

Smart3dMap::S3d_StyleManagerLib::~S3d_StyleManagerLib()
{
}
