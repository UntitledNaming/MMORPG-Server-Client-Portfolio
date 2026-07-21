# Architecture

## 1. 개요

이 프로젝트는 C++ IOCP 기반 MMORPG 월드 서버입니다. UE5 클라이언트와 연동되며, 서버는 AOI(관심 영역), 이동 동기화, 서버 권위 전투, 몬스터 AI, 아이템, 비동기 DB 저장을 구현했습니다.

핵심은 이 서버가 처음부터 새로 만든 것이 아니라, 앞선 프로젝트(P1)에서 직접 구현하고 검증한 IOCP 네트워크·게임 라이브러리를 기반으로, 그 위에 실제 게임 서버 기능을 올렸습니다. P1이 라이브러리의 구현과 검증이고, P4는 그 기반을 실제 MMORPG 월드 서버에 적용한 결과물 입니다.

## 2. P1 네트워크/게임 라이브러리와의 관계

- 이 레포 안에는 P1 기반의 `NetworkLib`가 포함되어 있으며, 하위에 `network_library`(IOCP 네트워크 코어·세션), `game_library`(그룹·게임 세션·유저 인터페이스), `common_files`(버퍼·TLS 메모리 풀·Lock-Free 등)가 있습니다.
- 이 라이브러리 계열이 P4 서버의 기반으로 사용됩니다. 컨텐츠 그룹(`FieldGroup`·`AuthGroup`)은 게임 라이브러리의 `CGroup`을 상속하고, 유저(`CUser`)는 `IUser`를 상속합니다.
- P1은 라이브러리 구현 및 검증, P4는 해당 기반을 실제 MMORPG 서버에 적용한 프로젝트입니다.
- P1의 게임(에코) 서버와 P4는 직접 전신 관계가 아니라, **같은 게임 라이브러리 기반을 공유하는 관계**입니다.

```
P1 Network/Game Library
└─ NetworkLib
   ├─ network_library     # IOCP 네트워크 코어, 세션
   ├─ game_library        # 그룹(CGroup)/게임 세션/유저 인터페이스(IUser)
   └─ common_files       # 버퍼, TLS 메모리 풀, Lock-Free 등
        ↓  (기반으로 사용)
P4 MMORPG World Server
├─ FieldGroup             # 월드 컨텐츠 (CGroup 상속)
├─ User / Monster        # 캐릭터 / 몬스터 + AI
├─ Item / Inventory       # 아이템 시스템 (Storage/슬롯)
├─ DB Job Queue         # 비동기 DB 저장 (CDBManager)
└─ LoadTester             # 부하 생성 및 응답 검증 (별도)
```

## 3. 프로젝트 전체 구조

```
UE5 Client
   │  (TCP / IOCP)
   ▼
GameServer
├─ NetworkLib (P1)                 # IOCP 네트워크·게임 라이브러리
├─ AuthGroup                       # 접속 / 로그인 / 캐릭터 선택
├─ FieldGroup                       # 월드 로직 (AOI/전투/스킬/아이템/몬스터)
│    ├─ User (CUser : IUser)
│    ├─ Monster (CMonster + MonsterAI)
│    ├─ Item (Storage/Inventory/Equipment/QuickSlot)
│    └─ FieldDropItem (FieldDropItemPool)
├─ CDBManager (DB Thread)          # 비동기 DB Job 처리 ──► MySQL(worlddb)
├─ Monitoring / Store Thread        # 지표 수집 ──► monitor.csv
└─ (별도) LoadTester               # 다수 봇 접속·행동·응답 검증
```

- 네트워크·프레임 처리는 P1 라이브러리가, 월드 컨텐츠는 `FieldGroup`이, 저장은 전용 DB 스레드(`CDBManager`)가 담당하도록 분리했습니다.
- 부하 테스트는 별도 프로젝트인 `LoadTester`가 다수 봇으로 접속·행동을 생성하고 서버 응답을 검증합니다.

## 4. 주요 스레드 구조

