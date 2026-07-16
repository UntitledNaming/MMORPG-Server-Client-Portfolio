# Code Flow

이 문서는 P4 월드 서버의 코드 흐름을 파일별 나열이 아니라 기능 흐름 기준으로 정리합니다. README보다 자세하되, 함수 전체 목록이 아니라 각 흐름의 핵심 함수만 설명합니다. getter/setter, 단순 helper는 생략합니다. 코드 흐름은 실제 코드 기준으로 작성했으며, 확인이 더 필요한 부분은 "확인 필요"에 남깁니다.

관련 파일 경로는 `Server/MMORPG_GameServer/` 기준입니다.

---

## 1. 서버 시작 / 모듈 초기화 흐름

- 목적: 게임 라이브러리·그룹·DB·모니터링을 준비해 서비스 상태로 만든다.
- 관련 파일: `MMORPG_GameServer.cpp`(main), `GameServer.cpp`, `CDBManager.cpp`
- 관련 클래스: `GameServer`, `CGameLibrary`(P1), `AuthGroup`, `FieldGroup`, `CDBManager`, `ProcessMonitor`
- 관련 함수: `GameServer::GameServer()/Init()`, `CGameLibrary::AttachGroup()/Run()`, `CDBManager::Init()`, `GameServer::Monitoring()/StoreThread()`
- 처리 순서:
  1. `main`이 `GameServer` 객체를 만들고, 생성자에서 서버가 기동한다.
  2. `Init()`이 아이템/드랍/UID 풀, 아이템 테이블, 모니터 스냅샷 풀·저장 큐를 초기화하고, `CGameLibrary`·`CDBManager`·`AuthGroup`·`FieldGroup`·`ProcessMonitor`를 생성한다.
  3. `CDBManager::Init()`이 DB 접속 정보를 읽어 DB 커넥션(DBTLS)과 DB Job 큐를 만들고 **DB 스레드**를 기동한다.
  4. DB 준비 후 아이템 UID 범위를 미리 확보하는 Job을 넣는다.
  5. 그룹에 DB 매니저 포인터를 전달하고, 모니터링 스레드와 CSV 저장 스레드를 기동한다.
  6. `AttachGroup`으로 `AuthGroup`·`FieldGroup`을 등록한 뒤 `Run()`으로 P1 게임 라이브러리(워커/Accept/프레임 스레드)를 돌린다.
- 설계 의도: 네트워크·프레임은 P1 라이브러리에 위임하고, P4는 컨텐츠 그룹과 DB·모니터링만 구현해 관심사를 분리한다.
- 관련 테스트: 부하 테스트 시작 시 초기화 안정성.
- 관련 트러블슈팅: 없음.
- 확인 필요: 선택적 Send 스레드 활성화 여부 등 라이브러리 설정 세부.

---

## 2. 접속 / 로그인 / 인증 흐름

- 목적: 접속한 클라이언트를 인증하고 캐릭터 선택까지 처리해 필드 진입을 준비한다.
- 관련 파일: `AuthGroup.cpp`, `DBJob.cpp`(CharacterSelectJob)
- 관련 클래스: `AuthGroup`(`CGroup` 상속), `CUser`, `CharacterSelectJob`
- 관련 함수: `AuthGroup::OnClientJoin/OnRecv/OnUpdate`, `LoginRequestProc`, `CharacterSelectProc`, `CharacterSelectJob::Execute/OnComplete`
- 처리 순서:
  1. 접속 시 라이브러리가 세션을 `AuthGroup`에 붙이고 `OnClientJoin`이 로그인 전 유저 테이블에 등록한다.
  2. `OnRecv`가 로그인 요청이면 `LoginRequestProc`(유저 객체 생성), 캐릭터 선택이면 `CharacterSelectProc`.
  3. `CharacterSelectProc`가 `CharacterSelectJob`을 만들어 DB 매니저에 넣는다(읽기 Job, 결과를 그룹 큐로 되돌려 받음).
  4. DB 스레드가 `CharacterSelectJob::Execute`로 캐릭터·아이템을 조회해 Job에 채운 뒤 그룹으로 되돌린다.
  5. `AuthGroup::OnUpdate`가 완료된 Job의 `OnComplete`를 호출 → 유저에 데이터를 적용하고 `GroupMove("Field")`로 필드로 이동시킨다.
