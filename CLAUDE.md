# home-cctv

Personal home CCTV / VMS system, open-source (MIT). This is a **greenfield project built 100% by
an LLM (Claude)** — every line of implementation, every design judgment, and every bugfix originates
from Claude following the workflow below. Long-term goal: natural-language-driven development with
automated review and testing.

See project memory for the confirmed tech stack, hexagonal architecture, and phase roadmap — always check those before proposing tech/architecture changes.

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
