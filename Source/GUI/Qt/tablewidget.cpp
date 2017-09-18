/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
*
*  Use of this source code is governed by a MIT-style license that can
*  be found in the License.html file in the root of the source tree.
*/

#include <QResizeEvent>
#include <QScrollBar>
#include <QFont>
#include <QDebug>

#include "tablewidget.h"

//***************************************************************************
// Info
//***************************************************************************

static const int ColumnSize[4] =
{
    55,
     5,
    20,
    20,
};

//***************************************************************************
// Helpers
//***************************************************************************


//---------------------------------------------------------------------------
AdIdValidator::AdIdValidator(QObject* Parent) : QValidator(Parent)
{
}

//---------------------------------------------------------------------------
QValidator::State AdIdValidator::validate(QString& Input, int& Pos) const
{
    Q_UNUSED(Pos)

    if(Input.size() > 12)
        return QValidator::Invalid;

    QString Acceptable = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    for(int Pos = 0; Pos < Input.size(); Pos++)
    {
        if (Input[Pos] >= 'a' && Input[Pos] <= 'z')
            Input[Pos] = Input[Pos].toUpper();

        if(!Acceptable.contains(Input.at(Pos)))
            return QValidator::Invalid;
    }

    if(Input.size() < 11)
        return QValidator::Intermediate;

    return QValidator::Acceptable;
}

//---------------------------------------------------------------------------
OtherValidator::OtherValidator(QObject* Parent) : QValidator(Parent)
{
}

//---------------------------------------------------------------------------
QValidator::State OtherValidator::validate(QString& Input, int& Pos) const
{
    Q_UNUSED(Pos)

    if(Input.size() > 255)
        return QValidator::Invalid;

    return QValidator::Acceptable;
}

//---------------------------------------------------------------------------
ComboBoxDelegate::ComboBoxDelegate(QObject* Parent, Core* C) : QItemDelegate(Parent), C(C)
{
}

//---------------------------------------------------------------------------
QWidget* ComboBoxDelegate::createEditor(QWidget* Parent,
                                        const QStyleOptionViewItem& Option,
                                        const QModelIndex& Index) const
{
    Q_UNUSED(Option)
    Q_UNUSED(Index)

    QComboBox* ComboBox = new QComboBox(Parent);

    ComboBox->setEditable(true);
    ComboBox->setInsertPolicy(QComboBox::InsertAtBottom);

    return ComboBox;
}

//---------------------------------------------------------------------------
void ComboBoxDelegate::updateEditorGeometry(QWidget* Editor,
                                            const QStyleOptionViewItem& Option,
                                            const QModelIndex& Index) const
{
    Q_UNUSED(Index)

    Editor->setGeometry(Option.rect);
}

//---------------------------------------------------------------------------
void ComboBoxDelegate::setEditorData(QWidget* Editor, const QModelIndex& Index) const
{
    if(!C)
        return;

    QComboBox* ComboBox = qobject_cast<QComboBox*>(Editor);

    QString FileName = Index.sibling(Index.row(), FILE_COLUMN).data().toString();

    MetaDataList* MetaData = C->Get_MetaData(FileName);

    for(MetaDataList::const_iterator It = MetaData->begin(); It != MetaData->end(); It++)
    {
        QString Registry = It.key();
        QString Value = It.value();

        ComboBox->addItem(Registry, Value);
    }
}

//---------------------------------------------------------------------------
void ComboBoxDelegate::setModelData(QWidget* Editor,
                                    QAbstractItemModel* Model,
                                    const QModelIndex& Index) const
{
    if(!C)
        return;

    QComboBox* ComboBox = qobject_cast<QComboBox*>(Editor);

    QString FileName = Index.sibling(Index.row(), FILE_COLUMN).data().toString();
    QString Registry = ComboBox->currentText();

    MetaDataList* MetaData = C->Get_MetaData(FileName);

    QString Value = (*MetaData)[Registry];

    Model->setData(Index, Registry);
    Model->setData(Index.sibling(Index.row(), VALUE_COLUMN), Value);
    (*C->Get_Files())[FileName].CurrentRegistry = Registry;
}

