# Building a NativeActivity Android App in C: Process Overview and Guide

<video width="320" height="240" controls>
  <source src="rec.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

This document explains the **step-by-step process** to create, build, package, and run a pure C Android app using the NativeActivity API, without any Java code. It also includes a simple example and the necessary commands.

---

## What is a NativeActivity?

- NativeActivity is an Android framework class that allows you to write your entire Android app in native C/C++ code.
- It eliminates the need for Java UI code by providing a native entry point and lifecycle callbacks.
- Useful for games, multimedia apps, or performance-critical applications.

---

## High-Level Process

1. **Write native C code** implementing `android_main` and handling lifecycle and input events.
2. **Prepare AndroidManifest.xml** specifying `android.app.NativeActivity` as the activity and linking your native library.
3. **Build the native code** into a shared library (`libyourlib.so`) using the Android NDK (`ndk-build` or CMake).
4. **Package the APK** with your native library, manifest, resources, and a minimal `classes.dex` file.
5. **Sign and align the APK** to make it installable on Android devices.
6. **Install and run the APK** on a device or emulator.

---

## Key Files and Components

| File/Component               | Purpose                                                   |
|------------------------------|-----------------------------------------------------------|
| `hello.c`                    | Native C source implementing `android_main` and rendering |
| `AndroidManifest.xml`        | Declares the app and specifies NativeActivity             |
| `jni/Android.mk`             | NDK build script to compile native code                  |
| `jni/Application.mk`         | NDK build config (ABI, platform version)                 |
| `android_native_app_glue.*`  | Glue code to interface with NativeActivity lifecycle     |
| `Dummy.java`                 | Minimal Java class to generate `classes.dex`             |
| `classes.dex`                | Dalvik bytecode file required by Android APKs            |
| `aapt`                       | Android packaging tool                                   |
| `zipalign`                   | APK alignment tool                                       |
| `apksigner`                  | APK signing tool                                         |

---

## Example Minimal `AndroidManifest.xml`

```xml
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.example.helloworld"
    android:versionCode="1"
    android:versionName="1.0">

    <uses-sdk android:minSdkVersion="16" android:targetSdkVersion="34" />

    <application android:label="@string/app_name">
        <activity android:name="android.app.NativeActivity"
                  android:label="@string/app_name"
                  android:configChanges="orientation|keyboardHidden">
            <meta-data android:name="android.app.lib_name"
                       android:value="hello" />
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
```

---

## Overview of the Native C Code (`hello.c`)

* Implements `android_main(struct android_app* app)` as the app entry.
* Uses `android_native_app_glue` to handle lifecycle and input.
* Draws text or graphics on the screen using `ANativeWindow`.
* Runs a main loop that processes events and renders frames.
* Keeps the app alive until the user quits.

---

## Build and Packaging Commands

Assuming your project structure is set and you have:

* `jni/hello.c`, `jni/Android.mk`, `jni/Application.mk`
* `android_native_app_glue.c/h` in `jni/`
* `AndroidManifest.xml`, `res/values/strings.xml`
* `Dummy.java` with minimal class (`public class Dummy {}`)

You can use the makefile:
    Clean build artifacts:
        ```sh
        make clean
        ```
    Build the APK (native lib + package + sign):
        ```sh
        make build
        ```
    Install the APK on your connected device:
        ```sh
        make install
        ```
    Run the app on your device:
        ```sh
        make run
        ```

or follow the guide:
### 1. Build native library

```sh
ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./jni/Android.mk NDK_APPLICATION_MK=./jni/Application.mk
```

### 2. Compile Dummy.java to classes.dex

```sh
javac Dummy.java
$ANDROID_SDK/build-tools/34.0.0/d8 --output . Dummy.class
```

### 3. Package APK with aapt

```sh
aapt package -f -M AndroidManifest.xml -S res -I $ANDROID_SDK/platforms/android-34/android.jar -F HelloWorld.unaligned.apk
```

### 4. Add native library and classes.dex to APK

```sh
mkdir -p lib/armeabi-v7a
cp libs/armeabi-v7a/libhello.so lib/armeabi-v7a/
zip -u HelloWorld.unaligned.apk lib/armeabi-v7a/libhello.so
zip -u HelloWorld.unaligned.apk classes.dex
```

### 5. Align APK

```sh
zipalign -v 4 HelloWorld.unaligned.apk HelloWorld.apk
```

### 6. Sign APK

```sh
keytool -genkey -v -keystore my-release-key.jks -keyalg RSA -keysize 2048 -validity 10000 -alias my-alias
apksigner sign --ks my-release-key.jks --out HelloWorld-signed.apk HelloWorld.apk
```

### 7. Install APK on device

```sh
adb install -r HelloWorld-signed.apk
```

### 8. Run the app

```sh
adb shell am start -n com.example.helloworld/android.app.NativeActivity
```

---

## Notes and Tips

* **`classes.dex` is mandatory**, even for native-only apps, to satisfy Android package requirements.
* Use the **native_app_glue** library from the NDK to simplify lifecycle management.
* The **package name** in the manifest and the `am start` command must match.
* You can view logs using `adb logcat` to debug native code.
* For graphics, you can draw directly on `ANativeWindow` or use OpenGL ES.
* The minimal font and drawing code can be replaced with more advanced rendering as needed.

---


