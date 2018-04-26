package com.samsung.vd.cinemacontrolpanel.Utils;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

/**
 * Created by doriya on 11/4/16.
 */
public class CinemaLog {
    private final String VD_DTAG = "CinemaLog";

    private static final String DATABASE_NAME = "cinema.log.db";
    private static final int DATABASE_VERSION = 1;

    public static final String TABLE_LOG            = "tbl_log";
    public static final String FIELD_LOG_ID         = "_id";
    public static final String FIELD_LOG_DATE       = "_date";
    public static final String FIELD_LOG_ACCOUNT    = "_account";
    public static final String FIELD_LOG_DESCRIBE   = "_describe";
    public static final String FIELD_LOG_LOCAL_DATE =  "datetime(" + FIELD_LOG_DATE + ", 'localtime')";

    private DatabaseHelper mDatabaseHelper;

    public CinemaLog(Context context) throws SQLException {
        mDatabaseHelper = new DatabaseHelper( context, DATABASE_NAME, null,  DATABASE_VERSION );
    }

    public void Insert( String account, String describe ) {
        SQLiteDatabase database = mDatabaseHelper.getWritableDatabase();

        ContentValues values = new ContentValues();
        values.put( FIELD_LOG_ACCOUNT, account );
        values.put( FIELD_LOG_DESCRIBE, describe );

        database.insert( TABLE_LOG, "", values );

        String query =
            "DELETE FROM " + TABLE_LOG + " " +
            "WHERE " + FIELD_LOG_DATE + " < DATETIME('NOW', '-1 DAY')";
        database.execSQL( query );
    }

    public void Delete( int id ) {
        SQLiteDatabase database = mDatabaseHelper.getWritableDatabase();
        database.delete( TABLE_LOG, FIELD_LOG_ID + "=" + "'" + id + "'", null );
    }

    public void DeleteAll() {
        SQLiteDatabase database = mDatabaseHelper.getWritableDatabase();
        database.execSQL( "DELETE FROM " + TABLE_LOG );
    }

    public Cursor GetCursorDatabases() {
        SQLiteDatabase database = mDatabaseHelper.getReadableDatabase();

        String query =
            "SELECT " +
            FIELD_LOG_ID        + "," +
            FIELD_LOG_LOCAL_DATE+ ',' +
            FIELD_LOG_ACCOUNT   + "," +
            FIELD_LOG_DESCRIBE  + " " +
            "FROM " +
            TABLE_LOG + " " +
            "ORDER BY " +
            FIELD_LOG_DATE + " DESC;";

        return database.rawQuery(query, null);
    }

    private class DatabaseHelper extends SQLiteOpenHelper {
        public DatabaseHelper(Context context, String name, SQLiteDatabase.CursorFactory factory, int version) {
            super(context, name, factory, version);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            String query =
                "CREATE TABLE "     +
                TABLE_LOG           + "(" +
                FIELD_LOG_ID        + " INTEGER PRIMARY KEY AUTOINCREMENT"  + " , " +
                FIELD_LOG_DATE      + " DATETIME DEFAULT CURRENT_TIMESTAMP" + " , " +
                FIELD_LOG_ACCOUNT   + " TEXT NOT NULL"                      + " , " +
                FIELD_LOG_DESCRIBE  + " TEXT NOT NULL"                      + " );";

            db.execSQL( query );
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            String query = "DROP TABLE IF EXISTS " + TABLE_LOG;
            db.execSQL( query );

            onCreate( db );
        }
    }
}
