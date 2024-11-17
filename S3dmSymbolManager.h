#pragma once
#include "ui_S3dmSymbolManager.h"
#include "s3d_stylemanagerlib_global.h"
#include "qmenu.h"
#include "qgsstylemodel.h"
#include "qgsstyle.h"


class S3D_STYLEMANAGERLIB_EXPORT S3dmSymbolManager: public QWidget
{
	Q_OBJECT

public:
	S3dmSymbolManager(QWidget *parent = Q_NULLPTR);
	~S3dmSymbolManager();
public:
	struct ItemDetails
	{
		QgsStyle::StyleEntity entityType;
		QgsSymbol::SymbolType symbolType;
		QString name;
		bool isFavorite;
	};
public:
	QgsStyle *getStyle();             //不能释放
	QgsStyleProxyModel *getModel();   //不能释放
private:
	void groupChanged(const QModelIndex &);      //左树界面：标签切换响应
	void grouptreeContextMenu(QPoint point);     //左树界面：右键菜单响应
	void listItemsContextMenu(QPoint point);	 //中间列表界面：右键菜单响应
	void tagSelectedSymbols(bool newTag);        //中间界面右键菜单中添加所选符号标签的响应
	void tabItemTypeChanged();                   //tab类型切换响应
	int addTag();								 //右键菜单：添加新标签的响应
	void importSymbol();                         //右键菜单：导入svg/image的响应
	void generateXml();                          //右键菜单：生成xml的响应
	void importSymbols();                        //右键菜单：导入xml的响应
	void importMapGIS();                         //右键菜单：导入MapGIS的响应
	void deleteTag();                            //右键菜单：删除标签的响应
	void editItem();                             //右键菜单：编辑项的响应
	void copyItem();                             //右键菜单：复制项的响应
	void pasteItem();                            //右键菜单：粘贴项的响应
	void selectItem();                           //右键菜单：选择项的响应
	void exportItemToSVG();                      //右键菜单：导出项的响应
	void exportItemToImage();                    //右键菜单：导出项的响应
	void deleteItem();                           //右键菜单：删除项的响应
	void filterSymbols(const QString &qword);    //过滤器切换响应
private:
	void populateGroups();						 //构造左界面树节点
	QList<ItemDetails> selectedItems();          //获取items内容
	bool deleteSymbols(QgsStyle::StyleEntity type, QgsStyle *);
private:
	Ui::S3dmSymbolManager ui;
	QgsStyle *mStyle = nullptr;
	QgsStyleProxyModel *mModel = nullptr;		 //左、中界面的model

	QMenu *mGroupTreeContextMenu = nullptr;		 //左树界面右菜单的menu
	QMenu *mGroupTreeFatherMenu = nullptr;       //左树界面大组的右菜单menu
	QAction *mAddTag = nullptr;					 //左界面菜单的事件：添加标签
	QAction *mImport = nullptr;					 //左界面菜单的事件：导入svg/image
	QAction *mImportXml = nullptr;				 //左界面菜单的事件：导入xml
	QAction *mGenerateXml = nullptr;             //左界面菜单的事件：生成xml
	QAction *mImportMapGIS = nullptr;            //左界面菜单的事件：导入MapGIS库（内部转换）
	QAction *mDeleteTag = nullptr;				 //左界面菜单的事件：删除标签

	QMenu *mGroupListMenu = nullptr;			 //中间列表界面的menu
	QMenu *mGroupListContextMenu = nullptr;		 //中间列表界面menu中的menu（添加到标签）
	QAction *mActionEditItem = nullptr;			 //编辑事件响应
	QAction *mActionCopyItem = nullptr;			 //复制事件响应
	QAction *mActionPasteItem = nullptr;		 //粘贴事件响应
	QAction *mActionAddToTag = nullptr;			 //添加到标签事件响应
	QAction *mActionExportSVG = nullptr;		 //导出事件响应
	QAction *mActionExportImage = nullptr;		 //导出事件响应
	QAction *mActionDelete = nullptr;			 //移除事件响应
};

