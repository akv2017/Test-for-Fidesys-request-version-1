#ifndef TREECOMBOBOX_H
#define TREECOMBOBOX_H

#include <QtWidgets/QComboBox>
#include <QtWidgets/QTreeView>
#include <QString>
#include <QStandardItemModel>
#include <QPersistentModelIndex>
#include <QMouseEvent>
#include <QKeyEvent>

#define TREECOMBOBOX_EDITLINE_STRING_LITERAL_SEPARATOR " - "
#define TREECOMBOBOX_LITERAL_SPACE " "
#define TREECOMBOBOX_LITERAL_DOT "."
#define TREECOMBOBOX_MODEL_ITEM_NAME_PREFIX_LITERAL_DEFAULT "Item"

class TreeComboBox : public QComboBox
{
    Q_OBJECT
public:
    TreeComboBox(QWidget *parent = nullptr);
    virtual ~TreeComboBox() override;

    void showPopup() override;
    void hidePopup() override;

    void hideColumn(int n);
    void expandAll();
    void selectIndex(const QModelIndex &index);
    QString itemText(int index) const;

    bool isPopupCurrentItemSelectable(const QModelIndex &itemIndex = QModelIndex());
    void getItemNameFullPath(QString& path
                                    , QModelIndex itemIndex = QModelIndex()
            , QStandardItemModel* amodel = nullptr);

    void setModel(QAbstractItemModel* amodel);
    void modelFill();
    QStandardItem* addTreeComboBoxModelItem(const QString &itemName
                                            , const QModelIndex &parentIndex = QModelIndex());
    QStandardItem* addTreeComboBoxModelItem(const QModelIndex &parentIndex = QModelIndex());

public Q_SLOTS:
    bool eventFilter(QObject *targetObject, QEvent* event) override;
    void onCurrentTextChanged(const QString &text);
    void onDataChanged(const QModelIndex &topLeft
                       , const QModelIndex &bottomRight
                       , const QVector<int> &roles);

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;

    inline void onModelItemChangedHandler(QStandardItem* item);
    QModelIndex getFirstSelectableItemModelIndex(QModelIndex parentIndex = QModelIndex()
            , QStandardItemModel* amodel = nullptr);

private:
    QTreeView *m_treeView = nullptr;
    QEvent::Type m_tree_view_event = QEvent::None;
    QPersistentModelIndex m_treeComboBoxModelItemIndexPrevious;
    const QString m_treeComboBoxDot = QStringLiteral(TREECOMBOBOX_LITERAL_DOT);
    const QString m_treeComboBoxSpace = QStringLiteral(TREECOMBOBOX_LITERAL_SPACE);
    QString m_treeComboBoxModelItemNamePrefixDefault;
    static const QString m_treeComboBoxDisplayStringSeparator;
};

#endif // TREECOMBOBOX_H
