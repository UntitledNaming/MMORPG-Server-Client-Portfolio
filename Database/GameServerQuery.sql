CREATE DATABASE IF NOT EXISTS `WorldDB`;

CREATE TABLE IF NOT EXISTS `character`(
  characterUID         BIGINT NOT NULL AUTO_INCREMENT,
  accountID            BIGINT NOT NULL,
  characterlevel       SMALLINT UNSIGNED NOT NULL,
  curEXP               INT NOT NULL,
  xpos                 FLOAT  NOT NULL,
  ypos                 FLOAT  NOT NULL,
  zpos                 FLOAT  NOT NULL,
  PRIMARY KEY (characterUID),
  INDEX   idx_account(accountid) 
);

CREATE TABLE IF NOT EXISTS `item`(
 itemUID              BIGINT NOT NULL,
 characterUID         BIGINT NOT NULL,
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

  INSERT INTO `character` (characterid, accountid, characterlevel, curexp, xpos, ypos, zpos)
  WITH RECURSIVE seq(n) AS (
      SELECT 1
      UNION ALL
      SELECT n + 1 FROM seq WHERE n < 10000
  )
  SELECT
      n,                                                                  -- characterid 1~10000
      n,                                                                  -- accountid  1~10000
      1,                                                                  -- characterlevel
      0,                                                                  -- curexp
      200000 + (42 + ((n - 1) % 71)) * 2500 + 1250,                       -- xpos (섹터 중앙)
      200000 + (51 + (FLOOR((n - 1) / 71) % 61)) * 2500 + 1250,           -- ypos (섹터 중앙)
      0                                                                   -- zpos (지면, 필요시 조정)
  FROM seq;


