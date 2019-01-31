#include "trialseditortool.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication trialsEditorApp(argc, argv);
    TrialsEditorTool trialsEditorTool;
    // Get track folder from command line arguments for
    // drag and drop functionality
    QString path = "";
    if(argc == 2) {
        path = argv[1];
    }
    if(!trialsEditorTool.initialize(path)) {
        return 0;
    }
    trialsEditorTool.show();

    return trialsEditorApp.exec();
}