- 설계 의도: 인증/입장 로직을 `AuthGroup`으로 격리하고, DB 읽기를 비동기 Job으로 돌려 네트워크 스레드를 막지 않는다.
- 관련 테스트: 부하 테스트의 접속·캐릭터 선택 경로.
- 관련 트러블슈팅: BadValue(잘못된 DB 접속으로 인한 로드 이상).
- 확인 필요: 로그인 요청의 토큰 인증은 코드상 미구현(주석 처리) 상태다. 부하·기능 검증 목적의 임시 구현으로 두었으며, 실서비스라면 인증 검증이 추가되어야 한다.

---

## 3. 캐릭터 로드 / 스폰 흐름

- 목적: DB에서 캐릭터·아이템을 로드해 유저 객체에 적용하고, 필드에 등장시켜 주변에 알린다.
- 관련 파일: `DBJob.cpp`(CharacterSelectJob), `CUser.cpp`(LoadDataFromDB), `FieldGroup.cpp`(OnIUserMove), `PacketBuilder.cpp`
- 관련 클래스: `CharacterSelectJob`, `CUser`, `FieldGroup`, `PacketBuilder`
- 관련 함수: `CharacterSelectJob::Execute/OnComplete`, `CUser::LoadDataFromDB`, `FieldGroup::OnIUserMove`, `PacketBuilder::CreateMyCharacter/CreateOtherCharacter/CreateMonster/CreateFieldDropItem`
- 처리 순서:
  1. `CharacterSelectJob::Execute`가 캐릭터 테이블(레벨·경험치·위치)과 아이템 테이블(슬롯·랜덤 스탯)을 조회한다.
  2. `OnComplete` → `CUser::LoadDataFromDB`가 스탯·위치·인벤/장비/퀵슬롯을 세팅한다.
  3. `GroupMove("Field")` → `FieldGroup::OnIUserMove`가 유저를 필드 조회 테이블과 현재 섹터에 등록한다.
  4. 본인에게 내 캐릭터 생성 패킷(`CreateMyCharacter`)을 보낸다.
  5. 주변 9섹터를 순회하며, 주변 유저들에게 내 캐릭터 생성을 뿌리고, 주변 섹터의 타 유저·몬스터·드랍 아이템 생성 패킷을 나에게 보낸다.
- 설계 의도: DB 읽기(느림)와 필드 등장(빠름)을 분리하고, 등장 시점에 주변 9섹터 상태를 한 번에 만들어 클라이언트 화면을 채운다.
- 관련 테스트: 부하 테스트의 스폰 경로.
- 관련 트러블슈팅: BadValue.
- 확인 필요: `LoadDataFromDB`의 슬롯 로드·랜덤 스탯 적용 세부는 일부 grep 기준이며 본문 재확인 권장.

---

## 4. 이동 패킷 처리 흐름

- 목적: 클라이언트 이동 입력을 받아 서버 위치를 갱신하고, 싱크를 검증하며, 주변에 브로드캐스트한다.
- 관련 파일: `FieldGroup.cpp`(HandleCharacterMovementUpdate, SectorUpdate), `CUser.cpp`, `PacketBuilder.cpp`
- 관련 클래스: `FieldGroup`, `CUser`, `PacketBuilder`
- 관련 함수: `HandleCharacterMovementUpdate`, `CUser::SetMoveYaw/SetMoveFlag/SetLocation`, `SectorUpdate`, `PacketBuilder::SyncMyCharacter/UpdateCharacterMovement`
- 처리 순서:
  1. `OnRecv`에서 이동 입력 패킷이면 `HandleCharacterMovementUpdate`로 진입한다.
  2. 좌표·yaw·moveFlag를 추출하고 유저를 찾는다(사망 시 리턴).
  3. yaw·moveFlag를 세팅한다.
  4. 서버가 아는 좌표와 클라 좌표의 차이가 싱크 임계를 벗어나면, 싱크 카운트를 올리고 서버 좌표로 강제 보정하는 싱크 패킷을 보낸다. 임계 안이면 클라 좌표를 신뢰해 위치를 갱신한다.
  5. 새 좌표로 섹터를 계산해 `SectorUpdate`(섹터가 바뀌면 AOI 갱신)를 호출한다.
  6. 이동 갱신 패킷을 주변 9섹터에 브로드캐스트한다.
