alias bk := build_mwd
alias bm := build_manager

build_mwd:
    cross build --target aarch64-linux-android --release

build_manager: build_mwd
    cp target/aarch64-linux-android/release/mwd manager/app/src/main/jniLibs/arm64-v8a/libmwd.so
    cd manager && ./gradlew aDebug

clippy:
    cargo fmt
    cross clippy --target aarch64-linux-android --release
