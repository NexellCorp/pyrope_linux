package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.os.AsyncTask;
import android.os.SystemClock;
import android.util.Log;
import android.util.Patterns;
import android.widget.Adapter;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.Locale;
import java.util.concurrent.Semaphore;

/**
 * Created by doriya on 2/6/18.
 */

public class CinemaTask {
    private final String VD_DTAG = "CinemaTask";

    public static final int CMD_TCON_REG_WRITE          = 0;
    public static final int CMD_TCON_REG_READ           = 1;
    public static final int CMD_PFPGA_REG_WRITE         = 2;
    public static final int CMD_PFPGA_REG_READ          = 3;

    public static final int CMD_TCON_STATUS             = 10;
    public static final int CMD_TCON_LVDS               = 11;
    public static final int CMD_LED_OPEN_NUM            = 12;
    public static final int CMD_LED_OPEN_POS            = 13;
    public static final int CMD_CABINET_DOOR            = 14;
    public static final int CMD_PERIPHERAL              = 15;
    public static final int CMD_TEST_PATTERN            = 16;
    public static final int CMD_ACCUMULATION            = 17;

    public static final int CMD_WHITESEAM_ENABLE        = 20;
    public static final int CMD_WHITESEAM_READ          = 21;
    public static final int CMD_WHITESEAM_EMULATE       = 22;
    public static final int CMD_WHITESEAM_WRITE         = 23;
    public static final int CMD_LOD_RESET               = 24;

    public static final int CMD_CHECK_CABINET_NUM       = 26;
    public static final int CMD_BOOTING_COMPLETE        = 27;
    public static final int CMD_DISPLAY_VERSION         = 28;
    public static final int CMD_PIXEL_CORRECTION_ADAPTER= 29;
    public static final int CMD_PIXEL_CORRECTION        = 30;
    public static final int CMD_PIXEL_CORRECTION_EXTRACT= 31;
    public static final int CMD_UNIFORMITY              = 32;

    public static final int CMD_PFPGA_MUTE              = 40;

    public static final int CMD_APP_VERSION             = 50;
    public static final int CMD_NAP_VERSION             = 51;
    public static final int CMD_SAP_VERSION             = 52;
    public static final int CMD_IPC_SERVER_VERSION      = 53;
    public static final int CMD_IPC_CLIENT_VERSION      = 54;
    public static final int CMD_TMS_SERVER_VERSION      = 55;
    public static final int CMD_TCON_VERSION            = 56;
    public static final int CMD_PFPGA_VERSION           = 57;

    public static final int CMD_SET_NETWORK             = 60;
    public static final int CMD_GET_NETWORK             = 61;

    public static final int CMD_CHANGE_3D               = 97;
    public static final int CMD_CHANGE_SCALE            = 98;
    public static final int CMD_CHANGE_MODE             = 99;

    public static final int CMD_TMS_QUE                 = 150;  // MODE: 0 - 149, CUE: 145 - 255
    public static final int TMS_P25_4K_2D               = 150;
    public static final int TMS_P25_2K_2D               = 151;
    public static final int TMS_P25_4K_3D               = 152;
    public static final int TMS_P25_2K_3D               = 153;
    public static final int TMS_P33_4K_2D               = 154;
    public static final int TMS_P33_4K_3D               = 155;
    public static final int TMS_P33_2K_2D               = 156;
    public static final int TMS_P33_2K_3D               = 157;
    public static final int TMS_MAX                     = 157;

    private Semaphore mSem = new Semaphore(1);
    private long mStartTime;

    private CinemaTask() {
    }

    private static class Holder {
        static final CinemaTask INSTANCE = new CinemaTask();
    }

    public static synchronized CinemaTask GetInstance() {
        return Holder.INSTANCE;
    }

    // public void Wait() {
    //    Lock();
    //    Unlock();
    // }

    // public boolean IsBusy() {
    //    return mSem.availablePermits() == 0;
    // }