//---------------------------------------------------------------------------
ItemDelegate::ItemDelegate(QObject* Parent, Core* C) : QItemDelegate(Parent), C(C)
{
}

//---------------------------------------------------------------------------
QWidget* ItemDelegate::createEditor(QWidget* Parent,
                                    const QStyleOptionViewItem& Option,
                                    const QModelIndex& Index) const
{
    Q_UNUSED(Option)

    QLineEdit *Editor = new QLineEdit(Parent);

    if(Index.sibling(Index.row(), REGISTRY_COLUMN).data(Qt::EditRole).toString() == "ad-id.org")
        Editor->setValidator(new AdIdValidator);
    else
        Editor->setValidator(new OtherValidator);

    return Editor;
}

//---------------------------------------------------------------------------
void ItemDelegate::updateEditorGeometry(QWidget* Editor,
                                        const QStyleOptionViewItem& Option,
                                        const QModelIndex& Index) const
{
    Q_UNUSED(Index)

    Editor->setGeometry(Option.rect);
}


//---------------------------------------------------------------------------
void ItemDelegate::setEditorData(QWidget *Editor, const QModelIndex& Index) const
{
    qobject_cast<QLineEdit*>(Editor)->setText(Index.data(Qt::EditRole).toString());
}

//---------------------------------------------------------------------------
void ItemDelegate::setModelData(QWidget* Editor,
                                QAbstractItemModel* Model,
                                const QModelIndex& Index) const
{
    if(!C)
        return;

    QLineEdit* LineEditor = qobject_cast<QLineEdit*>(Editor);

    QString OldValue = Model->data(Index, Qt::EditRole).toString();
    QString Value = LineEditor->text();

    if(Value != OldValue)
    {
        QString FileName = Index.sibling(Index.row(), FILE_COLUMN).data(Qt::EditRole).toString();
        QString Registry = Index.sibling(Index.row(), REGISTRY_COLUMN).data(Qt::EditRole).toString();
        MetaDataList* MetaData = C->Get_MetaData(FileName);

        if(!MetaData)
            return;

        MetaData->insert(Registry, Value);
        Model->setData(Index, Value);

        (*C->Get_Files())[FileName].Modified = true;
        emit Value_Changed(Index.row());
    }
}

//***************************************************************************
// TableWidget
//***************************************************************************

//---------------------------------------------------------------------------
TableWidget::TableWidget(QWidget* Parent) : QTableWidget(Parent)
{
}

//---------------------------------------------------------------------------
void TableWidget::Setup(Core *C)
{
    this->C = C;

    // Setup table widget
    QStringList Header_Labels;

    Header_Labels.append("File Name");
    Header_Labels.append("OK?");
    Header_Labels.append("Registry");
    Header_Labels.append("Value");
    setColumnCount(4);
    setHorizontalHeaderLabels(Header_Labels);
#if QT_VERSION < 0x050000
    horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#else
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#endif

    ItemDelegate* ItemEditor = new ItemDelegate(NULL, C);
    connect(ItemEditor, SIGNAL(Value_Changed(int)), this, SLOT(On_Value_Changed(int)));

    setItemDelegateForColumn(2, qobject_cast<QAbstractItemDelegate*>(new ComboBoxDelegate(NULL, C)));
    setItemDelegateForColumn(3, qobject_cast<QAbstractItemDelegate*>(ItemEditor));
}

//---------------------------------------------------------------------------
void TableWidget::Set_Modified(int Row, bool Modified)
{
    if(Row > this->rowCount())
        return;

    for(int Col = 0; Col < this->columnCount(); Col++)
    {
        QFont Font = this->item(Row, Col)->font();
        Font.setBold(Modified);
        this->item(Row, Col)->setFont(Font);
    }
}

