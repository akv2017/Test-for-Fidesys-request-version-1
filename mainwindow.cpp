#include "mainwindow.h"
#include <QLayout>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_treeComboBox(nullptr)
{
    QFont appFont(font());
    appFont.setPointSize(APP_FONT_POINT_SIZE);
    setFont(appFont);
    QWidget* mainWindowCentralWidget = new QWidget(this);
    mainWindowCentralWidget->setFont(appFont);
    setCentralWidget(mainWindowCentralWidget);
    mainWindowCentralWidget->setMinimumSize(600, 100);

    m_treeComboBox = new TreeComboBox(mainWindowCentralWidget);
    m_treeComboBox->setFont(appFont);

    auto mainWindowCentralWidgetVerticalLayout = new QVBoxLayout;
    mainWindowCentralWidget->setLayout(mainWindowCentralWidgetVerticalLayout);
    mainWindowCentralWidgetVerticalLayout->addWidget(m_treeComboBox);
    mainWindowCentralWidgetVerticalLayout->addStretch();

    installEventFilter(this);
}


