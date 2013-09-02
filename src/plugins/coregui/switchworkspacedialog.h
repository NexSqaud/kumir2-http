#ifndef COREGUI_SWITCHWORKSPACEDIALOG_H
#define COREGUI_SWITCHWORKSPACEDIALOG_H

#include "extensionsystem/settings.h"

#include <QtCore>
#include <QtGui>

namespace CoreGUI {

namespace Ui {
    class SwitchWorkspaceDialog;
}

class SwitchWorkspaceDialog
        : public QDialog
{
    Q_OBJECT

public:
    explicit SwitchWorkspaceDialog(ExtensionSystem::SettingsPtr settings);
    ~SwitchWorkspaceDialog();
    void setCurrentWorkspace(const QString &path);
    QString currentWorkspace() const;
private slots:
    void handleBrowseClicked();
    void handleAccepted();
private:
    Ui::SwitchWorkspaceDialog *ui;
    ExtensionSystem::SettingsPtr settings_;
};


} // namespace CoreGUI
#endif // COREGUI_SWITCHWORKSPACEDIALOG_H