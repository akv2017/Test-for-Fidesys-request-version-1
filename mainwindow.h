#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "treecombobox.h"
#include <QMainWindow>

#define APP_FONT_POINT_SIZE  14

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void modelFill(){m_treeComboBox->modelFill();}

protected:

private:
    TreeComboBox* m_treeComboBox;

};
#endif // MAINWINDOW_H
