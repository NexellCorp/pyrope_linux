package com.samsung.vd.cinemacontrolpanel;

import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.util.Arrays;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by doriya on 3/2/17.
 */

public class FileManager {
    private static final String VD_DTAG = "FileManager";

    public FileManager() {
    }

    public static String[] CheckFile( String topdir, String regularExpression ) {
        String[] result = new String[0];
        File topfolder = new File( topdir );
        File[] toplist = topfolder.listFiles();
        if( toplist == null || toplist.length == 0 )
            return result;

        Pattern pattern = Pattern.compile( regularExpression );
        for( File file : toplist ) {
            if( !file.isFile() )
                continue;

            Matcher matcher = pattern.matcher(file.getName());
            if( matcher.matches() ) {
                String[] temp = Arrays.copyOf( result, result.length + 1);
                temp[result.length] = file.getAbsolutePath();
                result = temp;
            }
        }

        return result;
    }

    public static String[] CheckFileInUsb( String topdir, String regularExpression ) {
        String[] result = new String[0];
        for( int i = 0; i < 10; i++ ) {
            File topfolder = new File( String.format(Locale.US, "/storage/usbdisk%d/%s", i, topdir) );
            File[] toplist = topfolder.listFiles();
            if( toplist == null || toplist.length == 0 )
                continue;

            Pattern pattern = Pattern.compile( regularExpression );
            for( File file : toplist ) {
                if( !file.isFile() )
                    continue;

                Matcher matcher = pattern.matcher(file.getName());
                if( matcher.matches() ) {
                    String[] temp = Arrays.copyOf( result, result.length + 1);
                    temp[result.length] = file.getAbsolutePath();
                    result = temp;
                }
            }
        }

        return result;
    }

    public static String[] CheckDirectoryInUsb( String topdir, String regularExpression ) {
        String[] result = new String[0];
        for( int i = 0; i < 10; i++ ) {
            File topfolder = new File( String.format(Locale.US, "/storage/usbdisk%d/%s", i, topdir) );
            File[] toplist = topfolder.listFiles();
            if( toplist == null || toplist.length == 0 )
                continue;

            Pattern pattern = Pattern.compile( regularExpression );
            for( File dir : toplist) {
                if( !dir.isDirectory() )
                    continue;

                Matcher matcher = pattern.matcher(dir.getName());
                if( matcher.matches() ) {
                    String[] temp = Arrays.copyOf( result, result.length + 1);
                    temp[result.length] = dir.getAbsolutePath();
                    result = temp;
                }
            }
        }

        return result;
    }

    public static boolean CopyFile( String inFile, String outFile ) {
        if( !new File(inFile).exists() ) {
            Log.i(VD_DTAG, String.format(Locale.US, "Fail, Invalid File. ( %s )", inFile));
            return false;
        }

        FileInputStream inStream = null;
        FileOutputStream outStream = null;

        FileChannel inChannel = null;
        FileChannel outChannel = null;

        try {
            inStream = new FileInputStream(inFile);
            outStream = new FileOutputStream(outFile);

            inChannel = inStream.getChannel();
            outChannel = outStream.getChannel();

            long size = inChannel.size();
            inChannel.transferTo(0, size, outChannel);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if( inChannel != null ) inChannel.close();
                if( outChannel != null ) outChannel.close();
                if( inStream != null ) inStream.close();
                if( outStream != null ) outStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        return true;
    }

    public static boolean RemoveFile( String inFile ) {
        File file = new File(inFile);
        if( !file.exists() ) {
            Log.i(VD_DTAG, String.format( "Fail, file is not exist. ( %s )", inFile ));
            return false;
        }

        if( !file.isFile() ) {
            Log.i(VD_DTAG, String.format( "Fail, is not file. ( %s )", inFile ));
            return false;
        }

        return file.delete();
    }
}
