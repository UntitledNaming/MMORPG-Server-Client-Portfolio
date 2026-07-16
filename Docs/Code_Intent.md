# 코드 의도 문서 (Code Intent) — P4 MMORPG 월드 서버

> 이 문서는 P4 고유 코드(.h/.cpp, 테스트 코드, 프로토콜)를 "무엇을 하는가"가 아니라 **"왜 이렇게 작성했는가"** 중심으로 설명한다.
> 동작 흐름은 `Docs/Code_Flow.md`, 프로토콜 상세는 `Docs/Protocol_Design.md`, DB 병목은 `Docs/Troubleshooting.md` 참고.
> `NetworkLib/`(P1 라이브러리 포함분)의 의도는 P1 저장소의 `docs/Code_Intent.md`에 있다. 이 문서는 P4가 그 위에 올린 것만 다룬다.

---

## 0. 저장소 구조 자체의 의도

| 폴더 | 의도 |
|---|---|
| `NetworkLib/` | P1에서 구현·검증한 네트워크·게임 라이브러리를 **그대로 포함** — "P1이 P4의 기반"이라는 연결의 물리적 증거 |
| `Common/Contents/` | 프로토콜·상수·구조체를 서버와 부하 테스터가 공유 — 정의 불일치 버그를 컴파일 타임에 차단 |
| `Server/MMORPG_GameServer/` | 월드 서버 본체. 컨텐츠는 전부 `CGroup` 상속 그룹(AuthGroup/FieldGroup)으로 — 라이브러리의 직렬 처리 보장 위에서만 동작 |
| `Server/LoadTester/` | 부하 봇. 서버와 같은 프로토콜 헤더를 쓰는 독립 IOCP 클라이언트 |
| `Server/Docs/` (측정 자료/안정성 테스트) | 부하·버그 로그 원본 보존 — 트러블슈팅 서사의 근거 |
| `Database/` | 스키마·시퀀스. `uid_sequence`는 ItemUID 블록 할당의 전제 |

---

## 1. 서버 시작·모니터링 — `MMORPG_GameServer.cpp`, `GameServer.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `main` | GameServer 생성 + ESC 대기 | 진입점 최소화 — 구성은 전부 GameServer::Init에 |
| `GameServer::Init` | 풀 초기화 → 그룹 생성·Attach → DB 매니저 → 모니터링 → Run | 컨텐츠 준비가 끝난 뒤 네트워크가 열리는 순서를 P1 규약대로 유지 |
| `Monitoring` | 매초 PDH+TPS+dbQueue 콘솔 출력 | dbQueue 크기를 1초 단위로 상시 노출 — DB 병목을 "느낌"이 아니라 큐 길이 발산으로 관측하게 한 장치. 2500 동접 병목 발견의 출처 |
| `StoreProc` / `StoreThread` / `WriteSnapshot` | LatencyHistogram 스냅샷 → monitor.csv 기록 | 측정과 기록을 별도 스레드로 — 파일 IO가 게임 프레임에 개입하지 않게 |
| `ItemUIDAllocate` | ItemUIDRangeAllocateJob 투입 | UID 발급을 DB 왕복 없이 하기 위한 블록 선확보의 트리거 |

`LatencyHistogram.h`: p50/p95/p99/p999/max 백분위를 버킷 히스토그램으로 계산. 평균은 꼬리 지연을 숨기므로 **백분위를 1급 지표**로 삼겠다는 의도. `MonitoringSnapShot.h`는 CSV 한 행의 스키마다.

---

## 2. 인증 그룹 — `AuthGroup.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `OnClientJoin/OnClientLeave` | NonUser/User 테이블 관리 | P1 검증 서버들과 동일한 미인증 격리 패턴 재사용 |
| `LoginRequestProc` | 형식 검증 → CUser 생성 (토큰 인증은 todo 상태의 임시 처리) | 부하 측정이 목적이라 인증 강도보다 입장 처리량 경로를 우선 구현. **미구현임을 숨기지 않고 todo로 명시** |
| `CharacterSelectProc` | CharacterSelectJob(flush) 투입 | 캐릭터 로드는 SELECT라 배치에 섞이면 안 됨 — flush 플래그로 즉시 커밋 경로 지정 |
| `OnUpdate` | DB 완료 Job 소비 → `GroupMove("Field")` | DB 응답을 그룹 프레임에서 소비 — 콜백이 DB 스레드에서 컨텐츠를 만지는 교차를 차단(직렬 처리 보장 유지) |

---

