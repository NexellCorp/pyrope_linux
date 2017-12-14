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

public class ConfigPfpgaInfo {
    private static final String VD_DTAG = "ConfigPfpgaInfo";

    public static final String PATH_SOURCE = "DCI/PFPGA";
    public static final String PATH_TARGET = "/storage/sdcard0/SAMSUNG/PFPGA";

    public static final String NAME = "P_REG.txt";

    private static final int MAX_MODE_NUM = 10;
    private static final int NUM_INDEX = 1;
    private static final int NUM_ENABLE = 1;
    private static final int NUM_REG_NUMBER = 1;

    private int mModeNum = 0;
    private boolean[] mEnable = new boolean[MAX_MODE_NUM];
    private int[] mDataNum = new int[MAX_MODE_NUM];
    private int[][][] mData = new int[MAX_MODE_NUM][2][];

    public ConfigPfpgaInfo() {
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
                        mEnable[idxLine-NUM_INDEX] = (Integer.parseInt( strSplit[i], 10 ) == 1);
                    }
                }
                else if( idxLine < NUM_INDEX + NUM_ENABLE + NUM_REG_NUMBER ) {
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

            Log.i(VD_DTAG, String.format("* mode %d", i));
            Log.i(VD_DTAG, String.format("-. Uniformity Correction ( %b )", mEnable[i] ));
            Log.i(VD_DTAG, String.format("> register number for writing : %d", mData[i][0].length));
            for( int j = 0; j < mData[i][0].length; j++ ) {
                Log.i(VD_DTAG, String.format("-. reg( %3d, 0x%02x ), data( %4d, 0x%04x )",
                        mData[i][0][j], mData[i][0][j], mData[i][1][j], mData[i][1][j] ));
            }
        }

        return true;
    }

    int GetModeNum() { return mModeNum; }

    boolean GetEnableUpdateUniformity( int mode ) {
        return (mModeNum > mode) && mEnable[mode];
    }

    int[] GetRegister( int mode ) {
        return (mModeNum > mode) ? mData[mode][0] : null;
    }

    int[] GetData( int mode ) {
        return (mModeNum > mode) ? mData[mode][1] : null;
    }

    int[][] GetRegData( int mode ) {
        return (mModeNum > mode) ? mData[mode] : null;
    }
}
