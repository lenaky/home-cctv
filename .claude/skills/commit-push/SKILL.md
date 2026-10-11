---
name: commit-push
description: git 변경사항을 커밋하고 푸시한다. "커밋해줘", "커밋하고 푸시해줘", "commit and push" 등의 요청에 사용. 작업 내용 파악, 커밋, 푸시를 모두 Haiku 5.5 서브에이전트에게 위임하는 가벼운 버전.
---

# commit-push

간단하고 가벼운 버전. 복잡한 체크포인트 없이, Haiku 5.5 서브에이전트 한 번 호출로 끝낸다.

## 동작

1. Agent tool로 `model: "haiku"`를 지정해 서브에이전트를 호출한다.
2. 서브에이전트에게 다음을 지시한다:
   - `git status`, `git diff`로 현재 변경사항을 직접 파악한다.
   - 변경된 파일을 적절히 `git add` 한다 (전체를 add해도 되지만, `.env`/`*.pem`/`credentials*.json`/`*.key` 등 시크릿으로 보이는 파일이 있으면 그 파일은 제외하고 사용자에게 알린다).
   - 변경 내용을 요약해 커밋 메시지를 작성한다 (리포의 기존 `git log` 스타일 참고, 짧고 명확한 한 줄 요약 중심).
   - **Co-Authored-By 등 attribution 줄은 추가하지 않는다** (이 스킬의 명시적 오버라이드).
   - `git commit`을 실행한다.
   - `git push`를 실행한다 (현재 브랜치의 upstream으로. upstream이 없으면 `git push -u origin HEAD`).
   - `--force`/`--force-with-lease`/`--no-verify`는 사용하지 않는다. push가 non-fast-forward로 거절되면 멈추고 그대로 보고한다 (자동으로 pull/rebase/merge 하지 않는다).
3. 서브에이전트가 끝나면 결과(커밋 메시지, 커밋 SHA, push 결과)를 사용자에게 간단히 보고한다.

사용자 승인은 받지 않고 바로 진행한다. 실패하거나 애매한 상황(충돌, hook 실패 등)이 생기면
멈추고 사용자에게 보고한다.