//---------------------------------------------------------------------------
void TableWidget::Set_Valid(int Row, bool Valid)
{
    if (Valid || Row > this->rowCount())
        return;

    for (int Col = 0; Col < this->columnCount(); Col++)
    {
        Qt::ItemFlags Flags = this->item(Row, Col)->flags();
        if(Flags.testFlag(Qt::ItemIsEnabled))
            Flags &= ~Qt::ItemIsEnabled;
        if(Flags.testFlag(Qt::ItemIsSelectable))
            Flags &= ~Qt::ItemIsSelectable;
        this->item(Row, Col)->setFlags(Flags);
    }
}

//---------------------------------------------------------------------------
void TableWidget::Update_Table()
{
    size_t Valid = 0;
    bool Modified = false;
    
    //Get opened files
    FileList* Files = C->Get_Files();

    //Get displayed entries
    QStringList Entries;
    for(int Row = rowCount() - 1; Row >= 0; Row--)
    {
        QString Entry = item(Row, 0)->data(Qt::DisplayRole).toString();

        if(Files->find(Entry) == Files->end())
        {
            //Remove deleted entry
            removeRow(Row);
            continue;
        }

        Entries.append(Entry);
        //Display modified entries in bold
        Set_Valid(Row, Files->value(Entry).Valid);
        Set_Modified(Row, Files->value(Entry).Modified);

        if(Files->value(Entry).Modified)
            Modified = true;

        if (Files->value(Entry).Valid)
            Valid++;
    }

    //Display new files
    for(FileList::iterator It = Files->begin(); It != Files->end(); It++)
    {
        if(!Entries.contains(It.key()))
        {
            QTableWidgetItem* Name = new QTableWidgetItem(It.key());
            Name->setFlags(Name->flags() ^ Qt::ItemIsEditable);
            QTableWidgetItem* OK = new QTableWidgetItem(It->Valid ? "Yes" : QString ("No: ")+It->H->PerFile_Error.str().c_str());
            OK->setFlags(OK->flags() ^ Qt::ItemIsEditable);
            if (!It->Valid)
                OK->setToolTip(It->H->PerFile_Error.str().c_str());
            QTableWidgetItem* Registry = new QTableWidgetItem(It->CurrentRegistry);
            if (It->Valid)
                Registry->setToolTip("Double-click for editing the Universal Ad-ID registry of this file.");
            QTableWidgetItem* Value = new QTableWidgetItem(It->MetaData[It->CurrentRegistry]);
            if (It->Valid)
                Value->setToolTip("Double-click for editing the Universal Ad-ID value of this file.\nA-Z 0-9 only.");

            insertRow(rowCount());
            setItem(rowCount() - 1, 0, Name);
            setItem(rowCount() - 1, 1, OK);
            setItem(rowCount() - 1, 2, Registry);
            setItem(rowCount() - 1, 3, Value);

            if(It->Modified)
                Modified = true;

            if (It->Valid)
            {
                Set_Modified(rowCount() - 1, It->Modified);
                Valid++;
            }
            else
                Set_Valid(rowCount() - 1, false);
        }
    }

    if (Valid)
        setStatusTip(QString::number(Valid)+ " editable files found, double-click on Registry or Value cells for editing then save");
    else
        setStatusTip("Drag and drop some MOV/MP4 files");

    emit Enable_Save(Modified);
}

//---------------------------------------------------------------------------
void TableWidget::resizeEvent(QResizeEvent* Event)
{
    //Do nothing if columns size exceed aviable space
    if(!horizontalScrollBar()->isVisible())
    {
        qreal Total_New = Event->size().width();
        setColumnWidth(0, Total_New * ColumnSize[0] / 100);
        setColumnWidth(1, Total_New * ColumnSize[1] / 100);
        setColumnWidth(2, Total_New * ColumnSize[2] / 100);
        setColumnWidth(3, Total_New * ColumnSize[3] / 100);
    }
    //Call base resizeEvent to handle the vertical resizing
    QTableView::resizeEvent(Event);
}

//---------------------------------------------------------------------------
void TableWidget::On_Value_Changed(int Row)
{
    Set_Modified(Row, true);
    emit Enable_Save(true);
}
