#include "CardWidget.h"
#include <QPainter>
#include <QFont>

CardWidget::CardWidget(Card *card, QWidget *parent)
    : QWidget(parent), m_card(card)
{
    setFixedSize(200, 280);
}

void CardWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 卡牌背景
    p.setBrush(QColor(44, 62, 80, 220));
    p.setPen(QPen(Qt::white, 2));
    p.drawRoundedRect(8, 8, width()-16, height()-16, 15, 15);

    // 角色卡 → 显示图片
    if (m_card->getType() == CARD_ROLE)
    {
        RoleCard* rCard = dynamic_cast<RoleCard*>(m_card);
        if(rCard)
        {
            QString imgPath;

            switch(rCard->getRoleId())
            {
            case Role_Babala:      imgPath = ":/char/babala.jpg"; break;
            case Role_Bannite:     imgPath = ":/char/bannite.jpg"; break;
            case Role_Chongyun:    imgPath = ":/char/chongyun.jpg"; break;
            case Role_Diluke:      imgPath = ":/char/diluke.jpg"; break;
            case Role_Feixieer:    imgPath = ":/char/feixieer.jpg"; break;
            case Role_Kaiya:       imgPath = ":/char/kaiya.jpg"; break;
            case Role_Kawei:       imgPath = ":/char/kawei.jpg"; break;
            case Role_Kelai:       imgPath = ":/char/kelai.jpg"; break;
            case Role_Keqing:      imgPath = ":/char/keqing.jpg"; break;
            case Role_Ningguang:   imgPath = ":/char/ningguang.jpg"; break;
            case Role_ShanHaiGong: imgPath = ":/char/shanhugongxinhai.jpg"; break;
            case Role_ShaTang:     imgPath = ":/char/shatang.jpg"; break;
            case Role_Wendi:       imgPath = ":/char/wendi.jpg"; break;
            case Role_Zhongli:     imgPath = ":/char/zhongli.jpg"; break;
            default: imgPath = ""; break;
            }

            if(!imgPath.isEmpty())
            {
                QPixmap pix(imgPath);
                pix = pix.scaled(150, 130, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                p.drawPixmap(25, 20, pix);
            }

            // 文字信息
            p.setPen(Qt::white);
            p.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));

            p.drawText(QRect(20, 160, width()-40, 30), Qt::AlignCenter, m_card->getName());
            p.drawText(QRect(20, 195, width()-40, 30), Qt::AlignCenter,
                       "血量：" + QString::number(rCard->getHp()));
            p.drawText(QRect(20, 230, width()-40, 30), Qt::AlignCenter,
                       "攻击：" + QString::number(rCard->getAtk()));
        }
    }
}