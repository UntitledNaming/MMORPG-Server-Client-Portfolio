-- =============================================================================
--  LoadTesterSeed.sql  —  부하 테스트용 캐릭터 아이템 시드 (분리 테이블 버전)
-- =============================================================================
--  목적:
--    더미(LoadTester) 봇이 아이템 경로(사용/스왑/삭제/줍기)를 부하에서 돌리려면
--    캐릭터 1..N 의 인벤/퀵슬롯에 아이템이 미리 깔려 있어야 한다. 이 스크립트가 그걸 채운다.
--
--  전제:
--    - 먼저 GameServerQuery.sql 을 실행해 worlddb 스키마 + character/uid_sequence 및
--      instanceitem / instanceitem_stat / stackitem 테이블이 만들어져 있고,
--      character 1..N 이 존재해야 한다(그 스크립트가 1..10000 생성).
--    - 아래 @N 을 "띄울 봇 수(LoadTester user count)" 와 똑같이 맞춘다.
--
--  테이블 분리 구조(단일 worlddb.item 에서 분리됨):
--    stackitem        : 소모품. itemUID, characterUID, itemID, slottype, slotindex, count
--    instanceitem     : 장비.   itemUID, characterUID, itemID, slottype, slotindex
--    instanceitem_stat: 장비 랜덤 스탯. itemUID, randomStatType, statValue
--                       → 붙어 있는 스탯만 행으로 존재(0 센티널 없음). PK(itemUID, randomStatType).
--
--  randomStatType 값(ContentsEnum.h RANDOM_STAT_TYPE):
--    1=ATK, 2=DEF, 3=MAX_HP, 4=MAX_MP, 5=HP_REGEN, 6=MP_REGEN
--
--  slottype 값(ContentsEnum.h SLOT_TYPE): 1=INVENTORY, 2=EQUIPMENT, 3=QUICKSLOT
--
--  itemUID 정책:
--    - stack/instance 는 같은 UID 네임스페이스를 공유한다(서버 ItemUIDAllocator 하나).
--    - 시드 UID = (characterUID-1)*6 + k + 1 (k=0..5) → 전역 유니크, 빈틈없이 1..6N.
--      k=0..3 이 stackitem, k=4..5 가 instanceitem 으로 갈 뿐 공식은 기존과 동일.
--
--  봇 행동과의 매핑(LoadTestManager 의 Do* 와 일치해야 함):
--    - DoUseItem  : 퀵슬롯 0(HP포션)/1(MP포션) 사용  → 아래 k=0,1 이 깔아둠
--    - DoSwapSlot : 인벤 0 <-> 1 스왑                → k=2,3 이 깔아둠
--    - DoDeleteItem(옵션) : 인벤 3 삭제              → k=5 가 깔아둠(장비 → stat 행도 같이 삭제돼야 함)
--    - DoPickup   : 몬스터 처치 드롭을 주움(시드 불필요, 런타임 생성)
-- =============================================================================

-- ▼▼ 봇 수에 맞춰 이 값만 바꾸면 된다(LoadTester 의 user count 와 동일하게) ▼▼
SET @N = 1;

-- 재귀 CTE 깊이 한도를 봇 수 이상으로(기본 1000이라 N이 크면 막힘).
SET SESSION cte_max_recursion_depth = 200000;

-- 기존 아이템 싹 비우고 새로 시드(중복 itemUID 충돌 방지).
-- stat 은 instanceitem 의 자식이므로 먼저 비운다(FK 걸려 있어도 안전한 순서).
TRUNCATE TABLE worlddb.instanceitem_stat;
TRUNCATE TABLE worlddb.instanceitem;
TRUNCATE TABLE worlddb.stackitem;

-- -----------------------------------------------------------------------------
-- 1) stackitem : 캐릭터 1..N 각각에 소모품 4행(k=0..3)
-- -----------------------------------------------------------------------------
INSERT INTO worlddb.stackitem
    (itemUID, characterUID, itemID, slottype, slotindex, count)
WITH RECURSIVE
    chars(c) AS (                                   -- characterUID 1..@N
        SELECT 1 UNION ALL SELECT c + 1 FROM chars WHERE c < @N
    ),
    tmpl(k, itemID, slottype, slotindex, cnt) AS (
                    SELECT 0, 10001, 3, 0, 500  -- 퀵0: SMALL_HP_POTION x500 (maxStack 500, 부하테스트 연속소비용)
        UNION ALL   SELECT 1, 10002, 3, 1, 500  -- 퀵1: SMALL_MP_POTION x500
        UNION ALL   SELECT 2, 10001, 1, 0, 9    -- 인벤0: HP포션(스왑/스택용)
        UNION ALL   SELECT 3, 10002, 1, 1, 9    -- 인벤1: MP포션(스왑용)
    )
