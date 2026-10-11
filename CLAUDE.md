# home-cctv

Personal home CCTV / VMS system, open-source (MIT). This is a **greenfield project built 100% by
an LLM (Claude)** — every line of implementation, every design judgment, and every bugfix originates
from Claude following the workflow below. Long-term goal: natural-language-driven development with
automated review and testing.

See project memory for the confirmed tech stack, hexagonal architecture, and phase roadmap — always check those before proposing tech/architecture changes.

## Mandatory procedure (모든 작업의 필수 절차)

**단순 질의응답을 제외한** 이 리포지토리의 모든 작업은 아래 6단계를 반드시 거친다. 단계를
건너뛰거나 순서를 바꾸지 않는다. 아래 절차는 바로 다음에 나오는 Superpowers 스킬 워크플로우와
함께 적용되며, 두 절차가 상충할 경우 이 절차가 우선한다.

1. **근본 의미 파악** — 사용자의 요청을 완벽에 가깝게 이해하기 위해, 그 요청의 근본/근원적인
   의미를 파악하고 필요한 사항을 사용자에게 질문한다. 모호한 요구사항을 추측해서 진행하지 않는다.
2. **설계** — 1단계에서 파악한 요청을 해결할 접근 순서·방법과 검증에 쓸 테스트케이스를
   설계한다. **Sonnet 5 / high effort**로 수행한다.
3. **설계 리뷰** — 2단계 설계를 **Opus 5.5**로 리뷰한다. Opus 5.5가 승인할 때까지 2~3단계를
   반복한다. Opus가 거절하면 거절 이유를 상세히 2단계에 전달하여 재설계한다.
4. **구현** — 작업 복잡도에 따라 모델을 나누어 서브에이전트 방식으로 구현한다.
   - 중간 이상 난이도: **Sonnet 5 / medium effort**
   - 중간 미만 난이도: **Haiku 5.5**
5. **검증** — 2단계에서 미리 설계해둔 테스트케이스로 4단계의 구현 결과를 검증한다. 검증에
   실패하면 4단계로 돌아가 재구현한다 (4~5단계 반복).
6. **교훈 기록** — 3~5단계를 거치며 얻은 교훈(lesson & learned)을 `compound/` 디렉토리에
   저장하고, 그 내용의 인덱스를 아래 "compound/ — 누적 교훈 인덱스" 섹션에 유지한다. 이는
   향후 유사 작업을 하는 에이전트가 참고할 수 있도록 하기 위함이다.

### compound/ — 누적 교훈 인덱스

`compound/`는 3~5단계(설계 리뷰, 구현, 검증)를 거치며 얻은 교훈을 기록하는 디렉토리다.
전체 정책/작성 형식은 [`compound/README.md`](compound/README.md) 참고. 새 항목을 추가할
때마다 아래 인덱스에 한 줄씩 추가한다 (형식: `- [제목](compound/파일명.md) — 한 줄 요약`).

- [Opus 5.5 설계 리뷰 루프는 라운드가 늘수록 희귀 엣지케이스로 수렴한다](compound/001-opus-review-loop-diminishing-returns.md) — 3라운드 넘게 반려가 이어지고 지적 사항이 이 리포 환경과 무관해지면 사용자에게 계속/확정을 물어본다

## Phase 1 스켈레톤: 빌드/테스트/린트

전체 설계 요약은 [`docs/design.md`](docs/design.md), 판단 근거는 [`docs/decisions/`](docs/decisions/README.md) 참고.

### 디렉토리 구조 / 모듈 책임

```
services/ingestion-server/   — RTSP 수집 + 카메라 CRUD + 녹화 + LL-HLS 세그먼트 생성 (포트 8080)
services/egress-server/      — 실시간 시청(LL-HLS/WebRTC) + 녹화 재생 서빙 (포트 8090)
web-admin/                   — React+TS+Vite+Tailwind 웹 어드민 (최소 골격)
deploy/                      — macOS launchd / Windows 서비스 등록 설정
```
각 서비스는 hexagonal: `domain/`(엔티티·port·Result<T,E>, 외부 의존 없음) →
`application/`(usecase, port만 의존) → `adapters/inbound|outbound/`(HTTP·SQLite·FFmpeg·IPC 구현)
→ `main.cpp`(합성 루트). `ingestion-server`/`egress-server`는 컴파일 타임 의존이 없다
(런타임으로만 REST/공유 파일시스템/SQLite/로컬 IPC로 연결).

### C++ 백엔드 빌드 (vcpkg 전제, 로컬 brew로도 검증 가능 — 아래 "로컬 검증" 참고)
```
cmake --preset default      # 또는: cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=<prefix>
cmake --build build
```
### 테스트 (GoogleTest + CTest)
```
ctest --test-dir build --output-on-failure
```
### 린트/포맷 (clang-format + clang-tidy, 둘 다 저장소 루트의 `.clang-format`/`.clang-tidy` 사용)
```
clang-format -i $(find services -name '*.h' -o -name '*.cpp')   # 포맷 적용
clang-format --dry-run --Werror $(find services -name '*.h' -o -name '*.cpp')  # 포맷 검사만
run-clang-tidy -p build $(find services -name '*.cpp' | grep -v tests/)        # 린트
```
`.clang-tidy`는 `WarningsAsErrors: ''`로 설정돼 있어 경고는 빌드를 막지 않는다 (스켈레톤
단계에서 스텁 코드가 경고로 실패하지 않도록 하는 의도적 선택).