    public void Run( int cmd, Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_TCON_STATUS:
                new AsyncTaskTconStatus(context, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_TCON_LVDS:
                new AsyncTaskTconLvds(context, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_LED_OPEN_NUM:
                new AsyncTaskLedOpenNum(context, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_LOD_RESET:
                new AsyncTaskLodReset(context, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_PERIPHERAL:
                new AsyncTaskPeripheral(context, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_ACCUMULATION:
                new AsyncTaskAccumulation(context, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_CHECK_CABINET_NUM:
                new AsyncTaskCheckCabinetNum(context, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_BOOTING_COMPLETE:
                new AsyncTaskBootingComplete(context, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_DISPLAY_VERSION:
                new AsyncTaskDisplayVersion(context, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_APP_VERSION:
            case CMD_NAP_VERSION:
            case CMD_SAP_VERSION:
            case CMD_IPC_SERVER_VERSION:
            case CMD_IPC_CLIENT_VERSION:
            case CMD_TCON_VERSION:
            case CMD_PFPGA_VERSION:
                new AsyncTaskVersion(context, cmd, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_GET_NETWORK:
                new AsyncTaskGetNetwork(context, preExecute, postExecute, progressUpdate).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_PIXEL_CORRECTION_ADAPTER:
                new AsyncTaskPixelCorrectionAdapter(context, adapter, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_PIXEL_CORRECTION:
                new AsyncTaskPixelCorrection(context, adapter, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int value, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_LED_OPEN_POS:
                new AsyncTaskLedOpenPos( context, value, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_UNIFORMITY:
                new AsyncTaskUniformity(context, value, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_CHANGE_MODE:
                new AsyncTaskChangeMode(context, value, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int index, int module, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_PIXEL_CORRECTION_EXTRACT:
                new AsyncTaskPixelCorrectionExtract(context, index, module, preExecute, postExecute, progressUpdate).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, boolean value, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_PFPGA_MUTE:
                new AsyncTaskPfpgaMute(context, value, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_CABINET_DOOR:
                new AsyncTaskCabinetDoor(context, value, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_CHANGE_3D:
                new AsyncTaskChange3D(context, value, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_CHANGE_SCALE:
                new AsyncTaskChangeScale(context, value, preExecute, postExecute, progressUpdate).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, Adapter adapter, int func, int pattern, boolean status, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_TEST_PATTERN:
                new AsyncTaskTestPattern(context, adapter, func, pattern, status, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int[] register, int[] data, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_TCON_REG_WRITE:
                new AsyncTaskTconRegWrite(context, register, data, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_PFPGA_REG_WRITE:
                new AsyncTaskPfpgaRegWrite(context, register, data, preExecute, postExecute, progressUpdate).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int[] register, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_TCON_REG_READ:
                new AsyncTaskTconRegRead(context, register, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_PFPGA_REG_READ:
                new AsyncTaskPfpgaRegRead(context, register, preExecute, postExecute, progressUpdate).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int index, int[] value, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_WHITESEAM_EMULATE:
                new AsyncTaskWhiteSeamEmulate(context, index, value, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_WHITESEAM_WRITE:
                new AsyncTaskWhiteSeamWrite(context, index, value, preExecute, postExecute, progressUpdate).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int index, boolean emulate, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_WHITESEAM_ENABLE:
                new AsyncTaskWhiteSeamEnable(context, index, emulate, preExecute, postExecute, progressUpdate).execute();
                break;
            case CMD_WHITESEAM_READ:
                new AsyncTaskWhiteSeamRead(context, index, emulate, preExecute, postExecute, progressUpdate).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, String[] param, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
        switch( cmd )
        {
            case CMD_SET_NETWORK:
                new AsyncTaskSetNetwork(context, param, preExecute, postExecute, progressUpdate).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    private void Lock() {
        // try {
        //    mSem.acquire();
        // } catch (InterruptedException e) {
        //    e.printStackTrace();
        // }
    }

    private void Unlock() {
        // mSem.release();
    }

    //
    //  Interface for callback
    //
    interface PreExecuteCallback {
        void onPreExecute( Object[] values );
    }

    interface PostExecuteCallback {
        void onPostExecute( Object[] values );
    }

    interface ProgressUpdateCallback {
        void onProgressUpdate( Object[] values );
    }

    private Integer[] ToInteger( int[] data ) {
        Integer[] result = new Integer[data.length];
        for( int i = 0; i < data.length; i++ )
            result[i] = data[i];

        return result;
    }

    //
    //  Implementation AsyncTask
    //
    private class AsyncTaskTconRegWrite extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int[] mRegister;
        private int[] mData;

        public AsyncTaskTconRegWrite( Context context, int[] register, int[] data, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mRegister = register;
            mData = data;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 || mRegister.length == 0 || mData.length == 0 )
                return null;

            if( mRegister.length != mData.length )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            boolean bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
            boolean bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

            for( int i = 0; i < mRegister.length; i++ ) {
                byte[] reg = ctrl.IntToByteArray(mRegister[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mData[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, dat);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                if(bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if(bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

                publishProgress();
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> TCON Register Write Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute(null);

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute(null);

            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON Register Write Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskTconRegRead extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int[] mRegister;
        private int[] mData;

        public AsyncTaskTconRegRead( Context context, int[] register, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mRegister = register;
            mData = new int[mRegister.length];
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 || mRegister.length == 0)
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mRegister.length; i++ )
            {
                byte[] result;
                byte[] reg = ctrl.IntToByteArray(mRegister[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(new byte[]{mCabinet[0]}, reg);

                result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], mRegister[i] ));
                    publishProgress( CinemaInfo.RET_ERROR );
                    continue;
                }

                mData[i] = ctrl.ByteArrayToInt(result);
                publishProgress( mData[i] );
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( values );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> TCON Register Read Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mData) );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON Register Read Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPfpgaRegWrite extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private int[] mRegister;
        private int[] mData;

        public AsyncTaskPfpgaRegWrite( Context context, int[] register, int[] data, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mRegister = register;
            mData = data;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mRegister.length == 0 || mData.length == 0 )
                return null;

            if( mRegister.length != mData.length )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mRegister.length; i++ )
            {
                byte[] reg = ctrl.IntToByteArray(mRegister[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mData[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, dat);

                ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );

                publishProgress();
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> PFPGA Register Write Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute(null);

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute(null);

            Log.i(VD_DTAG, String.format(Locale.US, ">>> PFPGA Register Write Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPfpgaRegRead extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private int[] mRegister;
        private int[] mData;

        public AsyncTaskPfpgaRegRead( Context context, int[] register, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mRegister = register;
            mData = new int[mRegister.length];
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mRegister.length == 0)
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mRegister.length; i++ )
            {
                byte[] result;
                byte[] reg = ctrl.IntToByteArray(mRegister[i], NxCinemaCtrl.FORMAT_INT16);

                result = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_READ, reg );
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( reg: 0x%04X )", mRegister[i] ));
                    publishProgress( CinemaInfo.RET_ERROR );
                    continue;
                }

                mData[i] = ctrl.ByteArrayToInt(result);
                publishProgress( mData[i] );
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( values );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> PFPGA Register Read Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mData) );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> PFPGA Register Read Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPfpgaMute extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private boolean mOnoff;

        public AsyncTaskPfpgaMute( Context context, boolean onoff, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mOnoff = onoff;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {mOnoff ? (byte)0x01 : (byte)0x00} );
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> PFPGA Mute Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> PFPGA Mute Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskCheckCabinetNum extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private int mCabinetNum =0;

        public AsyncTaskCheckCabinetNum( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            for( int i = 0; i < 255; i++ ) {
                if( (i & 0x7F) < CinemaInfo.TCON_BASE_OFFSET )
                    continue;

                byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{(byte)i});
                if (result == null || result.length == 0)
                    continue;

                if( 0x01 != result[0] )
                    continue;

                mCabinetNum++;
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Check Cabinet Number Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(new int[]{mCabinetNum}) );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Check Cabinet Number Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskBootingComplete extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private int mInitMode = -1;
        private int[] mResult = new int[]{-1, };

        public AsyncTaskBootingComplete(Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mInitMode = (((CinemaInfo)mContext).GetValue(CinemaInfo.KEY_INITIAL_MODE) == null) ?
                    0 : Integer.parseInt(((CinemaInfo)mContext).GetValue(CinemaInfo.KEY_INITIAL_MODE));

            if( 0 > mInitMode ) {
                Log.i(VD_DTAG, ">>> Wrong Initialize Mode. Set default Initialize Mode. ( mode: 0 )");
                mInitMode = 0;
            }
        }

        @Override
        protected Void doInBackground(Void... voids) {
            //
            //  Wait nap_server
            //
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bFirstBoot = ((CinemaInfo)mContext).IsFirstBoot();
            if( bFirstBoot ) {
                try {
                    do {
                        Log.i(VD_DTAG, String.format(Locale.US, "elapsed time : %d sec", SystemClock.elapsedRealtime() / 1000));
                        Thread.sleep(1000);

                        if( (SystemClock.elapsedRealtime() / 1000) > ((CinemaInfo)mContext).GetBootTime() )
                            break;
                    } while(true);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                ((CinemaInfo)mContext).SetFirstBoot(false);
            }

            //
            //  Set Global Register
            //
            boolean bValidPort0, bValidPort1;
            if( bFirstBoot ) {
                //
                //  Add Temporary Cabinet for global register restore.
                //
                ((CinemaInfo)mContext).CheckScreenType();
                ((CinemaInfo)mContext).AddCabinet();

                bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
                bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

                Log.i(VD_DTAG, ">>> Write TCON Global Register.");
                int [][] treg = ((CinemaInfo)mContext).GetDefaultTReg();
                for( int i = 0; i < treg.length; i++ ) {
                    if( treg[i][1] == -1 ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "Skip. Write TCON Global Register. ( reg: 0x%04X, data: 0x%04X )", treg[i][0], treg[i][1]));
                        continue;
                    }

                    byte[] reg = ctrl.IntToByteArray(treg[i][0], NxCinemaCtrl.FORMAT_INT16);
                    byte[] dat = ctrl.IntToByteArray(treg[i][1], NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, dat);

                    if( bValidPort0 ) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData));
                    if( bValidPort1 ) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData));

                    // Log.i(VD_DTAG, String.format(Locale.US, ">>> Write TCON Global Register. ( reg: 0x%04X, data: 0x%04X )", treg[i][0], treg[i][1]));
                }

                Log.i(VD_DTAG, ">>> Write PFPGA Global Register.");
                int [][] preg = ((CinemaInfo)mContext).GetDefaultPReg();
                for( int i = 0; i < preg.length; i++ ) {
                    if( preg[i][1] == -1 ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "Skip. Write PFPGA Global Register. ( reg: 0x%04X, data: 0x%04X )", preg[i][0], preg[i][1]));
                        continue;
                    }

                    byte[] reg = ctrl.IntToByteArray(preg[i][0], NxCinemaCtrl.FORMAT_INT16);
                    byte[] dat = ctrl.IntToByteArray(preg[i][1], NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, dat);

                    ctrl.Send(NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData);

                    // Log.i(VD_DTAG, String.format(Locale.US, ">>> Write PFPGA Global Register. ( reg: 0x%04X, data: 0x%04X )", preg[i][0], preg[i][1]));
                }
            }

            //
            //  Detection Cabinet
            //
            {
                //
                //  Add Real Cabinet.
                //
                ((CinemaInfo)mContext).AddCabinet();
                ((CinemaInfo)mContext).ShowCabinet();

                bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
                bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);
            }

            //
            //  Restore System at first time.
            //
            if( bFirstBoot ) {
                //
                //  Display Version
                //
                Date date = new Date( BuildConfig.BUILD_DATE + 3600 * 9 * 1000 );

                String appVersion = new SimpleDateFormat("HH:mm:ss, MMM dd yyyy ", Locale.US).format(date);
                byte[] napVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_NAP_VERSION, null );
                byte[] sapVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_SAP_VERSION, null );
                byte[] isvVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_SERVER_VERSION, null );
                byte[] icnVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_CLIENT_VERSION, null );
                byte[] tsvVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_TMS_SERVER_VERSION, null );

                Log.i(VD_DTAG, ">> Version Information");
                Log.i(VD_DTAG, String.format(Locale.US, "-. Application : %s", appVersion));
                Log.i(VD_DTAG, String.format(Locale.US, "-. N.AP        : %s", (napVersion != null && napVersion.length != 0) ? new String(napVersion).trim() : "Unknown"));
                Log.i(VD_DTAG, String.format(Locale.US, "-. S.AP        : %s", (sapVersion != null && sapVersion.length != 0) ? new String(sapVersion).trim() : "Unknown"));
                Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Server  : %s", (isvVersion != null && isvVersion.length != 0) ? new String(isvVersion).trim() : "Unknown"));
                Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Client  : %s", (icnVersion != null && icnVersion.length != 0) ? new String(icnVersion).trim() : "Unknown"));
                Log.i(VD_DTAG, String.format(Locale.US, "-. TMS Server  : %s", (tsvVersion != null && tsvVersion.length != 0) ? new String(tsvVersion).trim() : "Unknown"));

                for( byte id : ((CinemaInfo)mContext).GetCabinet() ) {
                    byte[] tconVersion = ctrl.Send(NxCinemaCtrl.CMD_TCON_VERSION, new byte[] {id});

                    int msbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_MSB) : 0;
                    int lsbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_LSB) : 0;

                    Log.i(VD_DTAG, String.format(Locale.US, "-. TCON #%02d    : %05d - %05d", ((CinemaInfo)mContext).GetCabinetNumber(id), msbVersion, lsbVersion));
                }

                {
                    byte[] pfpgaVersion = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_VERSION, null );

                    int msbVersion = (pfpgaVersion != null && pfpgaVersion.length != 0) ? ctrl.ByteArrayToInt32(pfpgaVersion, NxCinemaCtrl.MASK_INT32_MSB) : 0;
                    int lsbVersion = (pfpgaVersion != null && pfpgaVersion.length != 0) ? ctrl.ByteArrayToInt32(pfpgaVersion, NxCinemaCtrl.MASK_INT32_LSB) : 0;

                    Log.i(VD_DTAG, String.format(Locale.US, "-. PFPGA       : %05d - %05d", msbVersion, lsbVersion));
                }

                if( !((CinemaInfo)mContext).IsFirstBootAccessEEPRom() ) {
                    byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_EEPROM_READ, null);
                    if( result == null || result.length == 0 ) {
                        Log.i(VD_DTAG, "Unknown Error.");
                        return null;
                    }

                    if( result[0] == (byte)0xFF ) Log.i(VD_DTAG, ">> Fail, EEPROM Read.");
                    if( result[0] == (byte)0x01 ) Log.i(VD_DTAG, ">> Update is successful.");
                    if( result[0] == (byte)0x00 ) Log.i(VD_DTAG, ">> Update is not needed.");

                    if( result[0] != (byte)0xFF ) ((CinemaInfo)mContext).SetValidEEPRom( result[0] != (byte)0xFF );
                }

                //
                //  EEPRom Access
                //
                if( ((CinemaInfo)mContext).IsFirstBootAccessEEPRom() ) {
                    byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_EEPROM_READ, null);
                    if( result == null || result.length == 0 ) {
                        Log.i(VD_DTAG, "Unknown Error.");
                        return null;
                    }

                    if( result[0] == (byte)0xFF ) Log.i(VD_DTAG, ">> Fail, EEPROM Read.");
                    if( result[0] == (byte)0x01 ) Log.i(VD_DTAG, ">> Update is successful.");
                    if( result[0] == (byte)0x00 ) Log.i(VD_DTAG, ">> Update is not needed.");

                    if( result[0] != (byte)0xFF ) ((CinemaInfo)mContext).SetValidEEPRom( result[0] != (byte)0xFF );
                }

                //
                //  Prepare T_REG.txt parsing
                //
                String[] resultPath;
                ConfigTconInfo tconEEPRomInfo = new ConfigTconInfo();
                ConfigTconInfo tconUsbInfo = new ConfigTconInfo();

                resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
                for( String file : resultPath ) {
                    tconEEPRomInfo.Parse(file);
                }

                resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
                for( String file : resultPath ) {
                    tconUsbInfo.Parse(file);
                }

                if( ((10 > mInitMode) && (mInitMode >= tconEEPRomInfo.GetModeNum())) ||
                    (10 <= mInitMode) && ((mInitMode-10) >= tconUsbInfo.GetModeNum())) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail, Change Mode. ( mode: %d, eeprom: %d, usb: %d )",
                            mInitMode, tconEEPRomInfo.GetModeNum(), tconUsbInfo.GetModeNum() ));
                    return null;
                }

                if( 10 > mInitMode &&
                    !((CinemaInfo)mContext).IsMode3D() &&
                    tconEEPRomInfo.GetDataMode(mInitMode) == ConfigTconInfo.MODE_3D ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Skip. Change Mode. ( mode: %d, is3D: %b, dataMode: %d )",
                            mInitMode, ((CinemaInfo)mContext).IsMode3D(), tconEEPRomInfo.GetDataMode(mInitMode)));
                    return null;
                }

                if( 10 <= mInitMode &&
                    !((CinemaInfo)mContext).IsMode3D() &&
                    tconUsbInfo.GetDataMode(mInitMode-10) == ConfigTconInfo.MODE_3D ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Skip. Change Mode. ( mode: %d, is3D: %b, dataMode: %d )",
                            mInitMode, ((CinemaInfo)mContext).IsMode3D(), tconUsbInfo.GetDataMode(mInitMode)));
                    return null;
                }

                //
                //  Check TCON Booting Status
                //
                if( ((CinemaInfo)mContext).IsCheckTconBooting() ) {
                    boolean bTconBooting = true;
                    for( byte id : ((CinemaInfo)mContext).GetCabinet() ) {
                        byte[] result;
                        result = ctrl.Send(NxCinemaCtrl.CMD_TCON_BOOTING_STATUS, new byte[]{id});
                        if (result == null || result.length == 0) {
                            Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet: %d, port: %d, slave: 0x%02x )",
                                    ((CinemaInfo)mContext).GetCabinetNumber(id),
                                    ((CinemaInfo)mContext).GetCabinetPort(id),
                                    ((CinemaInfo)mContext).GetCabinetSlave(id)
                            ));
                            continue;
                        }

