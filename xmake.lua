add_rules("mode.debug", "mode.release")

set_languages("c++23")

target("core")
    set_kind("static")
    add_cxflags("-fPIC")
    add_files("src/core/*.cpp")
    add_files("src/renderer/*.cpp")
    add_includedirs("src/", {public=true})

target("particlessim_app")
    set_kind("shared")
    add_deps("core")
    add_files("src/platform/android_app.cpp")
    add_syslinks("android", "log")
    add_rules("android.native_app", {
        android_sdk_version = "36",
        android_manifest = "android/AndroidManifest.xml",
        android_res = "android/res",
        keystore = "android/debug.jks",
        keystore_pass = "123456",
        package_name = "com.lualvsil.particles",
        logcat_filters = {"particles", "simulation"}
    })

target("particles")
    set_kind("binary")
    add_deps("core")
    add_files("src/platform/main.cpp")
    add_links("termuxgui")