- 설계 의도: 정상 범위 안에서는 클라 좌표를 믿어 연산을 줄이고, 이상치만 싱크로 교정한다.
- 관련 테스트: 3500/4000 동접 싱크 발생 지표.
- 관련 트러블슈팅: 타 캐릭터 스냅샷 보간(클라 영역).
- 확인 필요: 좌표·Z축의 정밀 검증 일부는 코드상 미구현(주석). 싱크 임계 상수와 끊기 로직 활성화 여부.

---

## 5. AOI / 섹터 갱신 흐름

- 목적: 섹터 이동 시 시야에 들어오고 나가는 대상만 생성/삭제로 처리한다.
- 관련 파일: `FieldGroup.cpp`(SectorUpdate), `SectorPos.cpp`, `FieldSector.cpp`, `MonsterAI.cpp`(UpdateSector)
- 관련 클래스: `SectorPos`, `FieldSector`, `FieldGroup`, `MonsterAI`
- 관련 함수: `SectorPos::SectorFind/CalSectorTransitionMessageTargets/SameSector`, `FieldGroup::SectorUpdate`, `MonsterAI::UpdateSector`
- 처리 순서:
  1. 좌표로 현재 섹터를 계산하고, 이전 섹터와 같으면 무시한다.
  2. 이전 섹터에서 제거하고 새 섹터에 추가한다.
  3. `CalSectorTransitionMessageTargets`로 새로 보이는 섹터와 사라지는 섹터의 차집합을 구한다.
  4. 사라진 섹터의 유저들에게 내 삭제 패킷을, 새 섹터 유저들에게 내 생성 패킷을 보낸다.
  5. 사라진/새 섹터의 유저·몬스터·드랍 아이템 생성/삭제를 나에게 보낸다.
- 설계 의도: 전체 브로드캐스트 대신 섹터 기반 관심영역으로 트래픽을 제한하고, 이동 시 차집합만 갱신해 확산 전송 비용을 줄인다.
- 관련 테스트: 부하 테스트의 브로드캐스트 지표.
- 관련 트러블슈팅: 몬스터 생성/삭제 중복(섹터 경계 처리).
- 확인 필요: `CalSectorTransitionMessageTargets`의 경계 처리 본문 재확인 권장.

---

## 6. 타 캐릭터 스냅샷 보간 흐름

- 목적: 다른 캐릭터·몬스터를 부드럽게 보여주기 위해 위치 스냅샷을 주고받는다.
- 관련 파일(서버 측): `FieldGroup.cpp`(UpdateCharacterMovement/MoveMonster 전송), `PacketBuilder.cpp`, `CUser.cpp`(UserOnUpdate)
- 관련 클래스: `FieldGroup`, `PacketBuilder`, `CUser`, `CMonster`
- 관련 함수: `PacketBuilder::UpdateCharacterMovement/SyncMyCharacter/MoveMonster/StopMonster`, `FieldGroup::UserUpdate`
- 처리 순서(서버 측):
  1. 이동 입력 수신 시 위치+yaw+moveFlag를 주변에 전파하고, 이동 브로드캐스트에는 서버 타임스탬프를 함께 담는다.
  2. 몬스터는 목적지 좌표 기반으로 이동 패킷/정지 패킷을 보내 클라이언트가 그 목적지까지 보간하도록 한다.
  3. 렌더링 지연·보간·역행/외삽 처리는 **클라이언트(UE5) 영역**이다. 서버는 입력·목적지 스냅샷을 제공한다.
