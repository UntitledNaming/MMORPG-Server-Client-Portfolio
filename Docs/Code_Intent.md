# 코드 의도 문서 (Code Intent) — P4 MMORPG 월드 서버

> 이 문서는 P4 고유 코드(.h/.cpp, 테스트 코드, 프로토콜)를  **"왜 이렇게 작성했는가"** 중심으로 설명한다.
> 동작 흐름은 `Docs/Code_Flow.md`, 프로토콜 상세는 `Docs/Protocol_Design.md`, DB 병목은 `Docs/Troubleshooting.md` 참고.
> `NetworkLib/`(P1 라이브러리 포함분)의 의도는 P1 저장소의 `docs/Code_Intent.md`에 있다. 이 문서는 P4가 그 위에 올린 것만 다룬다.

---

## 0. 저장소 구조 자체의 의도

| 폴더 | 의도 |
|---|---|
| `NetworkLib/` | P1에서 구현·검증한 네트워크·게임 라이브러리를 **그대로 포함** — "P1이 P4의 기반"이라는 연결의 물리적 증거 |
| `Common/Contents/` | 프로토콜·상수·구조체를 서버와 부하 테스터가 공유  |
| `Server/MMORPG_GameServer/` | 월드 서버 본체. 컨텐츠는 전부 `CGroup` 상속 그룹(AuthGroup/FieldGroup)으로 — 라이브러리의 직렬 처리 보장 위에서만 동작 |
| `Server/LoadTester/` | 부하 봇. 서버와 같은 프로토콜 헤더를 쓰는 독립 IOCP 클라이언트 |
| `Server/Docs/` (측정 자료/안정성 테스트) | 부하·버그 로그 원본 보존  |
| `Database/` | 스키마·시퀀스. `uid_sequence`는 ItemUID 블록 할당의 전제 |

---

---

## 1. 인증 그룹 — `AuthGroup.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `OnClientJoin/OnClientLeave` | NonUser/User 테이블 관리 | P1 검증 서버들과 동일한 미인증 격리 패턴 재사용 |
| `LoginRequestProc` | 형식 검증 → CUser 생성 (토큰 인증은 todo 상태의 임시 처리) | 부하 측정이 목적이라 인증 강도보다 입장 처리량 경로를 우선 구현. |
| `CharacterSelectProc` | CharacterSelectJob(flush) 투입 | 캐릭터 로드는 이전 Job들 다 커밋 해서 DB 반영 해주고 읽기 위해 배치에 섞이면 안 됨  |
| `OnUpdate` | DB 완료 Job 소비 → `GroupMove("Field")` | DB 응답을 그룹 프레임에서 소비 |

---

## 2. 월드 본체 — `FieldGroup.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `OnIUserMove` | 필드 등록 + 주변 9섹터 생성 스냅샷 전송 | 입장 순간의 "보이는 세계"를 서버가 확정해서 보냄  |
| `OnRecv` | 9종 핸들러 분기 | 프로토콜 ID → 핸들러 1:1. 핸들러 밖에서 검증하지 않음 |
| `HandleCharacterMovementUpdate` | 좌표+yaw+moveFlag 수신 → 싱크 임계 검증 → 섹터 갱신 → 브로드캐스트 | 클라 좌표를 받되 서버 이동 시뮬레이션과의 오차가 임계를 넘으면 싱크 패킷으로 교정  |
| `HandleLeftAttackSwing` / `HandleSkillUse` | 방향/슬롯만 받아 서버가 대상·데미지 판정 | 논타겟이라 **타겟 대상을 보내지 않음** — 치팅 방지의 핵심. 스킬은 MP/쿨다운/사거리를 서버가 검증 |
| `CollectHitTarget` | 섹터로 후보 1차 거르기 → Circle/Cone/Box 정밀 판정 | 전 유저 순회 대신 섹터로 후보를 줄이고 도형 판정 |
| `SectorUpdate` | 이동 시 차집합 섹터만 생성/삭제 전파 | 9섹터 전체 재전송이 아니라 **차집합만** |
| `OnUpdate` | UserUpdate/MonsterUpdate/FieldDropItemExpired | 프레임 로직을 그룹 락 안에서 — 직렬 처리 보장 |
| `SendPacket_SectorOne/Around/HitSectors` | 섹터 확산 전송 | 확산 범위 범위를 함수 이름으로 구분해 "누구에게 가는 패킷인지"를 호출부에서 읽게 |
| `SendMonster*` 계열 | 몬스터 상태 전파 | 몬스터 이동/타겟 변경을 개별 패킷으로 |
| `MonsterSpawnInit` | 초기 대량 배치 | 부하 테스트의 몬스터 밀도를 섹터당 1개에 넣음|

