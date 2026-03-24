#!/bin/bash
# Bonfire post-build installer
# Creates a terminal symlink and desktop launcher entry.
# Run from the project root after compiling: bash install.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="$SCRIPT_DIR/build/Bonfire"
LINK_DIR="$HOME/.local/bin"
DESKTOP_DIR="$HOME/.local/share/applications"
DESKTOP_FILE="$DESKTOP_DIR/bonfire.desktop"

# ---- Preflight ----
if [ ! -f "$BINARY" ]; then
    echo "Error: binary not found at $BINARY"
    echo "Build the project first:"
    echo "  mkdir -p build && cd build && cmake .. && make -j\$(nproc)"
    exit 1
fi

# ---- Directories ----
mkdir -p "$LINK_DIR"
mkdir -p "$DESKTOP_DIR"

# ---- Terminal symlink ----
ln -sf "$BINARY" "$LINK_DIR/bonfire"
echo "OK  Symlink:  $LINK_DIR/bonfire -> $BINARY"

# ---- Desktop entry ----
cat > "$DESKTOP_FILE" <<EOF
[Desktop Entry]
Name=Bonfire
Comment=Developer syntax recall and spaced repetition
Exec=$BINARY
Icon=accessories-text-editor
Terminal=false
Type=Application
Categories=Development;Education;
Keywords=code;snippet;syntax;developer;spaced repetition;flashcard;
StartupWMClass=Bonfire
EOF
echo "OK  Desktop:  $DESKTOP_FILE"

# ---- Refresh desktop database ----
if command -v update-desktop-database &>/dev/null; then
    update-desktop-database "$DESKTOP_DIR"
    echo "OK  Desktop database refreshed"
fi

# ---- PATH check ----
if [[ ":$PATH:" != *":$LINK_DIR:"* ]]; then
    echo ""
    echo "Note: $LINK_DIR is not yet in your PATH."
    echo "Add the following line to your ~/.bashrc or ~/.zshrc, then restart your terminal:"
    echo ""
    echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo ""
fi

echo ""
echo "Done."
echo "  Terminal:  bonfire"
echo "  Launcher:  search for 'Bonfire' in your application menu"
