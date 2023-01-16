#include "treecombobox.h"
#include <QHeaderView>
#include <QTreeView>
#include <QLineEdit>

const QString TreeComboBox::m_treeComboBoxDisplayStringSeparator(QStringLiteral( TREECOMBOBOX_EDITLINE_STRING_LITERAL_SEPARATOR ));

TreeComboBox::~TreeComboBox()
{
    QObject::disconnect(this, SIGNAL(currentTextChanged(QString)),
                       this, SLOT(onCurrentTextChanged(QString)));
    if(nullptr != model()) {
        QObject::disconnect(qobject_cast<QStandardItemModel*>(model()), SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)),
                           this, SLOT(onDataChanged(QModelIndex, QModelIndex, QVector<int>)));
    }
}

TreeComboBox::TreeComboBox(QWidget *parent)
    : QComboBox(parent)
    , m_treeComboBoxModelItemNamePrefixDefault(tr(TREECOMBOBOX_MODEL_ITEM_NAME_PREFIX_LITERAL_DEFAULT))
{

    m_treeView = new QTreeView;

    m_treeView->setFrameShape(QFrame::NoFrame);
    m_treeView->setEditTriggers(QTreeView::NoEditTriggers);
    m_treeView->setAlternatingRowColors(false);
    m_treeView->setSelectionBehavior(QTreeView::SelectRows);
    m_treeView->setRootIsDecorated(false);
    m_treeView->setWordWrap(true);
    m_treeView->setAllColumnsShowFocus(true);
    m_treeView->setItemsExpandable(false);
    setView(m_treeView);
    m_treeView->header()->setVisible(false);

    m_treeView->installEventFilter(this);

    setEditable(true);
    QLineEdit* treeComboBoxLineEdit(lineEdit());
    treeComboBoxLineEdit->setReadOnly(true);
    treeComboBoxLineEdit->installEventFilter(this);

    setModel(new QStandardItemModel());

    QObject::connect(this, SIGNAL(currentTextChanged(QString)),
                       this, SLOT(onCurrentTextChanged(QString)));

    installEventFilter(this);
}

Q_SLOT bool TreeComboBox::eventFilter(QObject *targetObject, QEvent* event)
{
    QEvent::Type eventType(event->type());
    if(m_treeView == targetObject) {
        if(QEvent::Leave == eventType) {
            m_tree_view_event = QEvent::Leave;
            m_treeView->setCurrentIndex(m_treeComboBoxModelItemIndexPrevious);
        }
        else if(QEvent::FocusAboutToChange == eventType) {
            if(QEvent::Leave == m_tree_view_event) {
                m_tree_view_event = QEvent::None;
            } else {
                m_tree_view_event = QEvent::KeyPress;
            }
        }
    }
    else if(lineEdit() == targetObject) {
        if(QEvent::MouseButtonRelease == eventType) {
            if(!(m_treeView->hasFocus() || m_treeView->isVisible()))
            {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                Qt::MouseButton buttonId(mouseEvent->button());
                if(Qt::LeftButton == buttonId || Qt::RightButton == buttonId) {
                    showPopup();
                    return true;
                }
            }
        } else if(QEvent::KeyPress == eventType) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            Qt::Key keyId(static_cast<Qt::Key>(keyEvent->key()));
            if(keyId == Qt::Key_Return
                    || keyId == Qt::Key_Enter
                    || keyId == Qt::Key_Space) {
                return true;
            }
        }
    }
    return false;
}

void TreeComboBox::keyPressEvent(QKeyEvent* event)
{
    int keyId(event->key());
    if(Qt::Key_Enter == keyId
            || Qt::Key_Return == keyId
            || Qt::Key_Space == keyId) {
        if(QEvent::KeyPress == m_tree_view_event) {
            m_tree_view_event = QEvent::None;
            event->accept();
            return;
        }
        else if(!(m_treeView->hasFocus() || m_treeView->isVisible())) {
            showPopup();
            event->accept();
            return;
        }
    }
    if(Qt::Key_Escape == keyId) {
        if(QEvent::KeyPress == m_tree_view_event) {
            m_tree_view_event = QEvent::None;
        }
    }
    event->ignore();
}

void TreeComboBox::keyReleaseEvent(QKeyEvent* event)
{
    int keyId(event->key());
    if(Qt::Key_Enter == keyId
            || Qt::Key_Return == keyId
            || Qt::Key_Space == keyId) {
        if(QEvent::KeyPress == m_tree_view_event) {
            m_tree_view_event = QEvent::None;
            event->accept();
            return;
        }
    }
    if(Qt::Key_Escape == keyId) {
        if(QEvent::KeyPress == m_tree_view_event) {
            m_tree_view_event = QEvent::None;
        }
    }
    event->ignore();
}