                        if( result[0] == 0 ) {
                            Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet: %d, port: %d, slave: 0x%02x, result: %d )",
                                    ((CinemaInfo)mContext).GetCabinetNumber(id),
                                    ((CinemaInfo)mContext).GetCabinetPort(id),
                                    ((CinemaInfo)mContext).GetCabinetSlave(id),
                                    result[0]
                            ));
                            bTconBooting = false;
                        }
                    }

                    if( !bTconBooting ) {
                        Log.i(VD_DTAG, "Fail, TCON booting.");
                        return null;
                    }
                }

                //
                //  PFPGA Mute on
                //
                ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );

                //
                //  TCON Mute on
                //
                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_MUTE, new byte[] {(byte)0x09, (byte)0x01} );
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_MUTE, new byte[] {(byte)0x89, (byte)0x01} );

                //
                //  Parse P_REG.txt
                //
                int enableUniformity = 0;
                int[] enableGamma = {0, 0, 0, 0};

                resultPath = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
                for( String file : resultPath ) {
                    ConfigPfpgaInfo info = new ConfigPfpgaInfo();
                    if( info.Parse( file ) ) {
                        enableUniformity = info.GetEnableUpdateUniformity(mInitMode);
                        for( int i = 0; i < info.GetRegister(mInitMode).length; i++ ) {
                            byte[] reg = ctrl.IntToByteArray(info.GetRegister(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                            byte[] data = ctrl.IntToByteArray(info.GetData(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                            byte[] inData = ctrl.AppendByteArray(reg, data);

                            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
                        }
                    }
                }

                //
                //  Auto Uniformity Correction Writing
                //
                resultPath = FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
                for( String file : resultPath ) {
                    LedUniformityInfo info = new LedUniformityInfo();
                    if( info.Parse( file ) ) {
                        if( enableUniformity == 0 ) {
                            Log.i(VD_DTAG, String.format( "Skip. Update Uniformity. ( %s )", file ));
                            continue;
                        }

                        byte[] inData = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );
                        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_UNIFORMITY_DATA, inData );
                    }
                }

                //
                //  Parse T_REG.txt
                //
                if( (10 > mInitMode) && (mInitMode < tconEEPRomInfo.GetModeNum())) {
                    enableGamma = tconEEPRomInfo.GetEnableUpdateGamma(mInitMode);
                    for( int i = 0; i < tconEEPRomInfo.GetRegister(mInitMode).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(tconEEPRomInfo.GetRegister(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(tconEEPRomInfo.GetData(mInitMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    }
                }

                if( (10 <= mInitMode) && ((mInitMode-10) < tconUsbInfo.GetModeNum())) {
                    enableGamma = tconUsbInfo.GetEnableUpdateGamma(mInitMode-10);
                    for( int i = 0; i < tconUsbInfo.GetRegister(mInitMode-10).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(tconUsbInfo.GetRegister(mInitMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(tconUsbInfo.GetData(mInitMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                        if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                        if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                    }
                }

                //
                //  Write Gamma
                //
                if( (10 > mInitMode) && (mInitMode < tconEEPRomInfo.GetModeNum())) {
                    resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_EEPROM, LedGammaInfo.PATTERN_NAME);
                    for( String file : resultPath ) {
                        LedGammaInfo info = new LedGammaInfo();
                        if( info.Parse( file ) ) {
                            if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 1) ||
                                (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 1) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 1) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 1) ) {
                                Log.i(VD_DTAG, String.format( "Skip. Update EEPROM Gamma. ( %s )", file ));
                                continue;
                            }

                            int cmd;
                            if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                                cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                            else
                                cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                            byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                            byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                            byte[] inData = ctrl.AppendByteArray(table, data);

                            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                            if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                            if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                        }
                    }
                }

                if( (10 <= mInitMode) ) {
                    resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_EEPROM, LedGammaInfo.PATTERN_NAME);
                    for( String file : resultPath ) {
                        LedGammaInfo info = new LedGammaInfo();
                        if( info.Parse( file ) ) {
                            if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 1) ||
                                (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 1) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 1) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 1) ) {
                                Log.i(VD_DTAG, String.format( "Skip. Update EEPROM Gamma. ( %s )", file ));
                                continue;
                            }

                            int cmd;
                            if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                                cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                            else
                                cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                            byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                            byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                            byte[] inData = ctrl.AppendByteArray(table, data);

                            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                            if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                            if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                        }
                    }

                    resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_USB, LedGammaInfo.PATTERN_NAME);
                    for( String file : resultPath ) {
                        LedGammaInfo info = new LedGammaInfo();
                        if( info.Parse( file ) ) {
                            if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 2) ||
                                (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 2) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 2) ||
                                (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 2) ) {
                                Log.i(VD_DTAG, String.format( "Skip. Update USB Gamma. ( %s )", file ));
                                continue;
                            }

                            int cmd;
                            if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                                cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                            else
                                cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                            byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                            byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                            byte[] inData = ctrl.AppendByteArray(table, data);

                            byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                            byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                            if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                            if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                        }
                    }
                }

                //
                //  Update Gamma Status
                //
                ((CinemaInfo)mContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM0, String.valueOf(enableGamma[0]) );
                ((CinemaInfo)mContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM1, String.valueOf(enableGamma[1]) );
                ((CinemaInfo)mContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM0, String.valueOf(enableGamma[2]) );
                ((CinemaInfo)mContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM1, String.valueOf(enableGamma[3]) );

                //
                //  SW Reset
                //
                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x09});
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x89});

                //
                //  TCON Mute off
                //
                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_MUTE, new byte[] {(byte)0x09, (byte)0x00} );
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_MUTE, new byte[] {(byte)0x89, (byte)0x00} );

                //
                //  PFPGA Mute off
                //
                ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

                //
                //  TCON Initialize
                //
                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_INIT, new byte[]{(byte)0x09});
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_INIT, new byte[]{(byte)0x89});
            }

            mResult[0] = mInitMode;
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate(null);

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Booting Complete Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute(null);

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( 0 > mResult[0] ) {
                Log.i(VD_DTAG, String.format( Locale.US, ">>> Booting Complete Fail. ( expected: %d )", mInitMode));
            }

            ((CinemaInfo)mContext).SetValue(CinemaInfo.KEY_INITIAL_MODE, String.valueOf(mResult[0]));

            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mResult) );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Booting Complete Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskDisplayVersion extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;

        public AsyncTaskDisplayVersion( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            Date date = new Date( BuildConfig.BUILD_DATE + 3600 * 9 * 1000 );

            String appVersion = new SimpleDateFormat("HH:mm:ss, MMM dd yyyy ", Locale.US).format(date);
            byte[] napVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_NAP_VERSION, null );
            byte[] sapVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_SAP_VERSION, null );
            byte[] isvVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_SERVER_VERSION, null );
            byte[] icnVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_CLIENT_VERSION, null );
            byte[] tsvVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_TMS_SERVER_VERSION, null );

            Log.i(VD_DTAG, ">> Version Information");
            Log.i(VD_DTAG, String.format(Locale.US, "-. Application : %s", appVersion));
            Log.i(VD_DTAG, String.format(Locale.US, "-. N.AP        : %s", (napVersion != null && napVersion.length != 0) ? new String(napVersion).trim() : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. S.AP        : %s", (sapVersion != null && sapVersion.length != 0) ? new String(sapVersion).trim() : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Server  : %s", (isvVersion != null && isvVersion.length != 0) ? new String(isvVersion).trim() : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Client  : %s", (icnVersion != null && icnVersion.length != 0) ? new String(icnVersion).trim() : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. TMS Server  : %s", (tsvVersion != null && tsvVersion.length != 0) ? new String(tsvVersion).trim() : "Unknown"));

            for( byte id : mCabinet ) {
                byte[] tconVersion = ctrl.Send(NxCinemaCtrl.CMD_TCON_VERSION, new byte[] {id});

                int msbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_MSB) : 0;
                int lsbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_LSB) : 0;

                Log.i(VD_DTAG, String.format(Locale.US, "-. TCON #%02d    : %05d - %05d", ((CinemaInfo)mContext).GetCabinetNumber(id), msbVersion, lsbVersion));
            }

            {
                byte[] pfpgaVersion = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_VERSION, null );

                int msbVersion = (pfpgaVersion != null && pfpgaVersion.length != 0) ? ctrl.ByteArrayToInt32(pfpgaVersion, NxCinemaCtrl.MASK_INT32_MSB) : 0;
                int lsbVersion = (pfpgaVersion != null && pfpgaVersion.length != 0) ? ctrl.ByteArrayToInt32(pfpgaVersion, NxCinemaCtrl.MASK_INT32_LSB) : 0;

                Log.i(VD_DTAG, String.format(Locale.US, "-. PFPGA       : %05d - %05d", msbVersion, lsbVersion));
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate(null);

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Display Version Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute(null);

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute(null);

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Display Version Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskTconStatus extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int[] mResult;

        public AsyncTaskTconStatus( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mResult = new int[mCabinet.length];
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mCabinet.length; i++ ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{mCabinet[i]} );
                if( result == null || result.length == 0 ) {
                    mResult[i] = CinemaInfo.RET_ERROR;
                    publishProgress( i, mResult[i] );
                    continue;
                }

                mResult[i] = result[0];
                publishProgress( i, mResult[i] );
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            switch( values[1] ) {
                case CinemaInfo.RET_PASS:
                    break;
                default:
                    Log.i(VD_DTAG, String.format(Locale.US, "%s. ( cabinet: %d, port: %d, slave: 0x%02x, result: %d )",
                            (values[1] == CinemaInfo.RET_FAIL) ? "Fail" : "Unknown Error",
                            ((CinemaInfo)mContext).GetCabinetNumber(mCabinet[values[0]]),
                            ((CinemaInfo)mContext).GetCabinetPort(mCabinet[values[0]]),
                            ((CinemaInfo)mContext).GetCabinetSlave(mCabinet[values[0]]),
                            values[1]
                    ));
                    break;
            }

            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate(values);

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> TCON Status Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mResult) );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON Status Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskTconLvds extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int[] mResult;

        public AsyncTaskTconLvds( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mResult = new int[mCabinet.length];
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mCabinet.length; i++ ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_LVDS_STATUS, new byte[]{mCabinet[i]} );
                if( result == null || result.length == 0 ) {
                    mResult[i] = CinemaInfo.RET_ERROR;
                    publishProgress( i, mResult[i] );
                    continue;
                }

                mResult[i] = result[0];
                publishProgress( i, mResult[i] );
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            switch( values[1] ) {
                case CinemaInfo.RET_PASS:
                    break;
                default:
                    Log.i(VD_DTAG, String.format(Locale.US, "%s. ( cabinet: %d, port: %d, slave: 0x%02x, result: %d )",
                            (values[1] == CinemaInfo.RET_FAIL) ? "Fail" : "Unknown Error",
                            ((CinemaInfo)mContext).GetCabinetNumber(mCabinet[values[0]]),
                            ((CinemaInfo)mContext).GetCabinetPort(mCabinet[values[0]]),
                            ((CinemaInfo)mContext).GetCabinetSlave(mCabinet[values[0]]),
                            values[1]
                    ));
                    break;
            }

            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( values );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> TCON LVDS Check Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mResult) );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON LVDS Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLedOpenNum extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int[] mResult;

        public AsyncTaskLedOpenNum( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mResult = new int[mCabinet.length];
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            boolean bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
            boolean bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

            if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{(byte)0x09} );
            if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{(byte)0x89} );

            for( int i = 0; i < mCabinet.length; i++ ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_OPEN_NUM, new byte[]{mCabinet[i]} );
                if (result == null || result.length == 0) {
                    mResult[i] = CinemaInfo.RET_ERROR;
                    publishProgress( i, mResult[i] );
                    continue;
                }

                mResult[i] = ctrl.ByteArrayToInt16( result, NxCinemaCtrl.FORMAT_INT16 );
                publishProgress( i, mResult[i] );
            }

            if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{(byte)0x09} );
            if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{(byte)0x89} );

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            switch( values[1] ) {
                case 0:
                    break;
                default:
                    Log.i(VD_DTAG, String.format(Locale.US, "%s. ( cabinet: %d, port: %d, slave: 0x%02x, result: %d )",
                            (0 > values[1]) ? "Unknown Error" : "Led Open Detect",
                            ((CinemaInfo)mContext).GetCabinetNumber(mCabinet[values[0]]),
                            ((CinemaInfo)mContext).GetCabinetPort(mCabinet[values[0]]),
                            ((CinemaInfo)mContext).GetCabinetSlave(mCabinet[values[0]]),
                            values[1]
                    ));
                    break;
            }

            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( values );   // values[0]: index, values[1]: result

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> LED Open Check Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mResult) );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> LED Open Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLedOpenPos extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte mSlave;

        public AsyncTaskLedOpenPos( Context context, int slave, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mSlave = (byte)(slave & 0xFF);
        }

        @Override
        protected Void doInBackground(Void... voids) {
            byte[] result;
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            result = ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{mSlave} );
            if( result == null || result.length == 0 )
                return null;

            result = ctrl.Send( NxCinemaCtrl.CMD_TCON_OPEN_NUM, new byte[]{mSlave});
            if( result == null || result.length == 0 )
                return null;

            int numOfOpen = ctrl.ByteArrayToInt16(result, NxCinemaCtrl.FORMAT_INT16);

            Log.i(VD_DTAG, String.format( Locale.US, "0x%02X, numOfOpen = %d", mSlave, numOfOpen));

            for( int i = 0; i < numOfOpen; i++ ) {
                if(isCancelled())
                    break;

                result = ctrl.Send( NxCinemaCtrl.CMD_TCON_OPEN_POS, new byte[]{mSlave} );
                if( result == null || result.length == 0)
                    break;

                int posX = ctrl.ByteArrayToInt16( result, NxCinemaCtrl.MASK_INT16_MSB);
                int posY = ctrl.ByteArrayToInt16( result, NxCinemaCtrl.MASK_INT16_LSB);
                publishProgress( i, posX, posY );
            }

            ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{mSlave} );
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( values );   // values[0]: number, values[1]: X position, values[2]: Y position

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> LED Open Position Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> LED Open Position Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskCabinetDoor extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int[] mResult;
        private boolean mRead;

        public AsyncTaskCabinetDoor( Context context, boolean read, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mResult = new int[mCabinet.length];
            mRead = read;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            if( mRead ) {
                for( int i = 0; i < mCabinet.length; i++ ) {
                    byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOOR_STATUS, new byte[]{mCabinet[i], (byte)1} );
                    if (result == null || result.length == 0) {
                        mResult[i] = CinemaInfo.RET_ERROR;
                        publishProgress( i, mResult[i] );
                        continue;
                    }

                    if( 1 == result[0] || 2 == result[0] )  mResult[i] = CinemaInfo.RET_PASS;
                    else if( 0 == result[0] )               mResult[i] = CinemaInfo.RET_FAIL;
                    else                                    mResult[i] = CinemaInfo.RET_ERROR;

                    publishProgress( i, mResult[i] );
                }
            }
            else {
                boolean bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
                boolean bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_DOOR_STATUS, new byte[]{(byte)0x09, (byte)0} );
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_DOOR_STATUS, new byte[]{(byte)0x89, (byte)0} );
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            switch( values[1] ) {
                case CinemaInfo.RET_PASS:
                    break;
                default:
                    Log.i(VD_DTAG, String.format(Locale.US, "%s. ( cabinet: %d, port: %d, slave: 0x%02x, result: %d )",
                            (values[1] == CinemaInfo.RET_FAIL) ? "Fail" : "Unknown Error",
                            ((CinemaInfo)mContext).GetCabinetNumber(mCabinet[values[0]]),
                            ((CinemaInfo)mContext).GetCabinetPort(mCabinet[values[0]]),
                            ((CinemaInfo)mContext).GetCabinetSlave(mCabinet[values[0]]),
                            values[1]
                    ));
                    break;
            }

            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( values );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Cabinet Door Check Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mResult) );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Cabinet Door Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPeripheral extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private int[] mResult = new int[3];

        public AsyncTaskPeripheral( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mResult[0] = CinemaInfo.RET_ERROR;
            mResult[1] = CinemaInfo.RET_ERROR;
            mResult[2] = CinemaInfo.RET_ERROR;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            //
            //  Security AP
            //
            {
                mResult[0] = CinemaInfo.RET_FAIL;

                ((CinemaInfo)mContext).SetSecureAlive("false");
                for( int i = 0; i < 16; i++ ) {
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    if( ((CinemaInfo)mContext).GetSecureAlive().equals("true") ) {
                        mResult[0] = CinemaInfo.RET_PASS;
                        break;
                    }
                }

                publishProgress( 0, mResult[0] );
            }

            //
            //  P.FPGA
            //
            {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
                byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_STATUS, null);

                if( result != null && result.length != 0 ) {
                    mResult[1] = result[0];
                }

                publishProgress( 1, mResult[1] );
            }

            //
            //  IMB
            //
            {
                NetworkTools tools = new NetworkTools();
                String strImbAddress = "";
                try {
                    FileReader inFile = new FileReader("/system/bin/nap_network");
                    BufferedReader inReader = new BufferedReader(inFile);
                    try {
                        inReader.readLine();    // param: ip address ( mandatory )
                        inReader.readLine();    // param: netmask    ( mandatory )
                        inReader.readLine();    // param: gateway    ( mandatory )
                        inReader.readLine();    // param: network    ( mandatory but auto generate )
                        inReader.readLine();    // param: dns1       ( optional )
                        inReader.readLine();    // param: dns2       ( optional )
                        strImbAddress = inReader.readLine();// param: imb address( optional )

                        inReader.close();
                        inFile.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }

                if( strImbAddress != null && !strImbAddress.equals("") ) {
                    try {
                        if( tools.GetEthLink().equals("true") && tools.Ping(strImbAddress) ) {
                            mResult[2] = CinemaInfo.RET_PASS;
                        }
                        else {
                            mResult[2] = CinemaInfo.RET_FAIL;
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }

                publishProgress( 2, mResult[2] );
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            Log.i(VD_DTAG, String.format(Locale.US, "%s : %s. ( result: %d )",
                (values[0] == 0) ? "SecureAP" : ((values[0] == 1) ? "P.FPGA" : "IMB Network"),
                (values[1] == CinemaInfo.RET_PASS) ? "Pass" : ((values[1] == CinemaInfo.RET_FAIL) ? "Fail" : "Unknown Error"),
                values[1]
            ));

            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( values );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Peripheral Check Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Peripheral Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskVersion extends AsyncTask<Void, Object, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private int mType;

        private byte[] mCabinet;
        private String[] mResult;

        public AsyncTaskVersion( Context context, int type, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mType = type;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mResult = new String[type == CMD_TCON_VERSION ? mCabinet.length : 1];
        }

        @Override
        protected Void doInBackground(Void... voids) {
            int cmd = (mType == CMD_NAP_VERSION)        ? NxCinemaCtrl.CMD_PLATFORM_NAP_VERSION :
                      (mType == CMD_SAP_VERSION)        ? NxCinemaCtrl.CMD_PLATFORM_SAP_VERSION :
                      (mType == CMD_IPC_SERVER_VERSION) ? NxCinemaCtrl.CMD_PLATFORM_IPC_SERVER_VERSION :
                      (mType == CMD_IPC_CLIENT_VERSION) ? NxCinemaCtrl.CMD_PLATFORM_IPC_CLIENT_VERSION :
                      (mType == CMD_TMS_SERVER_VERSION) ? NxCinemaCtrl.CMD_PLATFORM_TMS_SERVER_VERSION :
                      (mType == CMD_TCON_VERSION)       ? NxCinemaCtrl.CMD_TCON_VERSION :
                      (mType == CMD_PFPGA_VERSION)      ? NxCinemaCtrl.CMD_PFPGA_VERSION : -1;

            if( 0 > cmd )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            if( mType == CMD_APP_VERSION ) {
                mResult[0] = new SimpleDateFormat("HH:mm:ss, MMM dd yyyy ", Locale.US).format(new Date( BuildConfig.BUILD_DATE + 3600 * 9 * 1000 ));
            }
            else if( mType == CMD_TCON_VERSION ) {
                for( int i = 0; i < mCabinet.length; i++ ) {
                    byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_VERSION, new byte[]{mCabinet[i]} );
                    mResult[i] = (result != null && result.length != 0) ? String.format(Locale.US, "%05d-%05d",
                            ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_MSB), ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_LSB)) :
                            "Unknown";

                    publishProgress( i, mResult[i] );
                }
            }
            else if( mType == CMD_PFPGA_VERSION ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_VERSION, null );
                mResult[0] = (result != null && result.length != 0) ? String.format(Locale.US, "%05d-%05d",
                        ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_MSB), ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_LSB)) :
                        "Unknown";
            }
            else {
                byte[] result = ctrl.Send( cmd, null );
                mResult[0] = (result != null && result.length != 0) ? new String(result).trim() : "Unknown";
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Object... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( values );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Version Check Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( mResult );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Version Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskTestPattern extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;

        private byte[] mCabinet;
        private SelectRunAdapter mAdapter;
        private int mFunc = 0;
        private int mPattern = 0;
        private boolean mStatus = false;

        private int[] mPatternReg = {
                CinemaInfo.REG_TCON_FLASH_CC,
                CinemaInfo.REG_TCON_CC_MODULE,
                CinemaInfo.REG_TCON_XYZ_TO_RGB,
                CinemaInfo.REG_TCON_SEAM_ON,
        };

        private int[] mPatternDat = {
                0x0000,
                0x0000,
                0x0000,
                0x0000,
        };

        public AsyncTaskTestPattern( Context context, Adapter adapter, int func, int pattern, boolean status, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof SelectRunAdapter )
                mAdapter = (SelectRunAdapter)adapter;

            mFunc = func;
            mPattern = pattern;
            mStatus = status;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            boolean bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
            boolean bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

            if( mFunc < 7 ) {
                byte[] data = { (byte)mFunc, (byte)mPattern };

                byte[] data0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, data);
                byte[] data1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, data);

                Log.i(VD_DTAG, String.format( Locale.US, ">>>>> mFunc = %d, mPattern = %d, mStatus = %b", mFunc, mPattern, mStatus));

                if( bValidPort0 ) ctrl.Send( mStatus ? NxCinemaCtrl.CMD_TCON_PATTERN_RUN : NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data0 );
                if( bValidPort1 ) ctrl.Send( mStatus ? NxCinemaCtrl.CMD_TCON_PATTERN_RUN : NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data1 );
            }
            else {
                byte[] reg = ctrl.IntToByteArray(mPatternReg[mFunc-7], NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mStatus ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
                byte[] data = ctrl.AppendByteArray(reg, dat);
                byte[] data0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, data);
                byte[] data1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, data);

                Log.i(VD_DTAG, String.format( Locale.US, ">>>>> mFunc = %d, mPattern = %d, mStatus = %b", mFunc, mPattern, mStatus));

                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data0 );
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, data1 );
            }

            //
            //  Update Enable/Disable Button
            //
            for( int i = 0; i < mPatternReg.length; i++ ) {
                byte[] result, inData;
                byte slave = mCabinet[0];
                mPatternDat[i] = 0;

                inData = new byte[] { slave };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mPatternReg[i], NxCinemaCtrl.FORMAT_INT16));

                result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", slave, mPatternReg[i] ));
                    continue;
                }

                mPatternDat[i] = ctrl.ByteArrayToInt(result);
            }

            for( int i = 0; i < mPatternReg.length; i++ ) {
                Log.i(VD_DTAG, String.format(">>> read pattern register. ( reg: 0x%04X, dat: 0x%04X )", mPatternReg[i], mPatternDat[i]) );
            }

            publishProgress(mPatternDat[0], mPatternDat[1], mPatternDat[2], mPatternDat[3]);
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            for( int i = 0; i < mPatternReg.length; i++ ) {
                SelectRunInfo info = mAdapter.getItem(i+7);
                if( info == null )
                    continue;

                info.SetStatus(values[i] == 0x0001);
            }
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Test Pattern Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Test Pattern Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLodReset extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;

        public AsyncTaskLodReset( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 ) {
                return null;
            }

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            boolean bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
            boolean bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

            byte[] reg, dat;
            byte[] inData, inData0, inData1;

            {
                reg = ctrl.IntToByteArray(0x011F, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
                inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);
                if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

                try {
                    Thread.sleep(75);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            {
                reg = ctrl.IntToByteArray(0x011F, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(0x0001, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
                inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);
                if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

                try {
                    Thread.sleep(75);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            {
                reg = ctrl.IntToByteArray(0x011F, NxCinemaCtrl.FORMAT_INT16);
                dat = ctrl.IntToByteArray(0x0000, NxCinemaCtrl.FORMAT_INT16);
                inData = ctrl.AppendByteArray(reg, dat);
                inData0 = ctrl.AppendByteArray(new byte[]{(byte) 0x09}, inData);
                inData1 = ctrl.AppendByteArray(new byte[]{(byte) 0x89}, inData);
                if (bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if (bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);

                try {
                    Thread.sleep(75);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Lod Reset Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Lod Reset Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskAccumulation extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int mResult;

        public AsyncTaskAccumulation( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mCabinet.length; i++ ) {
                for( int j = 0; j < CinemaInfo.TCON_MODULE_NUM; j++ ) {
                    byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_ACCUMULATE_TIME, new byte[] { mCabinet[i], (byte)j } );
                    if( result == null || result.length == 0 ) {
                        publishProgress( i, j, CinemaInfo.RET_ERROR );
                        continue;
                    }

                    mResult = CinemaInfo.RET_PASS;
                    publishProgress( i, j, mResult );
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            switch( values[2] ) {
                case CinemaInfo.RET_ERROR:
                    Log.i(VD_DTAG, String.format(Locale.US, ">>> Cabinet %02d-%02d : Error",
                            ((CinemaInfo)mContext).GetCabinetNumber(mCabinet[values[0]]),
                            values[1]
                    ));
                    break;
                default:
                    Log.i(VD_DTAG, String.format(Locale.US, ">>> Cabinet %02d-%02d : %d mSec",
                            ((CinemaInfo)mContext).GetCabinetNumber(mCabinet[values[0]]),
                            values[1],
                            values[2]
                    ));
                    break;
            }

            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate(values);

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Accumulation Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Accumulation Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskWhiteSeamEnable extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int mIndexPos;
        private boolean mEmulate;

        public AsyncTaskWhiteSeamEnable( Context context, int indexPos, boolean emulate, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mIndexPos = indexPos;   // Spinner Position. ( 0: all, 1: mCabinet[0], 2: mCabinet[1], ... )
            mEmulate = emulate;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            if( mIndexPos == 0 ) {
                boolean bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
                boolean bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

                byte[] reg = ctrl.IntToByteArray(0x0189, NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mEmulate ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, dat);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
            }
            else {
                int pos = mIndexPos - 1;
                byte[] reg = ctrl.IntToByteArray(0x0189, NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mEmulate ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, dat);

                inData = ctrl.AppendByteArray(new byte[]{(byte)mCabinet[pos]}, inData);

                ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, String.format(Locale.US, "WhiteSeam Enable Start. ( %b )", mEmulate));
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Enable Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskWhiteSeamRead extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int mIndexPos;
        private boolean mEmulate;

        private String[] mSeamStr = new String[] { "top", "bottom", "left", "right" };
        private int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        private int[] mSeamVal = new int[]{ -1, -1, -1, -1 };

        public AsyncTaskWhiteSeamRead( Context context, int indexPos, boolean emulate, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mIndexPos = indexPos;   // Spinner Position. ( 0: all, 1: mCabinet[0], 2: mCabinet[1], ... )
            mEmulate = emulate;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            //
            //  Emulate is
            //      true    : Emulate Register --> UI
            //      false   : Flash Register --> Emulate Register --> UI
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            int index = (mIndexPos != 0) ? mIndexPos - 1 : 0;
            byte slave = mCabinet[index];

            //
            //  1. Read White Seam Value in Flash Memory.
            //
            if( !mEmulate ) {
                Log.i(VD_DTAG, ">>> White Seam Read in Flash Memory.");
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_WHITE_SEAM_READ, new byte[] {slave} );
                if( result == null || result.length == 0 || result[0] != 0x01 ) {
                    Log.i(VD_DTAG, "Fail, WhiteSeam Read.");
                    return null;
                }
            }

            //
            //  2. Read White Seam Value in Emulate Register.
            //
            Log.i(VD_DTAG, ">>> White Seam Read in Emulate Register.");
            for( int i = 0; i < mSeamReg.length; i++ ) {
                byte[] result, inData;
                inData = new byte[] { slave };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamReg[i], NxCinemaCtrl.FORMAT_INT16));

                result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", slave, mSeamReg[i] ));
                    mSeamVal[i] = CinemaInfo.RET_ERROR;
                    publishProgress( i, mSeamVal[i] );
                    continue;
                }

                mSeamVal[i] = ctrl.ByteArrayToInt(result);
                publishProgress( i, mSeamVal[i] );

                Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Read Done. ( pos: %d, slave: 0x%02X, emulate: %b, %s: %d )",
                        index, slave, mEmulate, mSeamStr[i], mSeamVal[i]
                ));
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( values );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> WhiteSeam Read Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mSeamVal) );   // Top, Bottom, Left, Right

            Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Read Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskWhiteSeamEmulate extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int mIndexPos;

        private String[] mSeamStr = new String[] { "top", "bottom", "left", "right" };
        private int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        private int[] mSeamVal;

        public AsyncTaskWhiteSeamEmulate( Context context, int indexPos, int[] value, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mIndexPos = indexPos;   // Spinner Position. ( 0: all, 1: mCabinet[0], 2: mCabinet[1], ... )
            mSeamVal = value;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            if( mSeamReg.length != mSeamVal.length )
                return null;

            Log.i(VD_DTAG, String.format(Locale.US, "pos: %d", mIndexPos));
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            if( mIndexPos == 0 ) {
                Log.i(VD_DTAG, "WhiteSeam Emulate. ( index: all )");

                boolean bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
                boolean bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

                for( int j = 0; j < mSeamReg.length; j++ ) {
                    byte[] reg = ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16);
                    byte[] val = ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16);

                    byte[] inData1 = new byte[] {(byte)0x09};
                    inData1 = ctrl.AppendByteArray(inData1, reg);
                    inData1 = ctrl.AppendByteArray(inData1, val);

                    byte[] inData2 = new byte[] {(byte)0x89};
                    inData2 = ctrl.AppendByteArray(inData2, reg);
                    inData2 = ctrl.AppendByteArray(inData2, val);

                    if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
                    if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData2 );
                }
            }
            else {
                int pos = mIndexPos - 1;
                byte slave = mCabinet[pos];
                Log.i(VD_DTAG, String.format(Locale.US, "WhiteSeam Emulate. ( index: %d, slave: 0x%02x )", pos, slave));

                for( int j = 0; j < mSeamReg.length; j++ ) {
                    byte[] inData;
                    inData = new byte[] { slave };
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16));
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16));

                    ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
                }
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> WhiteSeam Emulate Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Emulate Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskWhiteSeamWrite extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private int mIndexPos;

        private int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        private int[] mSeamVal;

        public AsyncTaskWhiteSeamWrite( Context context, int indexPos, int[] value, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mIndexPos = indexPos;   // Spinner Position. ( 0: all, 1: mCabinet[0], 2: mCabinet[1], ... )
            mSeamVal = value;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            //
            //  1. Update White Seam in Emulate Register.
            //
            if( mIndexPos == 0 ) {
                boolean bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
                boolean bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

                for( int j = 0; j < mSeamReg.length; j++ ) {
                    byte[] reg = ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16);
                    byte[] val = ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16);

                    byte[] inData1 = new byte[] {(byte)0x09};
                    inData1 = ctrl.AppendByteArray(inData1, reg);
                    inData1 = ctrl.AppendByteArray(inData1, val);

                    byte[] inData2 = new byte[] {(byte)0x89};
                    inData2 = ctrl.AppendByteArray(inData2, reg);
                    inData2 = ctrl.AppendByteArray(inData2, val);

                    if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
                    if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData2 );
                }
            }
            else {
                int pos = mIndexPos - 1;
                byte slave = mCabinet[pos];
                for( int j = 0; j < mSeamReg.length; j++ ) {
                    byte[] inData;
                    inData = new byte[] { slave };
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamReg[j], NxCinemaCtrl.FORMAT_INT16));
                    inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[j], NxCinemaCtrl.FORMAT_INT16));

                    ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
                }
            }


            //
            //  2. Update White Seam in Flash Data.
            //
            int start   = (mIndexPos != 0) ? mIndexPos - 1  : 0;
            int end     = (mIndexPos != 0) ? mIndexPos      : mCabinet.length;

            for( int i = start; i < end; i++ ) {
                byte slave = mCabinet[i];
                byte[] result, inData;
                inData = new byte[] { slave };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[0], NxCinemaCtrl.FORMAT_INT16));
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[1], NxCinemaCtrl.FORMAT_INT16));
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[2], NxCinemaCtrl.FORMAT_INT16));
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mSeamVal[3], NxCinemaCtrl.FORMAT_INT16));

                result = ctrl.Send(NxCinemaCtrl.CMD_TCON_WHITE_SEAM_WRITE, inData);
                if( result == null || result.length == 0 || ctrl.ByteArrayToInt(result) != 0x01 ) {
                    Log.i(VD_DTAG, "Fail, Write WhiteSeam.");
                    return null;
                }

                Log.i(VD_DTAG, String.format(Locale.US, "WhiteSeam Write. ( pos: %d, slave: 0x%02x, top: %d, bottom: %d, left: %d, right: %d )",
                        i, slave, mSeamVal[0], mSeamVal[1], mSeamVal[2], mSeamVal[3]));
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> WhiteSeam Write Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Write Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskUniformity extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private int mMode = -1;

        public AsyncTaskUniformity( Context context, int mode, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mMode = mode;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            String[] result;
            int enableUniformity = 0;

            result = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
            for( String file : result ) {
                ConfigPfpgaInfo info = new ConfigPfpgaInfo();
                if( info.Parse( file ) ) {
                    enableUniformity = info.GetEnableUpdateUniformity(mMode);

                    for( int i = 0; i < info.GetRegister(mMode).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mMode)[i], NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
                    }
                }
            }

            result = FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
            for( String file : result ) {
                LedUniformityInfo info = new LedUniformityInfo();
                if( info.Parse(file) ) {
                    if( enableUniformity == 0 ) {
                        Log.i(VD_DTAG, String.format( "Skip. Update Uniformity Correction. ( %s )", file ));
                        continue;
                    }

                    byte[] inData = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );
                    ctrl.Send( NxCinemaCtrl.CMD_PFPGA_UNIFORMITY_DATA, inData );
                }
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Uniformity Correction Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Uniformity Correction Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPixelCorrectionAdapter extends AsyncTask<Void, String, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private CheckRunAdapter mAdapter;

        public AsyncTaskPixelCorrectionAdapter( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof CheckRunAdapter )
                mAdapter = (CheckRunAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            String[] resultDir = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
            for( String dir : resultDir ) {
                if( isCancelled() ) {
                    return null;
                }
                publishProgress( dir );
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(String... values) {
            String[] resultFile = FileManager.CheckFile(values[0], LedDotCorrectInfo.PATTERN_NAME);
            mAdapter.add( new CheckRunInfo(values[0].substring(values[0].lastIndexOf("/") + 1), String.format("total : %s", resultFile.length)) );
            Collections.sort(mAdapter.get(), new Comparator<CheckRunInfo>() {
                @Override
                public int compare(CheckRunInfo t0, CheckRunInfo t1) {
                    return (t0.GetTitle().compareTo(t1.GetTitle()) > 0) ? 1 : -1;
                }
            });

            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Pixel Correction Adapter Start.");
            mStartTime = System.currentTimeMillis();

            mAdapter.clear();
            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Pixel Correction Adapter Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPixelCorrection extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private CheckRunAdapter mAdapter;

        public AsyncTaskPixelCorrection( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof CheckRunAdapter )
                mAdapter = (CheckRunAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            String[] resultDir = FileManager.CheckDirectoryInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.PATTERN_DIR);
            if( resultDir == null || resultDir.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            String topdir = resultDir[0].substring(0, resultDir[0].lastIndexOf("/") + 1);

            for( int i = 0; i < mAdapter.getCount(); i++ ) {
                int success = 0;
                int fail = 0;

                CheckRunInfo item = mAdapter.getItem(i);
                if( null == item || !item.GetChecked() )
                    continue;

                String[] result = FileManager.CheckFile(topdir + item.GetTitle(), LedDotCorrectInfo.PATTERN_NAME);
                for( String file : result ) {
                    Log.i(VD_DTAG, "Dot Correct Info : " + file);

                    LedDotCorrectInfo info = new LedDotCorrectInfo(mContext);
                    if( info.Parse(file) ) {
                        byte[] sel = ctrl.IntToByteArray( info.GetModule(), NxCinemaCtrl.FORMAT_INT8 );         // size: 1
                        byte[] data = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );    // size: 61440
                        byte[] inData =  ctrl.AppendByteArray( sel, data );

                        byte[] res = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOT_CORRECTION, ctrl.AppendByteArray( new byte[]{(byte)info.GetIndex()}, inData ) );
                        if( res == null || res.length == 0 ) {
                            publishProgress(i, result.length, success, ++fail);
                            continue;
                        }

                        if( res[0] == (byte)0xFF ) {
                            publishProgress(i, result.length, success, ++fail);
                            continue;
                        }

                        publishProgress(i, result.length, ++success, fail);
                    }
                }
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            CheckRunInfo info = mAdapter.getItem(values[0]);
            if( null != info )
                info.SetDescription( String.format(Locale.US, "total: %d, success: %d, fail: %d", values[1], values[2], values[3]));

            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Pixel Correction Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Pixel Correction Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPixelCorrectionExtract extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private int mIndex;
        private int mModule;

        public AsyncTaskPixelCorrectionExtract( Context context, int index, int module, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mIndex = index;
            mModule = module;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            int start   = (mModule != LedDotCorrectInfo.MAX_MODULE_NUM) ? mModule     : 0;
            int end     = (mModule != LedDotCorrectInfo.MAX_MODULE_NUM) ? mModule + 1 : LedDotCorrectInfo.MAX_MODULE_NUM;

            for( int i = start; i < end; i++ ) {
                Log.i(VD_DTAG, String.format(Locale.US, "Pixel correction extract. ( slave: 0x%02X, module: %d )", (byte)mIndex, i) );

                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOT_CORRECTION_EXTRACT, new byte[]{(byte)mIndex, (byte)i} );
                if( result == null || result.length == 0)
                    continue;

                String[] extPath = FileManager.GetExternalPath();
                String strDir = String.format(Locale.US, "%s/DOT_CORRECTION_ID%03d", extPath[0], ((CinemaInfo)mContext).GetCabinetNumber((byte)mIndex));
                if( !FileManager.MakeDirectory( strDir ) ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail, Create Directory. ( %s )", strDir));
                    continue;
                }

                new LedDotCorrectInfo(mContext).Make(mIndex, i, result, strDir);
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate(null);

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Pixel Correction Extract Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Pixel Correction Extract Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskSetNetwork extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private String[] mNetworkParam;
        private int[] mResult = new int[]{-1, };

        public AsyncTaskSetNetwork( Context context, String[] param, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;

            mNetworkParam = param;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mNetworkParam.length != 6 )
                return null;

            if( !Patterns.IP_ADDRESS.matcher(mNetworkParam[0]).matches() ||
                !Patterns.IP_ADDRESS.matcher(mNetworkParam[1]).matches() ||
                !Patterns.IP_ADDRESS.matcher(mNetworkParam[2]).matches() ||
                mNetworkParam[0].equals(mNetworkParam[2]) ||
                (!mNetworkParam[3].equals("") && !Patterns.IP_ADDRESS.matcher(mNetworkParam[3]).matches()) ||
                (!mNetworkParam[4].equals("") && !Patterns.IP_ADDRESS.matcher(mNetworkParam[4]).matches()) ||
                (!mNetworkParam[5].equals("") && !Patterns.IP_ADDRESS.matcher(mNetworkParam[5]).matches()) ) {
                return null;
            }

            // NetworkTools tools = new NetworkTools();
            // tools.SetConfig( mNetworkParam[0], mNetworkParam[1], mNetworkParam[2], mNetworkParam[3], mNetworkParam[4], mNetworkParam[5] );
            // if( tools.Ping( mNetworkParam[2] ) ) ShowMessage( "Valid IP Address.");
            // else ShowMessage("Please Check IP Address. ( Invalid IP Address )");

            NetworkTools tools = new NetworkTools();
            tools.SetConfig( mNetworkParam[0], mNetworkParam[1], mNetworkParam[2], mNetworkParam[3], mNetworkParam[4], mNetworkParam[5] );

            mResult[0] = CinemaInfo.RET_PASS;
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate( null );

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Set Network Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( mNetworkParam );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mResult) );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Set Network Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskGetNetwork extends AsyncTask<Void, Object, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private String[] mNetworkParam = new String[7];

        public AsyncTaskGetNetwork( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mProgressUpdate = progressUpdate;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            try {
                FileReader inFile = new FileReader("/system/bin/nap_network");
                BufferedReader inReader = new BufferedReader(inFile);

                try {
                    for( int i = 0; i < mNetworkParam.length; i++ ) {
                        mNetworkParam[i] = inReader.readLine();
                        publishProgress( i, mNetworkParam[i] );
                    }
                    inReader.close();
                    inFile.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Object... values) {
            if( mProgressUpdate != null ) {
                Log.i(VD_DTAG, String.format(Locale.US, ">> network #%d : %s", (Integer)values[0], (String)values[1]) );
                mProgressUpdate.onProgressUpdate(values);
            }
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Get Network Start.");
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( null );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Get Network Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskChange3D extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private boolean mMode = false;
        private int[] mResult = new int[]{ -1 };

        public AsyncTaskChange3D( Context context, boolean mode, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mMode = mode;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            ((CinemaInfo)mContext).SetMode3D(mMode);
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate(null);

            super.onProgressUpdate();
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Change 3D Start.");
            Log.i(VD_DTAG, String.format( Locale.US, ">>> Change 3D Start. ( curTime: %d mSec )", System.currentTimeMillis()));
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mResult) );

            Log.i(VD_DTAG, String.format( Locale.US, ">>> Change 3D Done. ( curTime: %d mSec )", System.currentTimeMillis()));
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Change 3D Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskChangeScale extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private ProgressUpdateCallback mProgressUpdate = null;

        private byte[] mCabinet;
        private boolean mMode = false;
        private int[] mResult = new int[]{ -1 };

        public AsyncTaskChangeScale( Context context, boolean mode, PreExecuteCallback preExecute, PostExecuteCallback postExecute, ProgressUpdateCallback progressUpdate ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mMode = mode;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0, bValidPort1;
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );
            {
                byte[] reg = ctrl.IntToByteArray(CinemaInfo.REG_PFPGA_0x0199, NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mMode ? 0x0000 : 0x0001, NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, dat);
                ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
            }
            {
                ((CinemaInfo)mContext).AddCabinet();
                ((CinemaInfo)mContext).ShowCabinet();

                bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
                bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);
            }
            {
                byte[] reg = ctrl.IntToByteArray(CinemaInfo.REG_TCON_0x018D, NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mMode ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, dat);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0 );
                if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1 );
            }
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

            mResult[0] = mMode ? 1 : 0;
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            if( mProgressUpdate != null )
                mProgressUpdate.onProgressUpdate(null);

            super.onProgressUpdate();
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Change Scale Start.");
            Log.i(VD_DTAG, String.format( Locale.US, ">>> Change Scale Start. ( curTime: %d mSec )", System.currentTimeMillis()));
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mResult) );

            Log.i(VD_DTAG, String.format( Locale.US, ">>> Change Scale Done. ( curTime: %d mSec )", System.currentTimeMillis()));
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Change Scale Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskChangeMode extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;

        private byte[] mCabinet;
        private int mMode = -1;
        private int[] mUpdate = new int[4];
        private int[] mResult = new int[]{ -1 };

        public AsyncTaskChangeMode( Context context, int mode, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mMode = mode;

            mUpdate[0] = Integer.parseInt(((CinemaInfo)mContext).GetValue(CinemaInfo.KEY_UPDATE_TGAM0));
            mUpdate[1] = Integer.parseInt(((CinemaInfo)mContext).GetValue(CinemaInfo.KEY_UPDATE_TGAM1));
            mUpdate[2] = Integer.parseInt(((CinemaInfo)mContext).GetValue(CinemaInfo.KEY_UPDATE_DGAM0));
            mUpdate[3] = Integer.parseInt(((CinemaInfo)mContext).GetValue(CinemaInfo.KEY_UPDATE_DGAM1));
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mMode < 0 || mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            boolean bValidPort0 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_L);
            boolean bValidPort1 = ((CinemaInfo)mContext).IsCabinetValidPort(CinemaInfo.PORT_R);

            //
            //  Prepare T_REG.txt parsing
            //
            String[] resultPath;
            ConfigTconInfo tconEEPRomInfo = new ConfigTconInfo();
            ConfigTconInfo tconUsbInfo = new ConfigTconInfo();

            resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
            for( String file : resultPath ) {
                tconUsbInfo.Parse(file);
            }

            resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
            for( String file : resultPath ) {
                tconEEPRomInfo.Parse(file);
            }

            if( ((10 > mMode) && (mMode >= tconEEPRomInfo.GetModeNum())) ||
                (10 <= mMode) && ((mMode-10) >= tconUsbInfo.GetModeNum())) {
                Log.i(VD_DTAG, String.format(Locale.US, "Fail, Change Mode. ( mode: %d, eeprom: %d, usb: %d )",
                        mMode, tconEEPRomInfo.GetModeNum(), tconUsbInfo.GetModeNum() ));
                return null;
            }

            if( 10 > mMode &&
                !((CinemaInfo)mContext).IsMode3D() &&
                tconEEPRomInfo.GetDataMode(mMode) == ConfigTconInfo.MODE_3D ) {
                Log.i(VD_DTAG, String.format(Locale.US, "Skip. Change Mode. ( mode: %d, is3D: %b, dataMode: %d )",
                        mMode, ((CinemaInfo)mContext).IsMode3D(), tconEEPRomInfo.GetDataMode(mMode)));
                return null;
            }

            if( 10 <= mMode &&
                !((CinemaInfo)mContext).IsMode3D() &&
                tconUsbInfo.GetDataMode(mMode-10) == ConfigTconInfo.MODE_3D ) {
                Log.i(VD_DTAG, String.format(Locale.US, "Skip. Change Mode. ( mode: %d, is3D: %b, dataMode: %d )",
                        mMode, ((CinemaInfo)mContext).IsMode3D(), tconUsbInfo.GetDataMode(mMode)));
                return null;
            }

            //
            //  Check TCON Booting Status
            //
            if( ((CinemaInfo)mContext).IsCheckTconBooting() ) {
                boolean bTconBooting = true;
                for( byte id : mCabinet ) {
                    byte[] result;
                    result = ctrl.Send(NxCinemaCtrl.CMD_TCON_BOOTING_STATUS, new byte[]{id});
                    if (result == null || result.length == 0) {
                        Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet: %d, port: %d, slave: 0x%02x )",
                                ((CinemaInfo)mContext).GetCabinetNumber(id),
                                ((CinemaInfo)mContext).GetCabinetPort(id),
                                ((CinemaInfo)mContext).GetCabinetSlave(id)
                        ));
                        continue;
                    }

                    if( result[0] == 0 ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet: %d, port: %d, slave: 0x%02x, result: %d )",
                                ((CinemaInfo)mContext).GetCabinetNumber(id),
                                ((CinemaInfo)mContext).GetCabinetPort(id),
                                ((CinemaInfo)mContext).GetCabinetSlave(id),
                                result[0]
                        ));
                        bTconBooting = false;
                    }
                }

                if( !bTconBooting ) {
                    Log.i(VD_DTAG, "Fail, TCON booting.");
                    return null;
                }
            }

            //
            //  PFPGA Mute on
            //
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x01} );

            //
            //  TCON Mute on
            //
            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_MUTE, new byte[] {(byte)0x09, (byte)0x01} );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_MUTE, new byte[] {(byte)0x89, (byte)0x01} );

            //
            //  Parse P_REG.txt
            //
            int enableUniformity = 0;
            int[] enableGamma = {0, 0, 0, 0};

            resultPath = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
            for( String file : resultPath ) {
                ConfigPfpgaInfo info = new ConfigPfpgaInfo();
                if( info.Parse( file ) ) {
                    enableUniformity = info.GetEnableUpdateUniformity(mMode);
                    for( int i = 0; i < info.GetRegister(mMode).length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister(mMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mMode)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
                    }
                }
            }

            //
            //  Auto Uniformity Correction Writing
            //
            resultPath = FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
            for( String file : resultPath ) {
                LedUniformityInfo info = new LedUniformityInfo();
                if( info.Parse( file ) ) {
                    if( enableUniformity == 0 ) {
                        Log.i(VD_DTAG, String.format( "Skip. Update Uniformity. ( %s )", file ));
                        continue;
                    }

                    byte[] inData = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );
                    ctrl.Send( NxCinemaCtrl.CMD_PFPGA_UNIFORMITY_DATA, inData );
                }
            }

            //
            //  Parse T_REG.txt
            //
            if( (10 > mMode) && (mMode < tconEEPRomInfo.GetModeNum())) {
                enableGamma = tconEEPRomInfo.GetEnableUpdateGamma(mMode);
                for( int i = 0; i < tconEEPRomInfo.GetRegister(mMode).length; i++ ) {
                    byte[] reg = ctrl.IntToByteArray(tconEEPRomInfo.GetRegister(mMode)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] data = ctrl.IntToByteArray(tconEEPRomInfo.GetData(mMode)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, data);

                    byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                    byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                }
            }

            if( (10 <= mMode) && ((mMode-10) < tconUsbInfo.GetModeNum())) {
                enableGamma = tconUsbInfo.GetEnableUpdateGamma(mMode-10);
                for( int i = 0; i < tconUsbInfo.GetRegister(mMode-10).length; i++ ) {
                    byte[] reg = ctrl.IntToByteArray(tconUsbInfo.GetRegister(mMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] data = ctrl.IntToByteArray(tconUsbInfo.GetData(mMode-10)[i], NxCinemaCtrl.FORMAT_INT16);
                    byte[] inData = ctrl.AppendByteArray(reg, data);

                    byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                    byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                    if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                    if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
                }
            }

            //
            //  Write Gamma
            //
            if( (10 > mMode) && (mMode < tconEEPRomInfo.GetModeNum())) {
                resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_EEPROM, LedGammaInfo.PATTERN_NAME);
                for( String file : resultPath ) {
                    LedGammaInfo info = new LedGammaInfo();
                    if( info.Parse( file ) ) {
                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 1) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Update EEPRom Gamma. ( %s )", file ));
                            continue;
                        }

                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] == mUpdate[0]) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] == mUpdate[1]) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] == mUpdate[2]) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] == mUpdate[3]) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Already Update EEPRom Gamma. ( %s )", file ));
                            continue;
                        }

                        int cmd;
                        if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                            cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                        else
                            cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                        byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                        byte[] inData = ctrl.AppendByteArray(table, data);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                        if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                        if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                    }
                }
            }

            if( (10 <= mMode) ) {
                resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_EEPROM, LedGammaInfo.PATTERN_NAME);
                for( String file : resultPath ) {
                    LedGammaInfo info = new LedGammaInfo();
                    if( info.Parse( file ) ) {
                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 1) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 1) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Update EEPRom Gamma. ( %s )", file ));
                            continue;
                        }

                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] == mUpdate[0]) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] == mUpdate[1]) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] == mUpdate[2]) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] == mUpdate[3]) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Already Update EEPRom Gamma. ( %s )", file ));
                            continue;
                        }

                        int cmd;
                        if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                            cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                        else
                            cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                        byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                        byte[] inData = ctrl.AppendByteArray(table, data);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                        if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                        if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                    }
                }

                resultPath = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_USB, LedGammaInfo.PATTERN_NAME);
                for( String file : resultPath ) {
                    LedGammaInfo info = new LedGammaInfo();
                    if( info.Parse( file ) ) {
                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] != 2) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] != 2) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] != 2) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] != 2) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Update USB Gamma. ( %s )", file ));
                            continue;
                        }

                        if( (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[0] == mUpdate[0]) ||
                            (info.GetType() == LedGammaInfo.TYPE_TARGET && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[1] == mUpdate[1]) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT0 && enableGamma[2] == mUpdate[2]) ||
                            (info.GetType() == LedGammaInfo.TYPE_DEVICE && info.GetTable() == LedGammaInfo.TABLE_LUT1 && enableGamma[3] == mUpdate[3]) ) {
                            Log.i(VD_DTAG, String.format( "Skip. Already Update USB Gamma. ( %s )", file ));
                            continue;
                        }

                        int cmd;
                        if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                            cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                        else
                            cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                        byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                        byte[] inData = ctrl.AppendByteArray(table, data);

                        byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                        byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                        if( bValidPort0 ) ctrl.Send( cmd + info.GetChannel(), inData0 );
                        if( bValidPort1 ) ctrl.Send( cmd + info.GetChannel(), inData1 );
                    }
                }
            }

            //
            //  Update Gamma Status
            //
            ((CinemaInfo)mContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM0, String.valueOf(enableGamma[0]) );
            ((CinemaInfo)mContext).SetValue( CinemaInfo.KEY_UPDATE_TGAM1, String.valueOf(enableGamma[1]) );
            ((CinemaInfo)mContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM0, String.valueOf(enableGamma[2]) );
            ((CinemaInfo)mContext).SetValue( CinemaInfo.KEY_UPDATE_DGAM1, String.valueOf(enableGamma[3]) );

            //
            //  SW Reset
            //
            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x09});
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_SW_RESET, new byte[]{(byte)0x89});

            //
            //  TCON Mute off
            //
            if( bValidPort0 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_MUTE, new byte[] {(byte)0x09, (byte)0x00} );
            if( bValidPort1 ) ctrl.Send( NxCinemaCtrl.CMD_TCON_MUTE, new byte[] {(byte)0x89, (byte)0x00} );

            //
            //  PFPGA Mute off
            //
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

            mResult[0] = mMode;
            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            Log.i(VD_DTAG, ">>> Change Mode Start.");
            Log.i(VD_DTAG, String.format( Locale.US, ">>> Change Mode Start. ( curTime: %d mSec )", System.currentTimeMillis()));
            Log.i(VD_DTAG, String.format( Locale.US, ">>> mode = %d", mMode));
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute( null );

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( 0 > mResult[0] ) {
                Log.i(VD_DTAG, String.format( Locale.US, ">>> Change Mode Fail. ( expected: %d )", mMode ));
            }

            if( 0 <= mResult[0] && 20 > mResult[0] ) {
                ((CinemaInfo)mContext).SetValue(CinemaInfo.KEY_INITIAL_MODE, String.valueOf(mMode));
            }

            if( mPostExecute != null )
                mPostExecute.onPostExecute( ToInteger(mResult) );

            Log.i(VD_DTAG, String.format( Locale.US, ">>> Change Mode Done. ( curTime: %d mSec )", System.currentTimeMillis()));
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Change Mode Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            Unlock();
            super.onPostExecute(aVoid);
        }
    }
}
