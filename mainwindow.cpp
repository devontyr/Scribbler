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

    connect(tabs, &QTabWidget::currentChanged, this, &MainWindow::fadeTabSlot);

    //Save Last Directory
    QSettings settings("DRT", "scribbler");
    lastDir = settings.value("lastDirectory", "").toString();
}

MainWindow::~MainWindow() {
    QSettings settings("DRT", "scribbler");
    settings.setValue("lastDirectory", lastDir);
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

    connect(currDataTable, &QTableWidget::currentCellChanged, this, &MainWindow::highlightRowsSlot);
}

void MainWindow::clearColors() {
    //loop through all events to turn black
    for (int iLine = 0; iLine < highlightedLines.size(); ++iLine) {
        highlightedLines[iLine]->setPen(QPen(Qt::black, 4.0, Qt::SolidLine, Qt::FlatCap));
    }
    for (int iDot = 0; iDot < highlightedDots.size(); ++iDot) {
        highlightedDots[iDot]->setBrush(QBrush(Qt::black));
    }
}

void MainWindow::highlightRowsSlot() {
    clearColors();
    int selectedTab = tabs->currentIndex();
    QTableWidget* curTable = (QTableWidget*) tabs->currentWidget();
    QList<QTableWidgetSelectionRange> range = curTable->selectedRanges();
    //loop through each rectangle and do a top to bottom
    for (int iList = 0; iList < range.size(); ++iList) {
        int topRow = range[iList].topRow();
        int bottomRow = range[iList].bottomRow();
        //for each selection range, loop through selected items in the table
        for (int iRow = topRow; iRow < bottomRow; ++iRow) {
            //use X row to index into the QList<MouseEvent>
            if (importedEvents[selectedTab][iRow].dot) {
                importedEvents[selectedTab][iRow].dot->setBrush(QBrush(Qt::red));
                highlightedDots.append(importedEvents[selectedTab][iRow].dot);
            }

            if (importedEvents[selectedTab][iRow].line) {
                importedEvents[selectedTab][iRow].line->setPen(QPen(Qt::red, 4.0, Qt::SolidLine, Qt::FlatCap));
                highlightedLines.append(importedEvents[selectedTab][iRow].line);
            }
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
    importedEvents.clear();
    highlightedDots.clear();
    highlightedLines.clear();
    scribbler->resetDrawingSlot();
}

void MainWindow::fadeTabSlot() {
    scribbler->fadeTab(tabs->currentIndex());
    clearColors();
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
    in >> import;

    for (int iTab=0; iTab<import.size(); ++iTab) {
        scribbler->drawMouseEvents(import[iTab]);
        scribbler->endCaptureSlot();
    }
}
