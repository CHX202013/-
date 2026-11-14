plugins {
    alias(libs.plugins.agp.app) apply false
    alias(libs.plugins.kotlin) apply false
    alias(libs.plugins.compose.compiler) apply false
}

extra["androidMinSdkVersion"] = 31
extra["androidTargetSdkVersion"] = 37
extra["androidCompileSdkVersion"] = 37
extra["androidCompileSdkVersionMinor"] = 0
extra["androidBuildToolsVersion"] = "37.0.0"
extra["androidCompileNdkVersion"] = libs.versions.ndk.get()
extra["androidSourceCompatibility"] = JavaVersion.VERSION_21
extra["androidTargetCompatibility"] = JavaVersion.VERSION_21
extra["managerVersionCode"] = getVersionCode()
extra["managerVersionName"] = getVersionName()

/**
 * 从 git 取版本号；在没有 .git 的源码包（tarball）里构建时退回固定值，
 * 避免 `git rev-list` 抛异常导致构建直接失败。
 */
fun git(vararg args: String): String? = runCatching {
    val process = Runtime.getRuntime().exec(arrayOf("git") + args)
    val out = process.inputStream.bufferedReader().use { it.readText().trim() }
    if (process.waitFor() == 0 && out.isNotEmpty()) out else null
}.getOrNull()

fun getGitCommitCount(): Int = git("rev-list", "--count", "HEAD")?.toIntOrNull() ?: 1

fun getGitDescribe(): String = git("describe", "--tags", "--always") ?: "v0.1.0"

fun getVersionCode(): Int {
    val commitCount = getGitCommitCount()
    return 30000 + commitCount
}

fun getVersionName(): String {
    return getGitDescribe()
}
