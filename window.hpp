// window.hpp - main application window.

#pragma once

#include <QMainWindow>
#include <QLinkedList>

#include "fishnet.hpp"

class Window : public QMainWindow
{
    Q_OBJECT

    private:
    QLinkedList<Node> nodes;
    QLinkedList<Joint> joints;
    Node *findNodeAtPosition(const QVector2D &position);
    Node *currentNode;
    bool dragging = false;

    public:
    static const int DefaultWidth = 640,
                     DefaultHeight = 480,
                     SimulationInterval = 50,
                     NetSize = 16;
    static constexpr qreal Friction = 0.05F;
    static QVector2D Gravity;

    Window();
    void createMenus();
    void startSimulation();

    private:
    virtual void paintEvent(QPaintEvent *event);
    virtual void timerEvent(QTimerEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    void connectNodes(Node *start, Node *end);

    private slots:
    void generateNet(bool);
};
