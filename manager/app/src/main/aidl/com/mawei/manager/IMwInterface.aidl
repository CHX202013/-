// IMwInterface.aidl
package com.mawei.manager;

import android.content.pm.PackageInfo;
import rikka.parcelablelist.ParcelableListSlice;

interface IMwInterface {
    ParcelableListSlice<PackageInfo> getPackages(int flags);

    int[] getUserIds();
}