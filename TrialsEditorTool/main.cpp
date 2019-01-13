#include "trialseditortool.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TrialsEditorTool w;
    w.show();

    return a.exec();
}
