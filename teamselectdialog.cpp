#include "TeamSelectDialog.h"
#include "Card.h"
#include <QMessageBox>
#include <QRandomGenerator>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>

// 图片加载函数（静态版，不依赖头文件）
static QPixmap getRolePixmap(RoleID id)
{
    QString path;
    switch(id)
    {
    case Role_Babala:       path = "babala.jpg";       break;
    case Role_Bannite:      path = "bannite.jpg";      break;
    case Role_Chongyun:     path = "chongyun.jpg";     break;
    case Role_Diluke:       path = "diluke.jpg";       break;
    case Role_Feixieer:     path = "feixieer.jpg";     break;
    case Role_Kaiya:        path = "kaiya.jpg";        break;
    case Role_Kawei:        path = "kawei.jpg";        break;
    case Role_Kelai:        path = "kelai.jpg";        break;
    case Role_Keqing:       path = "keqing.jpg";       break;
    case Role_Ningguang:    path = "ningguang.jpg";    break;
    case Role_ShanHaiGong:  path = "shanhugongxinhai.jpg"; break;
    case Role_ShaTang:      path = "shatang.jpg";      break;
    case Role_Wendi:        path = "wendi.jpg";        break;
    case Role_Zhongli:      path = "zhongli.jpg";      break;
    default: return QPixmap();
    }
    return QPixmap(path);
}

