#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <scribbler.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QString lastDir;
    QTabWidget *tabs;
    int tabNum;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void displayCaptureSlot(QList<MouseEvent>);
    void resetSlot();
    void saveFileSlot();
    void openFileSlot();
};

#endif // MAINWINDOW_H
