#ifndef TEAMSELECTDIALOG_H
#define TEAMSELECTDIALOG_H

#include <QDialog>
#include <QVector>
#include <QPair>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include "Card.h"

class TeamSelectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TeamSelectDialog(QWidget *parent = nullptr);
    QVector<RoleID> getSelectedTeam() const;

private slots:
    void onRoleClicked(RoleID id);
    void onRandomClicked();
    void onConfirmClicked();
    void showRolePreview(RoleID id);
    void updateUI();

private:
    QLabel *m_countLabel;
    QGridLayout *m_gridLayout;
    QLabel *m_previewImg;
    QLabel *m_previewName;
    QLabel *m_previewElem;

    // 新增：角色技能介绍标签（显示在角色右侧空白处）
    QLabel *m_skillDescription;

    QVector<QPushButton*> m_roleButtons;
    QVector<RoleID> m_selectedTeam;

    // 改成中文界面名 ↓（图片还是用原来的英文文件名，不用改！）
    QVector<QPair<RoleID, QString>> m_allRoles = {
        {Role_Wendi,      "温迪"},
        {Role_ShaTang,    "砂糖"},
        {Role_Zhongli,    "钟离"},
        {Role_Ningguang,  "凝光"},
        {Role_Feixieer,   "菲谢尔"},
        {Role_Keqing,     "刻晴"},
        {Role_Kawei,      "卡维"},
        {Role_Kelai,      "柯莱"},
        {Role_Babala,     "芭芭拉"},
        {Role_ShanHaiGong,"珊瑚宫心海"},
        {Role_Bannite,    "班尼特"},
        {Role_Diluke,     "迪卢克"},
        {Role_Chongyun,   "重云"},
        {Role_Kaiya,      "凯亚"}
    };

    QString getRoleElementName(RoleID id);

    // 新增：根据角色ID获取技能介绍文本
    QString getRoleSkillText(RoleID id);
};

#endif