package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by doriya on 2/28/17.
 */

public class ConfigTconInfo {
    private static final String VD_DTAG = "ConfigTconInfo";

    public static final String PATH_SOURCE = "DCI/TCON";
    public static final String PATH_TARGET_USB = "/storage/sdcard0/SAMSUNG/TCON_USB";
    public static final String PATH_TARGET_EEPROM = "/storage/sdcard0/SAMSUNG/TCON_EEPROM";

    public static final String NAME = "T_REG.txt";

    private static final int MAX_MODE_NUM = 10;
    private static final int NUM_INDEX = 1;
    private static final int NUM_ENABLE = 4;
    private static final int NUM_DESCRIPTION = 1;
    private static final int NUM_REG_NUMBER = 1;

    private int mModeNum = 0;
    private boolean[][] mEnable = new boolean[MAX_MODE_NUM][4];
    private String[] mDescription = new String[MAX_MODE_NUM];
    private int[] mDataNum = new int[MAX_MODE_NUM];
    private int[][][] mData = new int[MAX_MODE_NUM][2][];

    public ConfigTconInfo() {
    }

    public boolean Parse( String filePath ) {
        File file = new File( filePath );
        if( !file.isFile() )
            return false;

        String fileName = filePath.substring( filePath.lastIndexOf("/") + 1 );
        if( !fileName.equals(NAME) )
            return false;

        for( int i = 0; i < MAX_MODE_NUM; i++ )
            mDataNum[i] = 0;

        try {
            FileInputStream inStream = new FileInputStream( filePath );
            BufferedReader bufferedReader = new BufferedReader( new InputStreamReader(inStream) );

            int idxLine = 0, idxData = 0;
            String strLine;
            String[] strSplit;

            while( true )
            {
                strLine = bufferedReader.readLine();
                if( strLine == null ) {
                    Log.i(VD_DTAG, String.format( "EOF. ( %s )", filePath ));
                    break;
                }

//                Log.i(VD_DTAG, String.format(">>> %s", strLine) );
                strSplit = strLine.split("\\s+");

                if( idxLine < NUM_INDEX ) {
                    mModeNum = strSplit.length;
                }
                else if( idxLine < NUM_INDEX + NUM_ENABLE ) {
                    for( int i = 0; i < strSplit.length; i++ )
                    {
                        mEnable[i][idxLine-NUM_INDEX] = (Integer.parseInt( strSplit[i], 10 ) == 1);
                    }
                }
                else if( idxLine < NUM_INDEX + NUM_ENABLE + NUM_DESCRIPTION ) {
                    for( int i = 0; i < strSplit.length; i++ )
                    {
                        mDescription[i] = strSplit[i];
                    }
                }
                else if( idxLine < NUM_INDEX + NUM_ENABLE + NUM_DESCRIPTION + NUM_REG_NUMBER ) {
                    for( int i = 0; i < strSplit.length; i++ )
                    {
                        mDataNum[i] = Integer.parseInt( strSplit[i], 10 );

                        mData[i][0] = new int[mDataNum[i]];
                        mData[i][1] = new int[mDataNum[i]];
                    }
                }
                else {
                    int curPos = 0;
                    for( int i = 0; i < strSplit.length; i++ )
                    {
                        if( strSplit[i].equals("") )
                            continue;

                        for( int j = curPos; j < MAX_MODE_NUM; j++ ) {
                            if( idxData / 2 < mDataNum[j] ) {

                                mData[j][idxData % 2][idxData/2] =
                                        ((idxData % 2) == 0) ? Integer.decode( strSplit[i] ) : Integer.parseInt( strSplit[i], 10 );

                                curPos++;
                                break;
                            }
                            curPos++;
                        }
                    }

                    idxData++;
                }

                idxLine++;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        //
        // Debug Message
        //
        for( int i = 0; i < mModeNum; i++ ) {
            if( mData[i][0] == null )
                continue;

            Log.i(VD_DTAG, String.format("* mode %d : %s", i, mDescription[i]));
            Log.i(VD_DTAG, String.format("-. TGAM0 ( %b ), TGAM1 ( %b ), DGAM0 ( %b ), DGAM1 ( %b )",
                    mEnable[i][0], mEnable[i][1], mEnable[i][2], mEnable[i][3]) );

            Log.i(VD_DTAG, String.format("> register number for writing : %d", mData[i][0].length));
            for( int j = 0; j < mData[i][0].length; j++ ) {
                Log.i(VD_DTAG, String.format("-. reg( %3d, 0x%02x ), data( %4d, 0x%04x )",
                        mData[i][0][j], mData[i][0][j], mData[i][1][j], mData[i][1][j] ));
            }
        }

        return true;
    }

    int GetModeNum() {
        return mModeNum;
    }

    boolean[] GetEnableUpdateGamma( int mode ) {
        return (mModeNum > mode) ? mEnable[mode] : null;
    }

    int[] GetRegister( int mode ) {
        return (mModeNum > mode) ? mData[mode][0] : null;
    }

    int[] GetData( int mode ) {
        return (mModeNum > mode) ? mData[mode][1] : null;
    }

    String GetDescription( int mode ) {
        return (mModeNum > mode) ? mDescription[mode] : null;
    }

    int[][] GetRegData( int mode ) {
        return (mModeNum > mode) ? mData[mode] : null;
    }
}
