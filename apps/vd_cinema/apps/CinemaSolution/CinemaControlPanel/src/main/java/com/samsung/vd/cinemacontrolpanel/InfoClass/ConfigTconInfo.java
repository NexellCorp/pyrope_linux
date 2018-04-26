    package com.samsung.vd.cinemacontrolpanel.InfoClass;

import android.content.Context;
import android.util.Log;

import com.samsung.vd.cinemacontrolpanel.Utils.FileManager;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.text.NumberFormat;
import java.text.ParseException;
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
    private int[] mMode = new int[MAX_MODE_NUM];
    private int[][] mEnable = new int[MAX_MODE_NUM][4];
    private String[] mDescription = new String[MAX_MODE_NUM];
    private int[] mDataNum = new int[MAX_MODE_NUM];
    private int[][][] mData = new int[MAX_MODE_NUM][2][];

    String mfilePath = null;

    public ConfigTconInfo() {
    }

    public boolean Parse( String filePath ) {
        File file = new File( filePath );
        if( !file.isFile() ){
            return false;
        }

        mfilePath = filePath;

        String fileName = filePath.substring( filePath.lastIndexOf("/") + 1 );
        if( !fileName.equals(NAME) )
            return false;

        for( int i = 0; i < MAX_MODE_NUM; i++ ) {
            mDataNum[i] = 0;
            mMode[i] = -1;
        }


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
                    Log.i(VD_DTAG , "mModeNum : "+mModeNum);

                    //parse mode num
                    for(int i = 0 ; i < mModeNum ; i++) {
                        //Log.i(VD_DTAG , "orig : "+strSplit[i]);
                        if( strSplit[i].equals("") )
                            continue;

                        mMode[i] = Integer.parseInt(strSplit[i].replaceAll("[\\D]",""));
                        //Log.i(VD_DTAG , "mode : "+mMode[i]);
                    }


                }
                else if( idxLine < NUM_INDEX + NUM_ENABLE ) {
                    for( int i = 0; i < strSplit.length; i++ )
                    {
                        if( strSplit[i].equals("") )
                            continue;

                        try {
                            mEnable[i][idxLine - NUM_INDEX] = Integer.parseInt(strSplit[i], 10);
                        }
                        catch (NumberFormatException e) {
                            Log.i(VD_DTAG, String.format("Fail, Parse(). ( token: %s )", strSplit[i]));
                            return false;
                        }
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
                        if( strSplit[i].equals("") )
                            continue;

                        try {
                            mDataNum[i] = Integer.parseInt(strSplit[i], 10);
                        }
                        catch (NumberFormatException e) {
                            Log.i(VD_DTAG, String.format("Fail, Parse(). ( token: %s )", strSplit[i]));
                            return false;
                        }

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

                                try {
                                    mData[j][idxData % 2][idxData / 2] =
                                            ((idxData % 2) == 0) ? Integer.decode(strSplit[i]) : Integer.parseInt(strSplit[i], 10);
                                }
                                catch (NumberFormatException e) {
                                    Log.i(VD_DTAG, String.format("Fail, Parse(). ( token: %s )", strSplit[i]));
                                    return false;
                                }

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
//        for( int i = 0; i < mModeNum; i++ ) {
//            if( mData[i][0] == null )
//                continue;
//
//            Log.i(VD_DTAG, String.format("* mode %d : %s", i, mDescription[i]));
//            Log.i(VD_DTAG, String.format("-. TGAM0 ( %d ), TGAM1 ( %d ), DGAM0 ( %d ), DGAM1 ( %d )",
//                    mEnable[i][0], mEnable[i][1], mEnable[i][2], mEnable[i][3]) );
//
//            Log.i(VD_DTAG, String.format("> register number for writing : %d", mData[i][0].length));
//            for( int j = 0; j < mData[i][0].length; j++ ) {
//                Log.i(VD_DTAG, String.format("-. reg( %3d, 0x%02x ), data( %4d, 0x%04x )",
//                        mData[i][0][j], mData[i][0][j], mData[i][1][j], mData[i][1][j] ));
//            }
//        }

        return true;
    }

    public int GetModeNum() {
        return mModeNum;
    }

    public int[] GetModeArray() {
        return mMode;
    }


    public int GetMode(int mode) {
        if (mModeNum > mode) {
            return mMode[mode];
        }
        return -1;
    }

    public int GetDataNum(int mode) {
        return (mModeNum > mode) ? mDataNum[mode] : -1;
    }

    public int[] GetEnableUpdateGamma( int mode ) {
        return (mModeNum > mode) ? mEnable[mode] : null;
    }

    public int[] GetRegister( int mode ) {
        return (mModeNum > mode) ? mData[mode][0] : null;
    }

    public int[] GetData( int mode ) {
        return (mModeNum > mode) ? mData[mode][1] : null;
    }

    public String GetDescription( int mode ) {
        return (mModeNum > mode) ? mDescription[mode] : null;
    }

    public int[][] GetRegData( int mode ) {
        return (mModeNum > mode) ? mData[mode] : null;
    }

    public void Remove( int mode , Context context) {
        if (null == mfilePath) {
            //not parse before
            return;
        }

        //mode  is a actual mode in mMode[]
        int iModeIdx = -1;
        for(int k = 0 ; k < mMode.length ; k++) {
            if(mode == mMode[k]) {
                iModeIdx = k;
            }
        }

        if (-1 == iModeIdx) {
            //mode is not exist in target path..
            return;
        }

        if(0 != mModeNum && null != mData[iModeIdx][0] && null != mData[iModeIdx][1]) {
            mModeNum--;
            int[] tempMode = new int[MAX_MODE_NUM];
            int[][] tempEnable = new int[MAX_MODE_NUM][4];
            String[] tempDescription = new String[MAX_MODE_NUM];
            int[] tempDataNum = new int[MAX_MODE_NUM];
            int[][][] tempData = new int[MAX_MODE_NUM][2][];

            int foundedCount = 0;
            for(int n = 0 ; n < mMode.length ; n++){
                if (n == iModeIdx) {
                    foundedCount++;
                    continue;
                }
                tempMode[n-foundedCount] = mMode[n];
                tempEnable[n-foundedCount][0] = mEnable[n][0];
                tempEnable[n-foundedCount][1] = mEnable[n][1];
                tempEnable[n-foundedCount][2] = mEnable[n][2];
                tempEnable[n-foundedCount][3] = mEnable[n][3];
                tempDescription[n-foundedCount] = mDescription[n];
                tempDataNum[n-foundedCount] = mDataNum[n];
                tempData[n-foundedCount][0] = mData[n][0];
                tempData[n-foundedCount][1] = mData[n][1];
            }
            mMode = tempMode;
            mEnable = tempEnable;
            mDescription = tempDescription;
            mDataNum = tempDataNum;
            mData = tempData;

            UpdateFile(context);

        }else {
            //no parsed data
        }
    }

    public void Update( int mode , Context context , ConfigTconInfo sourceTconInfo) {
        if (null == mfilePath) {
            //not parse before
            return;
        }

        //mode  is a actual parsed mode in mMode[]
        int iModeIdx = -1;
        for(int k = 0 ; k < mMode.length ; k++) {
            if(mode == mMode[k]) {
                iModeIdx = k;
            }
        }

        if (-1 == iModeIdx) {
            //mode is not exist in target path..
            return;
        }

        //
        //update mode
        int[] sourceMode = sourceTconInfo.GetModeArray();
        int isourceIdx = -1;
        if(null != sourceMode) {
            for(int k = 0 ; k < sourceMode.length ; k++) {
                if(mode == sourceMode[k]) {
                    isourceIdx = k;
                }
            }
        }
        if (-1 == isourceIdx) {
            //mode is not exist in target path..
            return;
        }

        if( null != mData[iModeIdx][0] && null != sourceTconInfo.GetData(isourceIdx) ) {
            Log.i(VD_DTAG, "sourceTconInfo.GetDescription(isourceIdx); : "+sourceTconInfo.GetDescription(isourceIdx));
            mDescription[iModeIdx] = sourceTconInfo.GetDescription(isourceIdx);

            int[] tempEnable = sourceTconInfo.GetEnableUpdateGamma(isourceIdx);
//            Log.i(VD_DTAG , "tempEnable[0]; : "+tempEnable[0]);
//            Log.i(VD_DTAG , "tempEnable[1]; : "+tempEnable[1]);
//            Log.i(VD_DTAG , "tempEnable[2]; : "+tempEnable[2]);
//            Log.i(VD_DTAG , "tempEnable[3]; : "+tempEnable[3]);

            mEnable[iModeIdx][0] = tempEnable[0];
            mEnable[iModeIdx][1] = tempEnable[1];
            mEnable[iModeIdx][2] = tempEnable[2];
            mEnable[iModeIdx][3] = tempEnable[3];

//            Log.i(VD_DTAG , "prev DataNum = "+mDataNum[iModeIdx]+"  new DataNum = "+ sourceTconInfo.GetDataNum(isourceIdx));
            mDataNum[iModeIdx] = sourceTconInfo.GetDataNum(isourceIdx);

            int[] tempReg = sourceTconInfo.GetData(isourceIdx);
            int[] tempData = sourceTconInfo.GetData(isourceIdx);

            mData[iModeIdx][0] = new int[tempReg.length];
            mData[iModeIdx][1] = new int[tempData.length];

            for( int j = 0 ; j < tempReg.length ; j++ ) {
                mData[iModeIdx][0][j] = tempReg[j];
                mData[iModeIdx][1][j] = tempData[j];
            }

            UpdateFile(context);
        }else {
            //err
        }

        //
        // Debug Message
        //
//        for( int i = 0; i < mModeNum; i++ ) {
//            if( mData[i][0] == null )
//                continue;
//
//            Log.i(VD_DTAG, String.format("* mode %d : %s", i, mDescription[i]));
//            Log.i(VD_DTAG, String.format("-. TGAM0 ( %b ), TGAM1 ( %b ), DGAM0 ( %b ), DGAM1 ( %b )",
//                    mEnable[i][0], mEnable[i][1], mEnable[i][2], mEnable[i][3]) );
//
//            Log.i(VD_DTAG, String.format("> register number for writing : %d", mData[i][0].length));
//            for( int j = 0; j < mData[i][0].length; j++ ) {
//                Log.i(VD_DTAG, String.format("-. reg( %3d, 0x%02x ), data( %4d, 0x%04x )",
//                        mData[i][0][j], mData[i][0][j], mData[i][1][j], mData[i][1][j] ));
//            }
//        }
    }

    private void UpdateFile(Context context) {
        if (null == mfilePath) {
            //not parse before
            return;
        }

        //make updated temp file
        StringBuilder stringBuilder = new StringBuilder();

        for(int i = 0 ; i < mModeNum ; i++) {
            if( mData[i][0] == null || 0 == mData[i][0].length ) continue;
            stringBuilder.append( String.format("MODE%d", mMode[i]) );

            if( i != (mModeNum-1) ) stringBuilder.append( String.format("    ") );
        }
        stringBuilder.append("\n");
        for(int i = 0 ; i < mModeNum ; i++) {
            if( mData[i][0] == null || 0 == mData[i][0].length ) continue;
            int iBool = 0;
            if(1 == mEnable[i][0]) iBool = 1;
            stringBuilder.append( String.format("%d", iBool) );

            if( i != (mModeNum-1) ) stringBuilder.append( String.format("    ") );
        }
        stringBuilder.append("\n");
        for(int i = 0 ; i < mModeNum ; i++) {
            if( mData[i][0] == null || 0 == mData[i][0].length ) continue;
            int iBool = 0;
            if(1 == mEnable[i][1]) iBool = 1;
            stringBuilder.append( String.format("%d", iBool) );

            if( i != (mModeNum-1) ) stringBuilder.append( String.format("    ") );
        }
        stringBuilder.append("\n");
        for(int i = 0 ; i < mModeNum ; i++) {
            if( mData[i][0] == null || 0 == mData[i][0].length ) continue;
            int iBool = 0;
            if(1 == mEnable[i][2]) iBool = 1;
            stringBuilder.append( String.format("%d", iBool) );

            if( i != (mModeNum-1) ) stringBuilder.append( String.format("    ") );
        }
        stringBuilder.append("\n");

        for(int i = 0 ; i < mModeNum ; i++) {
            if( mData[i][0] == null || 0 == mData[i][0].length ) continue;
            int iBool = 0;
            if(1 == mEnable[i][3]) iBool = 1;
            stringBuilder.append( String.format("%d", iBool) );

            if( i != (mModeNum-1) ) stringBuilder.append( String.format("    ") );
        }
        stringBuilder.append("\n");

        for(int i = 0 ; i < mModeNum ; i++) {
            if( mData[i][0] == null || 0 == mData[i][0].length ) continue;
            stringBuilder.append( String.format("%s", mDescription[i]) );

            if( i != (mModeNum-1) ) stringBuilder.append( String.format("    ") );
        }
        stringBuilder.append("\n");

        int iMaxNum = 0;
        for(int i = 0 ; i < mModeNum ; i++) {
            if( mData[i][0] == null || 0 == mData[i][0].length ) continue;
            stringBuilder.append( String.format("%d", mDataNum[i]) );

            if( i != (mModeNum-1) ) stringBuilder.append( String.format("    ") );

            if(mDataNum[i] > iMaxNum) iMaxNum = mDataNum[i];
        }
        stringBuilder.append("\n");


        for(int j = 0 ; j < iMaxNum ; j++) {
            for(int i = 0 ; i < mModeNum ; i++) {
                if( mData[i][0] == null || 0 == mData[i][0].length ) continue;
                if(j > mData[i][0].length) continue;

                stringBuilder.append( String.format("0x%x", mData[i][0][j] ) );

                if( i != (mModeNum-1) ) stringBuilder.append( String.format("    ") );
            }
            stringBuilder.append("\n");

            for(int i = 0 ; i < mModeNum ; i++) {
                if( mData[i][0] == null || 0 == mData[i][0].length ) continue;
                if(j > mData[i][0].length) continue;

                stringBuilder.append( String.format("%d", mData[i][1][j] ) );

                if( i != (mModeNum-1) ) stringBuilder.append( String.format("    ") );
            }
            stringBuilder.append("\n");
        }

        String data = stringBuilder.toString();

        try {
            OutputStreamWriter outputStreamWriter = new OutputStreamWriter(context.openFileOutput("temp.txt", Context.MODE_PRIVATE));
            outputStreamWriter.write(data);
            outputStreamWriter.close();
        }
        catch (IOException e) {
            Log.e("Exception", "File write failed: " + e.toString());
        }
        //copy to path
        FileManager.CopyFile(context.getFilesDir()+"/temp.txt" , mfilePath );
//        Log.i(VD_DTAG , "context.getFilesDir() : "+context.getFilesDir());
    }
}
