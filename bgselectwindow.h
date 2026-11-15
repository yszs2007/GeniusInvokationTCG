#ifndef BGSELECTWINDOW_H
#define BGSELECTWINDOW_H

#include <QDialog>
#include <QGridLayout>

class BgSelectWindow : public QDialog
{
    Q_OBJECT
public:
    explicit BgSelectWindow(QWidget *parent = nullptr);

private:
    void initUI();
    QGridLayout *m_gridLayout;
};

#endif