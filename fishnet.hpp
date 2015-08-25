// fishnet.hpp - fishnet node and link declaration.

#pragma once

#include <QtGlobal>
#include <QVector2D>

class Node
{
    public:
    enum class Type { Fixed, Movable };
    static constexpr qreal Size = 2.5F,
                           Mass = 1.0F;

    Node(const QVector2D &position, Type type = Type::Fixed) : position(position), type(type) {}

    private:
    QVector2D position, speed, acceleration;
    Type type;

    public:
    Type getType() const { return type; }
    void setType(Type type) { this->type = type; }
    void switchType() { type = Type::Fixed == type ? Type::Movable : Type::Fixed; }
    QVector2D getPosition() const { return position; }
    void setPosition(const QVector2D &position) { this->position = position; }
    QVector2D getSpeed() const { return speed; }
    void setSpeed(const QVector2D &speed) { this->speed = speed; }
    QVector2D getAcceleration() const { return acceleration; }
    void setAcceleration(const QVector2D &acceleration) { this->acceleration = acceleration; }
};

class Joint
{
    public:
    Node *start;
    Node *end;

    private:
    qreal length;

    public:
    static constexpr qreal Length = 20.0F,
                           Coefficient = 0.1F;
    Joint(Node *start, Node *end, qreal length = Length) : start(start), end(end), length(length) {}
    qreal getLength() const { return length; }
    void setLength(qreal length) { this->length = length; }

    void Process()
    {
        if (!start || !end)
            return;

        auto startPosition = start->getPosition(),
             endPosition = end->getPosition(),
             connection = endPosition - startPosition;
        auto realLength = connection.length();
        auto direction = connection.normalized();
        auto deformation = realLength - length,
             force = -Joint::Coefficient * deformation,
             acceleration = force / Node::Mass;

        auto accelerationStart = -direction * acceleration,
             accelerationEnd = direction * acceleration;

        start->setAcceleration(start->getAcceleration() + accelerationStart);
        end->setAcceleration(end->getAcceleration() + accelerationEnd);
    }
};
