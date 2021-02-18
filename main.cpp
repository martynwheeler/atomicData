#include "atomicdata.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AtomicData w;
    w.show();
    return a.exec();
}
