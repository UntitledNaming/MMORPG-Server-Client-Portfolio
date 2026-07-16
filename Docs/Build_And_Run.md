# Build & Run

이 문서는 README보다 자세한 빌드·실행 안내입니다. 실제 IP·포트·계정·비밀번호 같은 민감 정보는 넣지 않으며, Config 키 이름과 예시(placeholder)만 적습니다. 환경에 따라 값은 직접 설정해야 합니다.

---

## 1. 개발 환경

- OS: Windows
- IDE: Visual Studio 2022
- 언어 표준: C++17
- 네트워크: WinSock2 / IOCP
- 데이터베이스: MySQL (`worlddb` 스키마)
- 클라이언트: Unreal Engine 5
- 성능 지표: PDH (Windows 성능 카운터) — 모니터링에 사용

> P4는 Redis를 사용하지 않습니다. 저장소는 MySQL 단일 구성입니다.

---

## 2. 서버 빌드 방법

- 서버 솔루션은 `Server/MMORPG_GameServer/`에, 부하 도구는 `Server/LoadTester/`에 있습니다. 각 솔루션(`.sln`)을 Visual Studio 2022로 엽니다.
- 빌드 구성은 x64 / Release를 기준으로 합니다.
- MySQL include/lib 경로: 서버 프로젝트의 `ThirdParty/mysql/` 아래 include와 lib를 참조합니다. 환경에 맞춰 경로를 조정해야 할 수 있습니다.
- 추가 종속성: `Ws2_32.lib`(WinSock2), `Pdh.lib`(성능 카운터), MySQL 클라이언트 라이브러리.
- 라이브러리 참조: 서버는 이 레포에 포함된 `NetworkLib`(network_library / game_library / common_files)를 참조합니다.
- 빌드 대상: 게임 서버(`MMORPG_GameServer`), 그리고 부하 테스트를 할 경우 `LoadTester`.

> 프로젝트별 정확한 참조 경로·라이브러리 목록은 각 `.vcxproj` 설정을 따릅니다. 환경별 경로 차이는 조정이 필요합니다. (환경별 차이 TODO)

---

## 3. DB 준비

- MySQL을 설치하고 `Database/` 폴더의 스키마(`GameServerQuery.sql` 등)를 적용해 `worlddb`와 관련 테이블(캐릭터·아이템·UID 시퀀스 등)을 만듭니다.
- 부하 테스트용 시드 데이터가 필요하면 `Database/`의 시드 스크립트를 적용합니다.
- DB 계정/비밀번호는 저장소에 포함하지 않습니다. 서버 Config의 DB 접속 키에 환경에 맞는 값을 설정합니다.

> 스키마 파일명과 필요한 테이블 목록은 `Database/` 폴더를 참고하세요. 시드 데이터 규모·구성은 테스트 시나리오에 맞춰 조정합니다. (TODO)

---

## 4. 서버 실행 순서

1. **DB 실행**: MySQL을 실행하고 `worlddb` 스키마가 준비되어 있는지 확인합니다.
2. **World Server 실행**: 게임 서버 Config에 DB 접속 정보·서버 IP·Port를 설정한 뒤 구동합니다. 서버가 기동하면서 아이템 UID 범위 확보 등 초기화가 진행됩니다.
3. **LoadTester 실행(부하 테스트 시)**: 부하 도구 설정에 대상 서버 IP·Port와 봇 수(UserCount) 등을 지정해 실행합니다.
4. **Client 접속**: UE5 클라이언트로 서버에 접속해 기능을 확인합니다.

> 서버·부하 도구·클라이언트의 접속 정보는 각 Config에서 설정합니다. 키 이름은 해당 Config 파일을 참고하세요.

---

## 5. UE5 클라이언트 실행

- 클라이언트는 `Client/` 아래에 있습니다(`M1`, `M1Client` 구조).
- UE5 버전은 프로젝트 설정을 따릅니다. (정확한 버전 TODO)
- 클라이언트에서 접속할 서버 IP·Port를 설정합니다(민감 정보는 저장소에 포함하지 않음).
- 서버를 먼저 구동한 뒤 클라이언트로 접속해 캐릭터 생성·이동·전투·아이템 등을 테스트합니다.

> 클라이언트 빌드·실행의 상세는 UE5 프로젝트 설정에 따릅니다. (TODO)

---

## 6. 부하 테스트 실행

- `LoadTester`를 사용해 다수 봇으로 부하를 만듭니다.
- 설정에서 대상 서버 IP·Port, 봇 수(UserCount), 각 행동(이동·공격·RTT·스킬·줍기·아이템 사용 등)의 주기, 접속 분할 접속 속도, RST 주입 비율 등을 지정합니다.
- 서버는 실행 중 `monitor.csv`에 지연 백분위·DB 큐 깊이·CPU·네트워크 지표 등을 기록합니다. 실행 후 이 CSV로 지표를 확인합니다.
- 주의: 시드 아이템을 소진하는 파괴적 행동(아이템 삭제 등)은 기본적으로 꺼두거나 시드 규모에 맞춰 조정합니다.

> `monitor.csv`의 기록 위치와 컬럼 구성은 서버 코드(`GameServer.cpp`)를 따릅니다. 부하 설정 기본값과 시나리오별 권장 값은 정리 후 보강합니다. (TODO)

---

## 7. 민감 정보 제외

- 실제 서버 IP·Port, DB 계정/비밀번호는 저장소에 포함하지 않습니다. 각 환경의 Config 파일에서 직접 설정하세요.
- 개인 PC의 절대경로(설치 위치 등)는 저장소에 넣지 않습니다.
- 서버 IP·Port는 예시(placeholder)로만 표기하거나 TODO로 둡니다.
- MySQL·UE5·서드파티 라이브러리의 버전에 따라 설정이 달라질 수 있습니다. (버전 호환성 TODO)
