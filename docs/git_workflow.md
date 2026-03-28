# Git Workflow – Mini-UnionFS

## Branch Strategy

- main → stable code only
- dev → integration branch
- feature branches → individual work

Example:
feature/readdir
feature/cow-logic
feature/whiteout

---

## Workflow Steps

1. Pull latest changes
   git checkout dev
   git pull origin dev

2. Create a feature branch
   git checkout -b feature/<feature-name>

3. Work and commit
   git add .
   git commit -m "Add: readdir basic implementation"

4. Push branch
   git push origin feature/<feature-name>

5. Create Pull Request → dev branch

---

## Commit Message Convention

Use structured messages:

Add: new feature
Fix: bug fix
Update: modification
Refactor: code cleanup
Docs: documentation changes

Example:
Add: initial FUSE mount stub
Fix: segmentation fault in getattr
Docs: update README setup steps

---

## Rules

- Do NOT push directly to main
- Always use pull requests
- Keep commits small and meaningful
- Test before pushing

---

## Conflict Resolution

If conflicts occur:

git pull origin dev
# resolve manually
git add .
git commit

---

## Code Review Checklist

- Does it compile?
- Does it break mount?
- Is logic correct?
- Are edge cases handled?

---

## Branch Naming Convention

feature/<name>
bugfix/<name>
docs/<name>

---

## Example

feature/copy-on-write
feature/whiteout-support
bugfix/read-crash