## 3. 월드 본체 — `FieldGroup.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `OnIUserMove` | 필드 등록 + 주변 9섹터 생성 스냅샷 전송 | 입장 순간의 "보이는 세계"를 서버가 확정해서 보냄 — 클라이언트 요청 기반 스폰의 경쟁을 제거 |
| `OnRecv` | 9종 핸들러 분기 | 프로토콜 ID → 핸들러 1:1. 핸들러 밖에서 검증하지 않음 — 검증 로직이 핸들러마다 문맥에 맞게 |
| `HandleCharacterMovementUpdate` | 좌표+yaw+moveFlag 수신 → 싱크 임계 검증 → 섹터 갱신 → 브로드캐스트 | "믿되 검증": 클라 좌표를 받되 서버 이동 시뮬레이션과의 오차가 임계를 넘으면 싱크 패킷으로 교정 — 반응성과 권위의 절충 |
| `HandleLeftAttackSwing` / `HandleSkillUse` | 방향/슬롯만 받아 서버가 대상·데미지 판정 | 클라는 **타겟 리스트를 보내지 않는다** — 치팅 방지의 핵심. 스킬은 MP/쿨다운/사거리를 서버가 검증 |
| `CollectHitTarget` | 섹터로 후보 1차 거르기 → Circle/Cone/Box 정밀 판정 | 전 유저 순회 대신 섹터로 후보를 줄이고 도형 판정 — 판정 비용을 동접과 무관하게 억제 |
| `SectorUpdate` | 이동 시 차집합 섹터만 생성/삭제 전파 | 9섹터 전체 재전송이 아니라 **차집합만** — AOI 트래픽 최소화의 구현 지점 |
| `OnUpdate` | UserUpdate/MonsterUpdate/FieldDropItemExpired | 프레임 로직을 그룹 락 안에서 — 직렬 처리 보장 |
| `SendPacket_SectorOne/Around/HitSectors` | 섹터 확산 전송 | 확산 범위 범위를 함수 이름으로 구분해 "누구에게 가는 패킷인지"를 호출부에서 읽게 |
| `SendMonster*` 계열 | 몬스터 상태 전파 | 몬스터 이동/타겟 변경을 개별 패킷으로 — AI 갱신 빈도 제한(6장)과 한 세트로 전송량 제어 |
| `MonsterSpawnInit` | 초기 대량 배치 | 부하 테스트의 몬스터 밀도를 재현 가능하게 고정 |
| `HandleRTTMessage` | RTT 에코 | 부하 중 지연 측정을 컨텐츠와 같은 처리 경로로 — 측정이 실제 경로를 대표하게 |

---

## 4. 유저 — `CUser.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `Alloc/Free` (TLS 풀) | CMPoolTLS<CUser> | 입퇴장이 잦은 고빈도 객체 — P1 풀 재사용 |
| `LoadDataFromDB` | 캐릭터/아이템 DB 값 적용 | DB 스레드가 읽은 원시 값을 유저 객체로 변환하는 단일 지점 |
| `UserOnUpdate` / `Move` | 서버측 이동 전진 + 저장 주기 처리 | 서버도 스스로 이동을 시뮬레이션해야 "믿되 검증"의 기준값이 생긴다 |
| `Damage/IsAlive/ResPawn/GainExp` | HP/사망/부활/경험치 | 상태 변화 전부 서버 계산 — 클라는 결과만 수신 |
| `CalBaseAttackDamage/CalSkillDamage` | 장비 스탯 합산 데미지 | Equipment 합산값을 참조 — Damage UnderFlow 버그의 수정 지점 |
| `CanUseSkill/UseSkill` | MP/쿨다운 검증 | 검증과 소모를 한 함수로 — 검증 통과 후 소모 전 틈을 없앰 |
| `CharacterProgressUpdate` | dirtyFlag + 저장 주기 도달 시에만 CharacterProgressJob | DB 병목 개선 1축: **변한 것만, 주기마다** 저장 |
| `ItemSlotUpdate` | CollectDirtyItems → ItemSlotUpdateJob | 개선 2축: 아이템 변경을 모아 CASE 한 방 UPDATE로 |
| `CalSectorTransitionMessageTargets` | SectorPos 래핑 | 차집합 계산을 유저 이동 문맥으로 노출 |

---

## 5. 전투 판정 — `CollisionCheck.h/.cpp`, `HitSearchBuilder.h/.cpp`, `SkillTable.h`

