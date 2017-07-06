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

    //
    //  Command Set
    //
    public static final int CMD_TCON_REG_WRITE				= 0x0000;
    public static final int CMD_TCON_REG_READ				= 0x0001;
    public static final int CMD_TCON_REG_BURST_WRITE		= 0x0002;
    public static final int CMD_TCON_INIT					= 0x0010;
    public static final int CMD_TCON_STATUS					= 0x0011;
    public static final int CMD_TCON_DOOR_STATUS			= 0x0012;
    public static final int CMD_TCON_LVDS_STATUS			= 0x0013;
    public static final int CMD_TCON_BOOTING_STATUS			= 0x0014;
    public static final int CMD_TCON_MODE_NORMAL			= 0x0015;
    public static final int CMD_TCON_MODE_LOD				= 0x0016;
    public static final int CMD_TCON_OPEN_NUM				= 0x0017;
    public static final int CMD_TCON_OPEN_POS				= 0x0018;
    public static final int CMD_TCON_PATTERN_RUN			= 0x0019;
    public static final int CMD_TCON_PATTERN_STOP			= 0x001A;
    public static final int CMD_TCON_TGAM_R					= 0x001B;
    public static final int CMD_TCON_TGAM_G					= 0x001C;
    public static final int CMD_TCON_TGAM_B					= 0x001D;
    public static final int CMD_TCON_DGAM_R					= 0x001E;
    public static final int CMD_TCON_DGAM_G					= 0x001F;
    public static final int CMD_TCON_DGAM_B					= 0x0020;
    public static final int CMD_TCON_DOT_CORRECTION			= 0x0021;
    public static final int CMD_TCON_DOT_CORRECTION_EXTRACT	= 0x0022;
    public static final int CMD_TCON_WHITE_SEAM_READ    	= 0x0023;
    public static final int CMD_TCON_WHITE_SEAM_WRITE       = 0x0024;
    public static final int CMD_TCON_ELAPSED_TIME			= 0x0025;
    public static final int CMD_TCON_ACCUMULATE_TIME		= 0x0026;
    public static final int CMD_TCON_VERSION				= 0x0027;

    public static final int CMD_PFPGA_REG_WRITE				= 0x0100;
    public static final int CMD_PFPGA_REG_READ				= 0x0101;
    public static final int CMD_PFPGA_REG_BURST_WRITE		= 0x0102;
    public static final int CMD_PFPGA_STATUS				= 0x0110;
    public static final int CMD_PFPGA_UNIFORMITY_DATA		= 0x0111;
    public static final int CMD_PFPGA_MUTE					= 0x0112;
    public static final int CMD_PFPGA_VERSION				= 0x0113;

    public static final int CMD_PLATFORM_NAP_VERSION		= 0x0210;
    public static final int CMD_PLATFORM_SAP_VERSION		= 0x0211;
    public static final int CMD_PLATFORM_IPC_SERVER_VERSION	= 0x0212;
    public static final int CMD_PLATFORM_IPC_CLIENT_VERSION	= 0x0213;

    //
    //  Register
    //
    public static final int REG_PFPGA_NUC_EN                = 0x01B0;
    public static final int REG_TCON_SEAM_EN                = 0x0192;
    public static final int REG_TCON_SEAM_TEST              = 0x0186;

    private static NxCinemaCtrl mInstance;

    private NxCinemaCtrl() {
    }

    public static synchronized NxCinemaCtrl GetInstance() {
        if( mInstance == null ) {
            mInstance = new NxCinemaCtrl();
        }
        return mInstance;
    }

    public synchronized byte[] Send( int cmd, byte[] data ) {
        return NX_CinemaCtrl( cmd, data );
    }

    public static final int FORMAT_INT8     = 1;
    public static final int FORMAT_INT16    = 2;
    public static final int FORMAT_INT24    = 3;
    public static final int FORMAT_INT32    = 4;

    public static final int MASK_INT16_MSB  = 0;
    public static final int MASK_INT16_LSB  = 2;
    public static final int MASK_INT32_MSB  = 0;
    public static final int MASK_INT32_LSB  = 4;

    public byte[] IntToByteArray( int value, int format ) {
        byte[] result = new byte[format];

        for( int i = 0 ;i < format; i++ ) {
//            int offset = (result.length - 1 - i) * 8;
            int offset = (format - 1 - i) * 8;
            result[i] = (byte)((value >> offset) & 0xFF);
        }

        return result;
    }

    public byte[] IntArrayToByteArray( int[] value, int format ) {
        byte[] result = new byte[format * value.length];

        for( int i = 0; i < value.length; i++ ) {
            for( int j = 0; j < format; j++ ) {
                int offset = (format - 1 - j) * 8;
                result[i * format + j] = (byte)((value[i] >> offset) & 0xFF);
            }
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

    public int ByteArrayToInt16( byte[] value, int mask ) {
        if( value.length != FORMAT_INT16 * 2 ) {
            Log.i(NX_DTAG, String.format( "Fail, Invalid ByteArray Size. ( in: %d, expected: %d )", value.length, FORMAT_INT16 * 2 ) );
            return -1;
        }

        byte[] inData = new byte[FORMAT_INT16];
        System.arraycopy( value, mask, inData, 0, FORMAT_INT16 );

        int result = 0;
        for( int i = 0; i < FORMAT_INT16; i++ ) {
            int offset = (FORMAT_INT16 -  1 - i) * 8;
            result += (inData[i] & 0x000000FF ) << offset;
        }

        return result;
    }

    public int ByteArrayToInt32( byte[] value, int mask ) {
        if( value.length != FORMAT_INT32 * 2 ) {
            Log.i(NX_DTAG, String.format( "Fail, Invalid ByteArray Size. ( in: %d, expected: %d )", value.length, FORMAT_INT32 * 2 ) );
            return -1;
        }

        byte[] inData = new byte[FORMAT_INT32];
        System.arraycopy( value, mask, inData, 0, FORMAT_INT32 );

        int result = 0;
        for( int i = 0; i < FORMAT_INT32; i++ ) {
            int offset = (FORMAT_INT32 - 1 - i) * 8;
            result += (inData[i] & 0x000000FF ) << offset;
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

    public void PrintByteArrayToHex( byte[] inData, int size ) {
        int loop = (size > inData.length) ? inData.length : size;

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

    public native byte[] NX_CinemaCtrl( int cmd, byte[] inData );
}
