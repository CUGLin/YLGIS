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
		//TODO...其他类型按需补充
	};
public:
	//model可传空指针，也可直接从管理器拿model(传空调用会很慢，需要重新缓存)，后续会尝试解决内部缓存问题从而取消此参数在外部使用
	S3dmApplySymbolDlg(QgsStyle *style, QgsStyleProxyModel *model, symbolType type, QWidget *parent = Q_NULLPTR);
	~S3dmApplySymbolDlg();

	void setBackGroundColor(QColor &color);   //设置符号背景色，默认为空
	void setStyle(QgsStyle*);				  //随外部释放而释放
	void setCurrentSymbolName(QString name);  //添加界面打开时的过滤名称
	void setCurrentColor(QColor color);       //添加界面打开时的符号颜色
	void setCurrentType(symbolType type);     //设置界面当前显示的符号类型
	QgsSymbol* getSymbol();					  //当点击应用时获取,需在外部释放，此symbol包含所有在应用界面内的调整
	QString getCurrentSymbolName();           //当点击应用时获取,为当前symbol的名字
	QColor getCurrentColor();                 //当点击应用时获取,为当前symbol的背景色
	QgsSymbol* getSymbolByName(QString name); //仅通过名字获取style里的symbol，没有任何背景色之类的调整
	QgsSymbol* getSymbolByName(QString name, QColor color); //仅通过名字获取style里的symbol，symbol背景将添加颜色
private:
	void tagChanged();						  //槽函数：符号标签组变化响应
	void changeToUseColor();                  //槽函数：是否使用颜色
	void openColor();                         //槽函数：打开颜色管理器
	void changeToSymbol();					  //槽函数：修改参数改变符号预览
	void changeToLayer();				      //槽函数：符号应用于图层
	void filterSymbols(const QString &qword); //槽函数：按名称过滤符号
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);  //槽函数：当前选择项改变响应
private:
	QgsUnitTypes::RenderUnit matchSizeUnit();
	QgsUnitTypes::RenderUnit matchIntervalUnit();
	void changeComboTags();
	void deleteSimpleLayers();
	void changeSelect();
	QImage getImage(QgsSymbol *symbol, QString &name, QgsExpressionContext *expressionContext);
private:
	Ui::S3dmApplySymbolDlg ui;
	QgsStyle *mStyle;				//管理器中获取，本界面不释放，随管理器释放而释放
	symbolType mType;				//选择器显示符号类型
	QgsSymbol *mCurrentSymbol;      //内部修改，用于应用于图层，应用后释放
	QColor m_backgroundColor;       //符号背景色
	QString m_colorButtonText;

	QgsStyleProxyModel *mModel;     //list界面的model
	QgsExpressionContext mPreviewExpressionContext; //用于预览界面绘图的参数
}; 