| 스레드 | 소속 | 역할 |
|---|---|---|
| IOCP Worker Thread | 게임 라이브러리(P1) | GQCS로 완료를 받아 수신 처리·그룹 콜백·세션 해제 처리 |
| Accept Thread | 게임 라이브러리(P1) | 연결 수락·세션 등록 |
| Frame / Update Thread | 게임 라이브러리(P1) | 그룹 프레임 주기가 되면 완료 포트로 프레임 작업을 던져 `OnUpdate` 실행 |
| DB Thread | `CDBManager` | DB Job 큐에서 Job을 꺼내 MySQL에 저장(트랜잭션 배치) |
| Monitor Thread | `GameServer` | PDH 지표·TPS·큐 깊이 등을 주기적으로 수집·출력 |
| Store Thread | `GameServer` | 수집한 스냅샷을 `monitor.csv`로 기록 |
| Send Thread(선택) | 게임 라이브러리(P1) | Config로 켤 수 있는 송신 전용 스레드 |
| LoadTester Worker/Behavior/Stats | LoadTester(별도) | 봇의 수신 처리·주기적 송신·통계 |

> 선택적 Send 스레드의 실제 활성화 여부와 프레임 스레드 개수 등 세부는 게임 라이브러리 설정에 따릅니다. 

## 5. FieldGroup 구조

- `FieldGroup`은 게임 라이브러리의 `CGroup`을 상속한 월드 컨텐츠 그룹입니다.
- 컨텐츠 처리 진입점은 `CGroup`의 콜백입니다: `OnClientJoin` / `OnClientLeave` / `OnRecv`(수신 메시지 분기) / `OnIUserMove`(그룹 이동으로 유저가 필드에 들어올 때) / `OnUpdate`(프레임 로직).
- `OnRecv`는 메시지 타입에 따라 이동·전투·스킬·아이템·줍기·리스폰 등 핸들러로 분기합니다.
- `OnUpdate`는 프레임마다 유저 갱신, 몬스터 갱신, 필드 드랍 아이템 만료를 처리합니다.
- 유저는 `m_userLookUpTable`, 드랍 아이템은 별도 조회 테이블, 몬스터는 풀 배열로 관리하고, 위치별로는 섹터 격자에 등록합니다.

## 6. Sector / AOI 구조

- 월드를 섹터 격자(`m_sectors[Y][X]`)로 나누고 유저·몬스터·드랍 아이템을 섹터 단위로 관리합니다.
- 캐릭터·몬스터 주변 9섹터를 시야로 삼습니다(`SectorPos::SectorFind`).
- 섹터가 바뀌면 새로 보이는 섹터와 사라지는 섹터의 **차집합만** 계산해(`CalSectorTransitionMessageTargets`) 생성/삭제 메시지를 보냅니다.
- 유저·몬스터·드랍 아이템의 시야 진입/이탈을 각각 갱신합니다. 유저 이동은 `SectorUpdate`, 몬스터 이동은 AI의 섹터 갱신으로 처리합니다.

## 7. DB 구조

- 게임 루프와 DB 저장을 분리한 **비동기 DB Job Queue** 구조입니다.
- 컨텐츠 로직이 저장할 내용을 DBJob(예: 아이템 삽입/삭제/개수 변경/슬롯 변경, 캐릭터 진행도, 로그아웃)으로 만들어 `CDBManager::EnqueueDBJob`으로 큐에 넣습니다.
- 전용 **DB Thread**가 큐에서 Job을 꺼내 MySQL(`worlddb`)에 씁니다.
- 저장 부하를 줄이기 위해 세 가지를 적용했습니다.
  - **dirtyFlag**: 변경된 항목만 저장(예: 캐릭터 진행도의 더티 플래그, 아이템 슬롯의 더티 수집).
  - **저장 주기 조정**: 매 변경/매 프레임이 아니라 누적 타이머 주기로 저장.
  - **트랜잭션 배치**: 여러 쓰기 Job을 한 트랜잭션으로 묶어 커밋(최대 일정 건수 단위).
- DB 큐 깊이(`dbQueue`)는 모니터링 지표로 관측합니다.

> 코드 흐름의 상세는 `docs/Code_Flow.md`, 프로토콜은 `docs/Protocol_Design.md`를 참고하세요.