SELECT
    (ch.c - 1) * 6 + t.k + 1,   -- itemUID (전역 유니크, 기존 공식 유지)
    ch.c,                       -- characterUID
    t.itemID, t.slottype, t.slotindex, t.cnt
FROM chars ch
CROSS JOIN tmpl t;

-- -----------------------------------------------------------------------------
-- 2) instanceitem : 캐릭터 1..N 각각에 장비 2행(k=4..5)
--    랜덤스탯은 여기 없다 → instanceitem_stat 에서 시드.
-- -----------------------------------------------------------------------------
INSERT INTO worlddb.instanceitem
    (itemUID, characterUID, itemID, slottype, slotindex)
WITH RECURSIVE
    chars(c) AS (
        SELECT 1 UNION ALL SELECT c + 1 FROM chars WHERE c < @N
    ),
    tmpl(k, itemID, slottype, slotindex) AS (
                    SELECT 4, 10007, 1, 2   -- 인벤2: NORMAL_WEAPON (NORMAL → 랜덤스탯 1개)
        UNION ALL   SELECT 5, 10008, 1, 3   -- 인벤3: MAGIC_HELMET (MAGIC → 랜덤스탯 2개)
    )
SELECT
    (ch.c - 1) * 6 + t.k + 1,   -- itemUID
    ch.c,
    t.itemID, t.slottype, t.slotindex
FROM chars ch
CROSS JOIN tmpl t;

-- -----------------------------------------------------------------------------
-- 3) instanceitem_stat : 장비에 붙는 랜덤 스탯만 행으로.
--    장비 랜덤스탯은 서버 규칙을 그대로 따른 유효값:
--      NORMAL = 랜덤스탯 1개 / MAGIC = 2개,
--      값은 등급별 min~max 안, 스탯 타입은 해당 장비 슬롯의 허용 목록 안.
--    - k=4 NORMAL_WEAPON(10007): WEAPON 허용 스탯에 ATK.
--        normal ATK 룰 = 1~3 → ATK(1)=2 (유효)
--    - k=5 MAGIC_HELMET(10008): HELMET 허용에 MAX_HP/MAX_MP.
--        magic MAX_HP/MAX_MP 룰 = 20~30 → MAX_HP(3)=25, MAX_MP(4)=25 (합 2개)
-- -----------------------------------------------------------------------------
INSERT INTO worlddb.instanceitem_stat
    (itemUID, randomStatType, statValue)
WITH RECURSIVE
    chars(c) AS (
        SELECT 1 UNION ALL SELECT c + 1 FROM chars WHERE c < @N
    ),
    tmpl(k, statType, statValue) AS (
                    SELECT 4, 1, 2    -- NORMAL_WEAPON: ATK +2
        UNION ALL   SELECT 5, 3, 25   -- MAGIC_HELMET: MAX_HP +25
        UNION ALL   SELECT 5, 4, 25   -- MAGIC_HELMET: MAX_MP +25
    )
SELECT
    (ch.c - 1) * 6 + t.k + 1,   -- 부모 instanceitem 과 동일한 UID 공식
    t.statType, t.statValue
FROM chars ch
CROSS JOIN tmpl t;

-- 런타임 itemUID 할당기가 시드 UID(최대 6N)와 겹치지 않도록 시작값을 6N+1 로 올린다.
--   (서버는 uid_sequence.startUID 에서 블록을 떼어 itemUID 를 발급한다 → 충돌 방지 필수.
--    stack/instance 가 같은 할당기를 쓰므로 공식은 기존과 동일하게 6N+1.)
INSERT INTO worlddb.uid_sequence (uidName, startUID)
VALUES ('ItemUIDAllocator', 6 * @N + 1)
ON DUPLICATE KEY UPDATE startUID = 6 * @N + 1;

-- 확인용:
-- SELECT COUNT(*) FROM worlddb.stackitem;          -- 4*N 이어야 함
-- SELECT COUNT(*) FROM worlddb.instanceitem;       -- 2*N 이어야 함
-- SELECT COUNT(*) FROM worlddb.instanceitem_stat;  -- 3*N 이어야 함 (N*(1+2))
-- SELECT * FROM worlddb.stackitem    WHERE characterUID = 1;  -- 봇1 소모품 확인
-- SELECT i.*, s.randomStatType, s.statValue                   -- 봇1 장비+스탯 JOIN 확인
--   FROM worlddb.instanceitem i
--   LEFT JOIN worlddb.instanceitem_stat s ON s.itemUID = i.itemUID
--  WHERE i.characterUID = 1;
-- SELECT * FROM worlddb.uid_sequence;              -- startUID = 6N+1 확인
--
-- 고아 행 검사(정합성, 로드테스트 후 0이어야 함):
-- SELECT COUNT(*) FROM worlddb.instanceitem_stat s
--   LEFT JOIN worlddb.instanceitem i ON i.itemUID = s.itemUID
--  WHERE i.itemUID IS NULL;
