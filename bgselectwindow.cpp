#include "BgSelectWindow.h"
#include "GlobalData.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>

BgSelectWindow::BgSelectWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(1600, 900);
    move(50, 50);
    setStyleSheet("background-color:#1b1b2f;");
    initUI();
}

void BgSelectWindow::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(40);

    QLabel *title = new QLabel("选择战斗背景");
    title->setStyleSheet("color:#ffd700; font-size:32px; font-weight:bold;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(30);

    // ✅ 正确相对路径（图片放在 exe 同级目录下）
    QVector<QPair<QString, QString>> bgList = {
        {"bg_mond.jpg",    "蒙德"},
        {"bg_liyue.jpg",   "璃月"},
        {"bg_inazuma.jpg", "稻妻"},
        {"bg_sumeru.jpg",  "须弥"},
        {"bg_fontaine.jpg","枫丹"},
        {"bg_natlan.jpg",  "纳塔"}
    };

    for (int i = 0; i < 6; i++) {
        QString path = bgList[i].first;
        QString name = bgList[i].second;

        QWidget *box = new QWidget;
        QVBoxLayout *vl = new QVBoxLayout(box);
        vl->setContentsMargins(0,0,0,0);
        vl->setSpacing(8);

        QLabel *imgLabel = new QLabel;
        imgLabel->setFixedSize(300, 160);
        imgLabel->setStyleSheet("border: 2px solid #666; border-radius:10px; background:#222;");
        imgLabel->setAlignment(Qt::AlignCenter);

        QPixmap pix(path);
        if (!pix.isNull()) {
            imgLabel->setPixmap(pix.scaled(300,160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            imgLabel->setText("加载失败");
            imgLabel->setStyleSheet("color:red; border:2px solid red;");
        }

        QLabel *textLabel = new QLabel(name);
        textLabel->setStyleSheet("color:white; font-size:18px;");
        textLabel->setAlignment(Qt::AlignCenter);

        vl->addWidget(imgLabel);
        vl->addWidget(textLabel);

        QPushButton *btn = new QPushButton;
        btn->setFixedSize(300, 200);
        btn->setLayout(vl);
        btn->setStyleSheet(R"(
            QPushButton {
                border:2px solid #555; border-radius:12px; background:transparent;
            }
            QPushButton:hover {
                border-color:#ffd700;
            }
        )");

        connect(btn, &QPushButton::clicked, this, [=]() {
            GlobalData::selectedBgPath = path;
            accept();
        });

        gridLayout->addWidget(btn, i/3, i%3);
    }

    mainLayout->addLayout(gridLayout);
    setLayout(mainLayout);
}