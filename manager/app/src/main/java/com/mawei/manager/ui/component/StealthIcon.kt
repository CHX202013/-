package com.mawei.manager.ui.component

import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.VisibilityOff
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.mawei.manager.R
import com.mawei.manager.ui.component.material.SegmentedSwitchItem
import com.mawei.manager.ui.util.IconVisibility
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.Icon
import top.yukonga.miuix.kmp.preference.SwitchPreference
import top.yukonga.miuix.kmp.theme.MiuixTheme.colorScheme

/** 设置页里的「隐藏桌面图标」开关（Material 风格）。 */
@Composable
fun StealthIconSwitchMaterial() {
    val context = LocalContext.current
    var hidden by remember { mutableStateOf(IconVisibility.isHidden(context)) }
    SegmentedSwitchItem(
        icon = Icons.Filled.VisibilityOff,
        title = stringResource(R.string.settings_hide_icon),
        summary = stringResource(R.string.settings_hide_icon_summary),
        checked = hidden,
        onCheckedChange = { want ->
            if (want) {
                hidden = IconVisibility.hide(context)
            } else {
                IconVisibility.show(context)
                hidden = false
            }
        }
    )
}

/** 设置页里的「隐藏桌面图标」开关（Miuix 风格）。 */
@Composable
fun StealthIconCard() {
    val context = LocalContext.current
    var hidden by remember { mutableStateOf(IconVisibility.isHidden(context)) }
    Card(
        modifier = Modifier
            .padding(top = 12.dp)
            .fillMaxWidth(),
    ) {
        SwitchPreference(
            title = stringResource(R.string.settings_hide_icon),
            summary = stringResource(R.string.settings_hide_icon_summary),
            startAction = {
                Icon(
                    Icons.Filled.VisibilityOff,
                    modifier = Modifier.padding(end = 6.dp),
                    contentDescription = stringResource(R.string.settings_hide_icon),
                    tint = colorScheme.onBackground
                )
            },
            checked = hidden,
            onCheckedChange = { want ->
                if (want) {
                    hidden = IconVisibility.hide(context)
                } else {
                    IconVisibility.show(context)
                    hidden = false
                }
            }
        )
    }
}
