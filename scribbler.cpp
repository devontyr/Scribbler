#include "scribbler.h"
#include <QtWidgets>

MouseEvent::MouseEvent(int _action, QPointF _pos, quint64 _time) //just a data storage class
    :action(_action), pos(_pos), time(_time) {}

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
    setMinimumSize(QSize(800, 600));
    setRenderHint(QPainter::Antialiasing, true);
    setBackgroundBrush(Qt::lightGray);

    scene.addRect(sceneRect()); //for debugging
}

void Scribbler::mousePressEvent(QMouseEvent *evt) {
    QGraphicsView::mousePressEvent(evt); // need to load previous abilites too... those that we didn't override

    QPointF p = mapToScene(evt->pos()); //evt pos is in widget coords (QPoint)... we want SCENE coords (QPointF)
    lastPoint = p;
    QGraphicsEllipseItem *dot = scene.addEllipse(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);

    //record event
    dots << dot;
    events << MouseEvent(MouseEvent::Press, p, evt->timestamp());
}

void Scribbler::mouseMoveEvent(QMouseEvent *evt) {
    QGraphicsView::mouseMoveEvent(evt);

    QPointF p = mapToScene(evt->pos());
    QGraphicsLineItem *line = scene.addLine(QLineF(lastPoint, p), QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
    QGraphicsEllipseItem *dot = scene.addEllipse(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);
    lastPoint = p;
    line->setVisible(isLineVisible);

    dots << dot;
    lines << line;
    events << MouseEvent(MouseEvent::Move, p, evt->timestamp());
}


void Scribbler::mouseReleaseEvent(QMouseEvent *evt) {
    QGraphicsView::mouseReleaseEvent(evt);

    QPointF p = mapToScene(evt->pos());

    events << MouseEvent(MouseEvent::Release, p, evt->timestamp());
}

void Scribbler::showLineSegmentsSlot() {
    //visibility of existing
    for (int iLine=0; iLine < lines.length(); ++iLine)
        lines[iLine]->setVisible(true);
    //visibility of future drawing
    isLineVisible = true;
}

void Scribbler::showDotsOnlySlot() {
    //visibility of existing
    for (int iLine=0; iLine < lines.length(); ++iLine)
        lines[iLine]->setVisible(false);
    //visibility of future drawing
    isLineVisible = false;
}

void Scribbler::startCaptureSlot() {
    //reset all the data to being capturing
    events.clear();
}

void Scribbler::endCaptureSlot() {
    //emit signal with the data out to MainWindow
    emit sendCapture(events);
    events.clear();
}

void Scribbler::resetDrawingSlot() {
    scene.clear();
}


/*
    QGraphicsLineItem *line = scene.addLine(QLineF(0.0, 0.0, 50.0, 100.0), QPen(Qt::green, 25.0, Qt::SolidLine, Qt::RoundCap));
    QGraphicsLineItem *line1 = scene.addLine(QLineF(0.0, 0.0, 50.0, 100.0), QPen(Qt::black, 2.0));
    //QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(QPointF(0.0, 0.0), QPointF(100.0, 100.0)));
    //scene.addItem(line);

    QGraphicsRectItem *rect = scene.addRect(QRectF(100.0, 100.0, 200.0, 300.0), QPen(Qt::red, 5.0, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin), Qt::yellow);
    QGraphicsEllipseItem *dot = scene.addEllipse(200.0 - 3.0, 250.0 - 3.0, 6.0, 6.0, Qt::NoPen, Qt::black);
    rect->setRotation(90.0);
    rect->setScale(0.5);
    rect->setTransformOriginPoint(200.0, 250.0);


    //GROUP ITEMS
    QGraphicsItemGroup *group = scene.createItemGroup(QList<QGraphicsItem *>() << line << line1 << rect << dot);

    //TRANSFORMATIONS AND COORDINATES
    group->setFlag(QGraphicsItem::ItemIsMovable);
    // group->setTransformOriginPoint(0.0, 0.0);
    group->setTransform(QTransform().translate(200.0, 200.0).scale(0.5, 0.5).rotate(90.0));
    */