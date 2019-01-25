#include "trialseditortool.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication trialsEditorApp(argc, argv);
    TrialsEditorTool trialsEditorTool;
    trialsEditorTool.show();

    return trialsEditorApp.exec();
}