- 설계 의도: 서버는 권위 위치와 목적지만 전달하고 표현(보간)은 클라에 위임해 서버 부하를 낮춘다.
- 관련 테스트: 부하 테스트의 이동/브로드캐스트 지표.
- 관련 트러블슈팅: 타 캐릭터 스냅샷 보간 문제(클라 렌더링 이슈).
- 확인 필요: 보간/역행/외삽의 실제 구현은 이 레포의 서버 소스 범위 밖(UE5 클라이언트)이다.

---

## 7. 기본 공격 / 서버 권위 전투 판정 흐름

- 목적: 클라이언트 공격 입력을 서버가 검증·판정하고 데미지/드랍/경험치를 처리한 뒤 결과를 브로드캐스트한다.
- 관련 파일: `FieldGroup.cpp`(HandleLeftAttackSwing, CollectHitTarget, SendPacket_HitSectors), `HitSearchBuilder.cpp`, `CollisionCheck.cpp`, `CUser.cpp`, `CMonster.cpp`
- 관련 클래스: `FieldGroup`, `HitSearchBuilder`, `HitSearchInfo`, `HitResult`, `CollisionCheck`, `CUser`, `CMonster`
- 관련 함수: `HandleLeftAttackSwing`, `HitSearchBuilder::MakeBaseAttack`, `CollectHitTarget`, `CollisionCheck::IsInCircle/IsInCone/IsInBox`, `CUser::CalBaseAttackDamage/Damage/GainExp`, `FieldGroup::CreateFieldDropItem`, `SendPacket_HitSectors`
- 처리 순서:
  1. 클라이언트는 공격 방향(yaw)과 스윙 종류를 보낸다(범위를 벗어나면 연결 종료).
  2. 서버가 공격자 위치·스탯을 사용해 `MakeBaseAttack`으로 타격 정보(도형·사거리·반각)를 만든다.
  3. `CollectHitTarget`이 사거리로 겹치는 섹터 범위를 계산해 그 섹터의 유저·몬스터만 순회한다.
  4. 도형에 따라 `IsInCircle`(원)/`IsInCone`(부채꼴)/`IsInBox`(직사각형)로 명중을 판정한다. 거리·각도 비교는 제곱·내적으로 처리해 제곱근 연산을 피한다.
  5. `CalBaseAttackDamage`로 데미지를 계산하고 `Damage`로 HP를 반영한다.
  6. 몬스터가 죽으면 드랍을 생성(`CreateFieldDropItem`)하고 경험치를 누적한다. 경험치는 `GainExp`로 반영한다.
  7. 공격 스윙을 주변에, 피격 결과를 피격자/피격 몬스터 주변 섹터에 브로드캐스트한다. 죽은 몬스터는 삭제 패킷 후 섹터에서 제거한다.
- 설계 의도: 명중·데미지를 전적으로 서버가 판정해 치팅을 막고, 섹터 1차 거르기 + 제곱근 회피로 광역 판정 비용을 낮춘다.
- 관련 테스트: 부하 테스트의 공격 요청 지표.
- 관련 트러블슈팅: Damage UnderFlow(장비 스탯 계산).
- 확인 필요: `CalBaseAttackDamage`/`Damage`/`GainExp` 본문은 일부 grep 기준이며 재확인 권장.

---

## 8. 스킬 처리 흐름

