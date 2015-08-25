// fishnet.cpp - main().

#include <QApplication>

#include "window.hpp"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    Window window;
    window.show();
    window.createMenus();
    window.startSimulation();

    return application.exec();
}
