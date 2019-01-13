#include "trialseditortool.h"
#include "ui_trialseditortool.h"

TrialsEditorTool::TrialsEditorTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrialsEditorTool)
{
    ui->setupUi(this);
}

TrialsEditorTool::~TrialsEditorTool()
{
    delete ui;
}
