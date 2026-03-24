#!/bin/bash
# Bonfire post-build installer
# Creates a terminal symlink, installs the app icon, and writes a desktop launcher entry.
# Run from the project root after compiling: bash install.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="$SCRIPT_DIR/build/Bonfire"
ICON_SRC="$SCRIPT_DIR/bonfire-image.png"
LINK_DIR="$HOME/.local/bin"
ICON_DIR="$HOME/.local/share/icons/hicolor/512x512/apps"
ICON_DEST="$ICON_DIR/bonfire.png"
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
mkdir -p "$ICON_DIR"
mkdir -p "$DESKTOP_DIR"

# ---- Terminal symlink ----
ln -sf "$BINARY" "$LINK_DIR/bonfire"
echo "OK  Symlink:  $LINK_DIR/bonfire -> $BINARY"

# ---- Icon ----
if [ -f "$ICON_SRC" ]; then
    cp "$ICON_SRC" "$ICON_DEST"
    echo "OK  Icon:     $ICON_DEST"
    ICON_VALUE="bonfire"
else
    echo "WARN  bonfire-image.png not found in project root, using fallback icon"
    ICON_VALUE="accessories-text-editor"
fi

# ---- Ensure hicolor index.theme exists at user level ----
# Required by the freedesktop icon spec — without it many launchers
# (including COSMIC) won't recognise the user hicolor directory.
HICOLOR_USER="$HOME/.local/share/icons/hicolor"
if [ ! -f "$HICOLOR_USER/index.theme" ]; then
    if [ -f "/usr/share/icons/hicolor/index.theme" ]; then
        cp /usr/share/icons/hicolor/index.theme "$HICOLOR_USER/index.theme"
        echo "OK  index.theme copied to $HICOLOR_USER"
    else
        echo "WARN  Could not find system hicolor/index.theme — icon may not appear in launcher"
    fi
fi

# ---- Refresh icon cache ----
if command -v gtk-update-icon-cache &>/dev/null; then
    gtk-update-icon-cache -f -t "$HICOLOR_USER" 2>/dev/null && \
    echo "OK  Icon cache refreshed"
fi

# ---- Desktop entry ----
cat > "$DESKTOP_FILE" <<EOF
[Desktop Entry]
Name=Bonfire
Comment=Developer syntax recall and spaced repetition
Exec=$BINARY
Icon=$ICON_VALUE
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
