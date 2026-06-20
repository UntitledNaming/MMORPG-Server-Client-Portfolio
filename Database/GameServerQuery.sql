-- 스키마 생성
CREATE DATABASE IF NOT EXISTS `WorldDB`;                

-- 캐릭터 테이블 생성
CREATE TABLE IF NOT EXISTS worlddb.`character`(
  characterUID         BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  accountID            BIGINT UNSIGNED NOT NULL,
  characterlevel       SMALLINT UNSIGNED NOT NULL,
  curEXP               INT NOT NULL,
  xpos                 FLOAT  NOT NULL,
  ypos                 FLOAT  NOT NULL,
  zpos                 FLOAT  NOT NULL,
  PRIMARY KEY (characterUID),
  INDEX   idx_account(accountid) 
);

-- 아이템 테이블 생성
CREATE TABLE IF NOT EXISTS worlddb.`item`(
 itemUID              BIGINT UNSIGNED NOT NULL,
 characterUID         BIGINT UNSIGNED NOT NULL,
 itemID               INT UNSIGNED NOT NULL,
 count                SMALLINT UNSIGNED NOT NULL,
 slottype             TINYINT UNSIGNED NOT NULL,
 slotindex            SMALLINT NOT NULL,
 atk                  SMALLINT NOT NULL,
 def                  SMALLINT NOT NULL,
 maxhp                SMALLINT NOT NULL,
 maxmp                SMALLINT NOT NULL,
 hpregen              SMALLINT NOT NULL,
 mpregen              SMALLINT NOT NULL,
 PRIMARY KEY(itemUID),
 INDEX   idx_character(characterUID)
);

-- UID 할당 테이블 생성
CREATE TABLE IF NOT EXISTS worlddb.`uid_sequence`(
uidName                 VARCHAR(20) NOT NULL,
startUID                BIGINT UNSIGNED NOT NULL,
PRIMARY KEY(uidName)
);
INSERT INTO worlddb.`uid_sequence` VALUES("ItemUIDAllocator", 1);

-- 깊이 재설정
SET SESSION cte_max_recursion_depth = 10000;