- 목적: 스킬 슬롯 사용을 MP/쿨다운으로 검증하고, 버프/광역 판정을 처리해 결과를 브로드캐스트한다.
- 관련 파일: `FieldGroup.cpp`(HandleSkillUse), `CUser.cpp`(CanUseSkill/UseSkill/CalSkillDamage), `HitSearchBuilder.cpp`
- 관련 클래스: `FieldGroup`, `CUser`, `HitSearchBuilder`
- 관련 함수: `HandleSkillUse`, `CUser::CanUseSkill/UseSkill/CalSkillDamage`, `HitSearchBuilder::MakeSkillAttack`, `PacketBuilder::UseSkillRes/UseSkillBroadCast`
- 처리 순서:
  1. 스킬 슬롯을 추출한다(범위를 벗어나면 연결 종료).
  2. `CanUseSkill`로 MP·쿨다운을 확인하고, 가능하면 `UseSkill`로 MP 차감·쿨/만료 시간을 세팅한다.
  3. 성공/실패 응답을 시전자에게, 성공 시 시전 브로드캐스트를 주변에 보낸다.
  4. 버프 슬롯이면 여기서 종료한다. 공격 스킬이면 `MakeSkillAttack`으로 타격 정보를 만들고, 평타와 같은 히트 판정 처리 절차(CollectHitTarget → 데미지 → 드랍/경험치 → 브로드캐스트)을 재사용한다.
- 설계 의도: 버프/공격 스킬을 슬롯 구간으로 분기하고, 공격 스킬은 평타와 동일한 판정 처리 절차을 재사용해 일관성을 확보한다. MP·쿨다운은 서버가 관리한다.
- 관련 테스트: 부하 테스트의 스킬 요청 지표.
- 관련 트러블슈팅: 없음.
- 확인 필요: 스킬별 도형/범위/데미지 계수(`SkillTable.h`) 세부.

---

## 9. 몬스터 AI Update 흐름

- 목적: 몬스터를 상태 기계로 구동해 순찰/추격/전투/복귀를 처리하고 관련 패킷을 뿌린다.
- 관련 파일: `MonsterAI.cpp`, `CMonster.cpp`, `FieldGroup.cpp`(MonsterUpdate, 몬스터 전송)
- 관련 클래스: `MonsterAI`, `CMonster`, `FieldGroup`
- 관련 함수: `MonsterAI::Update/UpdateIdle/UpdatePatrol/UpdateChase/UpdateReturn/UpdateCombat`, `Enter*`, `FindNearestPlayer`, `TargetUpdate`, `IsAttackRange/IsChaseRange`, `CMonster::MonsterUpdate`
- 처리 순서:
  1. Idle: 감지 범위에 플레이어가 있으면 Chase, 없으면 일정 시간 후 Patrol.
  2. Patrol: 감지 시 Chase, 아니면 랜덤 목적지로 이동하고 도착하면 잠깐 정지 후 Return.
  3. Chase: 타겟 사망/이탈 시 Return, 스폰과 멀어지면 Return, 아니면 목표 갱신 후 이동, 공격 범위에 들면 Combat.
  4. Combat: 공격 쿨마다 방향을 갱신하고 부채꼴 판정으로 명중 시 데미지·공격 패킷을 보낸다.
  5. Return: 스폰으로 이동, 도착하면 Idle.
  6. Dead: 리젠 시간이 지나면 부활해 섹터에 등록하고 주변에 생성 패킷을 보낸다.
- 설계 의도: 상태 기계로 AI를 단순화하고, 추격 목표 갱신 빈도 제한과 전투 전환 반경 여유로 패킷·상태 진동을 억제한다.
- 관련 테스트: 부하 테스트의 몬스터 관련 브로드캐스트 지표.
- 관련 트러블슈팅: 몬스터 Chase/Combat 떨림, 몬스터 생성/삭제 중복.
- 확인 필요: `MonsterConst`의 각 상수 값과 `CMonster.cpp` 본문 세부.

---

## 10. 필드 드랍 아이템 흐름

