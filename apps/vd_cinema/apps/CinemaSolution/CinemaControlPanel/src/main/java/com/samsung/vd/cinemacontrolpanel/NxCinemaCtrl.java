package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

/**
 * Created by doriya on 10/7/16.
 */
public class NxCinemaCtrl {
    private static final String NX_DTAG = "NxCinemaCtrl";

    public static final int CMD_TCON_STATE        = 0x001;
    public static final int CMD_TCON_OPEN         = 0x002;
    public static final int CMD_TCON_OPEN_POS     = 0x003;
    public static final int CMD_TCON_SHORT        = 0x004;
    public static final int CMD_TCON_SHORT_POS    = 0x005;
    public static final int CMD_TCON_DOOR         = 0x006;
    public static final int CMD_TCON_ON           = 0x007;
    public static final int CMD_TCON_BR_CTRL      = 0x020;
    public static final int CMD_TCON_ELAPSED_TIME = 0x021;
    public static final int CMD_TCON_VERSION      = 0x070;
    
    public static final int CMD_PFPGA_STATE       = 0x101;
    public static final int CMD_PFPGA_SOURCE      = 0x102;
    public static final int CMD_PFPGA_VERSION     = 0x170;
    
    public static final int CMD_BAT_STATE         = 0x201;

    private static NxCinemaCtrl mInstance;

    private NxCinemaCtrl() {

    }

    public static synchronized NxCinemaCtrl GetInstance() {
        if( mInstance == null ) {
            mInstance = new NxCinemaCtrl();
        }
        return mInstance;
    }

    public synchronized char[] Send( int id, int cmd )
    {
        if( CMD_TCON_STATE <= cmd && CMD_TCON_VERSION >= cmd )
        {
            return NX_CinemaCtrlTCON( id, cmd );
        }

        return null;
    }

    public synchronized char[] Send( int cmd )
    {
        if( CMD_PFPGA_STATE <= cmd && CMD_PFPGA_VERSION >= cmd )
        {
            return NX_CinemaCtrlPFPGA( cmd );
        }
        else if( CMD_BAT_STATE == cmd )
        {
            return NX_CinemaCtrlBAT( cmd );
        }

        return null;
    }

    static {
        Log.i( NX_DTAG, "Load JNI Library.");
        System.loadLibrary("nxcinemactrl");
    }

    public native char[] NX_CinemaCtrlTCON( int id, int cmd );
    public native char[] NX_CinemaCtrlPFPGA( int cmd );
    public native char[] NX_CinemaCtrlBAT( int cmd );
}
