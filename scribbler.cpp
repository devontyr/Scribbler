#include "scribbler.h"
#include <QtWidgets>
#include <mainwindow.h>

MouseEvent::MouseEvent(int _action, QPointF _pos, quint64 _time, QGraphicsEllipseItem* _dot, QGraphicsLineItem* _line) //just a data storage class
    :action(_action), pos(_pos), time(_time), dot(_dot), line(_line) {}

QDataStream &operator<<(QDataStream &out, const MouseEvent &evt) {
    return out << evt.action << evt.pos << evt.time;
}

QDataStream &operator>>(QDataStream &in, MouseEvent &evt) {
    return in >> evt.action >> evt.pos >> evt.time;
}

MouseEvent::MouseEvent(){}

// ======================================== Scribbler ========================================
Scribbler::Scribbler()
    :lineWidth(4.0), isLineVisible(true) {
    setScene(&scene); //widgets on the heap. pointers init with new. takes a pointer to the scene

    setSceneRect(QRectF(0.0, 0.0, 800.0, 600.0));
    //setMinimumSize(QSize(800, 600));
    setRenderHint(QPainter::Antialiasing, true);
    setBackgroundBrush(Qt::white);

    scene.addRect(sceneRect()); //for debugging
}

void Scribbler::addPoint(QPointF p, ulong timestamp) {
    QGraphicsEllipseItem* dot = scene.addEllipse(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);

    lastPoint = p;

    //record events
    drawnDots << dot;
    dots << dot;
    events << MouseEvent(MouseEvent::Press, p, timestamp, dot, NULL);
}

void Scribbler::addLineSegement(QPointF point1, QPointF point2, ulong timestamp) {
    QGraphicsLineItem* line = scene.addLine(QLineF(point1, point2), QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
    QGraphicsEllipseItem* dot = scene.addEllipse(QRectF(point2 - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);
    lastPoint = point2;
    line->setVisible(isLineVisible);

    //record events
    drawnDots << dot;
    dots << dot;
    drawnLines << line;
    lines << line;
    events << MouseEvent(MouseEvent::Move, point2, timestamp, dot, line);
}

void Scribbler::drawMouseEvents(QList<MouseEvent> &events) {
    for (int iEvt=0; iEvt<events.size(); ++iEvt) {
        QPointF p = events[iEvt].pos;
        int evtAction = events[iEvt].action;
        if (evtAction == 0) {
            addPoint(p, events[iEvt].time);
        }
        if (evtAction == 1) {
            addLineSegement(lastPoint, p, events[iEvt].time);
        }
    }
}

void Scribbler::mousePressEvent(QMouseEvent *evt) {
    QGraphicsView::mousePressEvent(evt); // need to load previous abilites too... those that we didn't override

    QPointF p = mapToScene(evt->pos()); //evt pos is in widget coords (QPoint)... we want SCENE coords (QPointF)
    addPoint(p, evt->timestamp());
}

void Scribbler::mouseMoveEvent(QMouseEvent *evt) {
    QGraphicsView::mouseMoveEvent(evt);

    QPointF p = mapToScene(evt->pos());

    addLineSegement(lastPoint, p, evt->timestamp());
    addPoint(p, evt->timestamp());

}


void Scribbler::mouseReleaseEvent(QMouseEvent *evt) {
    QGraphicsView::mouseReleaseEvent(evt);

    QPointF p = mapToScene(evt->pos());

    events << MouseEvent(MouseEvent::Release, p, evt->timestamp(), NULL, NULL);
}

void Scribbler::showLines(bool areLinesShown) {
    //visibility of existing
    for (int iLine=0; iLine < drawnLines.length(); ++iLine) {
        drawnLines[iLine]->setVisible(areLinesShown);
        //visibility of future drawing
        isLineVisible = areLinesShown;
    }
}

void Scribbler::showLineSegmentsSlot() {
    showLines(true);
}

void Scribbler::showDotsOnlySlot() {
    showLines(false);
}

void Scribbler::startCaptureSlot() {
    //reset all the data to being capturing
    events.clear();
    dots.clear();
    lines.clear();
}

void Scribbler::endCaptureSlot() {
    capturedDots << dots;
    capturedLines << lines;
    dots.clear();
    lines.clear();

    emit sendCapture(events); //emit signal with the data out to MainWindow
    events.clear();
}

void Scribbler::resetDrawingSlot() {
    scene.clear();
    dots.clear();
    lines.clear();
    drawnLines.clear();
    drawnDots.clear();
    capturedDots.clear();
    capturedLines.clear();
}

void Scribbler::fadeTab(int selectedTab) {
    if (selectedTab == -1) return;
    //set all graphics to .25 opacity
    for (int iLine=0; iLine < drawnLines.size(); ++iLine)
        drawnLines[iLine]->setOpacity(0.25);
    for (int iDot=0; iDot < drawnDots.size(); ++iDot)
        drawnDots[iDot]->setOpacity(0.25);

    //set all graphics of whichever tab you are on to 1 opacity
    for (int iLine=0; iLine < capturedLines[selectedTab].size(); ++iLine)
        capturedLines[selectedTab][iLine]->setOpacity(1.0);
    for (int iDot=0; iDot < capturedDots[selectedTab].size(); ++iDot)
        capturedDots[selectedTab][iDot]->setOpacity(1.0);
}