QString TreeComboBox::itemText(int index) const
{
    QStandardItemModel* treeComboBoxModel(qobject_cast<QStandardItemModel*>(model()));
    QModelIndex modelItemIndex(treeComboBoxModel->index(index, 0, rootModelIndex()));
    return qvariant_cast<QString>(treeComboBoxModel->data(modelItemIndex));
}

bool TreeComboBox::isPopupCurrentItemSelectable(const QModelIndex &itemIndex)
{
    QModelIndex modelItemIndex(itemIndex);
    if(!modelItemIndex.isValid()) modelItemIndex = m_treeView->currentIndex();
    QStandardItemModel* treeComboBoxModel = qobject_cast<QStandardItemModel*>(model());
    QStandardItem* item(treeComboBoxModel->itemFromIndex(modelItemIndex));
    if(nullptr == item) return false;
    return item->isSelectable();
}

void TreeComboBox::getItemNameFullPath(QString& path
                                       , QModelIndex itemIndex
                                       , QStandardItemModel* amodel)
{
    if(nullptr == amodel) amodel = qobject_cast<QStandardItemModel*>(model());
    if(!itemIndex.isValid()) itemIndex = m_treeComboBoxModelItemIndexPrevious;
    if(!itemIndex.isValid()) return;
    path = qvariant_cast<QString>(amodel->data(itemIndex));
    itemIndex = amodel->parent(itemIndex);
    while (itemIndex.isValid()) {
        path.prepend(m_treeComboBoxDisplayStringSeparator);
        path.prepend(qvariant_cast<QString>(amodel->data(itemIndex)));
        itemIndex = amodel->parent(itemIndex);
    }
}

QModelIndex TreeComboBox::getFirstSelectableItemModelIndex(QModelIndex parentIndex
                                                           , QStandardItemModel* amodel)
{
    if(nullptr == amodel) amodel = qobject_cast<QStandardItemModel*>(model());
    QStandardItem* parentItem(nullptr);
    if(parentIndex.isValid()) parentItem = amodel->itemFromIndex(parentIndex);
    else parentItem = amodel->invisibleRootItem();
    if(nullptr == parentItem) return QModelIndex();
    if(parentItem->isSelectable()) return parentIndex;

    QModelIndex itemIndex;
    const int itemRowCount(parentItem->rowCount());
    int i(0);
    for (; i < itemRowCount; ++i) {
        itemIndex = amodel->index(i, 0, parentIndex);
        if(!itemIndex.isValid()) continue;
        itemIndex = getFirstSelectableItemModelIndex(itemIndex, amodel);
        if(!itemIndex.isValid()) continue;
        return itemIndex;
    }
    return QModelIndex();
}

QStandardItem* TreeComboBox::addTreeComboBoxModelItem(const QString &itemName
                                                      , const QModelIndex &parentIndex)
{
    QStandardItemModel* treeComboBoxModel(qobject_cast<QStandardItemModel*>(model()));
    QStandardItem *createdChildItem(nullptr);
    if(0 == treeComboBoxModel->columnCount(parentIndex)) {
        if(!treeComboBoxModel->insertColumn(0, parentIndex)) return createdChildItem;
    }
    int rowIndex = treeComboBoxModel->rowCount(parentIndex);
    if(!treeComboBoxModel->insertRow(rowIndex, parentIndex)) return createdChildItem;

    QModelIndex createdChildItemIndex = treeComboBoxModel->index(rowIndex, 0, parentIndex);
    if(!createdChildItemIndex.isValid()) return createdChildItem;

    createdChildItem = treeComboBoxModel->itemFromIndex(createdChildItemIndex);
    if(nullptr == createdChildItem) return createdChildItem;

    const QString itemNameTrimmed(itemName.trimmed());
    createdChildItem->setText(itemNameTrimmed);
    return createdChildItem;
}

QStandardItem* TreeComboBox::addTreeComboBoxModelItem(const QModelIndex &parentIndex)
{
    QStandardItemModel* treeComboBoxModel(qobject_cast<QStandardItemModel*>(model()));
    QStandardItem *parentItem = treeComboBoxModel->itemFromIndex(parentIndex);
    if(nullptr == parentItem) parentItem = treeComboBoxModel->invisibleRootItem();
    if(nullptr == parentItem) return nullptr;//TODO: handle error of creating item
    QString itemName(parentItem->text());
    if(itemName.isEmpty())
    {
        itemName.append(m_treeComboBoxModelItemNamePrefixDefault).append(m_treeComboBoxSpace);
    }
    else {
        itemName.append(m_treeComboBoxDot);
    }
    itemName.append(QString(QStringLiteral("%0")).arg(parentItem->rowCount() + 1));
    return addTreeComboBoxModelItem(itemName, parentIndex);
}