| 항목 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `IsInCircle` | distSq > rangeSq 비교 | **sqrt 회피** — 제곱 비교로 동일 판정. P3에서 확인한 "수학 함수 비용" 감각의 적용 |
| `IsInCone` | dot·dot ≥ distSq·cos² 비교 | atan/acos 없이 내적만으로 각도 판정 — 콘 판정의 삼각함수 제거 |
| `IsInBox` | 로컬 좌표 변환 후 범위 비교 | 회전된 사각 범위를 월드에서 풀지 않고 로컬로 가져와 단순 비교 |
| `HitSearchBuilder::MakeBaseAttack/MakeSkillAttack` | 공격 종류→HitSearchInfo(도형/범위) 생성 | 판정 파라미터 생성과 판정 실행을 분리 — 새 스킬 추가가 "Info 생성 규칙 추가"로 끝나게 |
| `SkillTable.h` (`g_skillData` static 배열) | 스킬별 히트수/MP/쿨타임/범위/도형/데미지 타입 | 스킬 밸런스를 코드 로직이 아니라 **데이터 테이블**로 — 로직 수정 없이 수치 조정 가능. Buff는 도형 None, SpinSlash는 Circle 등 도형이 데이터로 선언됨 |

---

## 6. 몬스터 — `CMonster.h/.cpp`, `MonsterAI.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `MonsterAI::Update` + `UpdateIdle/Patrol/Chase/Return/Combat` | 6상태 FSM | 조건 분기 뭉치 대신 상태별 함수 — 상태 추가/수정이 국소화 |
| `Enter*` 계열 | 상태 진입 세팅 + Move 패킷 | 진입 시점에만 전파 — 상태 지속 중 반복 전송 방지 |
| `IsChaseRange/IsAttackRange` | 진입/이탈 반경을 다르게(반경 여유) | 경계에서 Chase↔Combat 떨림(패킷 폭주) 방지 |
| `TargetUpdate` | 최소 시간(CHASE_UPDATE_MIN_MS)·최소 거리(TARGET_UPDATE_CM)로 갱신 빈도 제한 | 추격 목표 갱신 패킷을 시간·거리 이중 문턱으로 억제 — AI 정확도와 트래픽 사이의 절충을 상수로 노출 |
| `FindNearestPlayer` | 섹터 순회 최근접 탐색 | 전 유저가 아닌 주변 섹터만 — AOI를 AI 탐색에도 재사용 |
| `UpdateSector` | 몬스터 AOI 갱신 | 유저와 같은 차집합 규칙 — 섹터 경계의 생성/삭제 중복 버그(Troubleshooting)와 연결되는 지점 |
| `CMonster::MonsterUpdate/Regen/Damage` | AI 구동 + 리젠/피격 | 몬스터도 서버 권위 — 클라이언트는 몬스터 상태를 결코 만들지 않음 |

---

## 7. 섹터/AOI — `SectorPos.h/.cpp`, `FieldSector.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `SectorFind` | 주변 9섹터 산출(경계 클램프) | AOI의 기본 단위. 9섹터=시야 반경을 섹터 크기로 정규화한 것 |
| `CalSectorTransitionMessageTargets` | 구 9섹터와 신 9섹터의 겹침을 set으로 제외, 삭제/생성 대상만 산출 | 이동 시 "나갔다 들어온" 섹터만 패킷을 만들기 위한 차집합. set 사용은 O(1) 중복 확인 — 9×9 이중 루프 규모라 상수 비용으로 충분하다는 판단 |
| `FieldSector::Add/Remove(User/Monster/Item)` | 섹터 원소 관리 | 유저·몬스터·드랍을 같은 섹터 격자에서 관리 — 전투 후보 거르기/AI 탐색/획득 판정이 하나의 공간 인덱스를 공유 |

---

## 8. 아이템 — `CUserItemStorage`, `Inventory`, `Equipment`, `QuickSlot`, `ItemUIDAllocator`, `FieldDropItemPool`, `ItemTable`

핵심 구조 의도: **아이템 실체는 Storage에 UID로 단일 보관, 슬롯(인벤/장비/퀵슬롯)은 UID 참조만.** 슬롯끼리 객체를 주고받으면 복제·유실 버그의 온상이 되므로, 이동/장착/스왑은 전부 "참조 이동"으로 만들었다.

