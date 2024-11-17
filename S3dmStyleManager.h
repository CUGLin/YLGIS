#pragma once
#include "s3d_stylemanagerlib_global.h"
#include "qgsstyle.h"
#include "qgssymbol.h"

namespace Smart3dMap
{

	class S3D_STYLEMANAGERLIB_EXPORT S3dmStyleManager
	{
	public:
		S3dmStyleManager();
		~S3dmStyleManager();

		static bool initStyle();

		static QgsStyle* getS3dmStyle();

		//背景为默认背景
		static QgsSymbol* getSymbol(std::string name);
		//带颜色背景
		static QgsSymbol* getSymbol(std::string name, const QColor& color);

	private:
		static QgsStyle m_style;
	};

	class S3D_STYLEMANAGERLIB_EXPORT S3d_StyleManagerLib
	{
	public:
		static S3d_StyleManagerLib* getSingletonPtr();


		//系统符号管理器
		void excuteSysStyleManager();
		//系统符号选择器
		__int64 excuteSysStyleSelector();

	private:
		S3d_StyleManagerLib();
		~S3d_StyleManagerLib();
	};
}