void TreeComboBox::modelFill()
{
    //TODO: The code below for demonstrative purposes only.
    QStandardItemModel* treeComboBoxModel(qobject_cast<QStandardItemModel*>(model()));
    QStandardItem *item(nullptr);
    QStandardItem *rootItem = treeComboBoxModel->invisibleRootItem();
    QStandardItem *parentItem(rootItem);
    QModelIndex rootIndex = treeComboBoxModel->indexFromItem(rootItem);

    item = addTreeComboBoxModelItem(rootIndex);
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;

    item = addTreeComboBoxModelItem(rootIndex);
    if(nullptr == item) return;
    parentItem = item;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;

    item = addTreeComboBoxModelItem(parentItem->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;

    parentItem = item;
    item = addTreeComboBoxModelItem(parentItem->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(parentItem->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(parentItem->index());
    if(nullptr == item) return;

    item = addTreeComboBoxModelItem(rootIndex);
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;

    parentItem = item;
    item = addTreeComboBoxModelItem(item->index());

    item = addTreeComboBoxModelItem(parentItem->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;

    item = addTreeComboBoxModelItem(rootIndex);
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;

    parentItem = item;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;

    item = addTreeComboBoxModelItem(parentItem->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(parentItem->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(parentItem->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;

    item = addTreeComboBoxModelItem(rootIndex);
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;

    item = addTreeComboBoxModelItem(rootIndex);
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());
    if(nullptr == item) return;
    item = addTreeComboBoxModelItem(item->index());


    expandAll();
    QModelIndex initialIndex(getFirstSelectableItemModelIndex());
    selectIndex(initialIndex);
}

void TreeComboBox::setModel(QAbstractItemModel *amodel)
{
    if(nullptr != model()) {
        QObject::disconnect(qobject_cast<QStandardItemModel*>(model()), SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)),
                           this, SLOT(onDataChanged(QModelIndex, QModelIndex, QVector<int>)));
    }
    QComboBox::setModel(amodel);
    if(nullptr == amodel) return;
    QObject::connect(qobject_cast<QStandardItemModel*>(model()), SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)),
                       this, SLOT(onDataChanged(QModelIndex, QModelIndex, QVector<int>)));
}

inline void TreeComboBox::onModelItemChangedHandler(QStandardItem* item)
{
    if(nullptr == item) return;
    if(item->hasChildren()) {
        if(item->isSelectable()) item->setSelectable(false);
    } else {
        if(!item->isSelectable()) item->setSelectable(true);
    }
    item = item->parent();
    if(nullptr == item) return;
    if(item->isSelectable()) item->setSelectable(false);
    expandAll();
}

Q_SLOT void TreeComboBox::onDataChanged(const QModelIndex &topLeft
                                        , const QModelIndex &bottomRight
                                        , const QVector<int> &roles)
{
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    QStandardItemModel* treeComboBoxModel = static_cast<QStandardItemModel*>(model());
    QStandardItem* item = treeComboBoxModel->itemFromIndex(topLeft);
    if(nullptr == item) return;
    onModelItemChangedHandler(item);
}

Q_SLOT void TreeComboBox::onCurrentTextChanged(const QString &text)
{
    Q_UNUSED(text)
    QLineEdit* treeComboBoxLineEdit(lineEdit());
    QString path;
    getItemNameFullPath(path);
    treeComboBoxLineEdit->blockSignals(true);
    treeComboBoxLineEdit->setText(path);
    treeComboBoxLineEdit->blockSignals(false);
    m_tree_view_event = QEvent::None;
}

void TreeComboBox::hideColumn(int n)
{
    m_treeView->hideColumn(n);
}

void TreeComboBox::expandAll()
{
    m_treeView->expandAll();
}

void TreeComboBox::selectIndex(const QModelIndex &index)
{
    m_treeComboBoxModelItemIndexPrevious = index;
    m_treeView->setCurrentIndex( index );
    setRootModelIndex(index.parent());
    setCurrentIndex(index.row());
}

void TreeComboBox::showPopup()
{
    setRootModelIndex(QModelIndex());
    QComboBox::showPopup();
}

void TreeComboBox::hidePopup()
{
    QStandardItemModel* treeComboBoxModel = qobject_cast<QStandardItemModel*>(model());
    QModelIndex modelItemIndex = m_treeView->currentIndex();
    QStandardItem* item = treeComboBoxModel->itemFromIndex(modelItemIndex);

    bool itemSelectionConfirmedNot(!(nullptr != item && item->isSelectable()));
    if(itemSelectionConfirmedNot) {
        m_tree_view_event = QEvent::None;
        return;
    }

    if(QEvent::Leave == m_tree_view_event) m_tree_view_event = QEvent::None;
    if(itemSelectionConfirmedNot) {
        m_treeView->setCurrentIndex(m_treeComboBoxModelItemIndexPrevious);
        modelItemIndex = m_treeComboBoxModelItemIndexPrevious;
    }

    bool same(m_treeComboBoxModelItemIndexPrevious == modelItemIndex);

    m_treeComboBoxModelItemIndexPrevious = modelItemIndex;
    setRootModelIndex(modelItemIndex.parent());

    QComboBox::hidePopup();

    if(same) emit currentTextChanged(QString());
}
