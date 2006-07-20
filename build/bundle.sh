#!/bin/sh

# Creates application bundles for use on Mac OS X.

if [ -z "$1" -o -z "$2" ]; then
  echo "usage: `basename $0` BUNDLE-NAME BINARY-NAME"
fi

bundle_name=$1
binary_name=$2

if [ ! -f "$binary_name" ]; then
  echo "`basename $0`: ${binary_name}: file not found"
fi

if [ ! -d "${bundle_name}.app/Contents/MacOS" ]; then
  mkdir -p "${bundle_name}.app/Contents/MacOS"
fi

if [ ! -d "${bundle_name}.app/Contents/Resources" ]; then
  mkdir -p "${bundle_name}.app/Contents/Resources"
fi

cp -f "$binary_name" "${bundle_name}.app/Contents/MacOS"

if [ ! -f "${bundle_name}.app/Contents/PkgInfo" ]; then
  echo -n "APPL????" > "${bundle_name}.app/Contents/PkgInfo"
fi

if [ ! -f "${bundle_name}.app/Contents/Info.plist" ]; then
  cat > "${bundle_name}.app/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
        <key>CFBundleDevelopmentRegion</key>
        <string>English</string>
        <key>CFBundleExecutable</key>
        <string>${binary_name}</string>
        <key>CFBundleInfoDictionaryVersion</key>
        <string>6.0</string>
        <key>CFBundlePackageType</key>
        <string>APPL</string>
        <key>CFBundleSignature</key>
        <string>????</string>
        <key>CFBundleVersion</key>
        <string>0.1</string>
</dict>
</plist>
EOF
fi