| 파일/함수 | 의도 |
|---|---|
| `CUserItemStorage::CreateItem/DeleteItem/ChangeItemCount` | 실체 변경의 유일한 관문 — 정합성 검증 지점을 한 곳으로 |
| `SetItemDirtyFlag/CollectDirtyItems` | 변경 표시/일괄 수집 — DB 배치 저장의 컨텐츠 쪽 절반 |
| `Inventory::InsertItemToSlot/ItemSlotChange` + 빈 슬롯 인덱스 관리 | 슬롯 배열 + 빈 슬롯 재사용 — 순회 탐색 제거 |
| `Equipment::EquippedItem/UnEquippedItem` + `GetATK/GetDEF` | 장착 시점에 스탯 합산 갱신 — 전투 계산이 매번 장비를 순회하지 않게 캐싱 |
| `QuickSlot` | 소모품 참조 슬롯 — 사용 시 실체는 Storage에서 감소 |
| `ItemUIDAllocator::Init/Alloc` | DB에서 UID **블록**을 선확보해 메모리에서 발급 | UID마다 DB 왕복하면 중앙 할당이 병목 — 블록 단위로 왕복을 상수화. `uid_sequence FOR UPDATE`로 서버 다중 기동에도 충돌 없음 |
| `FieldDropItemPool` (`CreateItem/RollFieldDropType/RollEquipSlot`) | 드랍은 DropID로 필드에서만 관리, 줍는 순간 ItemUID 부여 | 임시 개체(필드 드랍)와 유저 소유(인벤)의 생명주기를 분리 — 아무도 안 줍는 아이템에 UID·DB 비용을 쓰지 않음 |
| `ItemTable` | 정적 아이템 데이터 | SkillTable과 같은 데이터 주도 원칙 |

---

## 9. DB — `CDBManager.h/.cpp`, `DBJob.h/.cpp`, `CSizeClassMemoryPoolTLS`

| 함수/클래스 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `CDBManager::EnqueueDBJob` | Job 큐 삽입 + 이벤트 | 컨텐츠 스레드는 DB를 기다리지 않는다 — 비동기 저장의 진입점 |
| `CDBManager::DBThread` | 이벤트 대기 → Job 소비. **10건 단위 트랜잭션 배치**(vector 크기 10), `flush` Job은 열려 있던 배치를 즉시 COMMIT 후 단독 처리 | 병목의 본질이 commit 비용·디스크 IO 횟수였으므로 커밋 횟수 자체를 1/10로. flush 분리는 SELECT류(캐릭터 로드, UID 확보)가 배치 지연에 물리지 않게 하기 위함 |
| `DBJob` 파생 8종 (`CharacterSelect/ItemUIDRangeAllocate/InsertItem/DeleteItem/ItemCountUpdate/ItemSlotUpdate/CharacterProgress/LogOut`) | Job당 SQL 1책임 | 저장 로직을 컨텐츠에서 분리해 Job 단위로 측정 가능하게(`g_TPS/g_QueryProcTime`) — "어떤 Job이 얼마나 나가는지"를 수치로 본 것이 병목 분석의 시작. ItemCountUpdateJob은 분석 대상 중 하나였을 뿐 근본 원인이 아님 |
| `ItemSlotUpdateJob` | CASE WHEN 다건 한 방 UPDATE | 행당 UPDATE N번 → 문장 1번. 배치와 별개의 문장 수준 개선 |
| `DBJob::operator new/delete` + `CSizeClassMemoryPoolTLS` | 크기 클래스(32~512B)별 TLS 풀에서 Job 할당 | Job 생성이 초당 수천 건 — 힙 할당을 크기별 풀로 대체. new/delete 오버로딩이라 Job 코드는 풀의 존재를 모름 |

---

## 10. 패킷/프로토콜 — `PacketBuilder.h/.cpp`, `Common/Contents/*`

### 프로토콜 정의 의도 (`ContentsProtocol.h`)

- 모든 패킷은 상수 ID + **주석으로 바이트 레이아웃 명세**(예: `PACKET_CS_SWING_LEFT_ATTACK 1011 — 4 AttackYaw(float), 1 SwingIdx(uint8)`). 직렬화 버퍼는 순서를 지켜야 쓸 수 있으므로, 명세가 곧 문서이자 계약이 되게 헤더에 인라인.
- CS(클라→서버) 패킷은 **입력·의도만** 담는다: 이동은 좌표+yaw+moveFlag, 공격은 yaw+swingIdx, 스킬은 슬롯. 타겟·데미지·결과 필드가 아예 없음 — 프로토콜 수준에서 치팅 표면을 제거.
- SC(서버→클라) 패킷은 서버 확정 결과만: HitTarget(대상+데미지+남은HP ratio), 캐릭터 생성(타 캐릭터는 HP 미포함). **UID는 클라에 노출하지 않고** 슬롯 타입+인덱스로만 대화 — 복제/위조 요청의 근거 자체를 차단.
- 캐릭터 초기화는 사용 슬롯만 가변 전송(빈 슬롯 제외) — 패킷 크기 절약.
- `ContentsDefine/Enum/Struct/Type.h`: 상수·열거·구조체를 서버/테스터 공용으로 — LoadTester의 와이어 검증이 서버와 같은 정의로 돌게.

