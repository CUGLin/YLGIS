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
	QgsStyle *getStyle();             //�����ͷ�
	QgsStyleProxyModel *getModel();   //�����ͷ�
private:
	void groupChanged(const QModelIndex &);      //�������棺��ǩ�л���Ӧ
	void grouptreeContextMenu(QPoint point);     //�������棺�Ҽ��˵���Ӧ
	void listItemsContextMenu(QPoint point);	 //�м��б���棺�Ҽ��˵���Ӧ
	void tagSelectedSymbols(bool newTag);        //�м�����Ҽ��˵��������ѡ���ű�ǩ����Ӧ
	void tabItemTypeChanged();                   //tab�����л���Ӧ
	int addTag();								 //�Ҽ��˵�������±�ǩ����Ӧ
	void importSymbol();                         //�Ҽ��˵�������svg/image����Ӧ
	void generateXml();                          //�Ҽ��˵�������xml����Ӧ
	void importSymbols();                        //�Ҽ��˵�������xml����Ӧ
	void importMapGIS();                         //�Ҽ��˵�������MapGIS����Ӧ
	void deleteTag();                            //�Ҽ��˵���ɾ����ǩ����Ӧ
	void editItem();                             //�Ҽ��˵����༭�����Ӧ
	void copyItem();                             //�Ҽ��˵������������Ӧ
	void pasteItem();                            //�Ҽ��˵���ճ�������Ӧ
	void selectItem();                           //�Ҽ��˵���ѡ�������Ӧ
	void exportItemToSVG();                      //�Ҽ��˵������������Ӧ
	void exportItemToImage();                    //�Ҽ��˵������������Ӧ
	void deleteItem();                           //�Ҽ��˵���ɾ�������Ӧ
	void filterSymbols(const QString &qword);    //�������л���Ӧ
private:
	void populateGroups();						 //������������ڵ�
	QList<ItemDetails> selectedItems();          //��ȡitems����
	bool deleteSymbols(QgsStyle::StyleEntity type, QgsStyle *);
private:
	Ui::S3dmSymbolManager ui;
	QgsStyle *mStyle = nullptr;
	QgsStyleProxyModel *mModel = nullptr;		 //���н����model

	QMenu *mGroupTreeContextMenu = nullptr;		 //���������Ҳ˵���menu
	QMenu *mGroupTreeFatherMenu = nullptr;       //�������������Ҳ˵�menu
	QAction *mAddTag = nullptr;					 //�����˵����¼�����ӱ�ǩ
	QAction *mImport = nullptr;					 //�����˵����¼�������svg/image
	QAction *mImportXml = nullptr;				 //�����˵����¼�������xml
	QAction *mGenerateXml = nullptr;             //�����˵����¼�������xml
	QAction *mImportMapGIS = nullptr;            //�����˵����¼�������MapGIS�⣨�ڲ�ת����
	QAction *mDeleteTag = nullptr;				 //�����˵����¼���ɾ����ǩ

	QMenu *mGroupListMenu = nullptr;			 //�м��б�����menu
	QMenu *mGroupListContextMenu = nullptr;		 //�м��б����menu�е�menu����ӵ���ǩ��
	QAction *mActionEditItem = nullptr;			 //�༭�¼���Ӧ
	QAction *mActionCopyItem = nullptr;			 //�����¼���Ӧ
	QAction *mActionPasteItem = nullptr;		 //ճ���¼���Ӧ
	QAction *mActionAddToTag = nullptr;			 //��ӵ���ǩ�¼���Ӧ
	QAction *mActionExportSVG = nullptr;		 //�����¼���Ӧ
	QAction *mActionExportImage = nullptr;		 //�����¼���Ӧ
	QAction *mActionDelete = nullptr;			 //�Ƴ��¼���Ӧ
};

