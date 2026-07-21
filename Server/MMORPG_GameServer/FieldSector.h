#pragma once
#include "ContentsType.h"

class  CUser;
class  CMonster;
struct FieldDropItem;

class FieldSector
{
public:
    void Init();

    CUser* GetUser(uint16 idx) { return m_users[idx]; }
    void   AddUser(CUser* user);
    void   RemoveUser(CUser* user);

    void   AddMonster(CMonster* monster);
    void   RemoveMonster(CMonster* monster);

    void   AddItem(FieldDropItem* item);
    void   RemoveItem(FieldDropItem* item);

    CMonster*      GetMonster(uint16 idx) { return m_monsters[idx]; }
    FieldDropItem* GetFieldDropItem(uint16 idx) { return m_items[idx]; }
    uint16         GetUserCount() const { return m_usersCount; }
    uint16         GetMonsterCount() const { return m_monsterCount; }
    uint16         GetItemCount() const { return m_itemCount; }

private:
    std::vector<CUser*>         m_users;
    std::vector<CMonster*>      m_monsters;
    std::vector<FieldDropItem*> m_items;

    uint16 m_usersCount;
    uint16 m_monsterCount;
    uint16 m_itemCount;
};