### 로컬 검증 (vcpkg 미설치 환경)
vcpkg로 FFmpeg를 소스 빌드하면 매우 오래 걸릴 수 있다. Homebrew에 ffmpeg/sqlite3/cpp-httplib/
googletest/llvm(clang-tidy)이 있으면 `-DCMAKE_PREFIX_PATH=/opt/homebrew`만 추가해 같은 CMake
구성으로 빌드/테스트/린트를 검증할 수 있다 (각 서비스 CMakeLists.txt가 FFmpeg/sqlite3를
PkgConfig로 찾도록 되어 있어 vcpkg/brew 양쪽에서 동작). → [[compound/002-vcpkg-vs-brew-local-verification]]

### web-admin
```
cd web-admin && npm install && npm run build && npm run lint
```

### 코딩 규칙 (CLAUDE.md 상단 "반드시 지킬 것"의 구체화)
- **에러는 전부 명시적 반환**: 모든 포트/usecase/adapter 메서드는 `Result<T>`(또는
  `Result<void>`)를 반환한다. 예외를 던지지 않는다 (→ [[docs/decisions/001-error-handling-result-type]]).
- **전역 상태 금지**: 모든 의존성은 생성자 주입(참조). 소유권은 각 서비스 `main.cpp`의
  합성 루트에만 있다.
- **hexagonal/clean**: `domain` → `application` → `adapters` → `main.cpp` 방향으로만
  의존. 역방향 의존 금지, 두 서비스 간 컴파일 타임 의존 금지.
- **SOLID**: port(인터페이스)는 단일 책임으로 쪼개고, adapter는 port를 구현하는 것 외의
  역할을 갖지 않는다. 구체 타입이 아니라 port 인터페이스에 의존한다(DIP).

## Development workflow: Superpowers

This project follows the [obra/superpowers](https://github.com/obra/superpowers) skill workflow for all
non-trivial work. Skills are installed globally (`~/.agents/skills/`). **Skill invocation is mandatory,
not optional** — if a skill applies, use it, even for "just a quick fix."

### Standard flow

1. **New feature / behavior change** → `brainstorming` first, always, before any plan or code.
2. **Bug / test failure / unexpected behavior** → `systematic-debugging` before proposing a fix.
3. **Once requirements are clear** → `writing-plans` to produce the implementation plan.
4. **Executing a plan:**
   - In a separate/reviewed session → `executing-plans`
   - Independent tasks in the current session → `subagent-driven-development`
   - 2+ independent, non-overlapping tasks → `dispatching-parallel-agents`
   - Needs workspace isolation → `using-git-worktrees`
5. **Writing code** → `test-driven-development` (tests before implementation).
6. **Before claiming anything is done/fixed/passing** → `verification-before-completion`
   (run the actual verification commands, don't assert from memory).
7. **Code review** → `requesting-code-review` when asking for review, `receiving-code-review`
   when acting on feedback (verify, don't blindly comply).
8. **Wrapping up a branch** → `finishing-a-development-branch`.
9. **Creating/editing a skill** → `writing-skills`.

`using-superpowers` governs the meta-rule: check for an applicable skill before responding to
anything, including clarifying questions — not just before writing code.

### Priority

Process skills (brainstorming, systematic-debugging) run first and set the approach; implementation
skills (test-driven-development, frontend-design, etc.) carry it out. This CLAUDE.md, and any direct
instruction from the user in-session, overrides skill defaults when they conflict.

## docs/ — accumulated LLM judgment log

Because this project is built entirely by an LLM, `docs/` is where the reasoning behind the code
lives — not just what was built, but what was decided, why, what broke and how it was fixed, and
what was verified to actually work. Code review and future sessions should be able to answer "why
is it like this?" from `docs/` without re-deriving it. Full policy: [`docs/README.md`](docs/README.md).

| Directory | Contains | Write when | Read when |
|---|---|---|---|
| [`docs/decisions/`](docs/decisions/README.md) | Implementation/design judgment calls the LLM made, and why, during brainstorming/planning/coding | Right after resolving an ambiguous choice during `brainstorming` / `writing-plans` / implementation | Before starting new work, to check it doesn't contradict a prior decision |
| [`docs/fixes/`](docs/fixes/README.md) | Bug symptom, root cause, fix, and how it was verified | Right after `systematic-debugging` closes out a bug | When a similar symptom reappears, or to check for regressions |
| [`docs/learnings/`](docs/learnings/README.md) | Approaches/patterns confirmed to work well, and why | After `verification-before-completion` confirms an approach works | When facing a similar problem, to check if a verified pattern already exists |

Each subdirectory has its own `README.md` (index of entries) and `TEMPLATE.md` (format for new
entries). This is a running log, not a snapshot — never delete or overwrite past entries; superseded
decisions get an explicit "superseded by" note instead. Treat writing these docs as part of finishing
the task, not an optional afterthought — a feature/fix/verification isn't done until its entry exists.
