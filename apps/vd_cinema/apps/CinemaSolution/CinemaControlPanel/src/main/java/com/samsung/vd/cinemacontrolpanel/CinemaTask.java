package com.samsung.vd.cinemacontrolpanel;

import android.content.Context;
import android.os.AsyncTask;
import android.os.SystemClock;
import android.util.Log;
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
    public static final int CMD_VERSION                 = 16;
    public static final int CMD_TEST_PATTERN            = 17;
    public static final int CMD_ACCUMULATION            = 18;

    public static final int CMD_WHITESEAM_ENABLE        = 20;
    public static final int CMD_WHITESEAM_READ          = 21;
    public static final int CMD_WHITESEAM_EMULATE       = 22;
    public static final int CMD_WHITESEAM_WRITE         = 23;
    public static final int CMD_LOD_RESET               = 24;

    public static final int CMD_CHECK_CABINET           = 25;
    public static final int CMD_CHECK_CABINET_NUM       = 26;
    public static final int CMD_WAIT_BOOTING_COMPLETE   = 27;
    public static final int CMD_DISPLAY_VERSION         = 28;
    public static final int CMD_PIXEL_CORRECTION_ADAPTER= 29;
    public static final int CMD_PIXEL_CORRECTION        = 30;
    public static final int CMD_PIXEL_CORRECTION_EXTRACT= 31;
    public static final int CMD_UNIFORMITY              = 32;

    public static final int CMD_PFPGA_MUTE              = 40;

    public static final int CMD_CHANGE_MODE             = 99;

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

    public void Run( int cmd, Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_TCON_STATUS:
                new AsyncTaskTconStatus(context, adapter, preExecute, postExecute).execute();
                break;
            case CMD_TCON_LVDS:
                new AsyncTaskTconLvds(context, adapter, preExecute, postExecute).execute();
                break;
            case CMD_LED_OPEN_NUM:
                new AsyncTaskLedOpenNum(context, adapter, preExecute, postExecute).execute();
                break;
            case CMD_CABINET_DOOR:
                new AsyncTaskCabinetDoor(context, adapter, preExecute, postExecute).execute();
                break;
            case CMD_PERIPHERAL:
                new AsyncTaskPeripheral(context, adapter, preExecute, postExecute).execute();
                break;
            case CMD_VERSION:
                new AsyncTaskVersion(context, adapter, preExecute, postExecute).execute();
                break;
            case CMD_ACCUMULATION:
                new AsyncTaskAccumulation(context, adapter, preExecute, postExecute).execute();
                break;
            case CMD_PIXEL_CORRECTION_ADAPTER:
                new AsyncTaskPixelCorrectionAdapter(context, adapter, preExecute, postExecute).execute();
                break;
            case CMD_PIXEL_CORRECTION:
                new AsyncTaskPixelCorrection(context, adapter, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, Adapter adapter, int index, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_LED_OPEN_POS:
                new AsyncTaskLedOpenPos( context, adapter, index, preExecute, postExecute);
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int index, int module, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_PIXEL_CORRECTION_EXTRACT:
                new AsyncTaskPixelCorrectionExtract(context, index, module, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_LOD_RESET:
                new AsyncTaskLodReset(context, preExecute, postExecute).execute();
                break;
            case CMD_CHECK_CABINET:
                new AsyncTaskCheckCabinet(context, preExecute, postExecute).execute();
                break;
            case CMD_CHECK_CABINET_NUM:
                new AsyncTaskCheckCabinetNum(context, preExecute, postExecute).execute();
                break;
            case CMD_WAIT_BOOTING_COMPLETE:
                new AsyncTaskWaitBootingComplete(context, preExecute, postExecute).execute();
                break;
            case CMD_DISPLAY_VERSION:
                new AsyncTaskDisplayVersion(context, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, boolean onoff, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_PFPGA_MUTE:
                new AsyncTaskPfpgaMute(context, onoff, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int mode, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_UNIFORMITY:
                new AsyncTaskUniformity(context, mode, preExecute, postExecute).execute();
                break;
            case CMD_CHANGE_MODE:
                new AsyncTaskChangeMode(context, mode, preExecute, postExecute).execute();
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

    public void Run( int cmd, Context context, int[] register, int[] data, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_TCON_REG_WRITE:
                new AsyncTaskTconRegWrite(context, register, data, preExecute, postExecute).execute();
                break;
            case CMD_PFPGA_REG_WRITE:
                new AsyncTaskPfpgaRegWrite(context, register, data, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int[] register, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_TCON_REG_READ:
                new AsyncTaskTconRegRead(context, register, preExecute, postExecute).execute();
                break;
            case CMD_PFPGA_REG_READ:
                new AsyncTaskPfpgaRegRead(context, register, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int index, int[] value, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_WHITESEAM_EMULATE:
                new AsyncTaskWhiteSeamEmulate(context, index, value, preExecute, postExecute).execute();
                break;
            case CMD_WHITESEAM_WRITE:
                new AsyncTaskWhiteSeamWrite(context, index, value, preExecute, postExecute).execute();
                break;
            default:
                Log.i(VD_DTAG, String.format("Fail, Invalid Command. ( %d )", cmd));
                break;
        }
    }

    public void Run( int cmd, Context context, int index, boolean emulate, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        switch( cmd )
        {
            case CMD_WHITESEAM_ENABLE:
                new AsyncTaskWhiteSeamEnable(context, index, emulate, preExecute, postExecute).execute();
                break;
            case CMD_WHITESEAM_READ:
                new AsyncTaskWhiteSeamRead(context, index, emulate, preExecute, postExecute).execute();
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
        void onPreExecute();
    }

    interface PostExecuteCallback {
        void onPostExecute();
        void onPostExecute( int[] values );
    }

    //
    //  Implementation AsyncTask
    //
    private class AsyncTaskTconRegWrite extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private int[] mReg;
        private int[] mDat;

        public AsyncTaskTconRegWrite( Context context, int[] register, int[] data, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mReg = register;
            mDat = data;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 || mReg.length == 0 || mDat.length == 0 )
                return null;

            if( mReg.length != mDat.length )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            for( int i = 0; i < mReg.length; i++ ) {
                byte[] reg = ctrl.IntToByteArray(mReg[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mDat[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, dat);

                byte[] inData0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, inData);
                byte[] inData1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, inData);

                if(bValidPort0) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData0);
                if(bValidPort1) ctrl.Send(NxCinemaCtrl.CMD_TCON_REG_WRITE, inData1);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> TCON Register Write Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON Register Write Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskTconRegRead extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private int[] mRegister;
        private int[] mResult;

        public AsyncTaskTconRegRead( Context context, int[] register, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mRegister = register;
            mResult = new int[mRegister.length];
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
                    continue;
                }

                mResult[i] = ctrl.ByteArrayToInt(result);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> TCON Register Read Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON Register Read Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute( mResult );

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPfpgaRegWrite extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private int[] mReg;
        private int[] mDat;

        public AsyncTaskPfpgaRegWrite( Context context, int[] register, int[] data, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mReg = register;
            mDat = data;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mReg.length == 0 || mDat.length == 0 )
                return null;

            if( mReg.length != mDat.length )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mReg.length; i++ )
            {
                byte[] reg = ctrl.IntToByteArray(mReg[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mDat[i], NxCinemaCtrl.FORMAT_INT16);
                byte[] inData = ctrl.AppendByteArray(reg, dat);

                ctrl.Send( NxCinemaCtrl.CMD_PFPGA_REG_WRITE, inData );
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> PFPGA Register Write Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> PFPGA Register Write Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPfpgaRegRead extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private int[] mRegister;
        private int[] mResult;

        public AsyncTaskPfpgaRegRead( Context context, int[] register, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mRegister = register;
            mResult = new int[mRegister.length];
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

                result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, reg );
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( reg: 0x%04X )", mRegister[i] ));
                    continue;
                }

                mResult[i] = ctrl.ByteArrayToInt(result);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> PFPGA Register Read Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> PFPGA Register Read Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute( mResult );

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPfpgaMute extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private boolean mOnoff;

        public AsyncTaskPfpgaMute( Context context, boolean onoff, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mOnoff = onoff;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {mOnoff ? (byte)0x01 : (byte)0x00} );
            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> PFPGA Mute Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> PFPGA Mute Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskCheckCabinet extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;

        public AsyncTaskCheckCabinet( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            for( int i = 0; i < 255; i++ ) {
                if( (i & 0x7F) < 0x10 )
                    continue;

                byte[] result = ctrl.Send(NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{(byte)i});
                if (result == null || result.length == 0)
                    continue;

                if( 0x01 != result[0] )
                    continue;

                ((CinemaInfo)mContext).AddCabinet( (byte)i );
                Log.i(VD_DTAG, String.format(Locale.US, "Add Cabinet ( Cabinet : %d, port : %d, slave : 0x%02x )", (i & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (i & 0x80) >> 7, i & 0x7F ));
            }
            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Check Cabinet Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Check Cabinet Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskCheckCabinetNum extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private int mCabinetNum =0;

        public AsyncTaskCheckCabinetNum( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            for( int i = 0; i < 255; i++ ) {
                if( (i & 0x7F) < CinemaInfo.TCON_ID_OFFSET )
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
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Check Cabinet Number Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Check Cabinet Number Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute(new int[] {mCabinetNum});

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskWaitBootingComplete extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;

        public AsyncTaskWaitBootingComplete(Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            //
            //  Wait nap_server
            //
            boolean bFirstBoot = ((SystemClock.elapsedRealtime() / 1000) < ((CinemaInfo)mContext).GetBootTime());
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
            }

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            try {
                int value;
                do {
                    byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PLATFORM_IS_BUSY, null);
                    value = ctrl.ByteArrayToInt(result);
                    Thread.sleep(1000);

                    if( value != 0 ) {
                        Log.i(VD_DTAG, String.format(Locale.US, ">>> Wait I2C Resource. ( 0x%02X 0x%02X 0x%02X 0x%02X )",
                                result[0], result[1], result[2], result[3]));
                    }
                } while(value != 0);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Wait Booting Complete Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Wait Booting Complete Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskDisplayVersion extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;

        public AsyncTaskDisplayVersion( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            Date date = new Date( BuildConfig.BUILD_DATE + 3600 * 9 * 1000 );

            String appVersion = new SimpleDateFormat("HH:mm:ss, MMM dd yyyy ", Locale.US).format(date);
            byte[] napVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_NAP_VERSION, null );
            byte[] sapVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_SAP_VERSION, null );
            byte[] srvVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_SERVER_VERSION, null );
            byte[] clnVersion = ctrl.Send( NxCinemaCtrl.CMD_PLATFORM_IPC_CLIENT_VERSION, null );
            byte[] pfpgaVersion = ctrl.Send( NxCinemaCtrl.CMD_PFPGA_VERSION, null );

            Log.i(VD_DTAG, ">> Version Information");
            Log.i(VD_DTAG, String.format(Locale.US, "-. Application : %s", appVersion));
            Log.i(VD_DTAG, String.format(Locale.US, "-. N.AP        : %s", (napVersion != null && napVersion.length != 0) ? new String(napVersion).trim() : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. S.AP        : %s", (sapVersion != null && sapVersion.length != 0) ? new String(sapVersion).trim() : "Unknown"));

            for( byte cabinet : mCabinet ) {
                byte[] tconVersion = ctrl.Send(NxCinemaCtrl.CMD_TCON_VERSION, null);

                int msbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_MSB) : 0;
                int lsbVersion = (tconVersion != null && tconVersion.length != 0) ? ctrl.ByteArrayToInt32(tconVersion, NxCinemaCtrl.MASK_INT32_LSB) : 0;

                Log.i(VD_DTAG, String.format(Locale.US, "-. TCON #%02d    : %05d - %05d", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, msbVersion, lsbVersion));
            }
            Log.i(VD_DTAG, String.format(Locale.US, "-. PFPGA       : %s", (pfpgaVersion != null && pfpgaVersion.length != 0) ? String.format(Locale.US, "%05d", ctrl.ByteArrayToInt(pfpgaVersion)) : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Server  : %s", (srvVersion != null && srvVersion.length != 0) ? new String(srvVersion).trim() : "Unknown"));
            Log.i(VD_DTAG, String.format(Locale.US, "-. IPC Client  : %s", (clnVersion != null && clnVersion.length != 0) ? new String(clnVersion).trim() : "Unknown"));

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Display Version Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Display Version Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskTconStatus extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private StatusSimpleAdapter mAdapter = null;

        public AsyncTaskTconStatus( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof StatusSimpleAdapter )
                mAdapter = (StatusSimpleAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mAdapter == null || mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_STATUS, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1);
                    continue;
                }

                if( result[0] != StatusSimpleInfo.PASS ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (int)result[0]);
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            mAdapter.add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), values[1] ));
            mAdapter.notifyDataSetChanged();

            if( values[1] == StatusSimpleInfo.ERROR ) {
                String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON Status is Error.", values[0] );
                ((CinemaInfo)mContext).InsertLog(strLog);
            }
            if( values[1] == StatusSimpleInfo.FAIL ) {
                String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON Status is Failed.", values[0] );
                ((CinemaInfo)mContext).InsertLog(strLog);
            }

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> TCON Status Check Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON Status Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskTconLvds extends  AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private StatusSimpleAdapter mAdapter = null;

        public AsyncTaskTconLvds( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof StatusSimpleAdapter )
                mAdapter = (StatusSimpleAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mAdapter == null || mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_LVDS_STATUS, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1);
                    continue;
                }

                if( result[0] != StatusSimpleInfo.PASS ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (int)result[0]);
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            mAdapter.add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), values[1] ));
            mAdapter.notifyDataSetChanged();

            if( values[1] == StatusSimpleInfo.ERROR ) {
                String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON LVDS is Error.", values[0] );
                ((CinemaInfo)mContext).InsertLog(strLog);
            }
            if( values[1] == StatusSimpleInfo.FAIL ) {
                String strLog = String.format( Locale.US, "[ Cabinet%02d ] TCON LVDS is Failed.", values[0] );
                ((CinemaInfo)mContext).InsertLog(strLog);
            }

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> TCON LVDS Check Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON LVDS Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLedOpenNum extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private StatusDetailAdapter mAdapter = null;

        public AsyncTaskLedOpenNum( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof StatusDetailAdapter )
                mAdapter = (StatusDetailAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mAdapter == null || mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{(byte)0x09} );
            if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{(byte)0x89} );

            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_OPEN_NUM, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet ));
                    publishProgress(cabinet & 0xFF, (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1);
                    continue;
                }

                int value = ctrl.ByteArrayToInt16( result, NxCinemaCtrl.FORMAT_INT16 );
                if( value != 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d)", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, value ));
                    publishProgress(cabinet & 0xFF, (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, value);
                }
            }

            if( bValidPort0 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{(byte)0x09} );
            if( bValidPort1 )   ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{(byte)0x89} );

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            //
            //  values[0] : Cabinet ID, values[1] : Slave Address, values[2] : Result
            //
            mAdapter.add(new StatusDetailInfo(String.format(Locale.US, "Cabinet %02d", values[1]), values[0], 0, (0 > values[2]) ? "-" : String.valueOf(values[2])));
            mAdapter.notifyDataSetChanged();

            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> LED Open Check Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> LED Open Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLedOpenPos extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte mIndex;
        private LedPosAdapter mAdapter = null;

        public AsyncTaskLedOpenPos( Context context, Adapter adapter, int index, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mIndex = (byte)(index & 0xFF);
            if( adapter instanceof LedPosAdapter )
                mAdapter = (LedPosAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mAdapter == null )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            byte[] result;
            result = ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_LOD, new byte[]{mIndex} );
            if( result == null || result.length == 0 )
                return null;

            result = ctrl.Send( NxCinemaCtrl.CMD_TCON_OPEN_NUM, new byte[]{mIndex});
            if( result == null || result.length == 0 )
                return null;

            int numOfOpen = ctrl.ByteArrayToInt16(result, NxCinemaCtrl.FORMAT_INT16);
            for( int i = 0; i < numOfOpen; i++ ) {
                if(isCancelled())
                    break;

                result = ctrl.Send( NxCinemaCtrl.CMD_TCON_OPEN_POS, new byte[]{mIndex} );
                if( result == null || result.length == 0)
                    break;

                int posX = ctrl.ByteArrayToInt16( result, NxCinemaCtrl.MASK_INT16_MSB);
                int posY = ctrl.ByteArrayToInt16( result, NxCinemaCtrl.MASK_INT16_LSB);
                publishProgress( i, posX, posY );
            }

            ctrl.Send( NxCinemaCtrl.CMD_TCON_MODE_NORMAL, new byte[]{mIndex} );
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {

            mAdapter.add( new LedPosInfo(String.valueOf(values[0]), String.valueOf(values[1]), String.valueOf(values[2])));
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> LED Open Position Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> LED Open Position Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskCabinetDoor extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private StatusSimpleAdapter mAdapter = null;

        public AsyncTaskCabinetDoor( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof StatusSimpleAdapter )
                mAdapter = (StatusSimpleAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOOR_STATUS, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1);
                    continue;
                }
                if( 0 > result[0] ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, StatusSimpleInfo.ERROR);
                }
                else if( result[0] == 0 || result[0] == 2 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, cabinet, result[0] ));
                    publishProgress( (cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, StatusSimpleInfo.FAIL );
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            mAdapter.add( new StatusSimpleInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), values[1] ));
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Cabinet Door Check Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Cabinet Door Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPeripheral extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private StatusSimpleAdapter mAdapter = null;

        public AsyncTaskPeripheral( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof StatusSimpleAdapter )
                mAdapter = (StatusSimpleAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            //
            //  Security AP
            //
            {
                StatusSimpleInfo info = mAdapter.getItem(0);
                info.SetStatus(StatusSimpleInfo.FAIL);

                ((CinemaInfo)mContext).SetSecureAlive("false");
                for( int i = 0; i < 16; i++ ) {
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    if( ((CinemaInfo)mContext).GetSecureAlive().equals("true") ) {
                        info.SetStatus(StatusSimpleInfo.PASS);
                        break;
                    }
                }

                publishProgress();
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

                StatusSimpleInfo info = mAdapter.getItem(1);
                info.SetStatus(StatusSimpleInfo.ERROR);

                NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
                byte[] result = ctrl.Send(NxCinemaCtrl.CMD_PFPGA_STATUS, null);
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG,  "Unknown Error.");
                    info.SetStatus(StatusSimpleInfo.ERROR);
                }
                else {
                    info.SetStatus( result[0] );
                }

                publishProgress();
            }

            //
            //  IMB
            //
            {
                NetworkTools tools = new NetworkTools();
                StatusSimpleInfo info = mAdapter.getItem(2);

                String strImbAddress = "";
                try {
                    BufferedReader inReader = new BufferedReader(new FileReader("/system/bin/nap_network"));
                    try {
                        inReader.readLine();    // param: ip address ( mandatory )
                        inReader.readLine();    // param: netmask    ( mandatory )
                        inReader.readLine();    // param: gateway    ( mandatory )
                        inReader.readLine();    // param: network    ( mandatory but auto generate )
                        inReader.readLine();    // param: dns1       ( optional )
                        inReader.readLine();    // param: dns2       ( optional )
                        strImbAddress = inReader.readLine();// param: imb address( optional )
                        inReader.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }

                if( strImbAddress != null && !strImbAddress.equals("") ) {
                    try {
                        if( tools.GetEthLink().equals("true") && tools.Ping(strImbAddress) ) {
                            info.SetStatus(StatusSimpleInfo.PASS);
                        }
                        else {
                            info.SetStatus(StatusSimpleInfo.FAIL);
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                else {
                    info.SetStatus(StatusSimpleInfo.ERROR);
                }
            }

            publishProgress();
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Peripheral Check Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Peripheral Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskVersion extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private StatusDescribeAdapter mAdapter = null;

        public AsyncTaskVersion( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof StatusDescribeAdapter )
                mAdapter = (StatusDescribeAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            for( byte cabinet : mCabinet ) {
                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_VERSION, new byte[]{cabinet} );
                if (result == null || result.length == 0) {
                    publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, -1, -1);
                    continue;
                }

                int version1 = ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_MSB);
                int version2 = ctrl.ByteArrayToInt32(result, NxCinemaCtrl.MASK_INT32_LSB);
                publishProgress((cabinet & 0x7F) - CinemaInfo.TCON_ID_OFFSET, version1, version2);
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            if( values[1] == -1 || values[2] == -1 ) {
                mAdapter.add( new StatusDescribeInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), "Unknown Version") );
            }
            else {
                mAdapter.add( new StatusDescribeInfo( String.format( Locale.US, "Cabinet %02d", values[0] ), String.format( Locale.US, "%05d-%05d", values[1], values[2] )));
            }

            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Version Check Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Version Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

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
                NxCinemaCtrl.REG_TCON_FLASH_CC,
                NxCinemaCtrl.REG_TCON_CC_MODULE,
                NxCinemaCtrl.REG_TCON_XYZ_TO_RGB,
                NxCinemaCtrl.REG_TCON_SEAM_ON,
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

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            if( mFunc < 7 ) {
                byte[] data = { (byte)mFunc, (byte)mPattern };

                byte[] data0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, data);
                byte[] data1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, data);

                if( bValidPort0 ) ctrl.Send( mStatus ? NxCinemaCtrl.CMD_TCON_PATTERN_RUN : NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data0 );
                if( bValidPort1 ) ctrl.Send( mStatus ? NxCinemaCtrl.CMD_TCON_PATTERN_RUN : NxCinemaCtrl.CMD_TCON_PATTERN_STOP, data1 );
            }
            else {
                byte[] reg = ctrl.IntToByteArray(mPatternReg[mFunc-7], NxCinemaCtrl.FORMAT_INT16);
                byte[] dat = ctrl.IntToByteArray(mStatus ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16);
                byte[] data = ctrl.AppendByteArray(reg, dat);
                byte[] data0 = ctrl.AppendByteArray(new byte[]{(byte)0x09}, data);
                byte[] data1 = ctrl.AppendByteArray(new byte[]{(byte)0x89}, data);

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
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Test Pattern Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Test Pattern Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskLodReset extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;

        public AsyncTaskLodReset( Context context, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 ) {
                return null;
            }

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

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
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Lod Reset Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Lod Reset Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskAccumulation extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private StatusDescribeExpandableAdapter mAdapter = null;

        public AsyncTaskAccumulation( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            if( adapter instanceof StatusDescribeExpandableAdapter )
                mAdapter = (StatusDescribeExpandableAdapter)adapter;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            for( int i = 0; i < mCabinet.length; i++ ) {
                mAdapter.addGroup( new StatusDescribeExpandableInfo(String.format( Locale.US, "Cabinet %02d", (mCabinet[i] & 0x7F) - CinemaInfo.TCON_ID_OFFSET )) );

                for( int j = 0; j < CinemaInfo.TCON_MODULE_NUM; j++ ) {
                    byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_ACCUMULATE_TIME, new byte[] { mCabinet[i], (byte)j } );
                    if( result == null || result.length == 0 ) {
                        mAdapter.addChild(i, new StatusDescribeInfo(String.format( Locale.US, "Module #%02d", j), String.valueOf("Error")) );
                        continue;
                    }
                    mAdapter.addChild(i, new StatusDescribeInfo(String.format( Locale.US, "Module #%02d", j), new String(result) ));
                }
                publishProgress();
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Accumulation Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Accumulation Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskWhiteSeamEnable extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private int mIndexPos;
        private boolean mEmulate;

        public AsyncTaskWhiteSeamEnable( Context context, int indexPos, boolean emulate, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mIndexPos = indexPos;   // Spinner Position. ( 0: all, 1: mCabinet[0], 2: mCabinet[1], ... )
            mEmulate = emulate;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            if( mIndexPos == 0 ) {
                boolean bValidPort0 = false, bValidPort1 = false;
                for( byte id : mCabinet ) {
                    if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                    if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
                }

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
                byte slave = ((mCabinet[pos] % 16) < 8) ? (mCabinet[pos]) : (byte)(mCabinet[pos] | 0x80);
                byte[] inData;
                inData = new byte[] { slave };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(0x0189, NxCinemaCtrl.FORMAT_INT16));
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mEmulate ? 0x0001 : 0x0000, NxCinemaCtrl.FORMAT_INT16));

                ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_WRITE, inData );
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, String.format(Locale.US, "WhiteSeam Enable Start. ( %b )", mEmulate));
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Enable Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskWhiteSeamRead extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private int mIndexPos;
        private boolean mEmulate;

        private int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        private int[] mSeamVal = new int[4];

        public AsyncTaskWhiteSeamRead( Context context, int indexPos, boolean emulate, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
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
                    return null;
                }

                mSeamVal[i] = ctrl.ByteArrayToInt(result);

                Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Read Done. ( pos: %d, slave: 0x%02X, emulate: %b, top: %d, bottom: %d, left: %d, right: %d )",
                        index, slave, mEmulate, mSeamVal[0], mSeamVal[1], mSeamVal[2], mSeamVal[3]));
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> WhiteSeam Read Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Read Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute(mSeamVal);   // Top, Bottom, Left, Right

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskWhiteSeamEmulate extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private int mIndexPos;

        private int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        private int[] mSeamVal;

        public AsyncTaskWhiteSeamEmulate( Context context, int indexPos, int[] value, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
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

                boolean bValidPort0 = false, bValidPort1 = false;
                for( byte id : mCabinet ) {
                    if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                    if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
                }

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
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> WhiteSeam Emulate Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Emulate Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskWhiteSeamWrite extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private int mIndexPos;

        private int[] mSeamReg = new int[]{ 0x0180, 0x0181, 0x0182, 0x0183 };
        private int[] mSeamVal;

        public AsyncTaskWhiteSeamWrite( Context context, int indexPos, int[] value, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
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
                boolean bValidPort0 = false, bValidPort1 = false;
                for( byte id : mCabinet ) {
                    if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                    if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
                }

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
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> WhiteSeam Write Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> WhiteSeam Write Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskUniformity extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private int mMode = -1;

        public AsyncTaskUniformity( Context context, int mode, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
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
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Uniformity Correction Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Uniformity Correction Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPixelCorrectionAdapter extends AsyncTask<Void, String, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private CheckRunAdapter mAdapter;

        public AsyncTaskPixelCorrectionAdapter( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
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
            mStartTime = System.currentTimeMillis();
            mAdapter.clear();
            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Pixel Correction Adapter Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Pixel Correction Adapter Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

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

                    LedDotCorrectInfo info = new LedDotCorrectInfo();
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
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Pixel Correction Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Pixel Correction Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }

    private class AsyncTaskPixelCorrectionExtract extends AsyncTask<Void, Void, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private int mIndex;
        private int mModule;

        public AsyncTaskPixelCorrectionExtract( Context context, int index, int module, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mIndex = index;
            mModule = module;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            int start   = (mModule != LedDotCorrectInfo.MAX_MODULE_NUM) ? mModule     : 0;
            int end     = (mModule != LedDotCorrectInfo.MAX_MODULE_NUM) ? mModule + 1 : LedDotCorrectInfo.MAX_MODULE_NUM;

            for( int i = start; i < end; i++ ) {
                Log.i(VD_DTAG, String.format(Locale.US, "Dot correction extract. ( slave: 0x%02X, module: %d )", (byte)mIndex, i) );

                byte[] result = ctrl.Send( NxCinemaCtrl.CMD_TCON_DOT_CORRECTION_EXTRACT, new byte[]{(byte)mIndex, (byte)i} );
                if( result == null || result.length == 0)
                    continue;

                String[] extPath = FileManager.GetExternalPath();
                String strDir = String.format(Locale.US, "%s/DOT_CORRECTION_ID%03d", extPath[0], (mIndex & 0x7F) - CinemaInfo.TCON_ID_OFFSET);
                if( !FileManager.MakeDirectory( strDir ) ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail, Create Directory. ( %s )", strDir));
                    continue;
                }

                new LedDotCorrectInfo().Make(mIndex, i, result, strDir);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Pixel Correction Extract Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Pixel Correction Extract Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

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
        private int mUpdate[] = new int[4];

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

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
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
                        Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / port: %d / slave : 0x%02x )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (id & 0x80), id));
                        continue;
                    }

                    if( result[0] == 0 ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / port: %d / slave : 0x%02x / result : %d )", (id & 0x7F) - CinemaInfo.TCON_ID_OFFSET, (id & 0x80), id, result[0] ));
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
            //  Parse P_REG.txt
            //
            String[] resultPath;
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
            ConfigTconInfo tconEEPRomInfo = new ConfigTconInfo();
            resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
            for( String file : resultPath ) {
                tconEEPRomInfo.Parse(file);
            }

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

            ConfigTconInfo tconUsbInfo = new ConfigTconInfo();
            resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
            for( String file : resultPath ) {
                tconUsbInfo.Parse(file);
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
            //  PFPGA Mute off
            //
            ctrl.Send( NxCinemaCtrl.CMD_PFPGA_MUTE, new byte[] {0x00} );

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();
            mStartTime = System.currentTimeMillis();

            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            Log.i(VD_DTAG, ">>> Change Mode Start.");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            Log.i(VD_DTAG, String.format(Locale.US, ">>> Change Mode Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));

            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }
}
