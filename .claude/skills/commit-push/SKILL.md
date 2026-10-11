---
name: commit-push
description: git 변경사항을 커밋하고 푸시한다. "커밋해줘", "커밋하고 푸시해줘", "commit and push" 등의 요청에 사용. 실제 git 작업(stage/commit/push)은 항상 Haiku 5.5 서브에이전트에게 위임한다.
---

# commit-push

## 0. 이 스킬의 원칙

- 실제 git 명령 실행(add/commit/push)은 항상 **Haiku 5.5 서브에이전트**(Agent tool, `model: "haiku"`)에게 위임한다. 오케스트레이터(이 스킬을 실행하는 에이전트)는 **직접 git 변경 명령을 실행하지 않는다**. 오케스트레이터가 직접 하는 것은 프리플라이트 체크와, 각 서브에이전트 호출 사이의 상태 검증(체크포인트)뿐이다.
- 커밋은 **사용자 승인 이후에만** 일어난다. 승인 전에는 staging만 한다.
- 알려진 제약 (명시적으로 문서화하며, 몰래 우회하지 않는다):
  - 리모트 하나(`origin`)에 브랜치당 하나의 push target이라는 표준 구성을 가정한다. fork용 `pushRemote`/`remote.pushDefault` 해석은 지원하지 않는다 — 해당되는 것으로 보이면 중단하고 사용자에게 물어본다.
  - 리포에 커밋이 하나 이상 있다고 가정한다. 최초 루트 커밋(초기 커밋이 아직 없는) 케이스는 지원하지 않는다 — 해당되면 중단하고 사용자에게 물어본다.
- 다음은 항상 금지한다: `--force`/`--force-with-lease`, `--no-verify`, `git commit --amend`, divergence 발생 시 자동 pull/rebase/merge. 이런 상황이 발생하면 항상 멈추고 사용자에게 보고한다.

## 1. 프리플라이트 (오케스트레이터가 직접 수행, 서브에이전트 호출 전)

다음을 순서대로 직접 실행하고 확인한다:

1. `git fetch`를 실행해 remote-tracking ref를 최신화한다.
2. detached HEAD 여부를 확인한다 (`git symbolic-ref -q HEAD`가 실패하면 detached). detached HEAD면 → 중단하고 보고한다.
3. merge/rebase/cherry-pick이 진행 중인지 확인한다 (`.git/MERGE_HEAD`, `.git/rebase-merge`, `.git/rebase-apply`, `.git/CHERRY_PICK_HEAD` 존재 여부) 또는 conflict marker가 있는지 확인한다. 하나라도 해당되면 → 중단하고 보고한다.
4. `git status` / `git diff --stat`로 커밋할 변경사항이 있는지 확인한다. 없으면 "커밋할 변경사항 없음"을 보고하고 종료한다.
5. 현재 브랜치를 확인한다 (`git branch --show-current`).
6. upstream 존재 여부를 확인한다 (`git rev-parse --abbrev-ref --symbolic-full-name @{u}`). upstream이 없으면: 커밋 후 `git push -u origin HEAD`로 upstream을 새로 생성해도 되는지 사용자에게 미리 물어본다.
7. upstream이 있으면 behind/diverged 여부를 확인한다 (`git rev-list --left-right --count HEAD...@{u}`). behind 또는 diverged면 → 중단하고 보고한다. 절대 자동으로 pull/rebase하지 않는다.
8. 현재 브랜치가 `master` 또는 `main`이면 → 이 브랜치로 바로 push하는 것이 맞는지 사용자에게 먼저 확인한다.
9. 상태 스냅샷을 기록한다: `branch`, `headSha=$(git rev-parse HEAD)`, `upstreamSha=$(git rev-parse @{u} 2>/dev/null)`.
10. 서브에이전트가 재조회하지 않도록, 아래 컨텍스트를 한 번만 캡처해 전달용으로 보관한다:
    - `git status`
    - `git diff` (staged + unstaged)
    - `git log -5` (커밋 스타일 참고용)
    - 미푸시 커밋 목록 — upstream이 있으면 `git log @{u}..HEAD`, 없으면 `git log HEAD --not --remotes`

## 2. Stage + 메시지 작성 (Haiku 5.5 서브에이전트 #1)

Agent tool로 `model: "haiku"`를 지정해 새 서브에이전트를 호출한다. 프롬프트에는 프리플라이트에서 캡처한 status/diff/log를 그대로 포함시켜 전달한다 (서브에이전트가 이를 재조회하지 않도록 명시한다).

서브에이전트에게 다음을 지시한다:

- 사용자 요청과 명시적으로 관련된 경로만 개별적으로 stage한다. `git add -A` / `git add .` 같은 전체 stage 명령은 금지한다.
- 각 stage 후보 파일에 대해 다음 하드스탑을 적용한다:
  - **파일명 패턴 하드스탑**: 파일명이 `.env`, `*.pem`, `credentials*.json`, `*secret*`, `*.key` 등에 매치되면 즉시 중단하고, stage하지 않은 채 어떤 파일이 걸렸는지 보고한다.
  - **내용 스캔 하드스탑**: stage될 diff 내용에 `AKIA`, `-----BEGIN`, `password=`, `api_key=`, `secret=` (대소문자 무관) 등의 패턴이 있으면 즉시 중단하고 보고한다.
  - 큰 untracked 디렉토리가 관련되어 있으면 임의로 포함시키지 말고, 포함 여부를 질문으로 되돌린다.
