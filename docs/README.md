# docs/ — LLM 작업 로그

이 프로젝트는 100% LLM(Claude)이 [Superpowers 워크플로우](../CLAUDE.md#development-workflow-superpowers)를
따라 구현하는 그린필드 프로젝트다. `docs/` 는 그 과정에서 LLM이 내린 판단, 고친 버그, 확인된 동작을
누적하는 곳이다. 코드만 봐서는 알 수 없는 "왜"를 여기에 남긴다.

세 카테고리:

| 디렉터리 | 무엇을 담나 | 언제 쓰나 | 언제 읽나 |
|---|---|---|---|
| [`decisions/`](decisions/README.md) | 구현/설계 중 LLM이 스스로 내린 판단과 근거 (대안, 왜 이걸 골랐는지) | `brainstorming` / `writing-plans` 단계에서 애매한 선택지가 있었을 때, 결정 직후 | 새 기능을 시작하기 전, 기존 결정과 충돌하는지 확인할 때 |
| [`fixes/`](fixes/README.md) | 버그의 증상, 근본 원인, 고친 방법, 검증 방법 | `systematic-debugging`으로 버그를 고친 직후 | 비슷한 증상의 버그를 다시 만났을 때, 회귀 여부 확인할 때 |
| [`learnings/`](learnings/README.md) | 잘 동작한 접근/패턴과 왜 잘 됐는지 (재사용 가치가 있는 것) | `verification-before-completion` 통과 후, 검증된 접근을 확정할 때 | 비슷한 문제를 다시 풀 때, 이미 검증된 패턴이 있는지 확인할 때 |

각 디렉터리의 `README.md`가 해당 카테고리의 인덱스(항목 목록)이고, `TEMPLATE.md`가 새 문서 작성 시 쓸 양식이다.

## 규칙

- 문서는 항상 커밋 단위(하나의 판단/버그/검증)로 하나씩 쌓는다. 기존 문서를 지우거나 뭉개지 않는다.
- 파일명: `NNN-짧은-제목.md` (해당 디렉터리 내 순번, 3자리 숫자).
- 새 문서를 추가하면 해당 디렉터리 `README.md`의 인덱스 표에도 한 줄 추가한다.
- 오래된 결정이 새 결정으로 뒤집히면 기존 문서는 지우지 말고 상단에 `> Superseded by [NNN-xxx](NNN-xxx.md)`를 추가한다.
