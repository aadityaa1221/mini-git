# Mini-Git
A lightweight, custom implementation of Git built from scratch in C++.

## Features
- **init**: Initializes a repository with a hidden `.mygit` structure.
- **add**: Hashes file contents using SHA-1 and stores them as immutable blobs.
- **commit**: Creates a snapshot of your directory using a Tree object.

## Architecture
- **Objects**: Stores compressed data blobs.
- **Refs**: Tracks branches (like `main`).
- **Cryptography**: Uses SHA-1 for data integrity.

## How to use
1. `./mygit init`
2. `./mygit add <file>`
3. `./mygit commit "Your message"`
