#include "trialseditortool.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication trialsEditorApp(argc, argv);
    TrialsEditorTool trialsEditorTool;
    if(!trialsEditorTool.initialize()) {
        return 0;
    }
    trialsEditorTool.show();

    return trialsEditorApp.exec();
}
