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

		//����ΪĬ�ϱ���
		static QgsSymbol* getSymbol(std::string name);
		//����ɫ����
		static QgsSymbol* getSymbol(std::string name, const QColor& color);

	private:
		static QgsStyle m_style;
	};

	class S3D_STYLEMANAGERLIB_EXPORT S3d_StyleManagerLib
	{
	public:
		static S3d_StyleManagerLib* getSingletonPtr();


		//ϵͳ���Ź�����
		void excuteSysStyleManager();
		//ϵͳ����ѡ����
		__int64 excuteSysStyleSelector();

	private:
		S3d_StyleManagerLib();
		~S3d_StyleManagerLib();
	};
}


