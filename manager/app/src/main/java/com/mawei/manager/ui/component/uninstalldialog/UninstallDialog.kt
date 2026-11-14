package com.mawei.manager.ui.component.uninstalldialog

import androidx.compose.runtime.Composable
import com.mawei.manager.ui.LocalUiMode
import com.mawei.manager.ui.UiMode

@Composable
fun UninstallDialog(
    show: Boolean,
    onDismissRequest: () -> Unit
) {
    when (LocalUiMode.current) {
        UiMode.Miuix -> UninstallDialogMiuix(show, onDismissRequest)
        UiMode.Material -> UninstallDialogMaterial(show, onDismissRequest)
    }
}
