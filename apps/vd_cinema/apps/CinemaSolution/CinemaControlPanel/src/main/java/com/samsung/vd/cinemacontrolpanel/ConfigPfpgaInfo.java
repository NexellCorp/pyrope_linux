package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * Created by doriya on 2/28/17.
 */

public class ConfigPfpgaInfo {
    private static final String VD_DTAG = "ConfigPfpgaInfo";

    public static final String PATH_SOURCE = "DCI/PFPGA";
    public static final String PATH_TARGET = "/storage/sdcard0/SAMSUNG/PFPGA";

    public static final String NAME = "P_REG.txt";

    private static final int MAX_INFO_NUM = 30;
    private static final int NUM_INDEX = 1;
    private static final int NUM_ENABLE = 1;
    private static final int NUM_REG_NUMBER = 1;

    private class NX_PREG_INFO {
        int     mEnable     = 0;
        int     mDataNum    = 0;
        int[][] mData       = new int[2][];
    }

    private NX_PREG_INFO[] mInfo = new NX_PREG_INFO[MAX_INFO_NUM];

    public ConfigPfpgaInfo() {
    }

    public boolean Parse( String filePath ) {
        File file = new File( filePath );
        if( !file.isFile() )
            return false;

        String fileName = filePath.substring( filePath.lastIndexOf("/") + 1 );
        if( !fileName.equals(NAME) )
            return false;

        Delete();

        try {
            FileInputStream inStream = new FileInputStream( filePath );
            InputStreamReader inStreamReader = new InputStreamReader( inStream );
            BufferedReader bufferedReader = new BufferedReader( inStreamReader );

            String szMode = "";
            String szLine = "";
            String[] szLineSplit, szModeSplit;

            int lineCnt = 0;
            int dataCnt = 0;

            while( true )
            {
                szLine = bufferedReader.readLine();
                if( szLine == null ) {
                    Log.i(VD_DTAG, String.format( "EOF. ( %s )", filePath ));
                    break;
                }

                // Log.i(VD_DTAG, String.format(">>> %s", strLine) );
                szLineSplit = szLine.split("\\s+");

                if( lineCnt < NUM_INDEX ) {
                    szMode = szLine.toUpperCase();
                }
                else if( lineCnt < NUM_INDEX + NUM_ENABLE ) {
                    szModeSplit = szMode.split("\\s+");
                    for( int i = 0; i < szModeSplit.length; i++ )
                    {
                        try {
                            int mode = Integer.parseInt(szModeSplit[i].replaceAll("MODE", ""), 10);
                            mInfo[mode].mEnable = Integer.parseInt( szLineSplit[i], 10 );
                        }
                        catch (NumberFormatException e) {
                            Log.i(VD_DTAG, String.format("Fail, Parse(). ( token: %s )", szLineSplit[i]));
                            bufferedReader.close();
                            inStreamReader.close();
                            inStream.close();
                            return false;
                        }
                    }
                }
                else if( lineCnt < NUM_INDEX + NUM_ENABLE + NUM_REG_NUMBER ) {
                    szModeSplit = szMode.split("\\s+");
                    for( int i = 0; i < szModeSplit.length; i++ )
                    {
                        try {
                            int mode = Integer.parseInt(szModeSplit[i].replaceAll("MODE", ""), 10);
                            mInfo[mode].mDataNum = Integer.parseInt(szLineSplit[i], 10);
                            mInfo[mode].mData[0] = new int[mInfo[mode].mDataNum];
                            mInfo[mode].mData[1] = new int[mInfo[mode].mDataNum];

                        }
                        catch (NumberFormatException e) {
                            Log.i(VD_DTAG, String.format("Fail, Parse(). ( token: %s )", szLineSplit[i]));
                            bufferedReader.close();
                            inStreamReader.close();
                            inStream.close();
                            return false;
                        }
                    }
                }
                else {
                    int skipCnt = 0;
                    szModeSplit = szMode.split("\\s+");

                    for( int i = 0; i < szModeSplit.length; i++ )
                    {
                        int mode = Integer.parseInt(szModeSplit[i].replaceAll("MODE", ""), 10);
                        if( szLineSplit[i].equals("") )
                            continue;

                        if( dataCnt/2 > mInfo[mode].mDataNum-1 )
                        {
                            skipCnt++;
                            continue;
                        }

                        mInfo[mode].mData[dataCnt%2][dataCnt/2] =
                                ((dataCnt%2) == 0) ? Integer.decode(szLineSplit[i-skipCnt]) : Integer.parseInt(szLineSplit[i-skipCnt], 10);
                    }

                    dataCnt++;
                }

                lineCnt++;
            }

            bufferedReader.close();
            inStreamReader.close();
            inStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        //
        // Debug Message
        //
        // Dump();

        return true;
    }

    public boolean Update( String filePath ) {
        for( int i = 0; i < MAX_INFO_NUM; i++ ) {
            Update(filePath, i);
        }
        return true;
    }

    public boolean Update( String filePath, int mode ) {
        File file = new File( filePath );
        if( !file.isFile() )
            return false;

        String fileName = filePath.substring( filePath.lastIndexOf("/") + 1 );
        if( !fileName.equals(NAME) )
            return false;

        try {
            FileInputStream inStream = new FileInputStream( filePath );
            InputStreamReader inStreamReader = new InputStreamReader(inStream);
            BufferedReader bufferedReader = new BufferedReader( inStreamReader );

            String szMode = "";
            String szLine = "";
            String[] szLineSplit, szModeSplit;

            int lineCnt = 0;
            int dataCnt = 0;
            int posCnt = 0;

            int[] dataNum = new int[MAX_INFO_NUM];
            for( int i = 0; i < MAX_INFO_NUM; i++ ) {
                dataNum[i] = 0;
            }

            while( true )
            {
                szLine = bufferedReader.readLine();
                if( szLine == null ) {
                    Log.i(VD_DTAG, String.format( "EOF. ( %s )", filePath ));
                    break;
                }

                szLineSplit = szLine.split("\\s+");
                // Log.i(VD_DTAG, String.format(">>> %s", szLine) );

                if( lineCnt < NUM_INDEX ) {
                    szMode = szLine.toUpperCase();
                    szModeSplit = szMode.split("\\s+");

                    int temp = 0;
                    for( int i = 0; i < szModeSplit.length; i++ ) {
                        temp = Integer.parseInt(szModeSplit[i].replaceAll("MODE", ""), 10);
                        if( temp == mode )
                            break;

                        posCnt++;
                    }

                    if( temp == mode ) {
                        Delete(temp);
                        mInfo[mode] = new NX_PREG_INFO();
                    }
                    else {
                        bufferedReader.close();
                        inStreamReader.close();
                        inStream.close();
                        return false;
                    }
                }
                else if( lineCnt < NUM_INDEX + NUM_ENABLE ) {
                    try {
                        mInfo[mode].mEnable = Integer.parseInt(szLineSplit[posCnt], 10);
                    }
                    catch (NumberFormatException e) {
                        Log.i(VD_DTAG, String.format("Fail, Parse(). ( token: %s )", szLineSplit[posCnt]));
                        bufferedReader.close();
                        inStreamReader.close();
                        inStream.close();
                        return false;
                    }
                }
                else if( lineCnt < NUM_INDEX + NUM_ENABLE + NUM_REG_NUMBER ) {
                    szModeSplit = szMode.split("\\s+");
                    for( int i = 0; i < szModeSplit.length; i++ )
                    {
                        try {
                            int temp = Integer.parseInt(szModeSplit[i].replaceAll("MODE", ""), 10);
                            dataNum[temp] = Integer.parseInt(szLineSplit[i], 10);
                        }
                        catch (NumberFormatException e) {
                            Log.i(VD_DTAG, String.format("Fail, Parse(). ( token: %s )", szLineSplit[i]));
                            bufferedReader.close();
                            inStreamReader.close();
                            inStream.close();
                            return false;
                        }
                    }

                    mInfo[mode].mDataNum = Integer.parseInt(szLineSplit[posCnt], 10);
                    mInfo[mode].mData[0] = new int[mInfo[mode].mDataNum];
                    mInfo[mode].mData[1] = new int[mInfo[mode].mDataNum];
                }
                else {
                    int skipCnt = 0;
                    szModeSplit = szMode.split("\\s+");

                    for( int i = 0; i < szModeSplit.length; i++ )
                    {
                        int temp = Integer.parseInt(szModeSplit[i].replaceAll("MODE", ""), 10);
                        if( szLineSplit[i].equals("") )
                            continue;

                        if( dataCnt/2 > dataNum[temp]-1 )
                        {
                            skipCnt++;
                            continue;
                        }

                        if( temp == mode )
                        {
                            mInfo[mode].mData[dataCnt%2][dataCnt/2] =
                                    ((dataCnt%2) == 0) ? Integer.decode(szLineSplit[i-skipCnt]) : Integer.parseInt(szLineSplit[i-skipCnt], 10);
                            break;
                        }
                    }

                    if( (++dataCnt) / 2 == mInfo[mode].mDataNum )
                        break;
                }

                lineCnt++;
            }

            bufferedReader.close();
            inStreamReader.close();
            inStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        // Debug Message
        // Dump();

        return true;
    }

    void Delete() {
        for( int i = 0; i < MAX_INFO_NUM; i++ ) {
            Delete(i);
        }
    }

    void Delete( int mode ) {
        if( mInfo[mode] != null ) {
            mInfo[mode].mEnable = 0;
            mInfo[mode].mData   = new int[2][];
            mInfo[mode].mDataNum= 0;
            mInfo[mode] = null;
        }
    }

    void Dump() {
        for( int i = 0; i < MAX_INFO_NUM; i++ ) {
            if( mInfo[i] == null )
                continue;

            Log.i(VD_DTAG, String.format("* mode %d", i));
            Log.i(VD_DTAG, String.format("-. Uniformity Correction ( %d )", mInfo[i].mEnable ));
            Log.i(VD_DTAG, String.format("> register number for writing : %d", mInfo[i].mData[0].length));
            for( int j = 0; j < mInfo[i].mData[0].length; j++ ) {
                Log.i(VD_DTAG, String.format("-. reg( %3d, 0x%02x ), data( %4d, 0x%04x )",
                        mInfo[i].mData[0][j], mInfo[i].mData[0][j], mInfo[i].mData[1][j], mInfo[i].mData[1][j] ));
            }
        }
    }

    void Dump( int mode ) {
        if( mInfo[mode] == null )
            return;

        Log.i(VD_DTAG, String.format("* mode %d", mode));
        Log.i(VD_DTAG, String.format("-. Uniformity Correction ( %d )", mInfo[mode].mEnable ));
        Log.i(VD_DTAG, String.format("> register number for writing : %d", mInfo[mode].mData[0].length));
        for( int j = 0; j < mInfo[mode].mData[0].length; j++ ) {
            Log.i(VD_DTAG, String.format("-. reg( %3d, 0x%02x ), data( %4d, 0x%04x )",
                    mInfo[mode].mData[0][j], mInfo[mode].mData[0][j], mInfo[mode].mData[1][j], mInfo[mode].mData[1][j] ));
        }
    }

    int GetModeNum() {
        int count = 0;
        for(int i = 0; i < MAX_INFO_NUM; i++ ) {
            if( null == mInfo[i] )
                continue;

            count++;
        }
        return count;
    }

    boolean IsValid( int mode ) {
        return (mInfo[mode] != null);
    }

    int GetEnableUpdateUniformity( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mEnable : 0;
    }

    int[] GetRegister( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mData[1] : null;
    }

    int[] GetData( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mData[1] : null;
    }

    int[][] GetRegData( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mData : null;
    }
}
