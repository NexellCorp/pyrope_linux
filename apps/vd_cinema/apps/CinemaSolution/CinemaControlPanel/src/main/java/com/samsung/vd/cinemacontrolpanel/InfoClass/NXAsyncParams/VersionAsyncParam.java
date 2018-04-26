package com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams;

import android.util.Log;

import com.samsung.vd.cinemacontrolpanel.Activity.LoginActivity;
import com.samsung.vd.cinemacontrolpanel.Utils.NxCinemaCtrl;

/**
 * Created by spjun on 18. 4. 13.
 */

public class VersionAsyncParam {
    byte[] napVersion;
    byte[] sapVersion;
    byte[] srvVersion;
    byte[] clnVersion;
    byte[] pfpgaVersion;

    public byte[] getNapVersion() {
        return napVersion;
    }

    public void setNapVersion(byte[] napVersion) {
        this.napVersion = napVersion;
    }

    public byte[] getSapVersion() {
        return sapVersion;
    }

    public void setSapVersion(byte[] sapVersion) {
        this.sapVersion = sapVersion;
    }

    public byte[] getSrvVersion() {
        return srvVersion;
    }

    public void setSrvVersion(byte[] srvVersion) {
        this.srvVersion = srvVersion;
    }

    public byte[] getClnVersion() {
        return clnVersion;
    }

    public void setClnVersion(byte[] clnVersion) {
        this.clnVersion = clnVersion;
    }

    public byte[] getPfpgaVersion() {
        return pfpgaVersion;
    }

    public void setPfpgaVersion(byte[] pfpgaVersion) {
        this.pfpgaVersion = pfpgaVersion;
    }
}
