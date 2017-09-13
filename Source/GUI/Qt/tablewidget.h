/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
*
*  Use of this source code is governed by a MIT-style license that can
*  be found in the License.html file in the root of the source tree.
*/

#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QItemDelegate>
#include <QTableWidget>
#include <QHeaderView>
#include <QValidator>
#include <QLineEdit>
#include <QComboBox>

#include "core.h"

//***************************************************************************
// Helpers
//***************************************************************************

enum COLUMNS
{
    FILE_COLUMN = 0,
    OK_COLUMN = 1,
    REGISTRY_COLUMN = 2,
    VALUE_COLUMN = 3
};

//---------------------------------------------------------------------------
class AdIdValidator : public QValidator
{
    Q_OBJECT
public:
    AdIdValidator(QObject* Parent = 0);
    QValidator::State validate(QString& Input, int& Pos) const;
};

//---------------------------------------------------------------------------
class OtherValidator : public QValidator
{
    Q_OBJECT
public:
    OtherValidator(QObject* Parent = 0);
    QValidator::State validate(QString& Input, int& Pos) const;
};

//---------------------------------------------------------------------------
class ComboBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ComboBoxDelegate(QObject* Parent = 0, Core* C = 0);

protected:
    QWidget* createEditor(QWidget* Parent,
                          const QStyleOptionViewItem& Option,
                          const QModelIndex& Index) const;

    void updateEditorGeometry(QWidget* Editor,
                              const QStyleOptionViewItem& Option,
                              const QModelIndex& Index) const;

    void setEditorData(QWidget* Editor, const QModelIndex& Index) const;
    void setModelData(QWidget* Editor, QAbstractItemModel* Model, const QModelIndex& Index) const;

private:
    Core* C;
};

//---------------------------------------------------------------------------
class ItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
     ItemDelegate(QObject* Parent = 0, Core* C = 0);

protected:
     QWidget* createEditor(QWidget* Parent,
                           const QStyleOptionViewItem& Option,
                           const QModelIndex& Index) const;

     void updateEditorGeometry(QWidget* Editor,
                               const QStyleOptionViewItem& Option,
                               const QModelIndex& Index) const;

     void setEditorData(QWidget* Editor, const QModelIndex& Index) const;
     void setModelData(QWidget* Editor, QAbstractItemModel* Model, const QModelIndex& Index) const;

private:
    Core* C;

signals:
    void Value_Changed(int Row) const;
};

//***************************************************************************
// TableWidget
//***************************************************************************

class TableWidget : public QTableWidget
{
    Q_OBJECT

public:
    TableWidget(QWidget* Parent);

    void Setup(Core* C);
    void Set_Valid(int Row, bool Valid = true);
    void Set_Modified(int Row, bool Modified = true);
    void Update_Table();

    void resizeEvent(QResizeEvent* Event);

protected slots:
    void On_Value_Changed(int Row);

private:
    Core* C;
};

#endif // TABLEWIDGET_H
