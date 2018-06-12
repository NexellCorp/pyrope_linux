package com.samsung.vd.cinemacontrolpanel;

import android.app.Activity;
import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;
import android.view.View;
import android.widget.Adapter;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Locale;
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Created by doriya on 2/6/18.
 */

public class CinemaTask {
    private final String VD_DTAG = "CinemaTask";

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

    public boolean IsBusy() {
        return mSem.availablePermits() == 0;
    }

    public void RunTconStatus( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        new AsyncTaskTconStatus(context, adapter, preExecute, postExecute).execute();
    }

    public void RunTconLvds( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        new AsyncTaskTconLvds(context, adapter, preExecute, postExecute).execute();
    }

    public void RunLedOpenNum( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        new AsyncTaskLedOpenNum(context, adapter, preExecute, postExecute).execute();
    }

    public void RunCabinetDoor( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        new AsyncTaskCabinetDoor(context, adapter, preExecute, postExecute).execute();
    }

    public void RunPeripheral( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        new AsyncTaskPeripheral(context, adapter, preExecute, postExecute).execute();
    }

    public void RunVersion( Context context, Adapter adapter, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        new AsyncTaskVersion(context, adapter, preExecute, postExecute).execute();
    }

    public void RunChangeMode( Context context, int mode, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        new AsyncTaskChangeMode(context, mode, preExecute, postExecute).execute();
    }

    public void RunTconRegisterRead( Context context, int[] register, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
        new AsyncTaskTconRegisterRead(context, register, preExecute, postExecute).execute();
    }

    private void Lock() {
        try {
            mSem.acquire();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void Unlock() {
        mSem.release();
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
    private class AsyncTaskTconRegisterRead extends AsyncTask<Void, Integer, Void> {
        private Context mContext = null;
        private PreExecuteCallback mPreExecute = null;
        private PostExecuteCallback mPostExecute = null;
        private byte[] mCabinet;
        private int[] mRegister;
        private int[] mValue;

        public AsyncTaskTconRegisterRead( Context context, int[] register, PreExecuteCallback preExecute, PostExecuteCallback postExecute ) {
            mContext = context;
            mPreExecute = preExecute;
            mPostExecute = postExecute;
            mCabinet = ((CinemaInfo)mContext).GetCabinet();
            mRegister = register;
            mValue = new int[mRegister.length];
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if( mCabinet.length == 0 || mRegister.length == 0)
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for( int i = 0; i < mRegister.length; i++ )
            {
                byte[] result, inData;
                inData = new byte[] { mCabinet[0] };
                inData = ctrl.AppendByteArray(inData, ctrl.IntToByteArray(mRegister[i], NxCinemaCtrl.FORMAT_INT16));

                result = ctrl.Send( NxCinemaCtrl.CMD_TCON_REG_READ, inData );
                if( result == null || result.length == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "i2c read fail.( id: 0x%02X, reg: 0x%04X )", mCabinet[0], mRegister[i] ));
                    return null;
                }

                mValue[i] = ctrl.ByteArrayToInt(result);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            Lock();

            Log.i(VD_DTAG, ">>> TCON Register Read Start.");
            mStartTime = System.currentTimeMillis();

            CinemaLoading2.Show( mContext );
            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute( mValue );

            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON Register Read Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            CinemaLoading2.Hide();

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

            Log.i(VD_DTAG, ">>> TCON Status Check Start.");
            mStartTime = System.currentTimeMillis();

            CinemaLoading2.Show( mContext );
            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON Status Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            CinemaLoading2.Hide();

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

            Log.i(VD_DTAG, ">>> TCON LVDS Check Start.");
            mStartTime = System.currentTimeMillis();

            CinemaLoading2.Show( mContext );
            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Log.i(VD_DTAG, String.format(Locale.US, ">>> TCON LVDS Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            CinemaLoading2.Hide();

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

            Log.i(VD_DTAG, ">>> LED Open Check Start.");
            mStartTime = System.currentTimeMillis();

            CinemaLoading2.Show( mContext );
            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Log.i(VD_DTAG, String.format(Locale.US, ">>> LED Open Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            CinemaLoading2.Hide();

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

            Log.i(VD_DTAG, ">>> Cabinet Door Check Start.");
            mStartTime = System.currentTimeMillis();

            CinemaLoading2.Show( mContext );
            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Cabinet Door Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            CinemaLoading2.Hide();

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

            Log.i(VD_DTAG, ">>> Peripheral Check Start.");
            mStartTime = System.currentTimeMillis();

            CinemaLoading2.Show( mContext );
            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Peripheral Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            CinemaLoading2.Hide();

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

            Log.i(VD_DTAG, ">>> Version Check Start.");
            mStartTime = System.currentTimeMillis();

            CinemaLoading2.Show( mContext );
            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Version Check Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            CinemaLoading2.Hide();

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

            Log.i(VD_DTAG, ">>> Change Mode Start.");
            mStartTime = System.currentTimeMillis();

            CinemaLoading2.Show( mContext );
            if( mPreExecute != null )
                mPreExecute.onPreExecute();

            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if( mPostExecute != null )
                mPostExecute.onPostExecute();

            Log.i(VD_DTAG, String.format(Locale.US, ">>> Change Mode Done. ( %d mSec )", System.currentTimeMillis() - mStartTime ));
            CinemaLoading2.Hide();

            Unlock();
            super.onPostExecute(aVoid);
        }
    }
}
