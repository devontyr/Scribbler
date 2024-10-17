#include "mainwindow.h"
#include <QtWidgets>
#include "scribbler.h"
#include <math.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    setWindowTitle("Scribbler");

    QWidget *central = new QWidget;
    setCentralWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    scribbler = new Scribbler;
    mainLayout->addWidget(scribbler, 1);
    tabs = new QTabWidget;
    mainLayout->addWidget(tabs, 1);
    tabs->setVisible(false); //initially hidden


    // MENU BAR
    QAction *openFileAct = new QAction("Open File");
    connect(openFileAct, &QAction::triggered, this, &MainWindow::openFileSlot);
    openFileAct->setShortcut(Qt::CTRL | Qt::Key_O); //slot triggered with ctrl O key

    QAction *resetAct = new QAction("Reset Scribble");
    connect(resetAct, &QAction::triggered, scribbler, &Scribbler::resetDrawingSlot);
    connect(resetAct, &QAction::triggered, this, &MainWindow::resetSlot);
    resetAct->setShortcut(Qt::CTRL | Qt::Key_R); //slot triggered with ctrl R

    QAction *saveFileAct = new QAction("Save File");
    connect(saveFileAct, &QAction::triggered, this, &MainWindow::saveFileSlot);
    saveFileAct->setShortcut(Qt::CTRL | Qt::Key_S); //slot triggered with ctrl S

    QMenu *fileMenu = new QMenu("&File");
    fileMenu->addAction(openFileAct);
    fileMenu->addAction(resetAct);
    fileMenu->addAction(saveFileAct);
    menuBar()->addMenu(fileMenu);

    QAction *startCaptureAct = new QAction("Start Capture");
    connect(startCaptureAct, &QAction::triggered, scribbler, &Scribbler::startCaptureSlot);
    startCaptureAct->setShortcut(Qt::CTRL | Qt::Key_C); //slot triggered with ctrl C key

    QAction *endCaptureAct = new QAction("End Capture");
    connect(endCaptureAct, &QAction::triggered, scribbler, &Scribbler::endCaptureSlot);
    endCaptureAct->setShortcut(Qt::CTRL | Qt::Key_E); //slot triggered with ctrl E key

    connect(scribbler, &Scribbler::sendCapture, this, &MainWindow::displayCaptureSlot);

    QMenu *captureMenu = new QMenu("&Capture");
    captureMenu->addAction(startCaptureAct);
    captureMenu->addAction(endCaptureAct);
    menuBar()->addMenu(captureMenu);


    QAction *lineSegmentsAct = new QAction("Line Segments Shown");
    connect(lineSegmentsAct, &QAction::triggered, scribbler, &Scribbler::showLineSegmentsSlot);
    lineSegmentsAct->setShortcut(Qt::CTRL | Qt::Key_L); //slot triggered with ctrl L key

    QAction *dotsOnlyAct = new QAction("Dots Only Shown");
    connect(dotsOnlyAct, &QAction::triggered, scribbler, &Scribbler::showDotsOnlySlot);
    dotsOnlyAct->setShortcut(Qt::CTRL | Qt::Key_D); //slot triggered with ctrl D key

    QMenu *viewMenu = new QMenu("&View");
    viewMenu->addAction(lineSegmentsAct);
    viewMenu->addAction(dotsOnlyAct);
    menuBar()->addMenu(viewMenu);


    //Save Last Directory
    QSettings settings("DRT", "scribbler");
    lastDir = settings.value("lastDirectory", "").toString();
}

MainWindow::~MainWindow() {
    QSettings settings("DRT", "scribbler");
    settings.value("lastDirectory", lastDir);
}

void MainWindow::createTab(int captureNum) {
    QList<MouseEvent> &events = importedEvents[captureNum]; //prevents from being copied

    // adds a tab to tabs with a QTableWidget with a row for each MouseEvent
    int nRows = events.size(); int nCols = 4;
    QTableWidget *currDataTable = new QTableWidget(nRows, nCols);
    currDataTable->setHorizontalHeaderLabels(QStringList() << "Action Type" << "Position" << "Time" << "Points Dist");
    tabs->addTab(currDataTable, QString::number(captureNum));

    //load data into the rows
    for (int iRow = 0; iRow < nRows; ++iRow) {
        QString action = "";
        MouseEvent &evt = events[iRow];

        if (evt.action == 0) action = "Press";
        if (evt.action == 1) action = "Move";
        if (evt.action == 2) action = "Release";

        QTableWidgetItem *actionItem = new QTableWidgetItem(action);
        currDataTable->setItem(iRow, 0, actionItem);

        QTableWidgetItem *posItem = new QTableWidgetItem(QString("(%1, %2)").arg(evt.pos.x()).arg(evt.pos.y()));
        currDataTable->setItem(iRow, 1, posItem);

        QTableWidgetItem *timeItem = new QTableWidgetItem(QString::number(evt.time));
        currDataTable->setItem(iRow, 2, timeItem);

        if (iRow > 0 & evt.action != 0) {
            QTableWidgetItem *distanceItem = new QTableWidgetItem(QString("%1").arg(QLineF(evt.pos, events[iRow-1].pos).length()));
            currDataTable->setItem(iRow, 3, distanceItem);
        }
    }
}

void MainWindow::displayCaptureSlot(QList<MouseEvent> events) {
    importedEvents << events;

    int lastElement = importedEvents.size()-1;
    createTab(lastElement);

    tabs->setVisible(true);
}

void MainWindow::resetSlot() {
    tabs->clear();
    tabs->setVisible(false);
}

void MainWindow::saveFileSlot() {
    //prompt for an output file name, and save data there
    QString outName = QFileDialog::getSaveFileName(this, "select file to save to", lastDir);
    if (outName.isEmpty()) return;

    QFile outFile(outName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(this, "error", QString("Can't write to file \"%1\"").arg(outName));
        return;
    }
    lastDir = QFileInfo(outFile).absolutePath();

    QDataStream out(&outFile);
    out << importedEvents;
}

void MainWindow::openFileSlot() {
    //load the encoded binary file
    QString inName = QFileDialog::getOpenFileName(this, "select file you want to load", lastDir);
    if (inName.isEmpty()) return;
    QFile inFile(inName);

    //exit if the file doesn't open
    if (!inFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "file does not open", QString("Can't open file \"%1\"").arg(inName));
        return;
    }
    lastDir = QFileInfo(inFile).absolutePath();

    QDataStream in(&inFile);
    in >> importedEvents;

    for (int iTab=0; iTab<importedEvents.size(); ++iTab) {
        createTab(iTab);
        scribbler->drawMouseEvents(importedEvents[iTab]);
    }
}

// store a pointer to a line and dot (null if not used)
// when you draw, in the mouseevent record the dot and line
// then main window has pointers and can change opacity

// store in seperate lists, the graphicsitems of dots and lines
// capturedDots // capturedLines -- qL<qL<GItems>
// master of all drawn lines and dots (pointers)

// each qList is for each tab -- highlight wanted one

//
