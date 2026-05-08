# Deployment Notes

## macOS
1. Build the app with Qt 6 and CMake.
2. Run macdeployqt on the generated .app bundle.
3. Optionally package the .app into a DMG.

## Windows
1. Build the .exe with Qt 6 + MSVC.
2. Run windeployqt on the executable.
3. Package into an installer if desired.

## Linux
1. Build against your distro's Qt 6 packages or a Qt SDK.
2. Bundle the runtime if you want a portable distribution.
3. For Oracle Linux, RPM packaging is recommended.
