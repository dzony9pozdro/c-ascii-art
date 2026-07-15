#!/bin/bash
set -euo pipefail

# --- ensure ImageMagick is available ---
if ! command -v magick >/dev/null 2>&1; then
  echo "ImageMagick ('magick') is not installed." >&2
  if   command -v brew    >/dev/null 2>&1; then mgr="brew";   inst="brew install imagemagick"
  elif command -v apt-get >/dev/null 2>&1; then mgr="apt";    inst="sudo apt-get install -y imagemagick"
  elif command -v dnf     >/dev/null 2>&1; then mgr="dnf";    inst="sudo dnf install -y ImageMagick"
  elif command -v pacman  >/dev/null 2>&1; then mgr="pacman"; inst="sudo pacman -S --noconfirm imagemagick"
  else
    echo "No known package manager found. Install manually: https://imagemagick.org" >&2
    exit 1
  fi
  read -r -p "Install it now via $mgr? [y/N] " reply
  case "$reply" in
    [yY]|[yY][eE][sS]) echo "Running: $inst"; eval "$inst" ;;
    *) echo "Aborting. Install ImageMagick and re-run." >&2; exit 1 ;;
  esac
fi

# --- usage / file checks ---
if [ $# -lt 1 ]; then echo "usage: $0 <image>" >&2; exit 1; fi
img="$1"
if [ ! -f "$img" ]; then echo "error: '$img' not found" >&2; exit 1; fi

# --- terminal + image dimensions ---
cols=$(tput cols); rows=$(tput lines)
read ow oh < <(magick identify -format '%w %h\n' "$img")
rows=$(( rows - 1 ))   # leave a line so the prompt doesn't scroll the top off

# --- fit to terminal, preserving aspect ratio (10px x 20px blocks) ---
h1=$(( rows * 20 )); w1=$(( h1 * ow / oh ))
if (( w1 / 10 > cols )); then
  new_w=$(( cols * 10 )); new_h=$(( new_w * oh / ow ))
else
  new_h=$h1; new_w=$w1
fi
new_w=$(( (new_w / 10) * 10 )); new_h=$(( (new_h / 20) * 20 ))
if (( new_w < 10 || new_h < 20 )); then echo "error: terminal too small" >&2; exit 1; fi

# --- convert to PPM and render ---
tmp="${TMPDIR:-/tmp}/ascii_$$.ppm"
trap 'rm -f "$tmp"' EXIT
magick "$img" -resize "${new_w}x${new_h}!" -depth 16 "$tmp"
./main "$tmp"
