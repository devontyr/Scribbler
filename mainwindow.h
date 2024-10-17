#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <scribbler.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Scribbler *scribbler;

    QString lastDir;
    QTabWidget *tabs;

    QList<QList<MouseEvent>> importedEvents;

    void createTab(int captureNum);
    void pointsDistance(QPointF p1, QPointF p2);

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
