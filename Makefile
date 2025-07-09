APP_NAME = HelloWorld
LIB_NAME = hello
APK_UNALIGNED = $(APP_NAME).unaligned.apk
APK_ALIGNED = $(APP_NAME).apk
APK_SIGNED = $(APP_NAME)-signed.apk

# === Files ===
JNI_DIR = jni
LIB_OUTPUT_DIR = libs/armeabi-v7a
DUMMY_JAVA = Dummy.java
DUMMY_CLASS = Dummy.class
CLASSES_DEX = classes.dex

# === Targets ===

.PHONY: clean build install run

clean:
	@echo "Cleaning..."
	ndk-build clean
	rm -f $(DUMMY_CLASS) $(CLASSES_DEX)
	rm -f $(APK_UNALIGNED) $(APK_ALIGNED) $(APK_SIGNED)
	rm -rf lib

build:
	@echo "Building native code..."
	ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=$(JNI_DIR)/Android.mk NDK_APPLICATION_MK=$(JNI_DIR)/Application.mk

	@echo "Compiling Dummy.java..."
	javac $(DUMMY_JAVA)

	@echo "Converting to classes.dex..."
	d8 --output . $(DUMMY_CLASS)

	@echo "Packaging APK..."
	aapt package -f -M AndroidManifest.xml -S res -I /home/vid/android-sdk/platforms/android-34/android.jar -F $(APK_UNALIGNED)

	@echo "Adding native library and classes.dex..."
	mkdir -p lib/armeabi-v7a
	cp $(LIB_OUTPUT_DIR)/lib$(LIB_NAME).so lib/armeabi-v7a/
	zip -u $(APK_UNALIGNED) lib/armeabi-v7a/lib$(LIB_NAME).so
	zip -u $(APK_UNALIGNED) $(CLASSES_DEX)

	@echo "Aligning APK..."
	zipalign -v 4 $(APK_UNALIGNED) $(APK_ALIGNED)

	@echo "Signing APK..."
	apksigner sign --ks my-release-key.jks --out $(APK_SIGNED) $(APK_ALIGNED)

install:
	@echo "Installing APK on device..."
	adb install -r $(APK_SIGNED)

run:
	@echo "Launching app..."
	adb shell am start -n com.example.helloworld/android.app.NativeActivity