- 목적: 몬스터 사망 시 드랍을 생성해 섹터에 등록하고, 시야/만료/줍기를 처리한다.
- 관련 파일: `FieldGroup.cpp`(CreateFieldDropItem, FieldDropItemExpired, HandlePickUpItems), `FieldDropItemPool.cpp`, `CUser.cpp`
- 관련 클래스: `FieldDropItem`, `FieldDropItemPool`, `FieldSector`, `FieldGroup`, `CUser`
- 관련 함수: `FieldGroup::CreateFieldDropItem/FieldDropItemExpired/HandlePickUpItems`, `FieldDropItemPool::CreateItem/FreeItem`, `CUser::GetConsumableItem/GetEquipmentItem`
- 처리 순서:
  1. 몬스터가 죽으면 `CreateFieldDropItem` → `FieldDropItemPool::CreateItem`이 드랍 확률·랜덤 스탯으로 아이템을 만들고 DropID를 부여한다.
  2. 조회 테이블과 섹터에 등록하고, 주변 섹터에 생성 패킷을 보낸다.
  3. 프레임마다 `FieldDropItemExpired`가 만료된 드랍을 삭제 패킷 전파 후 섹터·풀에서 제거한다.
  4. 줍기 요청 시 `HandlePickUpItems`가 DropID로 드랍을 찾아, 소모품/장비 타입에 따라 인벤에 넣고 성공 시 삭제 패킷·픽업 결과를 보낸 뒤 드랍을 제거한다.
- 설계 의도: 드랍을 풀로 재사용해 할당 비용을 줄이고, 섹터 기반 생성/삭제로 필요한 클라에게만 노출한다.
- 관련 테스트: 부하 테스트의 줍기/드랍 지표.
- 관련 트러블슈팅: BadValue(드랍 초기화 누락).
- 확인 필요: 드랍 확률·랜덤 스탯 규칙 본문 세부.

---

## 11. 인벤토리 / 장비 / 퀵슬롯 / Storage 흐름

- 목적: 아이템 소유를 단일 저장소로 관리하고 슬롯 배치와 사용을 처리한다.
- 관련 파일: `CUserItemStorage.cpp`, `Inventory.cpp`, `Equipment.cpp`, `QuickSlot.cpp`, `ItemUIDAllocator.cpp`, `CUser.cpp`
- 관련 클래스: `CUserItemStorage`, `Inventory`, `Equipment`, `QuickSlot`, `ItemUIDAllocator`, `CUser`
- 관련 함수: `CUserItemStorage::CreateItem/DeleteItem/ChangeItemCount/CollectDirtyItems/SetItemDirtyFlag`, `Inventory::InsertItemToSlot/ItemSlotChange`, `Equipment::EquippedItem/UnEquippedItem`, `ItemUIDAllocator::Alloc`, `CUser::UseInventoryItem/UseEquipmentItem/UseQuickSlotItem/ItemSlotChange`
- 처리 순서:
  1. `CUserItemStorage`가 아이템 실체(UID→아이템)를 단일 관리하고, `Inventory`/`Equipment`/`QuickSlot`은 슬롯에 UID만 담는 뷰다.
  2. 아이템 UID는 `ItemUIDAllocator`가 DB에서 미리 확보한 범위에서 발급한다(런타임 DB 왕복 없음).
  3. 소모품 사용은 쿨타임 확인 후 개수 감소(0이면 삭제), 장비 장착/해제는 인벤↔장비 슬롯 교환과 스탯 재계산으로 처리한다.
  4. 슬롯 변경 시 더티 표시(`SetItemDirtyFlag`) → 주기적으로 더티 아이템을 모아(`CollectDirtyItems`) DB Job으로 일괄 저장한다.
- 설계 의도: 아이템 실체를 한 곳에 두고 슬롯은 UID 참조만 하게 해 정합성을 지키고, 변경분만 더티로 모아 DB 쓰기를 줄인다.
- 관련 테스트: 부하 테스트의 아이템 사용/스왑/삭제 지표.
- 관련 트러블슈팅: 없음.
- 확인 필요: 슬롯 스왑 규칙·스탯 합산 본문 세부.

---

## 12. DB Job Queue 저장 흐름

