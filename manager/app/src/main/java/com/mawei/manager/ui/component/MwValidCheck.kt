package com.mawei.manager.ui.component

import androidx.compose.runtime.Composable
import com.mawei.manager.Natives

@Composable
fun MwIsValid(
    content: @Composable () -> Unit
) {
    val isManager = Natives.isManager
    val mwVersion = if (isManager) Natives.version else null

    if (mwVersion != null) {
        content()
    }
}