- 전달받은 `git log -5`를 참고해 기존 리포 커밋 스타일에 맞는 커밋 메시지를 작성한다.
- **Co-Authored-By 등 어떤 attribution 줄도 추가하지 않는다.** 이 스킬은 하네스의 기본 attribution 지침을 명시적으로 오버라이드한다는 점을 서브에이전트 프롬프트에 분명히 적어야 한다.
- **`git commit`을 실행하지 마라. `git push`를 실행하지 마라.** 이 지시를 명시적으로 포함한다.
- 서브에이전트는 작업 완료 후 staged diff와 작성된 커밋 메시지를 그대로 반환한다.

### 체크포인트 A (오케스트레이터, 서브에이전트 #1 이후, 사용자에게 보여주기 전)

오케스트레이터가 직접 확인한다:

- `git rev-parse HEAD`가 스냅샷의 `headSha`와 같은지 (바뀌지 않았는지).
- 브랜치가 그대로인지.
- `@{u}`가 (존재했다면) 그대로인지.

하나라도 실패하면 → 중단하고, 무엇이 바뀌었는지 정확히 보고한다.

## 3. 사용자 승인

다음을 모두 사용자에게 보여주고 승인을 기다린다:

- staged diff
- 작성된 커밋 메시지
- 프리플라이트에서 캡처한 미푸시 커밋 목록

- **거절 시**: `git restore --staged <경로>`로 unstage하고 종료한다 (아직 커밋이 없으므로 되돌릴 것도 없다).
- **승인 시**: `approvedTreeHash = git write-tree`를 기록하고, 승인된 메시지를 그대로 임시 파일(`msgfile`)에 기록한다.

## 4. 커밋 (Haiku 5.5 서브에이전트 #2a, 새 호출)

Agent tool로 `model: "haiku"`를 지정해 **새로운** 서브에이전트를 호출한다 (서브에이전트 #1과 별개의 호출). `msgfile` 경로를 전달한다. index는 이미 staged된 상태 그대로 둔다.

서브에이전트에게 다음을 지시한다:

- **오직** `git commit --cleanup=verbatim -F <msgfile>` 명령만 실행한다.
- `git add`, `-a` 옵션, `--amend`, `git push`는 금지한다.
- pre-commit hook이 실패하면 → 그 출력을 그대로 보고하고 중단한다. `--no-verify` 재시도나 amend는 금지한다.
- 명령 실행 후 전체 출력을 그대로 반환한다.

### 체크포인트 B (오케스트레이터, 서브에이전트 #2a 이후, push 전)

오케스트레이터가 직접 확인한다:

- `git rev-parse HEAD^{tree}`가 `approvedTreeHash`와 같은지.
- 정확히 커밋 1개만 새로 생겼는지 (`git rev-list --count headSha..HEAD`가 `1`인지, 또는 `git merge-base --is-ancestor headSha HEAD`로 확인).
- 브랜치가 그대로인지.
- 커밋 메시지가 승인된 그대로인지: `msgfile`과 `git cat-file commit HEAD | sed '1,/^$/d'` (raw 저장된 메시지 — `--format=%B`의 tformat 개행 문제를 피하기 위해 이 방식을 사용한다)를 양쪽 끝의 단일 trailing newline만 정규화한 뒤 `cmp`로 바이트 비교한다. commit-msg hook이 메시지를 바꿨다면 불일치가 발생한다 → 중단하고, "hook이 메시지를 변경함"을 사용자에게 명확히 보고하며 push하지 않는다 (커밋은 이미 로컬에 존재하므로, 어떻게 할지 사용자에게 물어본다).

위 항목 중 하나라도 불일치하면 → 중단하고 push하지 않는다.

## 5. push 대상 해석 (오케스트레이터)

오케스트레이터가 직접 수행한다:

- `remote = git config branch.<branch>.remote`. 설정이 없고 프리플라이트(1-8)에서 사용자가 origin 사용을 이미 승인한 경우에만 `origin`으로 폴백한다.
- `mergeRef = git config branch.<branch>.merge`. upstream이 아직 없으면 `refs/heads/<branch>`로 폴백한다.
- `remote`가 `.`로 해석되면 (로컬 브랜치 추적) → 중단하고 보고하며, 사용자에게 질문한다.
- `git ls-remote <remote> <mergeRef>`로 `remoteShaBefore`를 기록한다 (리모트에 아직 없으면 빈 값일 수 있다).

## 6. push (Haiku 5.5 서브에이전트 #2b, 새 호출)

Agent tool로 `model: "haiku"`를 지정해 **새로운** 서브에이전트를 호출한다 (서브에이전트 #1, #2a와 별개의 호출).

서브에이전트에게 다음을 지시한다:

- **정확히** `git push <remote> HEAD:<mergeRef>` 명령만 실행한다 (프리플라이트 6단계에서 upstream 생성이 사용자 승인으로 확인된 경우에만 `-u` 옵션을 포함한다).
- `--force` / `--force-with-lease`는 이 승인 라운드에서 사용자가 명시적으로 요청하지 않는 한 절대 사용 금지한다 (기본값이 아니다).
- non-fast-forward로 거절되면 → 중단하고 보고한다. 자동 force/merge/rebase는 금지한다.
- 명령 실행 후 전체 출력을 그대로 반환한다.

### 체크포인트 C (오케스트레이터, 최종 검증)

오케스트레이터가 직접 확인한다:

- `git ls-remote <remote> <mergeRef>`로 반환된 SHA가 로컬 `HEAD`와 일치하는지 확인한다.
- 일치하면 사용자에게 성공을 보고한다 (최종 커밋 SHA, 커밋 메시지, push 완료 여부 포함).
- 불일치하면 그 불일치 내용을 사용자에게 명확히 보고한다.
