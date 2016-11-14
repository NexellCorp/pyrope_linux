package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.nio.ByteBuffer;
import java.util.Locale;
import java.util.Random;

/**
 * Created by doriya on 10/7/16.
 */
public class NxCinemaCtrl {
    private static final String NX_DTAG = "NxCinemaCtrl";

    public static final int CMD_TCON                = 0x0000;
    public static final int CMD_TCON_STATUS         = 0x0001;
    public static final int CMD_TCON_DOOR_STATUS    = 0x0002;
    public static final int CMD_TCON_MODE_NORMAL    = 0x0011;
    public static final int CMD_TCON_MODE_LOD       = 0x0012;
    public static final int CMD_TCON_OPEN_NUM       = 0x0013;
    public static final int CMD_TCON_OPEN_POS       = 0x0014;
    public static final int CMD_TCON_SHORT_NUM      = 0x0015;
    public static final int CMD_TCON_SHORT_POS      = 0x0016;
    public static final int CMD_TCON_PATTERN        = 0x0021;
    public static final int CMD_TCON_MASTERING      = 0x0031;
    public static final int CMD_TCON_ELAPSED_TIME   = 0x0041;
    public static final int CMD_TCON_ACCUMULATE_TIME= 0x0042;
    public static final int CMD_TCON_VERSION        = 0x0070;

    public static final int CMD_PFPGA               = 0x0100;
    public static final int CMD_PFPGA_STATUS        = 0x0101;
    public static final int CMD_PFPGA_SOURCE        = 0x0102;
    public static final int CMD_PFPGA_VERSION       = 0x0170;

    public static final int CMD_BAT                 = 0x0200;
    public static final int CMD_BAT_STATUS          = 0x0201;

    public static final int CMD_IMB                 = 0x0300;
    public static final int CMD_IMB_STATUS          = 0x0301;
    public static final int CMD_IMB_VERSION         = 0x0302;

    private static NxCinemaCtrl mInstance;

    private NxCinemaCtrl() {
    }

    public static synchronized NxCinemaCtrl GetInstance() {
        if( mInstance == null ) {
            mInstance = new NxCinemaCtrl();
        }
        return mInstance;
    }

    public synchronized byte[] Send( int id, int cmd, byte[] data ) {
        if( CMD_TCON == (cmd & 0xFF00) ) {
            return NX_CinemaCtrlTCON( id, cmd, data );
        }
        return null;
    }

    public synchronized byte[] Send( int cmd, byte[] data ) {
        if( CMD_PFPGA == (cmd & 0xFF00) ) {
            return NX_CinemaCtrlPFPGA( cmd, data );
        }
        if( CMD_BAT == (cmd & 0xFF00) ) {
            return NX_CinemaCtrlBAT( cmd, data );
        }

        return null;
    }

    public static final int FORMAT_CHAR     = 1;
    public static final int FORMAT_INT16    = 2;
    public static final int FORMAT_INT32    = 4;
    public static final int FORMAT_INT64    = 8;

    public static final int FORMAT_INT16_MSB= 0;
    public static final int FORMAT_INT16_LSB= 2;

    public static final int FORMAT_INT32_MSB= 0;
    public static final int FORMAT_INT43_LSB= 4;

    public byte[] IntToByteArray( int value, int format ) {
        byte[] result = new byte[format];

        for( int i = 0 ;i < format; i++ ) {
            int offset = (result.length - 1 - i) * 8;
            result[i] = (byte)((value >> offset) & 0xFF);
        }

        return result;
    }

    public int ByteArrayToInt( byte[] value ) {
        int result = 0;
        for( int i = 0; i < value.length; i++ ) {
            int offset = (value.length - 1 - i) * 8;
            result += (value[i] & 0x000000FF) << offset;
        }

        return result;
    }

    public int ByteArrayToInt16( byte[] value, int format ) {
        if( value.length != FORMAT_INT16 * 2 ) {
            Log.i(NX_DTAG, "Invalid ByteArray Size.");
            return -1;
        }

        int result = 0;
        for( int i = 0; i < FORMAT_INT16; i++ ) {
            int offset = (FORMAT_INT16*2 - format - 1 - i) * 8;
            result += (value[i + format] & 0x000000FF ) << offset;
        }

        return result;
    }

    public int ByteArrayToInt32( byte[] value, int format ) {
        if( value.length != FORMAT_INT32 * 2 ) {
            Log.i(NX_DTAG, "Invalid ByteArray Size.");
            return -1;
        }

        int result = 0;
        for( int i = 0; i < FORMAT_INT32; i++ ) {
            int offset = (FORMAT_INT32*2 - format - 1 - i) * 8;
            result += (value[i + format] & 0x000000FF ) << offset;
        }

        return result;
    }

    public byte[] AppendByteArray( byte[] inData1, byte[] inData2 ) {
        byte[] result = new byte[ inData1.length + inData2.length ];

        System.arraycopy( inData1, 0, result, 0, inData1.length );
        System.arraycopy( inData2, 0, result, inData1.length, inData2.length );

        return result;
    }

    public void PrintByteArrayToHex( byte[] inData ) {
        StringBuilder sb = new StringBuilder();

        sb.append( String.format(Locale.US, "data( %d ) : ", inData.length) );
        for( final byte data: inData ) {
            sb.append( String.format("%02x ", data & 0xFF) );
        }

        Log.i(NX_DTAG, sb.toString() );
    }

    public int GetRandomValue( int startNum, int endNum ) {
        if( startNum >= endNum ) {
            return -1;
        }

        return new Random().nextInt( endNum - startNum ) + startNum;
    }

    static {
        Log.i( NX_DTAG, "Load JNI Library.");
        System.loadLibrary("nxcinemactrl");
    }

    public native byte[] NX_CinemaCtrlTCON( int id, int cmd, byte[] inData );
    public native byte[] NX_CinemaCtrlPFPGA( int cmd, byte[] inData );
    public native byte[] NX_CinemaCtrlBAT( int cmd, byte[] inData );
}