---

## 3. 유저 — `CUser.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `Alloc/Free` (TLS 풀) | CMPoolTLS<CUser> | 객체 할당 경합 줄이기 |
| `UserOnUpdate` / `Move` | 서버측 이동 전진 + 저장 주기 처리 | 유저 객체 자체가 본인 이동 방향 및 속도를 가지고 이동함. |
| `Damage/IsAlive/ResPawn/GainExp` | HP/사망/부활/경험치 | 상태 변화 전부 서버 계산 — 클라는 결과만 수신 |
| `CalBaseAttackDamage/CalSkillDamage` | 장비 스탯 합산 데미지 | Equipment 합산값을 참조 — HP BadValue(미초기화) 버그의 수정 지점 |
| `CanUseSkill/UseSkill` | MP/쿨다운 검증 | 검증과 소모를 한 함수로 |
| `CharacterProgressUpdate` | dirtyFlag + 저장 주기 도달 시에만 CharacterProgressJob | DB 병목 개선 변한 것만, 주기마다 저장 |
| `ItemSlotUpdate` | CollectDirtyItems → ItemSlotUpdateJob | 아이템 변경을 모아 CASE 한 방 UPDATE로 |

---

## 4. 전투 판정 — `CollisionCheck.h/.cpp`, `HitSearchBuilder.h/.cpp`, `SkillTable.h`

| 항목 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `IsInCircle` | distSq > rangeSq 비교 | sqrt 회피 |
| `IsInCone` | dot·dot ≥ distSq·cos² 비교 | atan/acos 없이 내적만으로 각도 판정 — 콘 판정의 삼각함수 제거 |
| `HitSearchBuilder::MakeBaseAttack/MakeSkillAttack` | 공격 종류→HitSearchInfo(도형/범위) 생성 | 판정 파라미터 생성과 판정 실행을 분리 |

---

## 5. 몬스터 — `CMonster.h/.cpp`, `MonsterAI.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `MonsterAI::Update` + `UpdateIdle/Patrol/Chase/Return/Combat` | 6상태 FSM | 조건 분기 뭉치 대신 상태별 함수 — 상태 추가/수정이 국소화 |
| `Enter*` 계열 | 상태 진입 세팅 + Move 패킷 | 진입 시점에만 전파 |
| `TargetUpdate` | 최소 시간(CHASE_UPDATE_MIN_MS)·최소 거리(TARGET_UPDATE_CM)로 갱신 빈도 제한 | 추격 목표 갱신 패킷을 시간·거리 이중 문턱으로 억제 |
| `FindNearestPlayer` | 섹터 순회 최근접 탐색 | 전 유저가 아닌 주변 섹터만 — AOI를 AI 탐색에도 재사용 |
| `UpdateSector` | 몬스터 AOI 갱신 | 유저와 같은 차집합 규칙  |


---

## 6. 섹터/AOI — `SectorPos.h/.cpp`, `FieldSector.h/.cpp`

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `SectorFind` | 주변 9섹터 산출(경계 클램프) | AOI의 기본 단위. 9섹터=시야 반경을 섹터 크기로 정규화한 것 |
| `CalSectorTransitionMessageTargets` | 구 9섹터와 신 9섹터의 겹치는 부분 제외, 삭제/생성 대상만 산출 | 이동 시 "나갔다 들어온" 섹터만 패킷을 만들기 위한 차집합. |
| `FieldSector::Add/Remove(User/Monster/Item)` | 섹터 원소 관리 | 유저·몬스터·드랍을 같은 섹터 격자에서 관리 — 전투 후보 거르기/AI 탐색/획득 판정이 하나의 공간 인덱스를 공유 |

---

## 7. 아이템 — `CUserItemStorage`, `Inventory`, `Equipment`, `QuickSlot`, `ItemUIDAllocator`, `FieldDropItemPool`, `ItemTable`

핵심 구조 의도: **아이템 실체는 Storage에 UID로 단일 보관, 슬롯(인벤/장비/퀵슬롯)은 UID 참조만.** 슬롯끼리 객체를 주고받으면 복제·유실 버그의 온상이 되므로, 이동/장착/스왑은 전부 "참조 이동"으로 만들었다.

