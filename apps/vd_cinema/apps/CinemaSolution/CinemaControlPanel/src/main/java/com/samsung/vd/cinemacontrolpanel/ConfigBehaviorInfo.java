package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Locale;

/**
 * Created by doriya on 10/30/18.
 */

public class ConfigBehaviorInfo {
    private static final String VD_DTAG = "ConfigBehaviorInfo";

    public static final String PATH_SOURCE = "DCI/TCON_BEHAVIOR";
    public static final String PATH_TARGET = "/storage/sdcard0/SAMSUNG/TCON_BEHAVIOR";
    public static final String PATH_EXTRACT = "DCI/TCON_BEHAVIOR_READ";

    private static final int MAX_INFO_NUM = 30;

    public static final int COLOR_RED     = 0;
    public static final int COLOR_GREEN   = 1;
    public static final int COLOR_BLUE    = 2;

    private class NX_BEHAVIOR_INFO {
        String[] mFile = new String[3];
        String[] mDesc = new String[3];
    }

    private NX_BEHAVIOR_INFO[] mInfo = new NX_BEHAVIOR_INFO[MAX_INFO_NUM];
    private boolean mCryption = false;

    public ConfigBehaviorInfo() {
    }

    public ConfigBehaviorInfo( boolean cyprtion ) {
        mCryption = cyprtion;
        Log.i(VD_DTAG, String.format( Locale.US, "Cyption options: %b", mCryption));
    }

    public boolean Update( String fileName ) {
        if( !new File(fileName).isFile() )
            return false;

        byte[] inData = FileManager.ReadByte( fileName );
        if( inData == null || inData.length == 0 )
            return false;

        int mode = -1;
        int color = -1;
        String desc = "";

        if( mCryption ) {
            for (int i = 0; i < inData.length; i++)
            {
                inData[i] = (byte)((int)inData[i] ^ 0x55);
            }
        }

        try {
            // FileInputStream inStream = new FileInputStream(fileName);
            InputStream inStream = new ByteArrayInputStream(inData);
            InputStreamReader inStreamReader = new InputStreamReader(inStream);
            BufferedReader bufferedReader = new BufferedReader(inStreamReader);

            while(true)
            {
                String szLine = bufferedReader.readLine();
                if( szLine == null ) {
                    Log.i(VD_DTAG, String.format( "EOF. ( %s )", fileName ));
                    break;
                }

                szLine = szLine.trim();
                szLine = szLine.replaceAll("=", "");
                szLine = szLine.toUpperCase();

                String[] szLineSplit = szLine.split("\\s+");
                if( 2 != szLineSplit.length )
                    continue;

                if( szLineSplit[0].equals("MODE") ) {
                    mode = Integer.parseInt(szLineSplit[1], 10);
                }

                if( szLineSplit[0].equals("NAME")) {
                    desc = szLineSplit[1];
                }

                if( szLineSplit[0].equals("TRANSFER_COLOR")) {
                    if( szLineSplit[1].equals("R") ) color = COLOR_RED;
                    if( szLineSplit[1].equals("G") ) color = COLOR_GREEN;
                    if( szLineSplit[1].equals("B") ) color = COLOR_BLUE;
                }

                if( (0 < mode) && !desc.equals("") && (0 <= color)) {
                    break;
                }
            }

            bufferedReader.close();
            inStreamReader.close();
            inStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        if( (0 > mode) || desc.equals("") || (0 > color)) {
            return false;
        }

        if( mInfo[mode] == null )
            mInfo[mode] = new NX_BEHAVIOR_INFO();

        mInfo[mode].mFile[color] = fileName;
        mInfo[mode].mDesc[color] = desc;

        Dump();
        return true;
    }

    public void Delete() {
        for( int i = 0; i < MAX_INFO_NUM; i++ ) {
            Delete( i );
        }
    }

    public void Delete( int mode ) {
        if( mInfo[mode] != null ) {
            for( int i = 0; i < 3; i++ ) {
                mInfo[mode].mFile[i] = null;
                mInfo[mode].mDesc[i] = null;
            }
            mInfo[mode] = null;
        }
    }

    void Dump() {
        for( int i = 0; i < MAX_INFO_NUM; i++ ) {
            Dump(i);
        }
    }

    void Dump( int mode ) {
        if( mInfo[mode] == null )
            return;

        for( int j = 0; j < 3; j++ ) {
            Log.i(VD_DTAG, String.format(Locale.US, "mode: %02d, color: %d, desc: %s, name: %s",
                    mode, j, mInfo[mode].mDesc[j], mInfo[mode].mFile[j] )
            );
        }
    }

    boolean IsValid( int mode ) {
        return (mInfo[mode] != null);
    }

    boolean IsValid( int mode, int color ) {
        return ((mInfo[mode] != null) && (mInfo[mode].mFile[color] != null));
    }

    String GetName( int mode, int color ) {
        if((mInfo[mode] != null) && (mInfo[mode].mFile[color] != null)) {
            return mInfo[mode].mFile[color];
        }

        return null;
    }

    String GetDesc( int mode, int color ) {
        if((mInfo[mode] != null) &&  (mInfo[mode].mDesc[color] != null)) {
            return mInfo[mode].mDesc[color];
        }

        return null;
    }

    public byte[] GetByte( int mode, int color ) {
        if( null == mInfo[mode] )
            return null;

        if( null == mInfo[mode].mFile[color] )
            return null;

        FileInputStream inStream = null;
        byte[] result = null;

        try {
            File file = new File(mInfo[mode].mFile[color]);
            result = new byte[(int)file.length()];

            inStream = new FileInputStream(file);
            int readable = inStream.read(result);

            if( readable != file.length() ) {
                Log.i(VD_DTAG, String.format(Locale.US, "Fail, read size. ( expected: %d, readable: %d )",
                        file.length(), readable));
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (inStream != null) {
                try {
                    inStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        return result;
    }
}
