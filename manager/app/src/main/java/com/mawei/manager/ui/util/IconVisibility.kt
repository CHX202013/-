package com.mawei.manager.ui.util

import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.content.pm.ShortcutInfo
import android.content.pm.ShortcutManager
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Rect
import android.graphics.Typeface
import android.graphics.drawable.Icon
import android.os.Build
import com.mawei.manager.R

/**
 * 桌面图标隐藏。
 *
 * 启动器里可见的是 activity-alias（.ui.LauncherAlias），真正承载界面的是
 * .ui.MainActivity。隐藏时只禁用 alias，MainActivity 仍可被快捷方式或
 * `am start -n` 直接拉起，不会把应用锁死。
 */
object IconVisibility {

    private const val ALIAS_SUFFIX = ".ui.LauncherAlias"
    private const val MAIN_SUFFIX = ".ui.MainActivity"
    private const val SHORTCUT_ID = "console"

    // 用 applicationId 拼组件名：包名可在构建期改成任意值（-PMW_PACKAGE_NAME）
    private fun alias(context: Context) = ComponentName(context.packageName, context.packageName + ALIAS_SUFFIX)
    private fun mainActivity(context: Context) = ComponentName(context.packageName, context.packageName + MAIN_SUFFIX)

    fun isHidden(context: Context): Boolean =
        context.packageManager.getComponentEnabledSetting(alias(context)) ==
            PackageManager.COMPONENT_ENABLED_STATE_DISABLED

    /**
     * 先钉一个指向 MainActivity 的快捷方式，再禁用 alias。
     *
     * 系统不支持固定快捷方式时返回 false 且**不做任何修改**，
     * 避免出现图标没了、也进不去应用的死局。
     */
    fun hide(context: Context): Boolean {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) return false
        val shortcutManager = context.getSystemService(ShortcutManager::class.java) ?: return false
        if (!shortcutManager.isRequestPinShortcutSupported) return false

        return try {
            val shortcut = ShortcutInfo.Builder(context, SHORTCUT_ID)
                .setShortLabel(context.getString(R.string.app_name))
                .setIcon(buildShortcutIcon())
                .setIntent(
                    Intent(Intent.ACTION_MAIN).apply {
                        setComponent(mainActivity(context))
                        addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                    }
                )
                .build()
            shortcutManager.requestPinShortcut(shortcut, null)
            context.packageManager.setComponentEnabledSetting(
                alias(context),
                PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                PackageManager.DONT_KILL_APP
            )
            true
        } catch (_: Exception) {
            false
        }
    }

    /** 快捷方式图标用位图，避免部分启动器不接受 adaptive/vector 图标。 */
    private fun buildShortcutIcon(): Icon {
        val size = 192
        val bitmap = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        val paint = Paint(Paint.ANTI_ALIAS_FLAG)
        paint.color = 0xFF1F6FEB.toInt()
        canvas.drawRoundRect(0f, 0f, size.toFloat(), size.toFloat(), 48f, 48f, paint)
        paint.color = Color.WHITE
        paint.textSize = 120f
        paint.textAlign = Paint.Align.CENTER
        paint.typeface = Typeface.DEFAULT_BOLD
        val bounds = Rect()
        paint.getTextBounds("马", 0, 1, bounds)
        canvas.drawText("马", size / 2f, size / 2f - bounds.exactCenterY(), paint)
        return Icon.createWithBitmap(bitmap)
    }

    fun show(context: Context) {
        context.packageManager.setComponentEnabledSetting(
            alias(context),
            PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
            PackageManager.DONT_KILL_APP
        )
    }
}
