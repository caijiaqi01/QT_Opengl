#include "openglmathmode.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpenglMathMode w;
    w.show();

    return a.exec();
}
