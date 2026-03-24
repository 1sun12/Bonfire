# Bonfire

A native desktop application for developers who work across multiple languages and constantly need to recall syntax, patterns, and hard-won commands. Built with C++ and Qt6.

**The problem it solves:** You know what a binary tree traversal is. You forget how to write one in C versus Lua versus Python. You know rsync has an exclude flag — you can never remember the exact syntax. Bonfire is where you keep that stuff so you stop losing time looking it up again.

This is not a general-purpose note-taking app. It is a **code-first knowledge base** with spaced repetition built in, so syntax actually sticks long-term.

---

## Features

- **Shards** — every note revolves around a code block. Description, tags, and metadata are secondary context.
- **Language-aware** — every shard is tagged with a language. 35 languages included by default, with support for adding custom ones.
- **Full-text fuzzy search** — searches title, language, code, description, and tags simultaneously.
- **Filter & sort** — filter by language, category, familiarity level, or tag. Sort by recently modified, alphabetical, or familiarity (shaky-first to surface weak spots).
- **Spaced repetition (SM-2)** — opt any shard into the review queue. Bonfire surfaces due shards daily as flashcards and adjusts the schedule based on how well you recalled them.
- **Review mode** — flashcard-style sessions. Code is hidden until you reveal it. Rate yourself: Forgot / Hard / Good / Easy. The SM-2 algorithm adjusts the next review date accordingly.
- **Quick Capture** — `Ctrl+N` opens a minimal dialog: title, language, code, tag. Everything else can be filled in later. Zero friction when you find something worth saving.
- **Syntax highlighting** — code blocks are highlighted for C, C++, C#, Rust, Go, Java, JavaScript, TypeScript, Python, Lua, Bash/Shell, SQL, JSON, YAML, TOML, and more.
- **Import / Export** — full JSON dump and restore. Your data is portable and human-readable.
- **Fully local** — no accounts, no cloud, no network. Data lives in a SQLite database on your machine.

---

## Screenshots

> Dashboard showing stats, language breakdown, due reviews, and recently added shards.

> Library view with fuzzy search and per-language filtering active.

> Shard editor with live syntax highlighting.

> Review mode flashcard — code hidden, ready to reveal.

---

## Tech Stack

| Layer | Technology |
|---|---|
| Language | C++17 |
| UI Framework | Qt6 (Widgets) |
| Database | SQLite via Qt6::Sql |
| Syntax Highlighting | QSyntaxHighlighter (custom) |
| Build System | CMake 3.16+ |
| Styling | Qt Style Sheets (QSS), Fusion dark theme |

---

## Building from Source

### Prerequisites

**Linux (Ubuntu/Debian):**
```bash
sudo apt install cmake qt6-base-dev g++
```

**Arch Linux:**
```bash
sudo pacman -S cmake qt6-base gcc
```

**Fedora:**
```bash
sudo dnf install cmake qt6-qtbase-devel gcc-c++
```

**macOS (Homebrew):**
```bash
brew install cmake qt6
export PATH="/opt/homebrew/opt/qt6/bin:$PATH"
```

