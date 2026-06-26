-- =============================================================================
--  LoadTesterSeed.sql  —  부하 테스트용 캐릭터 아이템 시드
-- =============================================================================
--  목적:
--    더미(LoadTester) 봇이 아이템 경로(사용/스왑/삭제/줍기)를 부하에서 돌리려면
--    캐릭터 1..N 의 인벤/퀵슬롯에 아이템이 미리 깔려 있어야 한다. 이 스크립트가 그걸 채운다.
--
--  전제:
--    - 먼저 GameServerQuery.sql 을 실행해 worlddb 스키마 + character/item/uid_sequence
--      테이블이 만들어져 있고, character 1..N 이 존재해야 한다(그 스크립트가 1..10000 생성).
--    - 아래 @N 을 "띄울 봇 수(LoadTester user count)" 와 똑같이 맞춘다.
--
--  컬럼 의미(worlddb.item):
--    itemUID, characterUID, itemID, count, slottype, slotindex,
--    atk, def, maxhp, maxmp, hpregen, mpregen
--    → atk..mpregen 6칸은 "랜덤 스탯 값"만 저장한다(base 스탯은 서버 ItemTable 에 있음).
--      서버 로드(DBJob.cpp)는 이 6칸 중 0이 아닌 칸을 RandomStatResult 로 복원한다.
--      즉 0 = "그 스탯 없음". (FieldDropItemPool::CreateRandomStat 가 생성하는 형식과 동일)
--
--  slottype 값(ContentsEnum.h SLOT_TYPE): 1=INVENTORY, 2=EQUIPMENT, 3=QUICKSLOT
--
--  봇 행동과의 매핑(LoadTestManager 의 Do* 와 일치해야 함):
--    - DoUseItem  : 퀵슬롯 0(HP포션)/1(MP포션) 사용  → 아래 k=0,1 이 깔아둠
--    - DoSwapSlot : 인벤 0 <-> 1 스왑                → k=2,3 이 깔아둠
--    - DoDeleteItem(옵션) : 인벤 3 삭제              → k=5 가 깔아둠
--    - DoPickup   : 몬스터 처치 드롭을 주움(시드 불필요, 런타임 생성)
-- =============================================================================

-- ▼▼ 봇 수에 맞춰 이 값만 바꾸면 된다(LoadTester 의 user count 와 동일하게) ▼▼
SET @N = 2500;

-- 재귀 CTE 깊이 한도를 봇 수 이상으로(기본 1000이라 N이 크면 막힘).
SET SESSION cte_max_recursion_depth = 200000;

-- 기존 아이템 싹 비우고 새로 시드(중복 itemUID 충돌 방지).
TRUNCATE TABLE worlddb.item;

-- 캐릭터 1..N 각각에 아이템 6종을 깐다.
--   itemUID = (characterUID-1)*6 + k + 1  (k=0..5) → 전역 유니크, 빈틈없이 1..6N.
INSERT INTO worlddb.item
    (itemUID, characterUID, itemID, count, slottype, slotindex,
     atk, def, maxhp, maxmp, hpregen, mpregen)
WITH RECURSIVE
    chars(c) AS (                                   -- characterUID 1..@N
        SELECT 1 UNION ALL SELECT c + 1 FROM chars WHERE c < @N
    ),
    -- 봇 1명에게 깔 아이템 템플릿 6행. (k, itemID, count, slottype, slotindex, 랜덤스탯6칸)
    -- 장비의 랜덤스탯은 서버 규칙을 그대로 따른 유효값:
    --   NORMAL = 랜덤스탯 1개 / MAGIC = 2개,
    --   값은 등급별 min~max 안, 스탯 타입은 해당 장비 슬롯의 허용 목록 안.
    tmpl(k, itemID, cnt, slottype, slotindex, atk, def, maxhp, maxmp, hpregen, mpregen) AS (
                    SELECT 0, 10001, 500, 3, 0, 0,0,0,0,0,0  -- 퀵0: SMALL_HP_POTION x500 (maxStack 500, 부하테스트 연속소비용)
        UNION ALL   SELECT 1, 10002, 500, 3, 1, 0,0,0,0,0,0  -- 퀵1: SMALL_MP_POTION x500
        UNION ALL   SELECT 2, 10001, 9, 1, 0, 0,0,0,0,0,0    -- 인벤0: HP포션(스왑/스택용)
        UNION ALL   SELECT 3, 10002, 9, 1, 1, 0,0,0,0,0,0    -- 인벤1: MP포션(스왑용)
        -- 인벤2: NORMAL_WEAPON(10007). NORMAL → 랜덤스탯 1개. WEAPON 허용 스탯에 ATK 있음.
        --        normal ATK 룰 = 1~3 → atk=2 (유효). 나머지 칸 0.
        UNION ALL   SELECT 4, 10007, 1, 1, 2, 2,0,0,0,0,0
        -- 인벤3: MAGIC_HELMET(10008). MAGIC → 랜덤스탯 2개. HELMET 허용에 MAX_HP/MAX_MP 있음.
        --        magic MAX_HP/MAX_MP 룰 = 20~30 → maxhp=25, maxmp=25 (둘 다 유효, 합 2개).
        UNION ALL   SELECT 5, 10008, 1, 1, 3, 0,0,25,25,0,0
    )
SELECT
    (ch.c - 1) * 6 + t.k + 1,   -- itemUID (전역 유니크)
    ch.c,                       -- characterUID
    t.itemID, t.cnt, t.slottype, t.slotindex,
    t.atk, t.def, t.maxhp, t.maxmp, t.hpregen, t.mpregen
FROM chars ch
CROSS JOIN tmpl t;

-- 런타임 itemUID 할당기가 시드 UID(최대 6N)와 겹치지 않도록 시작값을 6N+1 로 올린다.
--   (서버는 uid_sequence.startUID 에서 블록을 떼어 itemUID 를 발급한다 → 충돌 방지 필수)
INSERT INTO worlddb.uid_sequence (uidName, startUID)
VALUES ('ItemUIDAllocator', 6 * @N + 1)
ON DUPLICATE KEY UPDATE startUID = 6 * @N + 1;

-- 확인용:
-- SELECT COUNT(*) FROM worlddb.item;                 -- 6*N 이어야 함
-- SELECT * FROM worlddb.item WHERE characterUID = 1; -- 봇1 인벤 확인
-- SELECT * FROM worlddb.uid_sequence;                -- startUID = 6N+1 확인
