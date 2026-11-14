package com.mawei.manager.data.repository

import com.mawei.manager.data.model.Module
import com.mawei.manager.data.model.ModuleUpdateInfo

interface ModuleRepository {
    suspend fun getModules(): Result<List<Module>>
    suspend fun checkUpdate(module: Module): Result<ModuleUpdateInfo>
}
