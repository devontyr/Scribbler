#ifndef SCRIBBLER_H
#define SCRIBBLER_H

#include <QGraphicsView>
#include <QtWidgets>

class MouseEvent {
public:
    enum {
        Press,
        Move,
        Release
    };

    int action;
    QPointF pos;
    quint64 time;

    //constructor:
    MouseEvent(int _action, QPointF _pos, quint64 _time);
    MouseEvent();

    //output methods to files (<< for input methods)
    friend QDataStream &operator<<(QDataStream &out, const MouseEvent &evt);
    friend QDataStream &operator>>(QDataStream &in, MouseEvent &evt);

    // output method for qDebugging:
    // friend QTextStream &operator<<(QTextStream &out, const MouseEvent &evt);
};

class Scribbler : public QGraphicsView
{    
    QGraphicsScene scene; //stores its items on the heap
    double lineWidth;
    QPointF lastPoint;
    bool isLineVisible;

    QGraphicsEllipseItem *dot;
    QGraphicsLineItem *line;

    QList<MouseEvent> events;
    QList<QGraphicsLineItem *> lines;
    QList<QGraphicsEllipseItem *> dots;

    void addPoint(QPointF point);
    void addLineSegement(QPointF point1, QPointF point2);


    Q_OBJECT

public:
    Scribbler();
    void drawMouseEvents(QList<MouseEvent> &events);

public slots:
    void showDotsOnlySlot();
    void showLineSegmentsSlot();
    void startCaptureSlot();
    void endCaptureSlot();
    void resetDrawingSlot();

signals:
    void sendCapture(QList<MouseEvent> events);

protected:
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
};

#endif // SCRIBBLER_H
