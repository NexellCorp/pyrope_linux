package com.samsung.vd.cinemacontrolpanel.InfoClass.NXAsyncParams;

import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigTconInfo;

/**
 * Created by spjun on 18. 4. 10.
 */

public class TconInfoCarrier {

    public int iMode;
    ConfigTconInfo tconEEPRomInfo = new ConfigTconInfo();
    ConfigTconInfo tconUsbInfo = new ConfigTconInfo();


    public int getiMode() {
        return iMode;
    }

    public void setiMode(int iMode) {
        this.iMode = iMode;
    }

    public ConfigTconInfo getTconEEPRomInfo() {
        return tconEEPRomInfo;
    }

    public void setTconEEPRomInfo(ConfigTconInfo tconEEPRomInfo) {
        this.tconEEPRomInfo = tconEEPRomInfo;
    }

    public ConfigTconInfo getTconUsbInfo() {
        return tconUsbInfo;
    }

    public void setTconUsbInfo(ConfigTconInfo tconUsbInfo) {
        this.tconUsbInfo = tconUsbInfo;
    }
}
