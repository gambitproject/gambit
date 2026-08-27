#!/bin/sh
#
# Build a branded, "drag to Applications" disk image from an already-built
# Gambit.app bundle.
#
# Usage: build_support/osx/make-dmg.sh <path-to-Gambit.app> <output.dmg>

set -e

APP_BUNDLE="$1"
OUTPUT_DMG="$2"
HERE="$(cd "$(dirname "$0")" && pwd)"

if [ -z "$APP_BUNDLE" ] || [ -z "$OUTPUT_DMG" ]; then
    echo "Usage: $0 <path-to-Gambit.app> <output.dmg>" >&2
    exit 1
fi

STAGING_DIR="$(mktemp -d /tmp/gambit-dmg-staging.XXXXXX)"
RW_DMG="$(mktemp /tmp/gambit-dmg-rw.XXXXXX)-rw.dmg"
VOLNAME="Gambit"
MOUNT_POINT="$(mktemp -d /tmp/gambit-dmg-mount.XXXXXX)"

cleanup() {
    hdiutil detach "$MOUNT_POINT" -quiet -force >/dev/null 2>&1 || true
    rm -rf "$STAGING_DIR" "$RW_DMG"
}
trap cleanup EXIT

rm -f "$OUTPUT_DMG"

mkdir -p "$STAGING_DIR/.background"
cp -r "$APP_BUNDLE" "$STAGING_DIR/Gambit.app"
ln -s /Applications "$STAGING_DIR/Applications"
cp "$HERE/dmg_background.tiff" "$STAGING_DIR/.background/background.tiff"

SIZE_MB=$(($(du -sm "$STAGING_DIR" | cut -f1) + 20))
hdiutil create -srcfolder "$STAGING_DIR" -volname "$VOLNAME" -fs HFS+ \
    -format UDRW -size "${SIZE_MB}m" "$RW_DMG" -quiet

hdiutil attach "$RW_DMG" -mountpoint "$MOUNT_POINT" -quiet -nobrowse

# When mounted at a custom path with -nobrowse, Finder addresses the disk
# object (via AppleScript) by the mount point's directory name, not by the
# volume's own HFS label -- so that is what we must use here, even though
# the label (and what end users see once the finished dmg ships) is "Gambit".
DISK_ID="$(basename "$MOUNT_POINT")"

# Finder also registers a freshly attached volume asynchronously, so
# addressing it by name via AppleScript immediately after attach is racy.
i=0
while [ $i -lt 20 ]; do
    if [ "$(osascript -e "tell application \"Finder\" to exists disk \"$DISK_ID\"" 2>/dev/null)" = "true" ]; then
        break
    fi
    i=$((i + 1))
    sleep 0.5
done
if [ $i -eq 20 ]; then
    echo "error: Finder never saw the mounted volume after mounting" >&2
    exit 1
fi

osascript <<EOF
tell application "Finder"
    tell disk "$DISK_ID"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {200, 200, 860, 622}
        set theViewOptions to the icon view options of container window
        set arrangement of theViewOptions to not arranged
        set icon size of theViewOptions to 128
        set background picture of theViewOptions to file ".background:background.tiff"
        set position of item "Gambit.app" of container window to {210, 227}
        set position of item "Applications" of container window to {450, 227}
        update without registering applications
        delay 1
        close
    end tell
end tell
EOF

cp "$HERE/gambit.icns" "$MOUNT_POINT/.VolumeIcon.icns"
SetFile -c icnC "$MOUNT_POINT/.VolumeIcon.icns"
SetFile -a C "$MOUNT_POINT"

sync
hdiutil detach "$MOUNT_POINT" -quiet
hdiutil convert "$RW_DMG" -format UDZO -imagekey zlib-level=9 -o "$OUTPUT_DMG" -quiet
