package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.Locale;

/**
 * Created by doriya on 2/28/17.
 */

public class ConfigTconInfo {
    private static final String VD_DTAG = "ConfigTconInfo";

    public static final String PATH_SOURCE = "DCI/TCON";
    public static final String PATH_TARGET_USB = "/storage/sdcard0/SAMSUNG/TCON_USB";
    public static final String PATH_TARGET_EEPROM = "/storage/sdcard0/SAMSUNG/TCON_EEPROM";

    public static final String NAME = "T_REG.txt";

    public static final int MODE_BOTH   = 0;
    public static final int MODE_3D     = 1;

    private static final int MAX_INFO_NUM = 30;
    private static final int NUM_INDEX = 1;
    private static final int NUM_ENABLE = 4;
    private static final int NUM_DESCRIPTION = 1;
    private static final int NUM_REG_NUMBER = 1;

    private class NX_TREG_INFO {
        int[]   mEnable      = new int[4];
        String  mDescription = "";
        int     mDataNum     = 0;
        int[][] mData        = new int[2][];
        int     mDataMode    = 0;
    }

    private NX_TREG_INFO[] mInfo = new NX_TREG_INFO[MAX_INFO_NUM];

    public ConfigTconInfo() {
    }

    public boolean Update( String filePath ) {
        File file = new File( filePath );
        if( !file.isFile() )
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

            while( true )
            {
                szLine = bufferedReader.readLine();
                if( szLine == null ) {
                    Log.i(VD_DTAG, String.format( "EOF. ( %s )", filePath ));
                    break;
                }

                szLine = szLine.trim();
                szLineSplit = szLine.split("\\s+");

                if( lineCnt < NUM_INDEX ) {
                    szMode = szLine.toUpperCase();

                    szMode = szLine.toUpperCase();
                    szModeSplit = szMode.split("\\s+");

                    for( int i = 0; i < szModeSplit.length; i++ ) {
                        int mode = Integer.parseInt(szModeSplit[i].replaceAll("MODE", ""), 10);
                        Delete(mode);
                        mInfo[mode] = new NX_TREG_INFO();
                    }
                }
                else if( lineCnt < NUM_INDEX + NUM_ENABLE ) {
                    szModeSplit = szMode.split("\\s+");
                    for( int i = 0; i < szModeSplit.length; i++ )
                    {
                        try {
                            int mode = Integer.parseInt(szModeSplit[i].replaceAll("MODE", ""), 10);
                            mInfo[mode].mEnable[lineCnt - NUM_INDEX] = Integer.parseInt(szLineSplit[i], 10);
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
                else if( lineCnt < NUM_INDEX + NUM_ENABLE + NUM_DESCRIPTION ) {
                    szModeSplit = szMode.split("\\s+");
                    for( int i = 0; i < szModeSplit.length; i++ )
                    {
                        int mode = Integer.parseInt(szModeSplit[i].replaceAll("MODE", ""), 10);
                        mInfo[mode].mDescription = szLineSplit[i];
                    }
                }
                else if( lineCnt < NUM_INDEX + NUM_ENABLE + NUM_DESCRIPTION + NUM_REG_NUMBER ) {
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

        for( int i = 0; i < MAX_INFO_NUM; i++ ) {
            int[] reg = GetRegister(i);
            int[] dat = GetData(i);

            if( reg == null || dat == null )
                continue;

            for(int j = 0; j < reg.length; j++ ) {
                if( reg[j] == 0x0030 ) {
                    mInfo[i].mDataMode = dat[j];
                    break;
                }
            }
        }

        // Debug Message
        // Dump();

        return true;
    }

    public boolean Update( String filePath, int mode ) {
        File file = new File( filePath );
        if( !file.isFile() )
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

                szLine = szLine.trim();
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
                        mInfo[mode] = new NX_TREG_INFO();
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
                        mInfo[mode].mEnable[lineCnt - NUM_INDEX] = Integer.parseInt(szLineSplit[posCnt], 10);
                    }
                    catch (NumberFormatException e) {
                        Log.i(VD_DTAG, String.format("Fail, Parse(). ( token: %s )", szLineSplit[posCnt]));
                        bufferedReader.close();
                        inStreamReader.close();
                        inStream.close();
                        return false;
                    }
                }
                else if( lineCnt < NUM_INDEX + NUM_ENABLE + NUM_DESCRIPTION ) {
                    mInfo[mode].mDescription = szLineSplit[posCnt];
                }
                else if( lineCnt < NUM_INDEX + NUM_ENABLE + NUM_DESCRIPTION + NUM_REG_NUMBER ) {
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

        int[] reg = GetRegister(mode);
        int[] dat = GetData(mode);
        if( reg != null && dat != null ) {
            for(int j = 0; j < reg.length; j++ ) {
                if( reg[j] == 0x0030 ) {
                    mInfo[mode].mDataMode = dat[j];
                    break;
                }
            }
        }

        // Debug Message
        // Dump();

        return true;
    }

    public void Delete() {
        for( int i = 0; i < MAX_INFO_NUM; i++ ) {
            Delete( i );
        }
    }

    public void Delete( int mode ) {
        if( mInfo[mode] != null ) {
            mInfo[mode].mEnable     = new int[4];
            mInfo[mode].mDescription= "";
            mInfo[mode].mDataNum    = 0;
            mInfo[mode].mData       = new int[2][];
            mInfo[mode].mDataMode   = 0;
            mInfo[mode] = null;
        }
    }

    public boolean Make( String fileName, int from, int to ) {
        try {
            FileOutputStream outStream = new FileOutputStream( fileName );
            OutputStreamWriter outStreamWriter = new OutputStreamWriter(outStream);
            BufferedWriter bufferedWriter = new BufferedWriter( outStreamWriter );

            int lineCnt = 0;
            int dataCnt = 0;
            int maxDataNum = 0;
            boolean run = (0 < GetModeNum( from, to) );

            while( run )
            {
                int written = 0;
                String szData;

                for( int i = 0; i < MAX_INFO_NUM; i++ )
                {
                    if( null == mInfo[i] )
                        continue;

                    if( i < from || i > to )
                        continue;

                    if( lineCnt < NUM_INDEX ) {
                        szData = String.format(Locale.US, "%sMODE%d",
                                (written > 0) ? "\t" : "",
                                i
                        );
                    }
                    else if( lineCnt < NUM_INDEX + NUM_ENABLE) {
                        szData = String.format(Locale.US, "%s%d",
                                (written > 0) ? "\t" : "",
                                mInfo[i].mEnable[lineCnt-NUM_INDEX]
                        );
                    }
                    else if( lineCnt < NUM_INDEX + NUM_ENABLE + NUM_DESCRIPTION ) {
                        szData = String.format(Locale.US, "%s%s",
                                (written > 0) ? "\t" : "",
                                mInfo[i].mDescription
                        );
                    }
                    else if( lineCnt < NUM_INDEX + NUM_ENABLE + NUM_DESCRIPTION + NUM_REG_NUMBER ) {
                        szData = String.format(Locale.US, "%s%d",
                                (written > 0) ? "\t" : "",
                                mInfo[i].mDataNum
                        );

                        if( maxDataNum < mInfo[i].mDataNum )
                            maxDataNum = mInfo[i].mDataNum;
                    }
                    else {
                        dataCnt = (lineCnt - NUM_INDEX - NUM_ENABLE - NUM_DESCRIPTION - NUM_REG_NUMBER);

                        if( (dataCnt/2) == maxDataNum ) {
                            run = false;
                            break;
                        }

                        if( (dataCnt/2) < mInfo[i].mDataNum )
                        {
                            if( 0 == (dataCnt%2) )
                            {
                                szData = String.format(Locale.US, "%s0x%X",
                                        (written > 0) ? "\t" : "",
                                        mInfo[i].mData[0][dataCnt/2]
                                );
                            }
                            else
                            {
                                szData = String.format(Locale.US, "%s%d",
                                        (written > 0) ? "\t" : "",
                                        mInfo[i].mData[1][dataCnt/2]
                                );
                            }
                        }
                        else
                        {
                            szData = String.format(Locale.US, "%s",
                                    (written > 0) ? "\t" : " "
                            );
                        }
                    }

                    written += szData.length();
                    bufferedWriter.write( szData );
                }

                bufferedWriter.write( "\r\n" );
                lineCnt++;
            }

            bufferedWriter.close();
            outStreamWriter.close();
            outStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        Log.i(VD_DTAG, String.format(Locale.US, "Make Done. ( %s )", fileName));

        return true;
    }

    void Dump() {
        for( int i = 0; i < MAX_INFO_NUM; i++ ) {
            if( mInfo[i] == null )
                continue;

            Log.i(VD_DTAG, String.format("* mode %d : %s", i, mInfo[i].mDescription));
            Log.i(VD_DTAG, String.format("-. 3D Mode : %b", mInfo[i].mDataMode));
            Log.i(VD_DTAG, String.format("-. TGAM0 ( %d ), TGAM1 ( %d ), DGAM0 ( %d ), DGAM1 ( %d )",
                    mInfo[i].mEnable[0], mInfo[i].mEnable[1], mInfo[i].mEnable[2], mInfo[i].mEnable[3]) );

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

        Log.i(VD_DTAG, String.format("* mode %d : %s", mode, mInfo[mode].mDescription));
        Log.i(VD_DTAG, String.format("-. 3D Mode : %b", mInfo[mode].mDataMode));
        Log.i(VD_DTAG, String.format("-. TGAM0 ( %d ), TGAM1 ( %d ), DGAM0 ( %d ), DGAM1 ( %d )",
                mInfo[mode].mEnable[0], mInfo[mode].mEnable[1], mInfo[mode].mEnable[2], mInfo[mode].mEnable[3]) );

        Log.i(VD_DTAG, String.format("> register number for writing : %d", mInfo[mode].mData[0].length));
        for( int j = 0; j < mInfo[mode].mData[0].length; j++ ) {
            Log.i(VD_DTAG, String.format("-. reg( %3d, 0x%02x ), data( %4d, 0x%04x )",
                    mInfo[mode].mData[0][j], mInfo[mode].mData[0][j], mInfo[mode].mData[1][j], mInfo[mode].mData[1][j] ));
        }
    }

    int GetModeNum() {
        return GetModeNum( 0, MAX_INFO_NUM );
    }

    int GetModeNum( int from, int to ) {
        int count = 0;
        for( int i = 0; i < MAX_INFO_NUM; i++ ) {
            if( null == mInfo[i] )
                continue;

            if( i < from || i > to )
                continue;

            count++;
        }
        return count;
    }


    boolean IsValid( int mode ) {
        return (mInfo[mode] != null);
    }

    int[] GetEnableUpdateGamma( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mEnable : null;
    }

    int[] GetRegister( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mData[0] : null;
    }

    int[] GetData( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mData[1] : null;
    }

    String GetDescription( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mDescription : null;
    }

    int[][] GetRegData( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mData : null;
    }

    int GetDataMode( int mode ) {
        return (mInfo[mode] != null) ? mInfo[mode].mDataMode : -1;
    }
}