TeamSelectDialog::TeamSelectDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(1600, 900);
    QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    move(screen.center() - rect().center());

    setStyleSheet("background-color: #1e1e2e;");

    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(40,40,40,40);
    rootLayout->setSpacing(30);

    QVBoxLayout *leftLayout = new QVBoxLayout();

    QLabel *title = new QLabel("选择出战队伍（限选3名）");
    title->setStyleSheet("color: #f0e68c; font-size:28px; font-weight:bold;");
    title->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(title);

    m_countLabel = new QLabel("已选择：0 / 3");
    m_countLabel->setStyleSheet("color:#ffcc00; font-size:20px;");
    m_countLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(m_countLabel);

    m_gridLayout = new QGridLayout();
    m_gridLayout->setSpacing(15);
    leftLayout->addLayout(m_gridLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *randomBtn = new QPushButton("随机选择3名");
    QPushButton *confirmBtn = new QPushButton("确认进入游戏");
    confirmBtn->setEnabled(false);

    randomBtn->setFixedSize(180,50);
    confirmBtn->setFixedSize(180,50);

    randomBtn->setStyleSheet(R"(
        QPushButton{background:#ff9500;color:white;font-size:18px;border-radius:8px;}
        QPushButton:hover{background:#ffaa22;}
    )");
    confirmBtn->setStyleSheet(R"(
        QPushButton{background:#27ae60;color:white;font-size:18px;border-radius:8px;}
        QPushButton:hover{background:#2ecc71;}
    )");

    btnLayout->addWidget(randomBtn);
    btnLayout->addWidget(confirmBtn);
    leftLayout->addLayout(btnLayout);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(20,20,20,20);

    m_previewImg = new QLabel();
    m_previewImg->setFixedSize(420,520);
    m_previewImg->setStyleSheet("border:2px solid #444; border-radius:12px; background:#2a2a3a;");
    m_previewImg->setAlignment(Qt::AlignCenter);

    m_previewName = new QLabel("角色名称");
    m_previewName->setStyleSheet("color:white; font-size:24px; font-weight:bold;");
    m_previewName->setAlignment(Qt::AlignCenter);

    m_previewElem = new QLabel("元素属性");
    m_previewElem->setStyleSheet("color:#87cefa; font-size:20px;");
    m_previewElem->setAlignment(Qt::AlignCenter);

    // 新增：技能介绍面板
    m_skillDescription = new QLabel(this);
    m_skillDescription->setStyleSheet(R"(
        QLabel {
            background-color: rgba(0,0,0,70);
            color: white;
            border: 2px solid #666;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
        }
    )");
    m_skillDescription->setWordWrap(true);
    m_skillDescription->setFixedWidth(380);
    m_skillDescription->setAlignment(Qt::AlignTop);
    m_skillDescription->hide();

    rightLayout->addWidget(m_previewImg);
    rightLayout->addWidget(m_previewName);
    rightLayout->addWidget(m_previewElem);
    rightLayout->addWidget(m_skillDescription);
    rightLayout->addStretch();

    rootLayout->addLayout(leftLayout, 3);
    rootLayout->addLayout(rightLayout, 2);

    for (int i = 0; i < m_allRoles.size(); ++i)
    {
        RoleID id = m_allRoles[i].first;
        QString name = m_allRoles[i].second;

        QPushButton *btn = new QPushButton(name);
        btn->setFixedSize(160,100);
        btn->setCheckable(true);
        btn->setStyleSheet(R"(
            QPushButton{
                background:#2d2d3f; color:#eee;
                border:2px solid #555; border-radius:10px;
                font-size:16px;
            }
            QPushButton:hover{border-color:#ffd700;}
            QPushButton:checked{border-color:#2ecc71;background:#253b3b;}
        )");

        connect(btn, &QPushButton::clicked, this, [=](){
            onRoleClicked(id);
        });
        connect(btn, &QPushButton::pressed, this, [=](){
            showRolePreview(id);
        });

        m_roleButtons.append(btn);
        m_gridLayout->addWidget(btn, i / 4, i % 4);
    }

    connect(randomBtn, &QPushButton::clicked, this, &TeamSelectDialog::onRandomClicked);
    connect(confirmBtn, &QPushButton::clicked, this, &TeamSelectDialog::onConfirmClicked);

    m_previewImg->setText("点击角色预览");
    m_previewName->setText("");
    m_previewElem->setText("");
}

// 角色技能文本获取函数
QString TeamSelectDialog::getRoleSkillText(RoleID id)
{
    switch (id) {
    case Role_Wendi:
        return "【温迪】\n"
               "A：造成2点物理伤害 | 消耗：1风 + 2任意\n"
               "E：造成2点风伤，2次切换角色不耗骰 | 消耗：3风\n"
               "Q：生成暴风之眼（2回合结束造成2点风伤）| 消耗：3风 | 充能2解锁";

    case Role_ShaTang:
        return "【砂糖】\n"
               "A：造成1点风元素伤害 | 消耗：1风 + 2任意\n"
               "E：造成3点风元素伤害 | 消耗：3风\n"
               "Q：生成大型风灵（3回合结束造成2点风伤）| 消耗：3风 | 充能2解锁";

    case Role_Bannite:
        return "【班尼特】\n"
               "A：造成2点物理伤害 | 消耗：1火 + 2任意\n"
               "E：造成3点火元素伤害 | 消耗：3火\n"
               "Q：技能额外伤+自回1血 | 消耗：4火 | 充能2解锁";

    case Role_Diluke:
        return "【迪卢克】\n"
               "A：造成2点物理伤害 | 消耗：1火 + 2任意\n"
               "E：造成3点火元素伤害 | 消耗：3火\n"
               "Q：造成8点火元素伤害 | 消耗：4火 | 充能3解锁";

    case Role_Zhongli:
        return "【钟离】\n"
               "A：造成2点物理伤害 | 消耗：1岩 + 2任意\n"
               "E：生成岩脊（2回合结束1点岩伤）| 消耗：3岩\n"
               "Q：造成6点岩元素伤害 | 消耗：3岩 | 充能3解锁";

    case Role_Ningguang:
        return "【凝光】\n"
               "A：造成1点岩元素伤害 | 消耗：1岩 + 2任意\n"
               "E：生成璇玑屏（抵消2次受伤各1点）| 消耗：3岩\n"
               "Q：造成6点岩元素伤害 | 消耗：3岩 | 充能3解锁";

    case Role_Feixieer:
        return "【菲谢尔】\n"
               "A：造成2点物理伤害 | 消耗：1雷 + 2任意\n"
               "E：生成奥兹（2回合结束1点雷伤）| 消耗：3雷\n"
               "Q：造成4点雷元素伤害 | 消耗：3雷 | 充能3解锁";

    case Role_Keqing:
        return "【刻晴】\n"
               "A：造成2点物理伤害 | 消耗：1雷 + 2任意\n"
               "E：造成3点雷元素伤害 | 消耗：3雷\n"
               "Q：造成5点雷元素伤害 | 消耗：4雷 | 充能3解锁";

    case Role_Kawei:
        return "【卡维】\n"
               "A：造成2点物理伤害 | 消耗：1草 + 2任意\n"
               "E：造成3点草元素伤害 | 消耗：3草\n"
               "Q：造成4点草元素伤害 | 消耗：3草 | 充能2解锁";

    case Role_Kelai:
        return "【柯莱】\n"
               "A：造成2点物理伤害 | 消耗：1草 + 2任意\n"
               "E：造成3点草元素伤害 | 消耗：3草\n"
               "Q：生成柯里安巴（2回合结束2点草伤）| 消耗：3草 | 充能2解锁";

    case Role_Babala:
        return "【芭芭拉】\n"
               "A：造成1点水元素伤害 | 消耗：1水 + 2任意\n"
               "E：造成1点水伤+自回2血 | 消耗：3水\n"
               "Q：全体回复4点生命值 | 消耗：3水 | 充能3解锁";

    case Role_ShanHaiGong:
        return "【珊瑚宫心海】\n"
               "A：造成1点水元素伤害 | 消耗：1水 + 2任意\n"
               "E：生成化海月（伤+回血）| 消耗：3水\n"
               "Q：造成2点水伤+全体回1血 | 消耗：3水 | 充能2解锁";

    case Role_Chongyun:
        return "【重云】\n"
               "A：造成2点物理伤害 | 消耗：1冰 + 2任意\n"
               "E：造成3点冰元素伤害 | 消耗：3冰\n"
               "Q：造成7点冰元素伤害 | 消耗：3冰 | 充能3解锁";

    case Role_Kaiya:
        return "【凯亚】\n"
               "A：造成2点物理伤害 | 消耗：1冰 + 2任意\n"
               "E：造成3点冰元素伤害 | 消耗：3冰\n"
               "Q：切换后额外2点冰伤 | 消耗：4冰 | 充能2解锁";

    default: return "暂无技能介绍";
    }
}

QString TeamSelectDialog::getRoleElementName(RoleID id)
{
    switch(id)
    {
    case Role_Babala:       return "水元素";
    case Role_Bannite:      return "火元素";
    case Role_Chongyun:     return "冰元素";
    case Role_Diluke:       return "火元素";
    case Role_Feixieer:     return "雷元素";
    case Role_Kaiya:        return "冰元素";
    case Role_Kawei:        return "草元素";
    case Role_Kelai:        return "草元素";
    case Role_Keqing:       return "雷元素";
    case Role_Ningguang:    return "岩元素";
    case Role_ShanHaiGong:  return "水元素";
    case Role_ShaTang:      return "风元素";
    case Role_Wendi:        return "风元素";
    case Role_Zhongli:      return "岩元素";
    default: return "未知元素";
    }
}

void TeamSelectDialog::showRolePreview(RoleID id)
{
    QPixmap pix = getRolePixmap(id);
    if(!pix.isNull()){
        m_previewImg->setPixmap(pix.scaled(m_previewImg->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    for(auto &p : m_allRoles){
        if(p.first == id){
            m_previewName->setText(p.second);
            break;
        }
    }
    m_previewElem->setText(getRoleElementName(id));

    // 显示技能介绍
    m_skillDescription->setText(getRoleSkillText(id));
    m_skillDescription->show();
}

void TeamSelectDialog::onRoleClicked(RoleID id)
{
    if(m_selectedTeam.contains(id)){
        m_selectedTeam.removeOne(id);
    }else{
        if(m_selectedTeam.size()>=3){
            QMessageBox::warning(this,"提示","最多只能选择3名角色！");
            return;
        }
        m_selectedTeam.append(id);
    }
    updateUI();
}

void TeamSelectDialog::onRandomClicked()
{
    m_selectedTeam.clear();
    for(auto btn : m_roleButtons) btn->setChecked(false);

    while(m_selectedTeam.size()<3){
        int idx = QRandomGenerator::global()->bounded(m_allRoles.size());
        RoleID id = m_allRoles[idx].first;
        if(!m_selectedTeam.contains(id)) m_selectedTeam.append(id);
    }
    updateUI();
}

void TeamSelectDialog::onConfirmClicked()
{
    if(m_selectedTeam.size()==3) accept();
    else QMessageBox::warning(this,"提示","请选择3名角色！");
}

void TeamSelectDialog::updateUI()
{
    m_countLabel->setText(QString("已选择：%1 / 3").arg(m_selectedTeam.size()));

    for(int i=0;i<m_allRoles.size();i++){
        RoleID id = m_allRoles[i].first;
        m_roleButtons[i]->setChecked(m_selectedTeam.contains(id));
    }

    for(auto *w : findChildren<QPushButton*>()){
        if(w->text()=="确认进入游戏"){
            w->setEnabled(m_selectedTeam.size()==3);
        }
    }
}

QVector<RoleID> TeamSelectDialog::getSelectedTeam() const
{
    return m_selectedTeam;
}