-- 캐릭터 테이블에 캐릭터 위치 분산 저장(초원 영역에 분산 저장)
INSERT INTO worlddb.`character` (characteruid, accountid, characterlevel, curexp, xpos, ypos, zpos)
WITH RECURSIVE
    nums(x) AS (                                  -- X 섹터 인덱스 1..160
        SELECT 1 UNION ALL SELECT x + 1 FROM nums WHERE x < 160
    ),
    area(minx, maxx, y) AS (                      -- 초원 영역(행별 X구간) = GROSS_FIELD_MONSTER_SPAWN_AREAS
        SELECT 83 AS minx, 89 AS maxx, 52 AS y
        UNION ALL SELECT 83,92,53
        UNION ALL SELECT 83,94,54
        UNION ALL SELECT 72,74,55
        UNION ALL SELECT 83,98,55
        UNION ALL SELECT 69,75,56
        UNION ALL SELECT 83,100,56
        UNION ALL SELECT 67,75,57
        UNION ALL SELECT 83,101,57
        UNION ALL SELECT 65,75,58
        UNION ALL SELECT 83,102,58
        UNION ALL SELECT 63,75,59
        UNION ALL SELECT 83,103,59
        UNION ALL SELECT 61,75,60
        UNION ALL SELECT 83,104,60
        UNION ALL SELECT 59,75,61
        UNION ALL SELECT 83,104,61
        UNION ALL SELECT 57,76,62
        UNION ALL SELECT 83,105,62
        UNION ALL SELECT 56,76,63
        UNION ALL SELECT 83,105,63
        UNION ALL SELECT 54,76,64
        UNION ALL SELECT 83,106,64
        UNION ALL SELECT 53,76,65
        UNION ALL SELECT 84,106,65
        UNION ALL SELECT 52,76,66
        UNION ALL SELECT 84,107,66
        UNION ALL SELECT 51,76,67
        UNION ALL SELECT 84,108,67
        UNION ALL SELECT 50,76,68
        UNION ALL SELECT 84,108,68
        UNION ALL SELECT 49,75,69
        UNION ALL SELECT 85,108,69
        UNION ALL SELECT 48,75,70
        UNION ALL SELECT 85,108,70
        UNION ALL SELECT 47,75,71
        UNION ALL SELECT 87,109,71
        UNION ALL SELECT 46,75,72
        UNION ALL SELECT 88,109,72
        UNION ALL SELECT 45,75,73
        UNION ALL SELECT 89,110,73
        UNION ALL SELECT 45,75,74
        UNION ALL SELECT 89,110,74
        UNION ALL SELECT 44,75,75
        UNION ALL SELECT 90,110,75
        UNION ALL SELECT 44,74,76
        UNION ALL SELECT 90,111,76
        UNION ALL SELECT 43,74,77
        UNION ALL SELECT 90,111,77
        UNION ALL SELECT 43,72,78
        UNION ALL SELECT 91,111,78
        UNION ALL SELECT 43,72,79
        UNION ALL SELECT 91,111,79
        UNION ALL SELECT 43,71,80
        UNION ALL SELECT 92,111,80
        UNION ALL SELECT 42,70,81
        UNION ALL SELECT 92,112,81
        UNION ALL SELECT 42,70,82
        UNION ALL SELECT 92,112,82
        UNION ALL SELECT 42,68,83
        UNION ALL SELECT 92,112,83
        UNION ALL SELECT 43,67,84
        UNION ALL SELECT 93,112,84
        UNION ALL SELECT 43,64,85
        UNION ALL SELECT 93,111,85
        UNION ALL SELECT 43,62,86
        UNION ALL SELECT 93,111,86
        UNION ALL SELECT 43,60,87
        UNION ALL SELECT 93,111,87
        UNION ALL SELECT 43,57,88
        UNION ALL SELECT 78,81,88
        UNION ALL SELECT 93,111,88
        UNION ALL SELECT 43,55,89
        UNION ALL SELECT 77,82,89
        UNION ALL SELECT 94,111,89
        UNION ALL SELECT 44,53,90
        UNION ALL SELECT 76,84,90
        UNION ALL SELECT 94,111,90
        UNION ALL SELECT 44,52,91
        UNION ALL SELECT 74,84,91
        UNION ALL SELECT 94,111,91
        UNION ALL SELECT 44,51,92
        UNION ALL SELECT 71,85,92
        UNION ALL SELECT 95,111,92
        UNION ALL SELECT 45,48,93
        UNION ALL SELECT 69,86,93
        UNION ALL SELECT 95,111,93
        UNION ALL SELECT 66,88,94
        UNION ALL SELECT 96,111,94
        UNION ALL SELECT 64,88,95
        UNION ALL SELECT 97,110,95
        UNION ALL SELECT 61,89,96
        UNION ALL SELECT 97,110,96
        UNION ALL SELECT 60,90,97
        UNION ALL SELECT 98,110,97
        UNION ALL SELECT 58,91,98
        UNION ALL SELECT 98,111,98
        UNION ALL SELECT 56,91,99
        UNION ALL SELECT 99,111,99
        UNION ALL SELECT 55,92,100
        UNION ALL SELECT 99,111,100
        UNION ALL SELECT 53,92,101
        UNION ALL SELECT 101,108,101
        UNION ALL SELECT 110,110,101
        UNION ALL SELECT 52,93,102
        UNION ALL SELECT 104,107,102
        UNION ALL SELECT 51,93,103
        UNION ALL SELECT 50,94,104
        UNION ALL SELECT 50,95,105
        UNION ALL SELECT 50,95,106
        UNION ALL SELECT 51,95,107
        UNION ALL SELECT 52,95,108
        UNION ALL SELECT 54,96,109
        UNION ALL SELECT 57,96,110
        UNION ALL SELECT 61,95,111
    ),
    sectors AS (                                  -- 유효 섹터 펼침 + 번호(sid)
        SELECT a.y AS secy, n.x AS secx,
               ROW_NUMBER() OVER (ORDER BY a.y, n.x) AS sid
        FROM area a JOIN nums n ON n.x BETWEEN a.minx AND a.maxx
    ),
    cnt(c) AS ( SELECT COUNT(*) FROM sectors ),   -- 총 유효 섹터 수(=2533, 하드코딩 회피)
    seq(n) AS (                                   -- 1..10000 캐릭터
        SELECT 1 UNION ALL SELECT n + 1 FROM seq WHERE n < 10000
    )
SELECT
    s.n,                                                        -- characteruid 1~10000
    s.n,                                                        -- accountid
    1,                                                          -- level
    0,                                                          -- exp
    200000 + sec.secx * 2500 + 10 + FLOOR(RAND() * (2500 - 20)),-- xpos (섹터 안 랜덤)
    200000 + sec.secy * 2500 + 10 + FLOOR(RAND() * (2500 - 20)),-- ypos
    -38775                                                      -- zpos
FROM seq s
CROSS JOIN cnt
JOIN sectors sec ON sec.sid = ((s.n - 1) % cnt.c) + 1;          -- 라운드로빈 균등 분산


-- 수정용 쿼리
TRUNCATE TABLE worlddb.character;
TRUNCATE TABLE worlddb.item;
TRUNCATE TABLE worlddb.uid_sequence;
DROP TABLE worlddb.character;
DROP TABLE worlddb.item;

-- 테스트 용 쿼리
SELECT * FROM worlddb.character;
SELECT * FROM worlddb.item;
SELECT * FROM worlddb.uid_sequence;
INSERT INTO worlddb.item VALUES(1, 1, 10012, 1,1, 5, 10,5,0,0,0,0); -- Test SQL
INSERT INTO worlddb.item VALUES(2,1,10001,2,3,0,0,0,0,0,0,0);
INSERT INTO worlddb.item VALUES(3,1,10002,3,3,1,0,0,0,0,0,0);
INSERT INTO worlddb.item VALUES(4,1,10003,1,2,1,1,0,0,0,0,0);
INSERT INTO worlddb.item VALUES(5,1,10001,4,1,1,0,0,0,0,0,0);