### `PacketBuilder`

패킷 조립을 핸들러에서 분리한 전용 클래스. 같은 패킷을 여러 곳(입장 스냅샷/브로드캐스트)에서 만들 때 레이아웃 불일치가 나지 않게 **조립 지점을 함수 하나로 고정**. 함수명이 프로토콜 ID와 1:1이라 프로토콜 문서→코드 추적이 즉시 된다.

---

## 11. 부하 테스터 — `Server/LoadTester/`

| 파일/함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `LoadTestManager::Init/Run/WorkerLoop` | IOCP 기반 봇 다중 접속(분할 접속/RST 등 제어) | 서버와 같은 IOCP 모델로 만든 이유: 테스터 자신이 병목이 되지 않아야 측정이 유효(P1 더미 recv 병목의 교훈) |
| `BehaviorLoop` + `DoMove/DoAttack/DoSkill/DoUseItem/DoSwapSlot/DoPickup` | 봇 행동 시나리오 | 이동만이 아니라 전투·아이템까지 섞은 행동 프로파일 — DB 저장 Job이 실제 비율로 발생해야 병목이 재현됨 |
| `ParseFrames/HandlePacket` | 수신 스트림 조립·프로토콜 해석 | 서버가 보낸 것을 "받는"게 아니라 "검증"한다 — 응답 파싱 실패 자체가 버그 신호 |
| `ValidateInventoryTail` 등 검증 | 값 범위·ID 일관성 검사 | **BadValue 계열 버그를 잡은 주체.** 부하 도구에 검증을 심어 부하와 정합성 검사를 동시에 수행 |
| `ByteStream.h`/`DummyClient.h` | 직렬화/봇 상태 | 서버의 CMessage와 같은 규약의 경량판 |

본문 확인으로 구체화한 설계 의도:

- **분할 접속**(`Run`): 접속을 초당 `rampPerSec`개로 나눠 연다. 서버 accept가 단일 직렬 루프라, 수천 개를 한 번에 열면 서버 상태와 무관하게 적체되어 측정이 오염되기 때문 — 접속 폭주와 정상 부하를 구분하려는 통제 장치.
- **RST 웨이브 / 재접속**(`RequestRST`/`ResetForReconnect`): SO_LINGER0으로 강제 끊고 슬롯을 재사용해 재접속 폭주 시나리오를 만든다 — 세션 수명 관리(입퇴장 반복)까지 부하 범위에 포함.
- **스레드 소유권 규약**(`DummyClient.h` 주석): 수신 필드는 워커 스레드, 이동 필드는 행동 스레드가 소유하고, 경계를 넘는 필드만 atomic — 테스터 자신의 동시성 버그가 측정을 오염시키지 않게 소유권을 주석으로 명문화.
- **랜덤워크 anchor**: 스폰 좌표를 첫 스폰에만 고정하고 재접속에도 유지 — 로그아웃 위치 드리프트가 누적되어 봇이 몬스터 섹터를 이탈하는 것을 방지(부하 밀도 유지).
- **스폰 타임아웃/끊김 원인 분류**(`CountConnReset`, `m_shuttingDown`): 접속 리셋을 스폰 전/세션 중으로 나눠 집계하고, 종료 중의 끊김은 서버 오류로 세지 않는다 — 오탐 없는 오류 통계가 목적.
- **행동 슬롯 순환**(`DoSkill` 슬롯 0~3, `DoCleanupInventory`): 버프/공격 스킬을 순환시키고 주운 아이템을 주기 삭제 — 특정 Job만이 아니라 INSERT/DELETE/UPDATE가 지속 발생하게 해 DB 부하를 실제 비율로 유지.

---

## 12. 확인 필요

| 항목 | 내용 |
|---|---|
| `CUser.cpp` 세부(1344줄) | 데미지 계산식·아이템 로드(랜덤스탯) 본문 일부 미정독 |
| `LoadTestManager.cpp` 본문 | 행동 시나리오 구현 상세(헤더 수준 확인) |
| `PacketBuilder.cpp` 바디 | 각 패킷 직렬화 순서를 프로토콜 주석과 전수 대조하지 않음 |
| 토큰 인증 todo | AuthGroup 인증은 측정용 스텁 — 발표 시 한계로 명시 |
| 측정 CSV 실값 | UserCount별 dbQueue/RTT/싱크 수치 미판독(제출물 인용 전 확정) |
