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
    QGraphicsEllipseItem *dot;
    QGraphicsLineItem *line;

    //constructor:
    MouseEvent(int _action, QPointF _pos, quint64 _time, QGraphicsEllipseItem* _dot, QGraphicsLineItem* _line);
    MouseEvent();

    //output methods to files (<< for input methods)
    friend QDataStream &operator<<(QDataStream &out, const MouseEvent &evt);
    friend QDataStream &operator>>(QDataStream &in, MouseEvent &evt);
};

class Scribbler : public QGraphicsView
{    
    QGraphicsScene scene; //stores its items on the heap
    double lineWidth;
    QPointF lastPoint;
    bool isLineVisible;

    QList<MouseEvent> events;
    QList<QGraphicsLineItem *> drawnLines;
    QList<QGraphicsEllipseItem *> drawnDots;

    QList<QGraphicsLineItem *> lines;
    QList<QGraphicsEllipseItem *> dots;

    QList<QList<QGraphicsLineItem *>> capturedLines;
    QList<QList<QGraphicsEllipseItem *>> capturedDots;

    void addPoint(QPointF p, ulong timestamp);
    void addLineSegement(QPointF point1, QPointF point2, ulong timestamp);

    void showLines (bool areLinesShown);


    Q_OBJECT

public:
    Scribbler();
    void drawMouseEvents(QList<MouseEvent> &events);
    void fadeTab(int selectedTab);

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
