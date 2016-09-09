package com.samsung.vd.baseutils;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;
import android.util.Log;

/**
 * Created by doriya on 8/16/16.
 */
public class VdLoginDatabase {
    public static final String LEVEL_OPERATION = "operator";
    public static final String LEVEL_CALIBRATOR = "calibrator";
    public static final String LEVEL_SERVICE = "service";
    public static final String LEVEL_ROOT = "root";

    private final String VD_DTAG = "VdLoginDatabase";

    private static final String DATABASE_NAME = "cinema.login.db";
    private static final int DATABASE_VERSION = 1;
    private static final String TABLE_NAME = "tbl_account";

    private static final String FIELD_ACCOUNT = "account";
    private static final String FIELD_PASSWORD = "password";
    private static final String FIELD_ID = "id";
    private static final String FIELD_LEVEL = "level";

    private Context mContext = null;
    private SQLiteDatabase mDatabase = null;

    public VdLoginDatabase( Context context ) throws SQLException {
        mContext = context;
    }

    public void Open() {
        mDatabase = new DatabaseHelper( mContext, DATABASE_NAME, null,  DATABASE_VERSION ).getWritableDatabase();
    }

    public void Close() {
        mDatabase.close();
    }

    public void InsertData( String account, String password, String id, String level ) {
        if( IsExist(account) ) {
            Log.i(VD_DTAG, "Fail, Already Valid Account. ( " + account + " )");
            return;
        }

        ContentValues values = new ContentValues();
        values.put( FIELD_ACCOUNT, account );
        values.put( FIELD_PASSWORD, password );
        values.put( FIELD_ID, id );
        values.put( FIELD_LEVEL, level );
        mDatabase.insert( TABLE_NAME, null, values );
    }

    public void UpdateData( String account, String password, String id, String level ) {
        if( !IsExist(account) ) {
            Log.i(VD_DTAG, "Fail, Invalid Account. ( " + account + " )");
            return;
        }

        ContentValues values = new ContentValues();
        values.put( FIELD_ACCOUNT, account );
        values.put( FIELD_PASSWORD, password );
        values.put( FIELD_ID, id );
        values.put( FIELD_LEVEL, level );
        mDatabase.update( TABLE_NAME, values, FIELD_ACCOUNT + "=?", new String[]{account});
    }

    public void RemoveData( String account ) {
        if( !IsExist(account) ) {
            Log.i(VD_DTAG, "Fail, Invalid Account. ( " + account + " )");
            return;
        }

        mDatabase.delete( TABLE_NAME, FIELD_ACCOUNT + "=" + "'" + account + "'", null);
    }

    public int GetLevel( String account ) {
        int iLevel = -1;
        Cursor cursor = mDatabase.query( TABLE_NAME, null, null, null, null, null, null );
        while( cursor.moveToNext() )
        {
            String strAccount = cursor.getString( cursor.getColumnIndex(FIELD_ACCOUNT));
            if( strAccount.equals(account) )
            {
                String strTemp = cursor.getString( cursor.getColumnIndex(FIELD_LEVEL) );
                if( strTemp.equals(LEVEL_OPERATION) ) iLevel = 0;
                if( strTemp.equals(LEVEL_CALIBRATOR) ) iLevel = 1;
                if( strTemp.equals(LEVEL_SERVICE) ) iLevel = 2;
                if( strTemp.equals(LEVEL_ROOT) ) iLevel = 3;
                break;
            }
        }
        cursor.close();
        return iLevel;
    }

    public boolean IsExist( String account ) {
        boolean bExist = false;
        Cursor cursor = mDatabase.query( TABLE_NAME, null, null, null, null, null, null );
        while( cursor.moveToNext() )
        {
            String strAccount = cursor.getString( cursor.getColumnIndex(FIELD_ACCOUNT));
            if( strAccount.equals(account) )
            {
                bExist = true;
                break;
            }
        }
        cursor.close();
        return bExist;
    }

    private class DatabaseHelper extends SQLiteOpenHelper {
        public DatabaseHelper(Context context, String name, SQLiteDatabase.CursorFactory factory, int version) {
            super(context, name, factory, version);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            final String query =
                    "create table " +
                    TABLE_NAME + "(" +
                    FIELD_ACCOUNT + " text not null , " +
                    FIELD_PASSWORD + " text not null , " +
                    FIELD_ID + " text not null , " +
                    FIELD_LEVEL + " text not null );";

            db.execSQL( query );
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            final String query = "DROP TABLE IF EXISTS " + TABLE_NAME;
            onCreate( db );
        }
    }
}
