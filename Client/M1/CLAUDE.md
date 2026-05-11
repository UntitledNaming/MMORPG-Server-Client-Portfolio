# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is **M1**, an Unreal Engine 5.7 MMO game client written in C++. It is part of a larger MMO portfolio project that also includes a server. The client connects to a custom TCP server over LAN using Windows IOCP (I/O Completion Ports).

## Build & Development

**Engine:** Unreal Engine 5.7  
**IDE:** Visual Studio 2022 (solution: `M1.sln`)  
**Build targets:** Development Editor, Development, Shipping (standard UE5 targets)

To build: open `M1.sln` in Visual Studio 2022 and build, or use Unreal Editor's built-in compilation. There are no custom Makefile/CMake/npm scripts — everything goes through Unreal Build Tool (UBT).

Module build dependencies are defined in `Source/M1/M1.Build.cs`. Public module dependencies include: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `GameplayTags`, `AnimGraphRuntime`, `UMG`.

**Shared code** lives outside this directory and is included via relative paths in `M1.Build.cs`:
- `../../../../Common/Contents/` — shared game constants, enums, and type definitions (`ContentsDefine.h`, `ContentsEnum.h`, `ContentsType.h`)
- `../../../../Common/Network/` — shared network protocol definitions (`ContentsProtocol.h`) used by both client and server

## Architecture

### Network Layer (most critical)

The entire game is networked. The network stack is custom (not UE5's built-in replication):

- **`Network/ClientCore/CLanGameClient.h`** — Low-level IOCP TCP client (Windows Sockets). Manages send/receive threads and a ring buffer for incoming bytes.
- **`Network/ClientCore/M1Client.h`** — Extends `CLanClient`; bridges raw socket events into UE5 by pushing packets onto a lock-free queue.
- **`Network/ClientCore/CMessage.h`** — Binary message serialization using `<<`/`>>` operators. Packets are length-prefixed.
- **`Network/ClientCore/LFQueue*.h`** / **`LockFreeMemoryPoolLive.h`** — Lock-free queue and memory pool for inter-thread packet passing (IOCP worker threads → game thread).
- **`Network/M1NetworkManager.h`** — `UGameInstanceSubsystem`; owns the `M1Client` instance, pumps the packet queue each tick, and routes packets to handlers.
- **`Network/M1PacketHandler.h`** — Dispatches incoming packet IDs (defined in `ContentsProtocol.h`) to game-logic handlers (login, spawn, movement sync, RTT response).

**Packet flow (inbound):** IOCP thread → ring buffer → `M1Client` pushes to LFQueue → `M1NetworkManager::Tick` pops from LFQueue → `M1PacketHandler` dispatches by packet ID → game-state update.

**Packet flow (outbound):** Game code calls `M1NetworkManager::Send(CMessage*)` → serialized to `CLanClient` send queue → IOCP send thread → socket.

### Character & Spawn System

- **`System/M1SpawnManager.h`** — Receives spawn/despawn packets from server; manages the lifecycle of all in-world entities. Also runs RTT clock synchronization (`TickClock`) to compute `ServerTimeOffset` used for movement interpolation.
- **`Character/M1Character.h`** — Base class for all characters; holds HP and change delegates used by the HUD.
- **`Character/M1BasePlayer.h`** — Adds MP, EXP, level, movement mode.
- **`Character/M1LocalPlayer.h`** — The locally controlled player; owns camera/spring arm; `M1PlayerController` sends movement packets from this character.
- **`Character/M1OtherPlayer.h`** — Remote players; uses snapshot buffer (`TCircularSnapBuffer<FMovementSnapshot>`) + interpolation to smooth network jitter.
- **`Character/M1Monster.h`** — Monster entities.

### Input & Control

- **`Controller/M1PlayerController.h`** — Binds Enhanced Input actions, routes movement/attack to both local simulation and outbound network packets.
- **`Data/M1InputDataAsset.h`** — Data asset mapping `UInputAction*` to Gameplay Tags (`Input_Action_Move`, `Input_Action_Look`, `Input_Action_Jump`, `Input_Action_LeftAttack`).
- **`M1GameplayTags.h`** — Declares all gameplay tags used for input.

### UI

- **`UI/M1MainHUDWidget.h`** — Main HUD; binds to delegates on `M1Character` (HP, MP, EXP) for reactive updates.

### Asset & Game Initialization

- **`System/M1GameInstance.h`** — Starts up subsystems on game launch; coordinates `M1NetworkManager` connection.
- **`System/M1AssetManager.h`** — Extends `UAssetManager`; loads `M1PrimaryDataAsset` bundles at startup.
- **`Data/M1PrimaryDataAsset.h`** — Registers in-game asset collections for async loading.

### Key Data Structures (`System/Type/M1Type.h`)

- `FM1SpawnData` — Data carried in spawn packets (character ID, type, location, rotation, stats).
- `FMovementSnapshot` — A single timestamped position/rotation sample for interpolation.
- `TCircularSnapBuffer<T>` — Fixed-size circular buffer holding recent snapshots for `M1OtherPlayer` interpolation.

## Important Patterns

**Thread safety:** The IOCP worker threads and the game thread communicate exclusively through the lock-free queue in `M1Client`. Never access UObjects from IOCP threads — push data onto the LFQueue and process it on the game thread tick.

**Time sync:** `M1SpawnManager::TickClock` periodically sends RTT probe packets to the server and computes `ServerTimeOffset`. All movement snapshot timestamps use server time (`FApp::GetCurrentTime() + ServerTimeOffset`). This offset must be stable before spawning remote characters.

**Snapshot interpolation:** `M1OtherPlayer` renders at `ServerTime - InterpolationDelay`. New `FMovementSnapshot` entries are pushed to the circular buffer by `M1PacketHandler`; the character's `Tick` reads the buffer and lerps between surrounding snapshots.

**Protocol IDs:** All packet type constants are in `Common/Network/ContentsProtocol.h` (shared with server). Adding a new packet requires updating that file and both client/server handlers.
