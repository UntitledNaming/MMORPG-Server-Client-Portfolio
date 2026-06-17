CREATE TABLE `character` (
  characterid  BIGINT      NOT NULL PRIMARY KEY,
  accountid    BIGINT      NOT NULL,
  name         VARCHAR(32) NOT NULL,
  level        INT         NOT NULL DEFAULT 1,
  currentexp   BIGINT      NOT NULL DEFAULT 0,
  xpos         FLOAT       NOT NULL DEFAULT 0,
  ypos         FLOAT       NOT NULL DEFAULT 0,
  zpos         FLOAT       NOT NULL DEFAULT 0,
  KEY idx_accountid (accountid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE item (
  ItemUID    BIGINT NOT NULL PRIMARY KEY,   -- 서버가 범위할당, auto_increment 아님
  ItemID     INT    NOT NULL,
  CharacterID BIGINT NOT NULL,
  Count      INT    NOT NULL DEFAULT 1,
  SlotType   INT    NOT NULL,
  SlotIndex  INT    NOT NULL,
  atk INT NOT NULL DEFAULT 0, def INT NOT NULL DEFAULT 0,
  maxhp INT NOT NULL DEFAULT 0, maxmp INT NOT NULL DEFAULT 0,
  hpregen INT NOT NULL DEFAULT 0, mpregen INT NOT NULL DEFAULT 0,
  KEY idx_characterid (CharacterID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
