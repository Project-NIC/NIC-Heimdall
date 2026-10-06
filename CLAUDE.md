# CLAUDE.md

## Git workflow

- **Never open a pull request** — not for finished work, not to ask for review, not when a branch
  is pushed and done. Do not offer to open one either.
- **Merge finished work straight into `main`:** push the working branch, then bring `main` up to it
  (`git push origin HEAD:main`). If `main` has moved on, merge it into the branch first, then push.
  Do not ask whether to merge — merging is the default.