**Windows:**
Install [Qt6](https://www.qt.io/download-qt-installer) via the Qt installer (select Qt 6.x → Desktop → MinGW or MSVC), then install [CMake](https://cmake.org/download/).

---

### Build

```bash
git clone https://github.com/1sun12/Bonfire.git
cd Bonfire
mkdir build && cd build
cmake ..
make -j$(nproc)        # Linux / macOS
# cmake --build . -j4  # Windows
```

The compiled binary will be at `build/Bonfire` (or `build/Bonfire.exe` on Windows).

---

### Install (Linux)

After building, run the included install script from the project root:

```bash
bash install.sh
```

This will:
- Create a symlink at `~/.local/bin/bonfire` so you can launch the app from any terminal by typing `bonfire`
- Write a `.desktop` entry to `~/.local/share/applications/bonfire.desktop` so Bonfire appears in your application launcher and can be searched like any installed app
- Refresh the desktop database automatically

If `~/.local/bin` is not in your `PATH`, the script will tell you. Add this to your `~/.bashrc` or `~/.zshrc` and restart your terminal:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

After that, running `bonfire` from any terminal will launch the app.

> **Note:** The symlink points to the binary inside the `build/` directory. If you recompile, the symlink automatically picks up the new binary — no need to re-run `install.sh`.

---

### Run

```bash
./build/Bonfire   # direct
bonfire           # after running install.sh
```

Data is stored at:
- **Linux:** `~/.local/share/Bonfire/Bonfire/vault.db`
- **macOS:** `~/Library/Application Support/Bonfire/vault.db`
- **Windows:** `%APPDATA%\Bonfire\vault.db`

---

## Usage

### Creating a Shard

**Quick Capture** (`Ctrl+N`) — the fastest path. Fill in a title, pick a language, paste your code. Hit Save. Everything else (description, tags, category, familiarity) can be added later from the Library.

**Full editor** — click **+ New** in the sidebar, or open any existing shard from the Library and hit **Edit**. All fields are available:

| Field | Description |
|---|---|
| Title | Short, descriptive name |
| Language | Selectable from 35 built-in languages, or add your own |
| Code | The primary content — syntax-highlighted |
| Description | Why this works, when to use it, what you were stuck on |
| Tags | Freeform comma-separated labels (e.g. `networking, one-liner`) |
| Category | `snippet`, `pattern`, `boilerplate`, `one-liner`, `troubleshoot`, `concept`, `config`, `cheatsheet` |
| Familiarity | `fresh`, `shaky`, `solid`, `mastered` — your honest self-rating |
| Source | Where you found it: URL, book, man page |
| Review | Toggle spaced repetition on/off for this shard |

---

### Searching & Browsing

Open the **Library** view (sidebar) or press `Ctrl+K`.

- Type anything in the search bar — it searches across title, language, code, description, and tags simultaneously.
- Use the filter dropdowns to narrow by language, category, familiarity level, or tag.
- Sort by **Recently Modified**, **Alphabetical**, or **Familiarity (shaky first)** to surface what needs work.

---

### Spaced Repetition

1. Open any shard in the editor and toggle **Enable Spaced Repetition** on.
2. The shard enters the review queue. Bonfire schedules the first review for the next day.
3. On the **Dashboard**, due shards appear in the "Due for Review" section.
4. Click **Start Review** to begin a flashcard session.
5. Each card shows the title, language, category, and description. The code block is hidden.
6. Try to recall the syntax, then click **Reveal**.
7. Rate yourself honestly:
   - **Forgot** — resets the interval back to 1 day
   - **Hard** — advances with a shorter interval
   - **Good** — advances normally
   - **Easy** — advances with a longer interval and increases the ease factor
8. The SM-2 algorithm calculates the next review date. Cards you nail consistently will be shown less and less often.

You can also hit **Mark Reviewed** directly from a shard's view mode to advance its schedule without a full session.

---

### Import & Export

**Export:** Dashboard → **Export** → saves a `.json` file to your chosen location.

**Import:** Dashboard → **Import** → select a `.json` file. Duplicate shards (matched by ID) are skipped automatically.

The JSON format is human-readable and matches the structure used in the original web prototype, so data is portable between versions.

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+N` | Open Quick Capture |
| `Ctrl+K` | Jump to Library / search |
| `Esc` | Close dialogs |

---

## Project Structure

```
Bonfire/
├── CMakeLists.txt
├── src/
│   ├── main.cpp                         # Entry point, dark theme setup
│   ├── mainwindow.h / .cpp              # App shell, navigation, shortcuts
│   ├── shard.h                          # Shard data structure, language/familiarity helpers
│   ├── database.h / .cpp                # SQLite persistence layer
│   ├── sm2.h                            # SM-2 spaced repetition algorithm
│   ├── syntaxhighlighter.h / .cpp       # QSyntaxHighlighter for 15+ language families
│   ├── views/
│   │   ├── dashboardwidget.h / .cpp     # Home: stats, due reviews, recent shards
│   │   ├── librarywidget.h / .cpp       # Browse: search, filter, sort
│   │   ├── shardeditorwidget.h / .cpp   # View + edit a shard
│   │   └── reviewwidget.h / .cpp        # Flashcard review session
│   └── dialogs/
│       └── quickcapturedialog.h / .cpp  # Ctrl+N quick-add dialog
├── memory.txt                           # Project context notes
└── react-code-artifact.txt             # Original web prototype (reference only)
```

---

## Data Model

Each shard is stored as a row in a local SQLite database. The JSON export format:

```json
{
  "shards": [
    {
      "id": "lk3x9aab2f",
      "title": "Recursive binary tree traversal",
      "language": "C",
      "code": "void inorder(Node* root) {\n    if (!root) return;\n    inorder(root->left);\n    printf(\"%d \", root->data);\n    inorder(root->right);\n}",
      "description": "Classic in-order DFS. Left → root → right.",
      "tags": ["data-structures", "recursion"],
      "category": "pattern",
      "familiarity": "shaky",
      "source": "CLRS Ch. 12",
      "reviewEnabled": true,
      "reviewInterval": 6,
      "reviewRepetitions": 2,
      "reviewEase": 2.5,
      "reviewNext": "2026-03-29"
    }
  ],
  "customLangs": [],
  "exportedAt": "2026-03-23T21:00:00"
}
```

---

## Author

**sun** — [github.com/1sun12](https://github.com/1sun12)

---

## AI Assistance

This project was built with the assistance of **Claude Opus 4.6** by Anthropic, used via [Claude Code](https://claude.ai/claude-code).

Although AI was used, I take the originality of my work very seriously and am always transparent in my work where AI was used and how.

---

## License

MIT License. Do whatever you want with it.
