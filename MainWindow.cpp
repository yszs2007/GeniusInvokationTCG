#include "MainWindow.h"
#include "bgselectwindow.h"
#include "GlobalData.h"
#include "card.h"
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QEvent>
#include <QTimer>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <algorithm>
#include <QGraphicsOpacityEffect>

MainWindow::MainWindow(QWidget *parent)
    : roleSelectDone(false)
    , currentActiveRoleIndex(-1)
    , enemyHp(20)
    , enemyAtk(2)
    , enemyEnergy(0)
    , enemyAttachedElement(ElementType::Any)
    , enemyFrozen(false)
    , enemyElectro(false)
    , selectedCardIndex(-1)
    , currentCostType(Cost_None)
    , switchRoleBtn(nullptr)
    , selectedSwitchRoleIndex(-1)
    , isSwitchingRole(false)
    , myTurnEnded(false)
    , enemyTurnEnded(false)
    , enemyActionTimes(0)
    , isPlayerActionPhase(true)
    , selectedSkill(SkillType::None)
    , skillSelected(false)
    , m_currentRound(1)
    , m_nextSwitchFreeAction(false)
    , m_nextSwitchFreeDice(false)
{
    setFixedSize(1600, 900);
    setWindowTitle("七圣召唤");
    move(50, 50);

    // 1. 先创建背景（必须第一个创建，设为中央控件）
    QLabel *bgLabel = new QLabel(this);
    bgLabel->setScaledContents(true);
    QPixmap bgPix(GlobalData::selectedBgPath);
    if (!bgPix.isNull())
    {
        bgLabel->setPixmap(bgPix.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    else
    {
        bgLabel->setText("背景加载失败");
        bgLabel->setStyleSheet("color:red; font-size:30px;");
        bgLabel->setAlignment(Qt::AlignCenter);
    }
    setCentralWidget(bgLabel); // 背景设为中央控件

    // 2. 创建回合标签
    m_roundLabel = new QLabel(this);
    m_roundLabel->setStyleSheet(R"(
        QLabel {
            color: #ffd700;
            font-size: 24px;
            font-weight: bold;
            background-color: rgba(0, 0, 0, 0.6);
            border-radius: 8px;
            padding: 8px 16px;
        }
    )");
    m_roundLabel->setAlignment(Qt::AlignCenter);
    m_roundLabel->setFixedSize(200, 50);
    m_roundLabel->move(20, 20);
    m_roundLabel->setText(QString("当前回合：%1").arg(m_currentRound));
    m_roundLabel->raise(); // 强制置顶

    // ========== 【修改版】更长的伤害记录栏 ==========
    // 高度从180→320，可显示更多条记录
    m_damageLogWidget = new QWidget(this);
    m_damageLogWidget->setStyleSheet("background-color:rgba(0,0,0,0.5); border-radius:10px; border:1px solid #ffd700;");
    m_damageLogWidget->setFixedSize(280, 320); // 关键修改：高度增加
    m_damageLogWidget->move(30, 110); // 位置不变，还是在回合标签下方
    m_damageLogWidget->show();
    m_damageLogWidget->raise(); // 强制置顶

    m_damageLogTitle = new QLabel("伤害记录", m_damageLogWidget);
    m_damageLogTitle->setStyleSheet("color:#ffd700; font-size:16px; font-weight:bold;");
    m_damageLogTitle->setAlignment(Qt::AlignCenter);
    m_damageLogTitle->setGeometry(10, 5, 260, 25);

    // 关键修改：从8条→14条，间距从18→20，保证不挤
    for(int i=0;i<14;i++){
        QLabel* lb = new QLabel(m_damageLogWidget);
        lb->setStyleSheet("color:white; font-size:14px;");
        lb->setGeometry(10, 35 + i*20, 260, 18);
        lb->hide();
        m_damageLogLabels.append(lb);
    }

    // 后面的代码和你原来的完全一样，不用修改
    damageBroadcastLabel = new QLabel(this);
    damageBroadcastLabel->setAlignment(Qt::AlignCenter);
    damageBroadcastLabel->setStyleSheet(R"(
        font-size: 18px;
        font-weight: bold;
        color: #ffffff;
        background-color: rgba(0, 0, 0, 0.7);
        padding: 8px 20px;
        border-radius: 8px;
    )");
    damageBroadcastLabel->setGeometry(240, 15, 320, 40);
    damageBroadcastLabel->hide();

    gameLogic = new GameLogic();
    gameLogic->initActionCardDeck();
    gameLogic->drawInitialHand();

    for (int i = 0; i < 5; i++)
        selectedFlags << false;

    tooltipLabel = new QLabel(this);
    tooltipLabel->setStyleSheet("background-color:#1a1a1a; color:white; font-size:14px; padding:6px; border:1px solid yellow; border-radius:4px;");
    tooltipLabel->setVisible(false);
    tooltipLabel->setAlignment(Qt::AlignCenter);
    currentHoverIndex = -1;

    initHandUI();
    refreshHandUI();
    initSummonArea();

    mulliganBtn = new QPushButton("确认更换", this);
    mulliganBtn->setFixedSize(180, 50);
    mulliganBtn->move(width() / 2 - 90, height() - 80);
    mulliganBtn->setStyleSheet("font-size:18px; color:white; background-color:#c0392b; border:none; border-radius:8px;");
    connect(mulliganBtn, &QPushButton::clicked, this, &MainWindow::onMulliganBtnClicked);

    rollDiceBtn = new QPushButton("点击投骰子", this);
    rollDiceBtn->setFixedSize(200, 60);
    rollDiceBtn->move((width() - 200)/2, height()/2 - 30);
    rollDiceBtn->setStyleSheet("font-size:20px; color:white; background-color:#0099ff; border:none; border-radius:10px;");
    rollDiceBtn->setVisible(false);
    connect(rollDiceBtn, &QPushButton::clicked, this, &MainWindow::rollDice);

    reRollTipLabel = new QLabel(this);
    reRollTipLabel->setText("选择不想要的骰子重投");
    reRollTipLabel->setStyleSheet("color: yellow; font-size: 26px; font-weight:bold;");
    reRollTipLabel->setAlignment(Qt::AlignCenter);
    reRollTipLabel->setFixedWidth(600);
    reRollTipLabel->move((width() - 600)/2, height()/2 - 300);
    reRollTipLabel->setVisible(false);

    reRollBtn = new QPushButton("重投选中骰子", this);
    reRollBtn->setFixedSize(180, 50);
    reRollBtn->move(width()/2 + 280, height()/2 + 170);
    reRollBtn->setStyleSheet("font-size:18px; color:white; background-color:#ff6600; border:none; border-radius:8px;");
    reRollBtn->setVisible(false);
    connect(reRollBtn, &QPushButton::clicked, this, &MainWindow::reRollSelectedDice);

    currentRoleElement = ElementType::Any;

    enemyLabel = new QLabel(this);
    enemyLabel->setFixedSize(150, 200);
    enemyLabel->setScaledContents(true);
    enemyLabel->hide();

    playCardBtn = new QPushButton("打出卡牌", this);
    playCardBtn->setFixedSize(180, 60);
    playCardBtn->move(width() - 220, height() - 120);
    playCardBtn->setStyleSheet(R"(
        QPushButton{background-color:#666; color:white; font-size:18px; border:none; border-radius:10px;}
        QPushButton:disabled{background-color:#666;}
        QPushButton:enabled{background-color:#0099ff;}
    )");
    playCardBtn->setVisible(false);
    playCardBtn->setEnabled(false);
    connect(playCardBtn, &QPushButton::clicked, this, &MainWindow::onPlayCardClicked);

    switchRoleBtn = new QPushButton("切换角色", this);
    switchRoleBtn->setFixedSize(180, 60);
    switchRoleBtn->move(width() - 410, height() - 200);
    switchRoleBtn->setStyleSheet(R"(
        QPushButton{background-color:#666; color:white; font-size:18px; border:none; border-radius:10px;}
        QPushButton:disabled{background-color:#666;}
        QPushButton:enabled{background-color:#0099ff;}
    )");
    switchRoleBtn->setVisible(false);
    switchRoleBtn->setEnabled(false);
    connect(switchRoleBtn, &QPushButton::clicked, this, &MainWindow::onSwitchRoleClicked);

    endTurnBtn = new QPushButton("结束回合", this);
    endTurnBtn->setFixedSize(140, 60);
    endTurnBtn->move(30, height()/2 - 30);
    endTurnBtn->setStyleSheet(R"(
        QPushButton{background-color:#666; color:white; font-size:20px; border-radius:10px;}
        QPushButton:enabled{background-color:#0099ff;}
    )");
    endTurnBtn->setVisible(false);
    endTurnBtn->setEnabled(false);
    connect(endTurnBtn, &QPushButton::clicked, this, &MainWindow::onEndTurnClicked);

    turnTipLabel = new QLabel(this);
    turnTipLabel->setStyleSheet("color:gold; font-size:34px; font-weight:bold;");
    turnTipLabel->setAlignment(Qt::AlignCenter);
    turnTipLabel->setFixedWidth(800);
    turnTipLabel->move((width() - 800)/2, 30);
    turnTipLabel->setVisible(false);
    turnTipLabel->setText("轮到我方行动");

    btn_castSkill = new QPushButton("释放技能", this);
    btn_castSkill->setFixedSize(180, 60);
    btn_castSkill->move(width() - 220, height() - 200);
    btn_castSkill->setStyleSheet(R"(
        QPushButton{background-color:#666; color:white; font-size:18px; border:none; border-radius:10px;}
        QPushButton:disabled{background-color:#666;}
        QPushButton:enabled{background-color:#0099ff;}
    )");
    btn_castSkill->setVisible(false);
    btn_castSkill->setEnabled(false);
    connect(btn_castSkill, &QPushButton::clicked, this, &MainWindow::onCastSkillClicked);

    int skillBtnY = 600;
    int skillBtnW = 60;
    int skillBtnH = 60;
    int skillGap = 20;
    int skillStartX = width() - 330;

    auto makeSkillBtn = [&](const QString& text, int x) -> QPushButton* {
        QPushButton* btn = new QPushButton(text, this);
        btn->setFixedSize(skillBtnW, skillBtnH);
        btn->move(x, skillBtnY);
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #333;
                color: white;
                font-size: 20px;
                font-weight: bold;
                border: 2px solid #888;
                border-radius: 10px;
            }
        )");
        btn->setVisible(false);
        btn->installEventFilter(this);
        return btn;
    };

    btn_A = makeSkillBtn("A", skillStartX);
    btn_E = makeSkillBtn("E", skillStartX + skillBtnW + skillGap);
    btn_Q = makeSkillBtn("Q", skillStartX + (skillBtnW + skillGap)*2);

    connect(btn_A, &QPushButton::clicked, this, [=](){ onSkillBtnClicked(SkillType::A); });
    connect(btn_E, &QPushButton::clicked, this, [=](){ onSkillBtnClicked(SkillType::E); });
    connect(btn_Q, &QPushButton::clicked, this, [=](){ onSkillBtnClicked(SkillType::Q); });

    // ===================== 新增：游戏胜利/失败大字 =====================
    m_gameResultLabel = new QLabel(this);
    m_gameResultLabel->setFixedSize(600, 120);
    m_gameResultLabel->setAlignment(Qt::AlignCenter);
    m_gameResultLabel->setStyleSheet("font-size:60px; font-weight:bold;");
    m_gameResultLabel->hide();

    // 重新开始按钮
    m_restartBtn = new QPushButton("重新开始", this);
    m_restartBtn->setFixedSize(200, 60);
    m_restartBtn->setStyleSheet(R"(
    QPushButton{
    font-size:20px;
    background-color:#444;
    color:white;
    border:none;
    border-radius:10px;
    }
    QPushButton:hover{
    background-color:#666;
    }
)");
    m_restartBtn->hide();

    connect(m_restartBtn, &QPushButton::clicked, this, &MainWindow::onRestartGameClicked);
}

// ========== 伤害记录函数 ==========
void MainWindow::addDamageLog(const QString& text)
{
    m_damageLog.append(text);
    // 关键修改：从8条→14条，和UI显示数量一致
    while(m_damageLog.size() > 14)
        m_damageLog.removeFirst();

    // 清空所有标签
    for(auto* l : m_damageLogLabels) l->hide();

    // 按顺序显示
    for(int i=0;i<m_damageLog.size() && i<m_damageLogLabels.size();i++){
        m_damageLogLabels[i]->setText(m_damageLog[i]);
        m_damageLogLabels[i]->show();
    }
}

void MainWindow::initSummonArea()
{
    summonAreaWidget = new QWidget(this);
    summonAreaWidget->setStyleSheet("background-color:rgba(0,0,0,0.5); border-radius:10px; border:1px solid #ffd700;");
    summonAreaWidget->setFixedSize(280, 180);
    summonAreaWidget->move(30, height() - 210);
    summonAreaWidget->show();
    summonAreaWidget->raise();

    summonAreaTitle = new QLabel("召唤物 / 持续效果", summonAreaWidget);
    summonAreaTitle->setStyleSheet("color:#ffd700; font-size:16px; font-weight:bold;");
    summonAreaTitle->setAlignment(Qt::AlignCenter);
    summonAreaTitle->setGeometry(10, 5, 260, 25);

    for(int i=0;i<7;i++){
        QLabel* lb = new QLabel(summonAreaWidget);
        lb->setStyleSheet("color:white; font-size:14px;");
        lb->setGeometry(10, 35 + i*22, 260, 20);
        lb->hide();
        summonItemLabels.append(lb);
    }
}

void MainWindow::addSummon(SummonType type, int lastTurns)
{
    PlacedEntry e;
    e.type = PlacedEntry::Summon;
    e.summon = type;
    e.status = StatusEffectType::None;
    e.remainingTurns = lastTurns;
    placedEntries.append(e);
    refreshSummonDisplay();
}

void MainWindow::addStatusEffect(StatusEffectType type, int lastTurns)
{
    PlacedEntry e;
    e.type = PlacedEntry::Status;
    e.summon = SummonType::None;
    e.status = type;
    e.remainingTurns = lastTurns;
    placedEntries.append(e);
    refreshSummonDisplay();
}

void MainWindow::refreshSummonDisplay()
{
    for(auto* l : summonItemLabels) l->hide();
    for(int i=0;i<placedEntries.size() && i<summonItemLabels.size();i++){
        auto& e = placedEntries[i];
        QString s;
        if(e.type == PlacedEntry::Summon){
            switch(e.summon){
            case SummonType::StormEye: s="暴风之眼"; break;
            case SummonType::LargeWindSpirit: s="大型风灵"; break;
            case SummonType::StoneStele: s="岩脊"; break;
            case SummonType::JadeScreen: s="璇玑屏"; break;
            case SummonType::Oz: s="奥兹"; break;
            case SummonType::CuileinAnbar: s="柯里安巴"; break;
            case SummonType::SeaHorses: s="化海月"; break;
            default: s="未知";
            }
        }else{
            switch(e.status){
            case StatusEffectType::WestCathedral: s="西风大教堂"; break;
            case StatusEffectType::WangShuInn: s="望舒客栈"; break;
            default: s="未知效果";
            }
        }
        s += QString(" (%1回合)").arg(e.remainingTurns);
        summonItemLabels[i]->setText(s);
        summonItemLabels[i]->show();
    }
}

// ====================== 【修复】完整召唤物伤害 + 支援牌回血逻辑（你给的正确数值） ======================
void MainWindow::updatePlacedEntriesOnTurnEnd()
{
    qDebug() << "=== 回合结束，处理召唤物/支援牌 ===";

    QVector<int> indicesToRemove;

    for (int i = 0; i < placedEntries.size(); ++i)
    {
        PlacedEntry& entry = placedEntries[i];

        if (entry.type == PlacedEntry::Summon)
        {
            int dmg = 0;
            switch (entry.summon)
            {
            case SummonType::StormEye:      dmg = 2; break;
            case SummonType::LargeWindSpirit: dmg = 2; break;
            case SummonType::StoneStele:   dmg = 1; break;
            case SummonType::JadeScreen:   dmg = 1; break;
            case SummonType::Oz:            dmg = 1; break;
            case SummonType::CuileinAnbar: dmg = 2; break;
            case SummonType::SeaHorses:    dmg = 1; break;
            default: dmg = 0;
            }

            if (dmg > 0)
            {
                enemyHp = qMax(0, enemyHp - dmg);
                addDamageLog(QString("召唤物 对怪物造成 %1 点伤害").arg(dmg));
                showDamageBroadcast(false, dmg);
                updateRoleStatus();
                qDebug() << "召唤物造成" << dmg << "点伤害";
            }
        }

        if (entry.type == PlacedEntry::Status)
        {
            if (entry.status == StatusEffectType::WestCathedral)
            {
                if (currentActiveRoleIndex >= 0 && currentActiveRoleIndex < roleHp.size())
                {
                    roleHp[currentActiveRoleIndex] = qMin(10, roleHp[currentActiveRoleIndex] + 2);
                    addDamageLog("西风大教堂 治疗出战角色 2 点");
                    qDebug() << "西风大教堂 回血";
                }
            }
            else if (entry.status == StatusEffectType::WangShuInn)
            {
                int minHp = 999;
                int targetIdx = -1;
                for (int j = 0; j < roleHp.size(); ++j)
                {
                    if (j != currentActiveRoleIndex && roleHp[j] < minHp)
                    {
                        minHp = roleHp[j];
                        targetIdx = j;
                    }
                }
                if (targetIdx != -1)
                {
                    roleHp[targetIdx] = qMin(10, roleHp[targetIdx] + 2);
                    addDamageLog("望舒客栈 治疗后台角色 2 点");
                    qDebug() << "望舒客栈 回血";
                }
            }
        }
    }

    for (int i = 0; i < placedEntries.size(); ++i)
    {
        PlacedEntry& entry = placedEntries[i];
        entry.remainingTurns--;

        if (entry.remainingTurns <= 0)
        {
            indicesToRemove.append(i);
        }
    }

    std::sort(indicesToRemove.begin(), indicesToRemove.end(), std::greater<int>());
    for (int idx : indicesToRemove)
    {
        placedEntries.remove(idx);
    }

    updateRoleStatus();
    refreshSummonDisplay();

    checkEnemyDead();
    qDebug() << "=== 回合结束处理完成 ===";
}

void MainWindow::clearAllPlacedEntries()
{
    placedEntries.clear();
    refreshSummonDisplay();
}

void MainWindow::playCard_运筹帷幄()
{
    while (gameLogic->playerHand.size() < 5 && !gameLogic->actionDeck.isEmpty()) {
        gameLogic->playerHand.append(gameLogic->actionDeck.takeFirst());
    }
    refreshHandUI();
}

void MainWindow::playCard_西风大教堂()
{
    addStatusEffect(StatusEffectType::WestCathedral, 2);
}

void MainWindow::playCard_望舒客栈()
{
    addStatusEffect(StatusEffectType::WangShuInn, 2);
}

void MainWindow::playCard_交给我吧()
{
    m_nextSwitchFreeAction = true;
}

void MainWindow::playCard_换班时间()
{
    m_nextSwitchFreeDice = true;
}

void MainWindow::updateRoundDisplay()
{
    m_roundLabel->setText(QString("当前回合：%1").arg(m_currentRound));
    m_roundLabel->raise();
}

void MainWindow::onSkillBtnClicked(SkillType type)
{
    if (!isPlayerActionPhase || currentActiveRoleIndex < 0) return;

    if (type == SkillType::Q) {
        int maxE = getRoleQMaxEnergy(myTeam[currentActiveRoleIndex]);
        if (roleEnergy[currentActiveRoleIndex] < maxE) {
            return;
        }
    }

    if (selectedSkill == type) {
        selectedSkill = SkillType::None;
        skillSelected = false;
        btn_A->setStyleSheet("QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");
        btn_E->setStyleSheet("QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");
        btn_Q->setStyleSheet("QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");
        selectedDiceIndices.clear();
        for (auto* d : diceLabels) d->setStyleSheet("border:none;");
        btn_castSkill->setEnabled(false);
        return;
    }

    selectedSkill = type;
    skillSelected = true;

    btn_A->setStyleSheet(selectedSkill == SkillType::A ?
                             "QPushButton{background:#444; color:white; border:3px solid gold; border-radius:10px;}" :
                             "QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");

    btn_E->setStyleSheet(selectedSkill == SkillType::E ?
                             "QPushButton{background:#444; color:white; border:3px solid gold; border-radius:10px;}" :
                             "QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");

    btn_Q->setStyleSheet(selectedSkill == SkillType::Q ?
                             "QPushButton{background:#444; color:white; border:3px solid gold; border-radius:10px;}" :
                             "QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");

    selectedDiceIndices.clear();
    for (auto* d : diceLabels) d->setStyleSheet("border:none;");
    refreshSkillButtonState();
}

void MainWindow::refreshSkillButtonState()
{
    bool ok = checkDiceForCurrentSkill();
    btn_castSkill->setEnabled(ok);
}

bool MainWindow::checkDiceForCurrentSkill()
{
    if (!skillSelected || selectedSkill == SkillType::None || currentActiveRoleIndex < 0)
        return false;

    ElementType elem = getRoleElementType(myTeam[currentActiveRoleIndex]);
    int needElem = 0, needTotal = 0;

    if (selectedSkill == SkillType::A) {
        needElem = 1;
        needTotal = 3;
    } else if (selectedSkill == SkillType::E) {
        needElem = 3;
        needTotal = 3;
    } else if (selectedSkill == SkillType::Q) {
        auto id = myTeam[currentActiveRoleIndex];
        if (id == RoleID::Role_Bannite || id == RoleID::Role_Diluke || id == RoleID::Role_Kaiya) {
            needElem = 4;
        } else {
            needElem = 3;
        }
        needTotal = needElem;
    }

    int cntElem = 0;
    int cntAny = 0;

    for (int i : selectedDiceIndices) {
        ElementType t = (ElementType)diceLabels[i]->property("elemType").toInt();
        if (t == elem || t == ElementType::Any) {
            cntElem++;
        }
        cntAny++;
    }

    bool ok = (cntElem >= needElem) && (cntAny >= needTotal);
    return ok;
}

int MainWindow::getRoleQMaxEnergy(RoleID id)
{
    switch (id) {
    case RoleID::Role_Wendi:
    case RoleID::Role_ShaTang:
    case RoleID::Role_Bannite:
    case RoleID::Role_Kawei:
    case RoleID::Role_Kelai:
    case RoleID::Role_ShanHaiGong:
    case RoleID::Role_Kaiya:
        return 2;
    case RoleID::Role_Diluke:
    case RoleID::Role_Zhongli:
    case RoleID::Role_Ningguang:
    case RoleID::Role_Feixieer:
    case RoleID::Role_Keqing:
    case RoleID::Role_Babala:
    case RoleID::Role_Chongyun:
        return 3;
    default: return 2;
    }
}

void MainWindow::onCastSkillClicked()
{
    if (!isPlayerActionPhase || !skillSelected || selectedSkill == SkillType::None) return;

    consumeDiceForSkill();

    if (selectedSkill == SkillType::A) {
        castSkill_A();
        int maxE = getRoleQMaxEnergy(myTeam[currentActiveRoleIndex]);
        if (roleEnergy[currentActiveRoleIndex] < maxE)
            roleEnergy[currentActiveRoleIndex]++;
    }
    else if (selectedSkill == SkillType::E) {
        castSkill_E();
        int maxE = getRoleQMaxEnergy(myTeam[currentActiveRoleIndex]);
        if (roleEnergy[currentActiveRoleIndex] < maxE)
            roleEnergy[currentActiveRoleIndex]++;
    }
    else if (selectedSkill == SkillType::Q) {
        castSkill_Q();
        roleEnergy[currentActiveRoleIndex] = 0;
    }

    updateRoleStatus();
    doEnemyOneAction();

    selectedSkill = SkillType::None;
    skillSelected = false;
    btn_A->setStyleSheet("QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");
    btn_E->setStyleSheet("QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");
    btn_Q->setStyleSheet("QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");
    btn_castSkill->setEnabled(false);
}

void MainWindow::consumeDiceForSkill()
{
    QVector<QLabel*> newDice;
    for (int i = 0; i < diceLabels.size(); ++i) {
        if (!selectedDiceIndices.contains(i))
            newDice << diceLabels[i];
        else
            diceLabels[i]->deleteLater();
    }
    diceLabels = newDice;
    selectedDiceIndices.clear();
    arrangeDiceToRightSide();
}

void MainWindow::castSkill_A()
{
    RoleID id = myTeam[currentActiveRoleIndex];
    ElementType elem = getRoleElementType(id);

    if (elem == ElementType::Wind || elem == ElementType::Rock || elem == ElementType::Water) {
        dealElementDamage(elem, 1);
    } else {
        int atkBonus = roleAtk[currentActiveRoleIndex] - 2;
        int total = 2 + atkBonus;
        enemyHp = qMax(0, enemyHp - total);
        addDamageLog(QString("%1 对怪物造成 %2 点伤害").arg(getRoleName(id)).arg(total));
        showDamageBroadcast(false, total);
        updateRoleStatus();
    }
    checkEnemyDead();
}

void MainWindow::castSkill_E()
{
    RoleID id = myTeam[currentActiveRoleIndex];
    ElementType elem = getRoleElementType(id);

    if (id == RoleID::Role_Babala) {
        dealElementDamage(elem, 1);
        roleHp[currentActiveRoleIndex] = qMin(10, roleHp[currentActiveRoleIndex] + 2);
        addDamageLog("芭芭拉 治疗自身 2 点");
    }
    else if (id == RoleID::Role_ShanHaiGong) {
        dealElementDamage(elem, 0);
        addSummon(SummonType::SeaHorses,2);
    }
    else if (id == RoleID::Role_Zhongli) {
        dealElementDamage(elem, 1);
        addSummon(SummonType::StoneStele,2);
    }
    else if (id == RoleID::Role_Ningguang) {
        dealElementDamage(elem, 2);
        addSummon(SummonType::JadeScreen,2);
    }
    else if (id == RoleID::Role_Feixieer) {
        dealElementDamage(elem, 1);
        addSummon(SummonType::Oz,2);
    }
    else {
        dealElementDamage(elem, 3);
    }
    checkEnemyDead();
}

void MainWindow::castSkill_Q()
{
    RoleID id = myTeam[currentActiveRoleIndex];
    ElementType elem = getRoleElementType(id);

    if (id == RoleID::Role_Wendi)       {dealElementDamage(elem, 2); addSummon(SummonType::StormEye,2);}
    if (id == RoleID::Role_ShaTang)     {dealElementDamage(elem, 1); addSummon(SummonType::LargeWindSpirit,2);}
    if (id == RoleID::Role_Bannite)     dealElementDamage(elem, 2);
    if (id == RoleID::Role_Diluke)      dealElementDamage(elem, 8);
    if (id == RoleID::Role_Zhongli)     dealElementDamage(elem, 6);
    if (id == RoleID::Role_Ningguang)   dealElementDamage(elem, 6);
    if (id == RoleID::Role_Feixieer)    dealElementDamage(elem, 4);
    if (id == RoleID::Role_Keqing)      dealElementDamage(elem, 5);
    if (id == RoleID::Role_Kawei)       dealElementDamage(elem, 4);
    if (id == RoleID::Role_Kelai)       {dealElementDamage(elem, 2); addSummon(SummonType::CuileinAnbar,2);}
    if (id == RoleID::Role_Babala)      { for(int& hp : roleHp) hp = qMin(10, hp+4); updateRoleStatus(); addDamageLog("芭芭拉 全队治疗 4 点"); }
    if (id == RoleID::Role_ShanHaiGong) { dealElementDamage(elem,2); for(int& hp : roleHp) hp=qMin(10,hp+1); updateRoleStatus(); addDamageLog("心海 全队治疗 1 点"); }
    if (id == RoleID::Role_Chongyun)    dealElementDamage(elem,7);
    if (id == RoleID::Role_Kaiya)       dealElementDamage(elem,1);
    checkEnemyDead();
}

void MainWindow::showDamageBroadcast(bool isEnemyAttack, int damage)
{
    QString myName = getRoleName(myTeam[currentActiveRoleIndex]);
    QString msg;

    if (isEnemyAttack) {
        msg = QString("敌方流血狗对我方%1造成%2点伤害").arg(myName).arg(damage);
    } else {
        msg = QString("我方%1对敌方流血狗造成%2点伤害").arg(myName).arg(damage);
    }

    damageBroadcastLabel->setText(msg);
    damageBroadcastLabel->show();
    QTimer::singleShot(2000, this, [=](){ damageBroadcastLabel->hide(); });
}

void MainWindow::onEndTurnClicked()
{
    if (!isPlayerActionPhase) return;

    isPlayerActionPhase = false;
    endTurnBtn->setEnabled(false);
    turnTipLabel->setText("我方结束行动，敌方行动");

    QTimer::singleShot(800, this, [this]() {
        doEnemyFullAction();
    });
}

void MainWindow::doEnemyFullAction()
{
    if (enemyActionTimes >= 3) {
        enemyActionTimes = 0;
        turnTipLabel->setText("敌方行动结束，新回合开始");

        updatePlacedEntriesOnTurnEnd();

        QTimer::singleShot(1000, this, [this]() {
            startNewBigRound();
        });
        return;
    }

    enemyActionTimes++;

    if (enemyActionTimes == 1) {
        turnTipLabel->setText("敌方第1次行动：攻击");
        if (currentActiveRoleIndex >= 0) {
            roleHp[currentActiveRoleIndex] = qMax(0, roleHp[currentActiveRoleIndex] - 2);
            QString roleName = getRoleName(myTeam[currentActiveRoleIndex]);
            addDamageLog(QString("怪物 对%1造成 2 点伤害").arg(roleName));
            updateRoleStatus();
            showDamageBroadcast(true, 2);
            checkActiveRoleDead();
        }
    } else if (enemyActionTimes == 2) {
        turnTipLabel->setText("敌方第2次行动：攻击");
        if (currentActiveRoleIndex >= 0) {
            roleHp[currentActiveRoleIndex] = qMax(0, roleHp[currentActiveRoleIndex] - 2);
            QString roleName = getRoleName(myTeam[currentActiveRoleIndex]);
            addDamageLog(QString("怪物 对%1造成 2 点伤害").arg(roleName));
            updateRoleStatus();
            showDamageBroadcast(true, 2);
            checkActiveRoleDead();
        }
    } else if (enemyActionTimes == 3) {
        turnTipLabel->setText("敌方第3次行动：收尾");
    }

    QTimer::singleShot(800, this, [this]() {
        doEnemyFullAction();
    });
}

void MainWindow::doEnemyOneAction()
{
    if (enemyActionTimes >= 3) return;

    enemyActionTimes++;

    if (enemyActionTimes == 1) {
        turnTipLabel->setText("敌方第1次行动：攻击");
        if (currentActiveRoleIndex >= 0) {
            roleHp[currentActiveRoleIndex] = qMax(0, roleHp[currentActiveRoleIndex] - 2);
            QString roleName = getRoleName(myTeam[currentActiveRoleIndex]);
            addDamageLog(QString("怪物 对%1造成 2 点伤害").arg(roleName));
            updateRoleStatus();
            showDamageBroadcast(true, 2);
            checkActiveRoleDead();
        }
    } else if (enemyActionTimes == 2) {
        turnTipLabel->setText("敌方第2次行动：攻击");
        if (currentActiveRoleIndex >= 0) {
            roleHp[currentActiveRoleIndex] = qMax(0, roleHp[currentActiveRoleIndex] - 2);
            QString roleName = getRoleName(myTeam[currentActiveRoleIndex]);
            addDamageLog(QString("怪物 对%1造成 2 点伤害").arg(roleName));
            updateRoleStatus();
            showDamageBroadcast(true, 2);
            checkActiveRoleDead();
        }
    } else if (enemyActionTimes == 3) {
        turnTipLabel->setText("敌方第3次行动：收尾");
    }
}

// ====================== 【修复】新回合自动刷新按钮显示 ======================
void MainWindow::startNewBigRound()
{
    myTurnEnded = false;
    enemyTurnEnded = false;
    enemyActionTimes = 0;
    isPlayerActionPhase = true;

    m_currentRound++;
    updateRoundDisplay();

    qDeleteAll(diceLabels);
    diceLabels.clear();
    diceSelected.clear();
    selectedDiceIndices.clear();

    if (gameLogic->actionDeck.size() >= 2) {
        gameLogic->playerHand.append(gameLogic->actionDeck.takeFirst());
        gameLogic->playerHand.append(gameLogic->actionDeck.takeFirst());
    }
    refreshHandUI();

    turnTipLabel->setText("新回合开始，请掷骰子");
    endTurnBtn->setEnabled(false);
    rollDiceBtn->setVisible(true);

    arrangeDiceToRightSide();
}

MainWindow::ReactionType MainWindow::calculateReaction(ElementType damageElem, ElementType attachedElem)
{
    if (attachedElem == ElementType::Any || damageElem == attachedElem)
        return ReactionType::None;

    if ((damageElem == ElementType::Fire && attachedElem == ElementType::Water) ||
        (damageElem == ElementType::Water && attachedElem == ElementType::Fire))
        return ReactionType::Vaporize;

    if ((damageElem == ElementType::Fire && attachedElem == ElementType::Ice) ||
        (damageElem == ElementType::Ice && attachedElem == ElementType::Fire))
        return ReactionType::Melt;

    if ((damageElem == ElementType::Ice && attachedElem == ElementType::Water) ||
        (damageElem == ElementType::Water && attachedElem == ElementType::Ice))
        return ReactionType::Freeze;

    if ((damageElem == ElementType::Thunder && attachedElem == ElementType::Water) ||
        (damageElem == ElementType::Water && attachedElem == ElementType::Thunder))
        return ReactionType::Electro;

    if ((damageElem == ElementType::Thunder && attachedElem == ElementType::Grass) ||
        (damageElem == ElementType::Grass && attachedElem == ElementType::Thunder))
        return ReactionType::Quicken;

    if ((damageElem == ElementType::Water && attachedElem == ElementType::Grass) ||
        (damageElem == ElementType::Grass && attachedElem == ElementType::Water))
        return ReactionType::Bloom;

    if (damageElem == ElementType::Wind && attachedElem != ElementType::Any)
        return ReactionType::Swirl;

    if (damageElem == ElementType::Rock && attachedElem != ElementType::Any)
        return ReactionType::Crystallize;

    return ReactionType::None;
}

int MainWindow::applyReactionEffect(ReactionType react, ElementType damageElem, bool& isFrozen, bool& isElectro)
{
    int bonus = 0;
    switch (react) {
    case ReactionType::Vaporize:
    case ReactionType::Melt:      bonus = 2; break;
    case ReactionType::Freeze:    bonus = 1; isFrozen = true; break;
    case ReactionType::Electro:   bonus = 1; isElectro = true; break;
    case ReactionType::Quicken:
    case ReactionType::Bloom:     bonus = 2; break;
    case ReactionType::Swirl:     bonus = 1; break;
    case ReactionType::Crystallize: bonus = 1; break;
    default: break;
    }
    return bonus;
}

// ========== 攻击力加成核心函数 ==========
void MainWindow::dealElementDamage(ElementType damageElement, int baseDamage)
{
    int atkBonus = roleAtk[currentActiveRoleIndex] - 2;
    int realBase = baseDamage + atkBonus;

    ReactionType react = calculateReaction(damageElement, enemyAttachedElement);
    int bonus = applyReactionEffect(react, damageElement, enemyFrozen, enemyElectro);
    int totalDamage = realBase + bonus;

    enemyHp = qMax(0, enemyHp - totalDamage);

    QString roleName = getRoleName(myTeam[currentActiveRoleIndex]);
    addDamageLog(QString("%1 对怪物造成 %2 点伤害").arg(roleName).arg(totalDamage));

    showDamageBroadcast(false, totalDamage);
    enemyAttachedElement = damageElement;
    updateRoleStatus();
}

MainWindow::WeaponType MainWindow::getRoleWeaponType(RoleID id)
{
    switch(id)
    {
    case RoleID::Role_Wendi:       return MainWindow::WeaponType::Bow;
    case RoleID::Role_ShaTang:     return MainWindow::WeaponType::Catalyst;
    case RoleID::Role_Zhongli:     return MainWindow::WeaponType::Polearm;
    case RoleID::Role_Ningguang:   return MainWindow::WeaponType::Catalyst;
    case RoleID::Role_Feixieer:    return MainWindow::WeaponType::Bow;
    case RoleID::Role_Keqing:      return MainWindow::WeaponType::Sword;
    case RoleID::Role_Kawei:       return MainWindow::WeaponType::Claymore;
    case RoleID::Role_Kelai:       return MainWindow::WeaponType::Bow;
    case RoleID::Role_Babala:      return MainWindow::WeaponType::Catalyst;
    case RoleID::Role_ShanHaiGong: return MainWindow::WeaponType::Catalyst;
    case RoleID::Role_Bannite:     return MainWindow::WeaponType::Sword;
    case RoleID::Role_Diluke:      return MainWindow::WeaponType::Claymore;
    case RoleID::Role_Chongyun:    return MainWindow::WeaponType::Claymore;
    case RoleID::Role_Kaiya:       return MainWindow::WeaponType::Sword;
    default:                       return MainWindow::WeaponType::Unknown;
    }
}

QString MainWindow::getRoleElementName(RoleID id)
{
    switch(id)
    {
    case RoleID::Role_Wendi:       return "风系";
    case RoleID::Role_ShaTang:     return "风系";
    case RoleID::Role_Zhongli:     return "岩系";
    case RoleID::Role_Ningguang:   return "岩系";
    case RoleID::Role_Feixieer:    return "雷系";
    case RoleID::Role_Keqing:      return "雷系";
    case RoleID::Role_Kawei:       return "草系";
    case RoleID::Role_Kelai:       return "草系";
    case RoleID::Role_Babala:      return "水系";
    case RoleID::Role_ShanHaiGong: return "水系";
    case RoleID::Role_Bannite:     return "火系";
    case RoleID::Role_Diluke:      return "火系";
    case RoleID::Role_Chongyun:    return "冰系";
    case RoleID::Role_Kaiya:       return "冰系";
    default:                       return "未知";
    }
}

QString MainWindow::getRoleWeaponName(RoleID id)
{
    switch(id)
    {
    case RoleID::Role_Wendi:       return "弓箭";
    case RoleID::Role_ShaTang:     return "法器";
    case RoleID::Role_Zhongli:     return "长枪";
    case RoleID::Role_Ningguang:   return "法器";
    case RoleID::Role_Feixieer:    return "弓箭";
    case RoleID::Role_Keqing:      return "单手剑";
    case RoleID::Role_Kawei:       return "双手剑";
    case RoleID::Role_Kelai:       return "弓箭";
    case RoleID::Role_Babala:      return "法器";
    case RoleID::Role_ShanHaiGong: return "法器";
    case RoleID::Role_Bannite:     return "单手剑";
    case RoleID::Role_Diluke:      return "双手剑";
    case RoleID::Role_Chongyun:    return "双手剑";
    case RoleID::Role_Kaiya:       return "单手剑";
    default:                       return "未知";
    }
}

MainWindow::CardCostType MainWindow::getCardCostType(const QString& name)
{
    if (name == "白铁大剑" || name == "白缨枪" || name == "魔导绪论" ||
        name == "鸦羽弓" || name == "旅行剑" || name == "西风大教堂" || name == "望舒客栈")
        return Cost_Same2;

    if (name == "蒙德土豆饼" || name == "运筹帷幄" || name == "骑士团图书馆")
        return Cost_Any1;

    if (name == "星天之兆" || name == "最好的伙伴")
        return Cost_Any2;

    if (name == "甜甜花酿鸡" || name == "换班时间" || name == "交给我吧")
        return Cost_None;

    return Cost_None;
}

bool MainWindow::checkDiceMatchCost()
{
    QVector<ElementType> types;
    for (int i : selectedDiceIndices) {
        types << (ElementType)diceLabels[i]->property("elemType").toInt();
    }

    switch (currentCostType) {
    case Cost_None:  return true;
    case Cost_Any1:  return types.size() >= 1;
    case Cost_Any2:  return types.size() >= 2;
    case Cost_Same2:
        if (types.size() < 2) return false;
        ElementType main = types[0];
        for (ElementType t : types) {
            if (t == main || t == ElementType::Any) continue;
            return false;
        }
        return true;
    }
    return false;
}

void MainWindow::selectCardForPlay(int index)
{
    if (!isPlayerActionPhase) return;

    if (selectedCardIndex == index) {
        handLabels[index]->setStyleSheet("border:2px solid white;");
        selectedCardIndex = -1;
        selectedDiceIndices.clear();
        for (auto* d : diceLabels) d->setStyleSheet("border:none;");
        playCardBtn->setEnabled(false);
        return;
    }

    if (selectedCardIndex != -1) {
        handLabels[selectedCardIndex]->setStyleSheet("border:2px solid white;");
    }

    selectedCardIndex = index;
    handLabels[index]->setStyleSheet("border:4px solid gold;");
    ActionCard* card = gameLogic->playerHand[index];
    currentCostType = getCardCostType(card->getName());

    selectedDiceIndices.clear();
    for (auto* d : diceLabels) d->setStyleSheet("border:none;");

    playCardBtn->setEnabled(checkDiceMatchCost());
}

void MainWindow::selectDiceForPlay(int index)
{
    if (!isPlayerActionPhase) return;

    // 情况1：正在选技能骰子
    if (skillSelected && selectedSkill != SkillType::None) {
        if (selectedDiceIndices.contains(index)) {
            selectedDiceIndices.removeOne(index);
            diceLabels[index]->setStyleSheet("border:none;");
        } else {
            selectedDiceIndices.append(index);
            diceLabels[index]->setStyleSheet("border:3px solid gold; border-radius:8px;");
        }
        refreshSkillButtonState();
        return;
    }

    // 情况2：正在选切换角色骰子
    if (isSwitchingRole && selectedSwitchRoleIndex != -1) {
        // ==============================================
        // 免费切换时，禁止选骰子
        // ==============================================
        if (m_nextSwitchFreeDice) {
            return;
        }

        if (selectedDiceIndices.contains(index)) {
            selectedDiceIndices.removeOne(index);
            diceLabels[index]->setStyleSheet("border:none;");
        } else {
            selectedDiceIndices.append(index);
            diceLabels[index]->setStyleSheet("border:3px solid gold; border-radius:8px;");
        }
        switchRoleBtn->setEnabled(selectedDiceIndices.size() >= 1);
        return;
    }

    // 情况3：正在选打牌骰子
    if (selectedCardIndex == -1) return;

    if (selectedDiceIndices.contains(index)) {
        selectedDiceIndices.removeOne(index);
        diceLabels[index]->setStyleSheet("border:none;");
    } else {
        selectedDiceIndices.append(index);
        diceLabels[index]->setStyleSheet("border:3px solid gold; border-radius:8px;");
    }

    playCardBtn->setEnabled(checkDiceMatchCost());
}

void MainWindow::onPlayCardClicked()
{
    if (!isPlayerActionPhase || selectedCardIndex < 0 || selectedCardIndex >= gameLogic->playerHand.size()) return;

    QString cardName = gameLogic->playerHand[selectedCardIndex]->getName();
    gameLogic->playerHand.removeAt(selectedCardIndex);

    QVector<QLabel*> newDice;
    for (int i = 0; i < diceLabels.size(); ++i) {
        if (!selectedDiceIndices.contains(i)) newDice << diceLabels[i];
        else diceLabels[i]->deleteLater();
    }
    diceLabels = newDice;
    selectedDiceIndices.clear();

    if (cardName == "白铁大剑") {
        for (int i = 0; i < myTeam.size(); i++) {
            if (getRoleWeaponType(myTeam[i]) == WeaponType::Claymore) {
                roleAtk[i]++;
            }
        }
    }
    if (cardName == "白缨枪") {
        for (int i = 0; i < myTeam.size(); i++) {
            if (getRoleWeaponType(myTeam[i]) == WeaponType::Polearm) {
                roleAtk[i]++;
            }
        }
    }
    if (cardName == "鸦羽弓") {
        for (int i = 0; i < myTeam.size(); i++) {
            if (getRoleWeaponType(myTeam[i]) == WeaponType::Bow) {
                roleAtk[i]++;
            }
        }
    }
    if (cardName == "旅行剑") {
        for (int i = 0; i < myTeam.size(); i++) {
            if (getRoleWeaponType(myTeam[i]) == WeaponType::Sword) {
                roleAtk[i]++;
            }
        }
    }
    if (cardName == "魔导绪论") {
        for (int i = 0; i < myTeam.size(); i++) {
            if (getRoleWeaponType(myTeam[i]) == WeaponType::Catalyst) {
                roleAtk[i]++;
            }
        }
    }
    if (cardName == "甜甜花酿鸡") {
        if (currentActiveRoleIndex >= 0) {
            roleHp[currentActiveRoleIndex]++;
            addDamageLog("甜甜花酿鸡 治疗 1 点");
        }
    }
    if (cardName == "蒙德土豆饼") {
        if (currentActiveRoleIndex >= 0) {
            roleHp[currentActiveRoleIndex] += 2;
            addDamageLog("蒙德土豆饼 治疗 2 点");
        }
    }
    if (cardName == "星天之兆") {
        if (currentActiveRoleIndex >= 0)
            roleEnergy[currentActiveRoleIndex]++;
    }
    if (cardName == "最好的伙伴") {
        for (int i = 0; i < 2; i++) {
            QLabel* d = new QLabel(this);
            d->setFixedSize(80, 80);
            d->setScaledContents(true);
            d->installEventFilter(this);
            d->setPixmap(QPixmap("万能元素骰.jpg"));
            d->setProperty("elemType", (int)ElementType::Any);
            diceLabels.append(d);
            d->show();
        }
    }
    if (cardName == "骑士团图书馆") {
        reRollTipLabel->setVisible(true);
        reRollBtn->setVisible(true);
        diceSelected.fill(false, diceLabels.size());
    }
    if(cardName == "运筹帷幄") playCard_运筹帷幄();
    if(cardName == "西风大教堂") playCard_西风大教堂();
    if(cardName == "望舒客栈") playCard_望舒客栈();
    if(cardName == "交给我吧") playCard_交给我吧();
    if(cardName == "换班时间") playCard_换班时间();

    selectedCardIndex = -1;
    refreshHandUI();
    arrangeDiceToRightSide();
    updateRoleStatus();
    playCardBtn->setEnabled(false);
}

void MainWindow::onSwitchRoleClicked()
{
    if (!isPlayerActionPhase || selectedSwitchRoleIndex == -1 || !isSwitchingRole)
        return;

    bool freeAct = m_nextSwitchFreeAction;
    bool freeCost = m_nextSwitchFreeDice;
    m_nextSwitchFreeAction = false;
    m_nextSwitchFreeDice = false;

    // 【修复】只有不免费时才消耗骰子
    if (!freeCost && !selectedDiceIndices.isEmpty()) {
        QVector<QLabel*> newDice;
        int consumed = 0;
        for (int i = 0; i < diceLabels.size(); ++i) {
            // 切换角色只需要消耗1个骰子
            if (consumed < 1 && selectedDiceIndices.contains(i)) {
                diceLabels[i]->deleteLater();
                consumed++;
            } else {
                newDice << diceLabels[i];
            }
        }
        diceLabels = newDice;
    }

    int oldIdx = currentActiveRoleIndex;
    int newIdx = selectedSwitchRoleIndex;
    roleCards[oldIdx]->move(roleCards[oldIdx]->x(), roleCards[oldIdx]->y() + 5);
    roleCards[newIdx]->move(roleCards[newIdx]->x(), roleCards[newIdx]->y() - 5);

    QLabel* newRoleCard = roleCards[newIdx];
    QTimer* glowTimer = new QTimer(this);
    int glowCount = 0;
    connect(glowTimer, &QTimer::timeout, this, [=]() mutable {
        if (glowCount % 2 == 0)
            newRoleCard->setStyleSheet("border: 4px solid gold; border-radius: 8px;");
        else
            newRoleCard->setStyleSheet("border: 2px solid white;");
        glowCount++;
        if (glowCount > 6) {
            glowTimer->stop();
            glowTimer->deleteLater();
            newRoleCard->setStyleSheet("border: 2px solid white;");
        }
    });
    glowTimer->start(120);

    currentActiveRoleIndex = newIdx;
    currentRoleElement = getRoleElementType(myTeam[newIdx]);
    updateRoleStatusHighlight(newIdx);

    for (auto* card : roleCards)
        card->setStyleSheet("border: 2px solid white;");

    isSwitchingRole = false;
    selectedSwitchRoleIndex = -1;
    selectedDiceIndices.clear();
    switchRoleBtn->setEnabled(false);
    arrangeDiceToRightSide();

    if (!freeAct)
        doEnemyOneAction();
}

void MainWindow::setPlayerTeam(const QVector<RoleID>& team)
{
    myTeam = team;
    m_teamElements.clear();
    for (RoleID id : team) {
        m_teamElements.insert(getRoleElementType(id));
    }
}

QString MainWindow::getRoleName(RoleID id)
{
    switch(id)
    {
    case RoleID::Role_Babala:      return "芭芭拉";
    case RoleID::Role_Bannite:     return "班尼特";
    case RoleID::Role_Chongyun:    return "重云";
    case RoleID::Role_Diluke:      return "迪卢克";
    case RoleID::Role_Feixieer:    return "菲谢尔";
    case RoleID::Role_Kaiya:       return "凯亚";
    case RoleID::Role_Kawei:       return "卡维";
    case RoleID::Role_Kelai:       return "柯莱";
    case RoleID::Role_Keqing:      return "刻晴";
    case RoleID::Role_Ningguang:   return "凝光";
    case RoleID::Role_ShanHaiGong: return "珊瑚宫心海";
    case RoleID::Role_ShaTang:     return "砂糖";
    case RoleID::Role_Wendi:       return "温迪";
    case RoleID::Role_Zhongli:     return "钟离";
    default:                       return "未知角色";
    }
}

void MainWindow::initHandUI()
{
    QWidget *handWidget = new QWidget(this);
    handWidget->setObjectName("HandArea");
    handWidget->setFixedSize(1000, 180);
    handWidget->move((width() - handWidget->width()) / 2, height() - 220);

    QHBoxLayout *layout = new QHBoxLayout(handWidget);
    layout->setSpacing(20);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);

    for (int i = 0; i < 5; ++i)
    {
        QLabel *label = new QLabel;
        label->setFixedSize(120, 160);
        label->setStyleSheet("border:2px solid white;");
        label->installEventFilter(this);
        handLabels.append(label);
        layout->addWidget(label);
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == btn_A || watched == btn_E || watched == btn_Q) {
        QPushButton* btn = qobject_cast<QPushButton*>(watched);
        if (!btn) return false;

        if (event->type() == QEvent::Enter) {
            if (btn != btn_A || selectedSkill != SkillType::A)
                btn->setStyleSheet("QPushButton{background:#444; color:white; border:3px solid gold; border-radius:10px;}");
            return true;
        } else if (event->type() == QEvent::Leave) {
            if (btn == btn_A && selectedSkill != SkillType::A)
                btn->setStyleSheet("QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");
            if (btn == btn_E && selectedSkill != SkillType::E)
                btn->setStyleSheet("QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");
            if (btn == btn_Q && selectedSkill != SkillType::Q)
                btn->setStyleSheet("QPushButton{background:#333; color:white; border:2px solid #888; border-radius:10px;}");
            return true;
        }
    }

    for (int i = 0; i < handLabels.size(); i++)
    {
        if (watched == handLabels[i])
        {
            if (event->type() == QEvent::Enter)
            {
                if (gameLogic->canMulligan) showToolTip(i);
                return true;
            }
            else if (event->type() == QEvent::Leave)
            {
                hideToolTip();
                return true;
            }
            else if (event->type() == QEvent::MouseButtonPress)
            {
                if (gameLogic->canMulligan)
                {
                    selectedFlags[i] = !selectedFlags[i];
                    refreshHandUI();
                }
                else
                {
                    selectCardForPlay(i);
                }
                return true;
            }
        }
    }

    for (int i = 0; i < roleCards.size(); ++i)
    {
        QLabel *card = roleCards[i];
        if (watched == card)
        {
            if (event->type() == QEvent::Enter)
            {
                if (i != selectedSwitchRoleIndex)
                    card->setStyleSheet("border: 3px solid gold;");
                return true;
            }
            else if (event->type() == QEvent::Leave)
            {
                if (i != selectedSwitchRoleIndex)
                    card->setStyleSheet("border: 2px solid white;");
                return true;
            }
            else if (event->type() == QEvent::MouseButtonPress)
            {
                if (!roleSelectDone)
                {
                    card->move(card->x(), card->y() - 5);
                    roleSelectDone = true;
                    if (tipLabel) tipLabel->setVisible(false);
                    rollDiceBtn->setVisible(true);
                    currentRoleElement = getRoleElementType(myTeam[i]);
                    currentActiveRoleIndex = i;
                    updateRoleStatusHighlight(i);
                }
                else
                {
                    if (i != currentActiveRoleIndex)
                    {
                        if (selectedSwitchRoleIndex == i)
                        {
                            selectedSwitchRoleIndex = -1;
                            isSwitchingRole = false;
                            card->setStyleSheet("border: 2px solid white;");
                            selectedDiceIndices.clear();
                            switchRoleBtn->setEnabled(false);
                            for (auto* d : diceLabels) d->setStyleSheet("border:none;");
                        }
                        else
                        {
                            if (selectedSwitchRoleIndex != -1)
                                roleCards[selectedSwitchRoleIndex]->setStyleSheet("border: 2px solid white;");

                            selectedSwitchRoleIndex = i;
                            isSwitchingRole = true;
                            card->setStyleSheet("border: 4px solid red; background-color:rgba(255,0,0,30);");

                            selectedDiceIndices.clear();
                            // ========== 核心修复：换班时间生效，直接点亮切换按钮 ==========
                            if (m_nextSwitchFreeDice) {
                                switchRoleBtn->setEnabled(true);
                            } else {
                                switchRoleBtn->setEnabled(false);
                            }

                            for (auto* d : diceLabels) d->setStyleSheet("border:none;");
                        }
                    }
                }
                return true;
            }
        }
    }

    for (int i = 0; i < diceLabels.size(); ++i)
    {
        QLabel *dice = diceLabels[i];
        if (watched == dice)
        {
            if (event->type() == QEvent::Enter)
            {
                dice->setStyleSheet("border: 3px solid gold; border-radius:8px;");
                return true;
            }
            else if (event->type() == QEvent::Leave)
            {
                if (reRollBtn->isVisible()) {
                    dice->setStyleSheet(diceSelected[i] ? "border: 3px solid gold; border-radius:8px;" : "border: none;");
                } else {
                    if (!selectedDiceIndices.contains(i))
                        dice->setStyleSheet("border: none;");
                }
                return true;
            }
            else if (event->type() == QEvent::MouseButtonPress)
            {
                if (reRollBtn->isVisible())
                {
                    diceSelected[i] = !diceSelected[i];
                    dice->setStyleSheet(diceSelected[i] ? "border: 3px solid gold; border-radius:8px;" : "border: none;");
                }
                else {
                    selectDiceForPlay(i);
                }
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

ElementType MainWindow::getRoleElementType(RoleID id)
{
    switch(id)
    {
    case RoleID::Role_Diluke:
    case RoleID::Role_Bannite:
        return ElementType::Fire;
    case RoleID::Role_Kelai:
    case RoleID::Role_Kawei:
        return ElementType::Grass;
    case RoleID::Role_ShaTang:
    case RoleID::Role_Wendi:
        return ElementType::Wind;
    case RoleID::Role_Zhongli:
    case RoleID::Role_Ningguang:
        return ElementType::Rock;
    case RoleID::Role_Chongyun:
    case RoleID::Role_Kaiya:
        return ElementType::Ice;
    case RoleID::Role_Feixieer:
    case RoleID::Role_Keqing:
        return ElementType::Thunder;
    case RoleID::Role_Babala:
    case RoleID::Role_ShanHaiGong:
        return ElementType::Water;
    default:
        return ElementType::Any;
    }
}

void MainWindow::rollDice()
{
    rollDiceBtn->setVisible(false);

    for (QLabel* lab : diceLabels) lab->deleteLater();
    diceLabels.clear();
    diceSelected.clear();

    QList<QPair<QString, ElementType>> diceTypeMap = {
        {"冰元素骰.jpg", ElementType::Ice},
        {"草元素骰.jpg", ElementType::Grass},
        {"风元素骰.jpg", ElementType::Wind},
        {"火元素骰.jpg", ElementType::Fire},
        {"雷元素骰.jpg", ElementType::Thunder},
        {"水元素骰.jpg", ElementType::Water},
        {"万能元素骰.jpg", ElementType::Any},
        {"岩元素骰.jpg", ElementType::Rock}
    };

    for (int i = 0; i < 8; ++i)
    {
        int randType = QRandomGenerator::global()->bounded(diceTypeMap.size());
        QString path = diceTypeMap[randType].first;
        ElementType elem = diceTypeMap[randType].second;

        QLabel* diceLab = new QLabel(this);
        diceLab->setFixedSize(80, 80);
        diceLab->setScaledContents(true);
        diceLab->installEventFilter(this);

        QPixmap pix(path);
        if (!pix.isNull()) {
            diceLab->setPixmap(pix);
            diceLab->setProperty("elemType", (int)elem);
        } else {
            diceLab->setStyleSheet("color:white; font-size:24px;");
            diceLab->setText("?");
            diceLab->setAlignment(Qt::AlignCenter);
        }

        int startX = width() - 85;
        int startY = 110;
        diceLab->move(startX, startY + i * (80 + 8));
        diceLab->show();

        diceLabels.append(diceLab);
        diceSelected.append(false);
    }

    reRollTipLabel->setVisible(true);
    reRollBtn->setVisible(true);
}

void MainWindow::reRollSelectedDice()
{
    reRollTipLabel->setVisible(false);
    reRollBtn->setVisible(false);

    QList<QPair<QString, ElementType>> diceTypeMap = {
        {"冰元素骰.jpg", ElementType::Ice},
        {"草元素骰.jpg", ElementType::Grass},
        {"风元素骰.jpg", ElementType::Wind},
        {"火元素骰.jpg", ElementType::Fire},
        {"雷元素骰.jpg", ElementType::Thunder},
        {"水元素骰.jpg", ElementType::Water},
        {"万能元素骰.jpg", ElementType::Any},
        {"岩元素骰.jpg", ElementType::Rock}
    };

    for (int i = 0; i < diceLabels.size(); ++i)
    {
        if (diceSelected[i])
        {
            int randType = QRandomGenerator::global()->bounded(diceTypeMap.size());
            QString path = diceTypeMap[randType].first;
            ElementType elem = diceTypeMap[randType].second;

            QPixmap pix(path);
            if (!pix.isNull()) {
                diceLabels[i]->setPixmap(pix);
                diceLabels[i]->setProperty("elemType", (int)elem);
            }
        }
        diceSelected[i] = false;
        diceLabels[i]->setStyleSheet("border: none;");
    }

    QTimer::singleShot(1500, this, [=]() {
        arrangeDiceToRightSide();
    });
}

void MainWindow::arrangeDiceToRightSide()
{
    struct Dice {
        QLabel* label;
        ElementType elem;
    };
    QVector<Dice> list;

    for (auto* d : diceLabels) {
        ElementType elem = (ElementType)d->property("elemType").toInt();
        list.push_back({d, elem});
    }

    std::sort(list.begin(), list.end(), [this](const Dice& a, const Dice& b) {
        auto getPrio = [this](ElementType e) -> int {
            if (e == ElementType::Any)      return 0;
            if (m_teamElements.contains(e)) return 1;
            return 2;
        };

        int pa = getPrio(a.elem);
        int pb = getPrio(b.elem);

        if (pa != pb) return pa < pb;
        return (int)a.elem < (int)b.elem;
    });

    int x = width() - 85;
    int y = 110;
    int spacing = 8;

    for (int i = 0; i < list.size(); ++i) {
        list[i].label->move(x, y + i * (80 + spacing));
    }

    roleStatusPanel->setVisible(true);
    playCardBtn->setVisible(true);
    playCardBtn->setEnabled(false);
    switchRoleBtn->setVisible(true);
    switchRoleBtn->setEnabled(false);

    btn_A->setVisible(true);
    btn_E->setVisible(true);
    btn_Q->setVisible(true);
    btn_castSkill->setVisible(true);

    turnTipLabel->setVisible(true);
    endTurnBtn->setVisible(true);
    endTurnBtn->setEnabled(isPlayerActionPhase);
}

void MainWindow::showToolTip(int index)
{
    currentHoverIndex = index;

    ActionCard* card = gameLogic->playerHand[index];
    QString desc = getCardDesc(card->getName());

    tooltipLabel->setText(desc);
    tooltipLabel->adjustSize();

    QPoint pos = handLabels[index]->mapToGlobal(QPoint(0, -40));
    pos = this->mapFromGlobal(pos);
    tooltipLabel->move(pos);
    tooltipLabel->setVisible(true);
}

void MainWindow::hideToolTip()
{
    tooltipLabel->setVisible(false);
    currentHoverIndex = -1;
}

QString MainWindow::getCardDesc(const QString& name)
{
    if (name == "白铁大剑") return "白铁大剑：角色造成的伤害+1。（双手剑角色才能装备。角色最多装备1件武器）";
    if (name == "白缨枪") return "白缨枪：角色造成的伤害+1。（长柄角色才能装备。角色最多装备1件武器）";
    if (name == "鸦羽弓") return "鸦羽弓：角色造成的伤害+1。（弓角色才能装备。角色最多装备1件武器）";
    if (name == "魔导绪论") return "魔导绪论：角色造成的伤害+1。（法器角色才能装备。角色最多装备1件武器）";
    if (name == "旅行剑") return "旅行剑：角色造成的伤害+1。（单手剑角色才能装备。角色最多装备1件武器）";
    if (name == "甜甜花酿鸡") return "甜甜花酿鸡：治疗目标角色1点。（每回合每个角色最多食用1次料理）";
    if (name == "蒙德土豆饼") return "蒙德土豆饼：治疗目标角色2点。（每回合每个角色最多食用1次料理）";
    if (name == "星天之兆") return "星天之兆：我方当前出战角色获得1点充能。";
    if (name == "最好的伙伴") return "最好的伙伴：将所花费的2个元素骰转化为2个万能元素骰。";
    if (name == "骑士团图书馆") return "骑士团图书馆：入场时：选择任意元素骰重投。投掷阶段：获得额外一次重投机会。";
    if (name == "换班时间") return "换班时间：下次切换角色时不消耗元素骰。";
    if (name == "西风大教堂") return "西风大教堂：回合结束治疗出战角色2点，持续2回合。";
    if (name == "望舒客栈") return "望舒客栈：回合结束治疗后台最低血量角色2点，持续2回合。";
    if (name == "运筹帷幄") return "运筹帷幄：将手牌补至5张。";
    if (name == "交给我吧") return "交给我吧：下次切换角色不消耗行动次数。";
    return "暂无卡牌介绍";
}

void MainWindow::refreshHandUI()
{
    if (!gameLogic) return;

    for (int i = 0; i < handLabels.size(); ++i)
    {
        if (i < gameLogic->playerHand.size())
        {
            ActionCard *card = gameLogic->playerHand[i];
            QString path = card->getName() + ".jpg";

            QPixmap pix(path);
            if (!pix.isNull())
            {
                handLabels[i]->setPixmap(pix.scaled(handLabels[i]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                handLabels[i]->setScaledContents(true);
            }
            else
            {
                handLabels[i]->setText(card->getName());
                handLabels[i]->setScaledContents(false);
            }

            if (gameLogic->canMulligan && selectedFlags[i])
                handLabels[i]->setStyleSheet("border:4px solid gold;");
            else
                handLabels[i]->setStyleSheet("border:2px solid white;");
        }
        else
        {
            handLabels[i]->clear();
            handLabels[i]->setStyleSheet("border:2px solid white;");
        }
    }
}

void MainWindow::selectCard(int index)
{
    if (!gameLogic->canMulligan) return;
    selectedFlags[index] = !selectedFlags[index];
}

void MainWindow::createEnemyCharacter()
{
    QPixmap enemyPix("流血狗.jpg");
    if (!enemyPix.isNull())
    {
        enemyLabel->setPixmap(enemyPix);
    }
    else
    {
        enemyLabel->setText("流血狗");
        enemyLabel->setStyleSheet("color:white; font-size:18px; border:2px solid red; background:#333;");
        enemyLabel->setAlignment(Qt::AlignCenter);
    }

    enemyLabel->raise();
    enemyLabel->move( (width() - 150)/2, 220 );
    enemyLabel->show();
}

void MainWindow::onMulliganBtnClicked()
{
    hideToolTip();

    if (!gameLogic->canMulligan)
    {
        mulliganBtn->setText("已更换");
        mulliganBtn->setStyleSheet("background-color:#7f8c8d;");
        return;
    }

    QVector<int> indexes;
    for (int i = 0; i < 5; i++)
        if (selectedFlags[i]) indexes << i;

    gameLogic->mulligan(indexes);
    selectedFlags.fill(false);
    refreshHandUI();

    mulliganBtn->setText("已更换");
    mulliganBtn->setStyleSheet("background-color:#7f8c8d;");
    mulliganBtn->setEnabled(false);

    QTimer::singleShot(1000, this, [=]()
                       {
                           mulliganBtn->setVisible(false);

                           QWidget *handArea = findChild<QWidget*>("HandArea");
                           if (handArea)
                           {
                               QHBoxLayout *ly = qobject_cast<QHBoxLayout*>(handArea->layout());
                               ly->setSpacing(0);
                               ly->setContentsMargins(0,0,0,0);
                               handArea->setFixedSize(600, 160);
                               handArea->move((width() - 600)/2, height() - 180);
                           }

                           QWidget *teamWidget = new QWidget(this);
                           teamWidget->setFixedSize(600, 210);
                           teamWidget->move((width() - 600)/2, height() - 425);

                           QHBoxLayout *teamLayout = new QHBoxLayout(teamWidget);
                           teamLayout->setSpacing(20);
                           teamLayout->setAlignment(Qt::AlignCenter);
                           teamLayout->setContentsMargins(0,0,0,0);

                           roleCards.clear();
                           roleSelectDone = false;

                           for (const auto& rid : myTeam)
                           {
                               QLabel *lb = new QLabel;
                               lb->setFixedSize(150, 200);
                               lb->setStyleSheet("border:2px solid white;");
                               lb->installEventFilter(this);
                               roleCards.append(lb);

                               QString roleImgName;
                               switch(rid)
                               {
                               case RoleID::Role_Babala:      roleImgName = "babala.jpg";          break;
                               case RoleID::Role_Bannite:     roleImgName = "bannite.jpg";         break;
                               case RoleID::Role_Chongyun:    roleImgName = "chongyun.jpg";        break;
                               case RoleID::Role_Diluke:      roleImgName = "diluke.jpg";          break;
                               case RoleID::Role_Feixieer:    roleImgName = "feixieer.jpg";        break;
                               case RoleID::Role_Kaiya:       roleImgName = "kaiya.jpg";           break;
                               case RoleID::Role_Kawei:       roleImgName = "kawei.jpg";           break;
                               case RoleID::Role_Kelai:       roleImgName = "kelai.jpg";           break;
                               case RoleID::Role_Keqing:      roleImgName = "keqing.jpg";          break;
                               case RoleID::Role_Ningguang:   roleImgName = "ningguang.jpg";       break;
                               case RoleID::Role_ShanHaiGong: roleImgName = "shanhugongxinhai.jpg";break;
                               case RoleID::Role_ShaTang:     roleImgName = "shatang.jpg";         break;
                               case RoleID::Role_Wendi:       roleImgName = "wendi.jpg";           break;
                               case RoleID::Role_Zhongli:     roleImgName = "zhongli.jpg";         break;
                               default:                       roleImgName = ""; break;
                               }

                               QPixmap p(roleImgName);
                               if (!p.isNull())
                               {
                                   lb->setPixmap(p.scaled(lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                                   lb->setScaledContents(true);
                               }
                               else
                               {
                                   lb->setText("角色");
                                   lb->setStyleSheet("color:white; font-size:16px; border:2px solid white; background:rgba(0,0,0,180);");
                                   lb->setAlignment(Qt::AlignCenter);
                               }

                               teamLayout->addWidget(lb);
                           }

                           teamWidget->show();

                           tipLabel = new QLabel(this);
                           tipLabel->setText("请选择初始出战角色");
                           tipLabel->setStyleSheet("color: yellow; font-size: 28px; font-weight: bold;");
                           tipLabel->setAlignment(Qt::AlignCenter);
                           tipLabel->setFixedWidth(600);
                           tipLabel->move((width() - 600)/2, 440);
                           tipLabel->show();

                           createEnemyCharacter();
                           createRoleStatusPanel();
                           updateRoleStatus();
                       });
}

void MainWindow::createRoleStatusPanel()
{
    roleHp.clear();
    roleAtk.clear();
    roleEnergy.clear();
    for (int i = 0; i < 3; i++) {
        roleHp << 10;
        roleAtk << 2;
        roleEnergy << 0;
    }

    roleStatusPanel = new QWidget(this);
    roleStatusPanel->setFixedSize(300, 480);
    roleStatusPanel->setStyleSheet(R"(
        QWidget {
            background-color: #1a1a1a;
            border: 2px solid #ffd700;
            border-radius: 10px;
        }
    )");
    roleStatusPanel->move(width() - 360, 110);
    roleStatusPanel->setVisible(false);

    QVBoxLayout *mainLayout = new QVBoxLayout(roleStatusPanel);
    mainLayout->setSpacing(18);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setAlignment(Qt::AlignTop);

    QLabel *title = new QLabel("角色状态");
    title->setStyleSheet("color: #ffd700; font-size: 22px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QWidget *enemyItem = new QWidget;
    QVBoxLayout *enemyVLayout = new QVBoxLayout(enemyItem);
    enemyVLayout->setSpacing(4);
    enemyVLayout->setContentsMargins(0,0,0,0);

    QWidget *enemyTopRow = new QWidget;
    QHBoxLayout *enemyLy = new QHBoxLayout(enemyTopRow);
    enemyLy->setSpacing(6);
    enemyLy->setContentsMargins(0,0,0,0);

    QLabel *enemyNameLabel = new QLabel("流血狗");
    enemyNameLabel->setStyleSheet("color: #ff4444; font-size: 15px; font-weight: bold;");
    enemyNameLabel->setFixedWidth(60);

    enemyHpLabel = new QLabel();
    enemyHpLabel->setStyleSheet("color: #ff5555; font-size: 15px; font-weight: bold;");

    enemyAtkLabel = new QLabel();
    enemyAtkLabel->setStyleSheet("color: #ffaa00; font-size: 15px; font-weight: bold;");

    enemyEnergyLabel = new QLabel();
    enemyEnergyLabel->setStyleSheet("color: #55aaff; font-size: 15px; font-weight: bold;");

    enemyLy->addWidget(enemyNameLabel);
    enemyLy->addWidget(enemyHpLabel);
    enemyLy->addWidget(enemyAtkLabel);
    enemyLy->addStretch();
    enemyLy->addWidget(enemyEnergyLabel);

    QLabel *enemyDesc = new QLabel("魔物 · 无元素");
    enemyDesc->setStyleSheet("color:#cccccc; font-size:13px;");
    enemyVLayout->addWidget(enemyTopRow);
    enemyVLayout->addWidget(enemyDesc);
    mainLayout->addWidget(enemyItem);

    roleNameLabels.clear();
    roleHpLabels.clear();
    roleAtkLabels.clear();
    roleElementLabels.clear();
    roleWeaponLabels.clear();
    roleEnergyLabels.clear();

    for (int i = 0; i < 3; i++) {
        QWidget *item = new QWidget;
        QVBoxLayout *itemVLayout = new QVBoxLayout(item);
        itemVLayout->setSpacing(4);
        itemVLayout->setContentsMargins(0,0,0,0);

        QWidget *topRow = new QWidget;
        QHBoxLayout *topLy = new QHBoxLayout(topRow);
        topLy->setSpacing(6);
        topLy->setContentsMargins(0,0,0,0);

        QLabel *nameLab = new QLabel(getRoleName(myTeam[i]));
        nameLab->setStyleSheet("color: white; font-size: 15px;");
        nameLab->setFixedWidth(60);
        roleNameLabels.append(nameLab);

        QLabel *hpLab = new QLabel();
        hpLab->setStyleSheet("color: #ff5555; font-size: 15px; font-weight: bold;");
        roleHpLabels.append(hpLab);

        QLabel *atkLab = new QLabel();
        atkLab->setStyleSheet("color: #ffaa00; font-size: 15px; font-weight: bold;");
        roleAtkLabels.append(atkLab);

        QLabel *energyLab = new QLabel();
        energyLab->setStyleSheet("color: #55aaff; font-size: 15px; font-weight: bold;");
        roleEnergyLabels.append(energyLab);

        topLy->addWidget(nameLab);
        topLy->addWidget(hpLab);
        topLy->addWidget(atkLab);
        topLy->addStretch();
        topLy->addWidget(energyLab);

        QWidget *infoRow = new QWidget;
        QHBoxLayout *infoLy = new QHBoxLayout(infoRow);
        infoLy->setSpacing(8);
        infoLy->setContentsMargins(0,0,0,0);

        QLabel *elemLab = new QLabel(getRoleElementName(myTeam[i]));
        elemLab->setStyleSheet("color: #4fc3f7; font-size:13px;");
        roleElementLabels.append(elemLab);

        QLabel *weaponLab = new QLabel(getRoleWeaponName(myTeam[i]));
        weaponLab->setStyleSheet("color: #ffd54f; font-size:13px;");
        roleWeaponLabels.append(weaponLab);

        infoLy->addWidget(elemLab);
        infoLy->addWidget(weaponLab);
        infoLy->addStretch();

        itemVLayout->addWidget(topRow);
        itemVLayout->addWidget(infoRow);
        mainLayout->addWidget(item);
    }

    roleStatusPanel->show();
}

void MainWindow::updateRoleStatus()
{
    enemyHpLabel->setText(QString("❤️ %1").arg(enemyHp));
    enemyAtkLabel->setText(QString("⚔️ %1").arg(enemyAtk));
    enemyEnergyLabel->setText(QString("⚡ %1").arg(enemyEnergy));

    for (int i = 0; i < 3; i++) {
        roleHpLabels[i]->setText(QString("❤️ %1").arg(roleHp[i]));
        roleAtkLabels[i]->setText(QString("⚔️ %1").arg(roleAtk[i]));
        roleEnergyLabels[i]->setText(QString("⚡ %1").arg(roleEnergy[i]));
    }
}

void MainWindow::updateRoleStatusHighlight(int activeIndex)
{
    for (int i = 0; i < roleNameLabels.size(); i++) {
        if (i == activeIndex) {
            roleNameLabels[i]->setStyleSheet("color: #ffd700; font-size: 16px; font-weight: bold;");
        } else {
            roleNameLabels[i]->setStyleSheet("color: white; font-size: 16px;");
        }
    }
}

// ===================== 新增：阵亡/胜利/失败 完整机制 =====================
void MainWindow::checkActiveRoleDead()
{
    if(currentActiveRoleIndex < 0) return;
    if(roleHp[currentActiveRoleIndex] > 0) return;

    QString deadName = getRoleName(myTeam[currentActiveRoleIndex]);
    addDamageLog(deadName + " 已阵亡！");
    damageBroadcastLabel->setText(deadName + " 已阵亡！");
    damageBroadcastLabel->show();

    switchToHighestHpBackRole();
}

void MainWindow::checkEnemyDead()
{
    if(enemyHp <= 0)
    {
        addDamageLog("敌方流血狗已阵亡！");
        showGameResult(true);
    }
}

void MainWindow::hideGameResult()
{
    m_gameResultLabel->hide();
    m_restartBtn->hide();
}

void MainWindow::switchToHighestHpBackRole()
{
    int maxHp = -1;
    int targetIdx = -1;

    for (int i = 0; i < 3; ++i)
    {
        if (i == currentActiveRoleIndex) continue;
        if (roleHp[i] <= 0) continue;

        if (roleHp[i] > maxHp)
        {
            maxHp = roleHp[i];
            targetIdx = i;
        }
    }

    if (targetIdx != -1)
    {
        QString name = getRoleName(myTeam[targetIdx]);
        addDamageLog(QString("自动切换至 %1").arg(name));
        currentActiveRoleIndex = targetIdx;
        updateRoleStatusHighlight(targetIdx);
        updateRoleStatus();
    }

    checkAllRoleDead();
}

void MainWindow::checkAllRoleDead()
{
    bool allDead = true;
    for (int i = 0; i < 3; ++i)
    {
        if (roleHp[i] > 0)
        {
            allDead = false;
            break;
        }
    }

    if (allDead)
    {
        showGameResult(false);
    }
}

void MainWindow::showGameResult(bool isWin)
{
    if (isWin) {
        m_gameResultLabel->setText("游戏胜利");
        m_gameResultLabel->setStyleSheet("font-size:60px; font-weight:bold; color:#ffd700;");
    } else {
        m_gameResultLabel->setText("游戏失败");
        m_gameResultLabel->setStyleSheet("font-size:60px; font-weight:bold; color:#888888;");
    }

    // 把文字放在窗口正中央
    m_gameResultLabel->setGeometry(this->width()/2 - 200, this->height()/2 - 100, 400, 80);
    m_gameResultLabel->show();
    m_gameResultLabel->raise(); // 强制置顶

    // 显示并置顶重新开始按钮
    m_restartBtn->show();
    m_restartBtn->raise();
}

void MainWindow::onRestartGameClicked()
{
    m_damageLog.clear();
    for (auto* lb : m_damageLogLabels) { lb->setText(""); lb->hide(); }
    hideGameResult();

    enemyHp = 20;
    enemyAtk = 2;
    enemyEnergy = 0;
    enemyAttachedElement = ElementType::Any;
    enemyFrozen = false;
    enemyElectro = false;

    myTurnEnded = false;
    enemyTurnEnded = false;
    enemyActionTimes = 0;
    isPlayerActionPhase = true;
    m_currentRound = 1;
    currentActiveRoleIndex = -1;

    for(int i=0;i<3;i++){
        roleHp[i] = 10;
        roleAtk[i] = 2;
        roleEnergy[i] = 0;
    }

    placedEntries.clear();
    refreshSummonDisplay();
    updateRoleStatus();

    this->close();
    BgSelectWindow* w = new BgSelectWindow;
    w->show();
}

MainWindow::~MainWindow()
{
    delete gameLogic;
}