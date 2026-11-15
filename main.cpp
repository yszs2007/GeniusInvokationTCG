#include <QApplication>
#include "BgSelectWindow.h"
#include "TeamSelectDialog.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    BgSelectWindow bgWin;
    if (bgWin.exec() != QDialog::Accepted)
        return 0;

    TeamSelectDialog teamDlg;
    if (teamDlg.exec() != QDialog::Accepted)
        return 0;

    MainWindow w;
    w.setPlayerTeam(teamDlg.getSelectedTeam());
    w.show();

    return a.exec();
}