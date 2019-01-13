#ifndef TRIALSEDITORTOOL_H
#define TRIALSEDITORTOOL_H

#include <QMainWindow>

namespace Ui {
class TrialsEditorTool;
}

class TrialsEditorTool : public QMainWindow
{
    Q_OBJECT

public:
    explicit TrialsEditorTool(QWidget *parent = nullptr);
    ~TrialsEditorTool();

private:
    Ui::TrialsEditorTool *ui;
};

#endif // TRIALSEDITORTOOL_H