- 목적: 컨텐츠 변경을 비동기 Job으로 만들어 단일 DB 스레드가 트랜잭션 배치로 저장한다.
- 관련 파일: `DBJob.cpp`, `CDBManager.cpp`, `CUser.cpp`(Job 생성부), `CUserItemStorage.cpp`
- 관련 클래스: `CDBManager`, `DBJob`(및 파생), `CUserItemStorage`
- 관련 함수: `CDBManager::EnqueueDBJob/DBThread`, `DBJob::Execute/OnComplete`, `CUser::CharacterProgressUpdate/ItemSlotUpdate`, `CUserItemStorage::SetItemDirtyFlag/CollectDirtyItems`
- 처리 순서:
  1. 컨텐츠 로직이 상황에 따라 DBJob을 만든다(아이템 삽입/삭제/개수 변경/슬롯 변경, 캐릭터 진행도, 로그아웃, 캐릭터 선택 읽기 등).
  2. `EnqueueDBJob`이 Job을 DB 큐에 넣고 DB 스레드를 깨운다.
  3. DB 스레드가 큐를 비우며, 쓰기 Job들을 `START TRANSACTION … 여러 건 실행 … COMMIT`로 묶어 저장한다. 읽기/즉시 반영이 필요한 Job은 열린 배치를 먼저 커밋한 뒤 단독 처리하고 결과 큐로 되돌린다.
  4. 캐릭터 진행도는 더티 플래그가 켜졌을 때만, 누적 타이머 주기로 저장한다. 아이템 슬롯은 더티 아이템을 모아 일괄 저장한다.
- 설계 의도: 네트워크·프레임 스레드를 DB 지연에서 분리하고, 변경분만·주기적으로·묶어서 저장해 커밋·디스크 IO를 줄인다. Job별 쿼리 처리시간과 DB 스레드 단계별 시간을 측정해 병목 위치를 데이터로 특정한다.
- 관련 테스트: UserCount별 부하 테스트, DB 큐 관측.
- 관련 트러블슈팅: DB 저장 큐 폭증.
- 확인 필요: DB 접속 래퍼(DBTLS) 본문 세부.

---

## 13. 부하 테스트 / 모니터링 흐름

- 목적: 다수 봇으로 실부하를 만들고 지표를 CSV로 남겨 병목·안정성을 검증한다.
- 관련 파일: `Server/LoadTester/`(별도 프로젝트), `GameServer.cpp`(Monitoring/StoreThread), `MonitoringSnapShot.h`, `LatencyHistogram.h`
- 관련 클래스: `LoadTestManager`, `DummyClient`, `GameServer`, `MonitorSnapshot`, `LatencyHistogram`, `ProcessMonitor`
- 관련 함수: `LoadTestManager::Run/WorkerLoop/BehaviorLoop/ParseFrames/HandlePacket`, `GameServer::Monitoring/StoreProc`
- 처리 순서:
  1. `LoadTester`가 IOCP 기반으로 봇을 접속(분할 접속)시키고, 이동/공격/RTT/스킬/줍기/아이템 사용 등을 주기적으로 보내 실부하를 만든다.
  2. 봇은 수신 프레임을 파싱해 서버 이상(값 범위 밖, 중복 생성/모르는 삭제 등)을 와이어 레벨에서 검출한다.
  3. 서버는 프레임마다 지연 히스토그램·CPU·큐 깊이·네트워크 지표를 스냅샷으로 만들어 `monitor.csv`에 기록한다.
- 설계 의도: 부하 도구가 서버 이상을 자동 검출하고, 서버는 지연을 백분위로 남겨 평균이 아니라 꼬리까지 병목을 정량화한다.
- 관련 테스트: UserCount 100~4000 부하, 안정성 로그.
- 관련 트러블슈팅: DB 저장 큐 폭증, 안정성 로그 3종.
- 확인 필요: `LoadTester` 본문 세부와 CSV 실측 수치.

> 각 흐름을 그렇게 설계한 이유는 `docs/Design_Rationale.md`, 프로토콜은 `docs/Protocol_Design.md`, 측정 결과는 `docs/Test_Report.md`, 문제 해결은 `docs/Troubleshooting.md`를 참고하세요.
