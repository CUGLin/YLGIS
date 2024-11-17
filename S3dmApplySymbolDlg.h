#pragma once

#include "s3d_stylemanagerlib_global.h"
#include <QDialog>
#include "ui_S3dmApplySymbolDlg.h"
#include "qgsunittypes.h"
#include "qitemselectionmodel.h"
#include "qgsexpressioncontext.h"


class QgsStyle;
class QgsSymbol;
class QgsVectorLayer;
class QgsStyleProxyModel;

class S3D_STYLEMANAGERLIB_EXPORT S3dmApplySymbolDlg : public QDialog
{
	Q_OBJECT

public:
	enum symbolType
	{
		POINT = 0,
		LINE = 1,
		POLYGON = 2,
		//TODO...�������Ͱ��貹��
	};
public:
	//model�ɴ���ָ�룬Ҳ��ֱ�Ӵӹ�������model(���յ��û��������Ҫ���»���)�������᳢�Խ���ڲ���������Ӷ�ȡ���˲������ⲿʹ��
	S3dmApplySymbolDlg(QgsStyle *style, QgsStyleProxyModel *model, symbolType type, QWidget *parent = Q_NULLPTR);
	~S3dmApplySymbolDlg();

	void setBackGroundColor(QColor &color);   //���÷��ű���ɫ��Ĭ��Ϊ��
	void setStyle(QgsStyle*);				  //���ⲿ�ͷŶ��ͷ�
	void setCurrentSymbolName(QString name);  //��ӽ����ʱ�Ĺ�������
	void setCurrentColor(QColor color);       //��ӽ����ʱ�ķ�����ɫ
	void setCurrentType(symbolType type);     //���ý��浱ǰ��ʾ�ķ�������
	QgsSymbol* getSymbol();					  //�����Ӧ��ʱ��ȡ,�����ⲿ�ͷţ���symbol����������Ӧ�ý����ڵĵ���
	QString getCurrentSymbolName();           //�����Ӧ��ʱ��ȡ,Ϊ��ǰsymbol������
	QColor getCurrentColor();                 //�����Ӧ��ʱ��ȡ,Ϊ��ǰsymbol�ı���ɫ
	QgsSymbol* getSymbolByName(QString name); //��ͨ�����ֻ�ȡstyle���symbol��û���κα���ɫ֮��ĵ���
	QgsSymbol* getSymbolByName(QString name, QColor color); //��ͨ�����ֻ�ȡstyle���symbol��symbol�����������ɫ
private:
	void tagChanged();						  //�ۺ��������ű�ǩ��仯��Ӧ
	void changeToUseColor();                  //�ۺ������Ƿ�ʹ����ɫ
	void openColor();                         //�ۺ���������ɫ������
	void changeToSymbol();					  //�ۺ������޸Ĳ����ı����Ԥ��
	void changeToLayer();				      //�ۺ���������Ӧ����ͼ��
	void filterSymbols(const QString &qword); //�ۺ����������ƹ��˷���
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);  //�ۺ�������ǰѡ����ı���Ӧ
private:
	QgsUnitTypes::RenderUnit matchSizeUnit();
	QgsUnitTypes::RenderUnit matchIntervalUnit();
	void changeComboTags();
	void deleteSimpleLayers();
	void changeSelect();
	QImage getImage(QgsSymbol *symbol, QString &name, QgsExpressionContext *expressionContext);
private:
	Ui::S3dmApplySymbolDlg ui;
	QgsStyle *mStyle;				//�������л�ȡ�������治�ͷţ���������ͷŶ��ͷ�
	symbolType mType;				//ѡ������ʾ��������
	QgsSymbol *mCurrentSymbol;      //�ڲ��޸ģ�����Ӧ����ͼ�㣬Ӧ�ú��ͷ�
	QColor m_backgroundColor;       //���ű���ɫ
	QString m_colorButtonText;

	QgsStyleProxyModel *mModel;     //list�����model
	QgsExpressionContext mPreviewExpressionContext; //����Ԥ�������ͼ�Ĳ���
}; 
