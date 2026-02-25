#!/bin/bash

# ===== CONFIG =====
REPO_URL="https://github.com/weedu230/2048-game.git"
BRANCH="main"
COMMIT_MSG="first commit"

# ===== INIT GIT =====
echo "Initializing git repo..."
git init

# ===== SET BRANCH =====
git branch -M $BRANCH

# ===== ADD FILES =====
echo "Adding files..."
git add .

# ===== COMMIT =====
echo "Committing..."
git commit -m "$COMMIT_MSG"

# ===== ADD REMOTE =====
echo "Adding remote..."
git remote remove origin 2> /dev/null
git remote add origin $REPO_URL

# ===== PUSH =====
echo "Pushing to GitHub..."
git push -u origin $BRANCH

echo "Done! Project pushed successfully."