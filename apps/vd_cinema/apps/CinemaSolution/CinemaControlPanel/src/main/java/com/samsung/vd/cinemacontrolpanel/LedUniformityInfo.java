package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.StringTokenizer;

/**
 * Created by doriya on 12/23/16.
 */

public class LedUniformityInfo {
    private static final String VD_DTAG = "LedUniformityInfo";

    public static final String PATH_SOURCE = "DCI";
    public static final String PATH_TARGET = "/storage/sdcard0/SAMSUNG/PFPGA";
    public static final String NAME = "UC_COEF.txt";

    public int[] mData = new int[4096];

    public LedUniformityInfo() {
        for( int i = 0; i < mData.length; i++ ) {
            mData[i] = 65535;
        }
    }

    public boolean Parse( String filePath ) {
        File file = new File( filePath );
        if( !file.isFile() )
            return false;

        String fileName = filePath.substring( filePath.lastIndexOf("/") + 1 );
        if( !fileName.equals( NAME ) )
            return false;

        try {
            FileInputStream inStream = new FileInputStream(filePath);
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inStream));

            String strValue;
            int index = 0;

            while( (strValue = bufferedReader.readLine()) != null ) {
                StringTokenizer token = new StringTokenizer(strValue);
                while(token.hasMoreTokens()) {
                    String strTemp = token.nextToken();
                    try {
                        mData[index] = Integer.parseInt(strTemp, 10);
                    }
                    catch (NumberFormatException e) {
                        Log.i(VD_DTAG, String.format("Fail, Parse(). ( token: %s )", strTemp));
                        return false;
                    }

                    index++;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return true;
    }

    public int[] GetData() {
        return mData;
    }
}
