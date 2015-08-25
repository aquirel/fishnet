// window.cpp - main application window.

#include <Qt>
#include <QPainter>
#include <QMouseEvent>
#include <QMenuBar>

#include "window.hpp"

QVector2D Window::Gravity = QVector2D(0, .5F);

Window::Window()
{
    setWindowTitle("Fishnet Simulation");
    resize(Window::DefaultWidth, Window::DefaultHeight);

    // Set background to white.
    setAutoFillBackground(true);
    auto modified_palette = palette();
    modified_palette.setColor(QPalette::Background, Qt::white);
    setPalette(modified_palette);
}

void Window::startSimulation()
{
    startTimer(Window::SimulationInterval);
}

void Window::createMenus()
{
    auto action = menuBar()->addAction("&Generate Net");
    connect(action, SIGNAL(triggered(bool)), SLOT(generateNet(bool)));
}

void Window::generateNet(bool)
{
    joints.clear();
    nodes.clear();
    currentNode = nullptr;
    dragging = false;

    auto netWidth = Window::NetSize * Joint::Length,
         paddingWidth = (width() - netWidth) / 2.0F,
         paddingHeight = (height() - netWidth) / 2.0F;

    for (auto i = 0; i < Window::NetSize; i++)
        for (auto j = 0; j < Window::NetSize; j++)
            nodes.append(Node(QVector2D(paddingWidth + j * Joint::Length, paddingHeight + i * Joint::Length)));

    auto nodes_iterator = nodes.begin();

    for (auto i = 0; i < Window::NetSize * Window::NetSize; i++)
    {
        if (i > 0 && 0 != i % Window::NetSize)
            connectNodes(&*(nodes_iterator + i - 1), &*(nodes_iterator + i));

        if (i >= Window::NetSize)
            connectNodes(&*(nodes_iterator + i - Window::NetSize), &*(nodes_iterator + i));

        if (i + Window::NetSize + 1 < Window::NetSize * Window::NetSize &&
            (Window::NetSize - 1) != i % Window::NetSize)
            connectNodes(&*(nodes_iterator + i), &*(nodes_iterator + i + Window::NetSize + 1));

        if (i + Window::NetSize - 1 < Window::NetSize * Window::NetSize &&
            0 != i % Window::NetSize)
            connectNodes(&*(nodes_iterator + i), &*(nodes_iterator + i + Window::NetSize - 1));
    }

    for (auto i = 1; i < Window::NetSize - 1; i++)
        (nodes_iterator + i)->switchType();

    for (auto i = Window::NetSize; i < Window::NetSize * Window::NetSize; i++)
        (nodes_iterator + i)->switchType();
}

void Window::timerEvent(QTimerEvent *event)
{
    event->accept();

    for (auto node = nodes.begin(); node != nodes.end();)
    {
        if (Node::Type::Fixed == node->getType() ||
            (currentNode == &(*node) && dragging))
        {
            node->setSpeed(QVector2D(0, 0));
            node->setAcceleration(QVector2D(0, 0));
            node++;
            continue;
        }

        auto position = node->getPosition(),
             speed = node->getSpeed(),
             acceleration = node->getAcceleration();

        auto frictionForce = -speed * Window::Friction,
             frictionAcceleration = frictionForce / Node::Mass;
        acceleration += frictionAcceleration + Window::Gravity;

        speed += acceleration;
        position += speed;

        acceleration.setX(0);
        acceleration.setY(0);

        if (position.y() >= 2 * height())
        {
            for (auto joint = joints.begin(); joint != joints.end();)
            {
                if (joint->start == &(*node) || joint->end == &(*node))
                    joint = joints.erase(joint);
                else
                    joint++;
            }

            if (&(*node) == currentNode)
                currentNode = nullptr;

            node = nodes.erase(node);
        }
        else
        {
            node->setPosition(position);
            node->setSpeed(speed);
            node->setAcceleration(acceleration);
            node++;
        }
    }

    auto jointsEnd = joints.end();
    for (auto joint = joints.begin(); joint != jointsEnd; joint++)
    {
        joint->Process();
    }

    update();
}

void Window::paintEvent(QPaintEvent *event)
{
    static QPen fixedNodePen(Qt::gray, Node::Size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),
                movableNodePen(Qt::black, Node::Size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),
                jointPen(Qt::black, .5F, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    event->accept();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(jointPen);
    auto jointsEnd = joints.constEnd();
    for (auto joint = joints.constBegin(); joint != jointsEnd; ++joint)
    {
        if (!joint->start || !joint->end)
            continue;
        painter.drawLine(joint->start->getPosition().toPointF(), joint->end->getPosition().toPointF());
    }

    auto nodesEnd = nodes.constEnd();
    for (auto node = nodes.constBegin(); node != nodesEnd; ++node)
    {
        if (Node::Type::Fixed == node->getType())
            painter.setPen(fixedNodePen);
        else
            painter.setPen(movableNodePen);

        painter.drawEllipse(node->getPosition().toPointF(), Node::Size / 2.0F, Node::Size / 2.0F);
    }
}

void Window::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->buttons())
    {
        event->accept();
        nodes.append(Node(QVector2D(event->posF())));
        currentNode = &nodes.last();
        update();
        return;
    }

    if (Qt::RightButton == event->buttons())
    {
        event->accept();
        Node *node = findNodeAtPosition(QVector2D(event->posF()));
        if (node)
        {
            node->switchType();
            update();
        }
        return;
    }

    event->ignore();
}

void Window::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->buttons())
    {
        event->accept();
        currentNode = findNodeAtPosition(QVector2D(event->posF()));
        dragging = true;
        return;
    }

    if (Qt::RightButton == event->buttons())
    {
        event->accept();
        if (currentNode)
        {
            Node *nextNode = findNodeAtPosition(QVector2D(event->posF()));
            if (nextNode)
            {
                connectNodes(currentNode, nextNode);
                currentNode = nextNode;
                update();
            }
        }
        return;
    }

    event->ignore();
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
    dragging = false;
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    if (Qt::LeftButton != event->buttons() ||
        nullptr == currentNode)
    {
        event->ignore();
        return;
    }

    currentNode->setPosition(QVector2D(event->posF()));
    update();
}

Node *Window::findNodeAtPosition(const QVector2D &position)
{
    auto nodesEnd = nodes.end();
    for (auto node = nodes.begin(); node != nodesEnd; ++node)
    {
        auto distance = position - node->getPosition();
        if (distance.length() < Node::Size)
        {
            return &(*node);
        }
    }

    return nullptr;
}

void Window::connectNodes(Node *start, Node *end)
{
    if (start == end || !start || !end)
        return;

    auto jointsEnd = joints.end();
    for (auto joint = joints.begin(); joint != jointsEnd; ++joint)
    {
        if ((joint->start == start && joint->end == end) ||
            (joint->start == end && joint->end == start))
        {
            joints.erase(joint);
            return;
        }
    }

    joints.append(Joint(start, end));
}