| 파일/함수 | 의도 |
|---|---|
| `CUserItemStorage::CreateItem/DeleteItem/ChangeItemCount` | 아이템 객체 생성 삭제 담당|
| `SetItemDirtyFlag/CollectDirtyItems` | 변경 표시/일괄 수집  |
| `Inventory::InsertItemToSlot/ItemSlotChange` + 빈 슬롯 인덱스 관리 | 슬롯 배열 + 빈 슬롯 재사용  |
| `Equipment::EquippedItem/UnEquippedItem` + `GetATK/GetDEF` | 장착 시점에 스탯 합산 갱신 — 전투 계산이 매번 장비를 순회하지 않게 캐싱 |
| `QuickSlot` | 소모품 참조 슬롯 — 사용 시 실체는 Storage에서 감소 |
| `ItemUIDAllocator::Init/Alloc` | DB에서 UID **블록**을 선확보해 메모리에서 발급 | UID 부여는 DB가 아니라 서버가 주체 |
| `FieldDropItemPool` (`CreateItem/RollFieldDropType/RollEquipSlot`) | 드랍은 DropID로 필드에서만 관리, 줍는 순간 ItemUID 부여 | 임시 개체(필드 드랍)와 유저 소유(인벤)의 생명주기를 분리 — 아무도 안 줍는 아이템에 UID·DB 비용을 쓰지 않음 |

---

## 8. DB — `CDBManager.h/.cpp`, `DBJob.h/.cpp`, `CSizeClassMemoryPoolTLS`

| 함수/클래스 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `CDBManager::EnqueueDBJob` | Job 큐 삽입 + 이벤트 | 컨텐츠 스레드는 DB를 기다리지 않는다 — 비동기 저장의 진입점 |
| `CDBManager::DBThread` | 이벤트 대기 → Job 소비. **10건 단위 트랜잭션 배치**(vector 크기 10), `flush` Job은 열려 있던 배치를 즉시 COMMIT 후 단독 처리 | 병목의 본질이 commit 비용·디스크 IO 횟수였으므로 커밋 횟수 자체를 1/10로. flush 분리는 SELECT류(캐릭터 로드, UID 확보)가 배치 지연에 물리지 않게 하기 위함 |
| `DBJob` 파생 8종 (`CharacterSelect/ItemUIDRangeAllocate/InsertItem/DeleteItem/ItemCountUpdate/ItemSlotUpdate/CharacterProgress/LogOut`) | Job당 SQL 1책임 | 저장 로직을 컨텐츠에서 분리해 Job 단위로 측정 가능하게(`g_TPS/g_QueryProcTime`)  |
| `ItemSlotUpdateJob` | CASE WHEN 다건 한 방 UPDATE | 행당 UPDATE N번 → 문장 1번. 배치와 별개로 쿼리 문장 수준 개선 |
| `DBJob::operator new/delete` + `CSizeClassMemoryPoolTLS` | 크기 클래스(32~512B)별 TLS 풀에서 Job 할당 | Job 생성이 초당 수천 건 — 힙 할당을 크기별 풀로 대체. new/delete 오버로딩이라 Job 코드는 풀의 존재를 모름 |

---

## 9. 패킷/프로토콜 — `PacketBuilder.h/.cpp`, `Common/Contents/*`

### 프로토콜 정의 의도 (`ContentsProtocol.h`)

- 모든 패킷은 상수 ID + **주석으로 바이트 레이아웃 명세**(예: `PACKET_CS_SWING_LEFT_ATTACK 1011 — 4 AttackYaw(float), 1 SwingIdx(uint8)`). 직렬화 버퍼는 순서를 지켜야 쓸 수 있으므로, 명세가 곧 문서이자 계약이 되게 헤더에 인라인.
- CS(클라→서버) 패킷은 **입력·의도만** 담는다: 이동은 좌표+yaw+moveFlag, 공격은 yaw+swingIdx, 스킬은 슬롯.
- SC(서버→클라) 패킷은 서버 확정 결과만: HitTarget(대상+데미지+실제 HP+비율(ratio); 당사자는 HP로·주변은 비율로 사용), 캐릭터 생성(타 캐릭터는 HP 미포함). **UID는 클라에 노출하지 않고** 슬롯 타입+인덱스로만 대화
- 캐릭터 초기화는 사용 슬롯만 가변 전송(빈 슬롯 제외) — 패킷 크기 절약.
- `ContentsDefine/Enum/Struct/Type.h`: 상수·열거·구조체를 서버/테스터 공용으로

### `PacketBuilder`

패킷 조립을 핸들러에서 분리한 전용 클래스. 같은 패킷을 여러 곳(입장 스냅샷/브로드캐스트)에서 만들 때 레이아웃 불일치가 나지 않게 **조립 지점을 함수 하나로 고정**. 

---

