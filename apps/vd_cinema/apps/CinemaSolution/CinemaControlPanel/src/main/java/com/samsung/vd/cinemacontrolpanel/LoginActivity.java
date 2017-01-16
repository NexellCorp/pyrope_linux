package com.samsung.vd.cinemacontrolpanel;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.os.AsyncTask;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.File;
import java.util.Arrays;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class LoginActivity extends AppCompatActivity {
    private static final String VD_DTAG = "LoginActivity";
    private static boolean mFirstBoot = true;

    private Spinner mSpinnerAccount;

    private EditText mEditPassword;
    private EditText mEditPassword1;
    private EditText mEditPassword2;
    private EditText mEditPassword3;

    private boolean mRescueMode = false;

    private AccountPreference mAccountPreference;
    private String[] mStrAccount;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SetScreenRotation();
        if( !((CinemaInfo)getApplicationContext()).IsCheckLogin() ) {
            new AsyncTaskCheckCabinet().execute();
            return;
        }

        setContentView(R.layout.activity_login);

        //
        //  Configuration Title Bar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Login" );
        titleBar.SetListener(VdTitleBar.BTN_ROTATE, new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ChangeScreenRotation();
            }
        });

        //
        //  For Debugging
        //
        titleBar.SetVisibility(VdTitleBar.BTN_BACK, View.GONE);
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                startActivity( new Intent(v.getContext(), InitialActivity.class) );
//                overridePendingTransition(0, 0);
//                finish();
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mService.TurnOff();
            }
        });

        if( !((CinemaInfo)getApplicationContext()).IsEnableRotate() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_ROTATE, View.GONE);
        }

        if( !((CinemaInfo)getApplicationContext()).IsEnableExit() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);
        }

        //
        //  Configuration Status Bar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        //
        //
        //
        mAccountPreference = new AccountPreference(getApplicationContext());
        mStrAccount = new String[] {
            AccountPreference.GROUP_ROOT,
            AccountPreference.GROUP_SERVICE,
            AccountPreference.GROUP_CALIBRATOR,
            AccountPreference.GROUP_OPERATOR };

        mSpinnerAccount = (Spinner)findViewById(R.id.spinnerAccount);
        ArrayAdapter<String> adapterAccount = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, mStrAccount );
        mSpinnerAccount.setAdapter(adapterAccount);
        mSpinnerAccount.setSelection( 3 );

        Button btnLoginNormal = (Button)findViewById(R.id.btnLoginNormal);
        btnLoginNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginRescue).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginNormal).setVisibility(View.VISIBLE);

                mEditPassword1.setText("");
                mEditPassword2.setText("");
                mEditPassword3.setText("");

                mRescueMode = false;
            }
        });

        Button btnLoginRescue = (Button)findViewById(R.id.btnLoginRescue);
        btnLoginRescue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                findViewById(R.id.layoutLoginNormal).setVisibility(View.GONE);
                findViewById(R.id.layoutLoginRescue).setVisibility(View.VISIBLE);

                mEditPassword.setText("");

                mRescueMode = true;
            }
        });

        Button btnAccpetNormal = (Button)findViewById(R.id.btnLoginNormalAccept);
        btnAccpetNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    new AsyncTaskCheckCabinet().execute();
                }
            }
        });

        Button btnAcceptRescue = (Button)findViewById(R.id.btnLoginRescueAccept);
        btnAcceptRescue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( CheckAccount() ) {
                    new AsyncTaskCheckCabinet().execute();
                }
            }
        });

        //
        //  IMM Handler
        //
        mEditPassword = (EditText)findViewById(R.id.editLoginPw);
        mEditPassword1 = (EditText)findViewById(R.id.editLoginPw1);
        mEditPassword2 = (EditText)findViewById(R.id.editLoginPw2);
        mEditPassword3 = (EditText)findViewById(R.id.editLoginPw3);

        RelativeLayout parent = (RelativeLayout)findViewById(R.id.layoutParent);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditPassword.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditPassword1.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditPassword2.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditPassword3.getWindowToken(), 0);
            }
        });
    }

    private boolean CheckAccount() {
        if( !mRescueMode ) {
            //  Normal Login Mode
            Log.i(VD_DTAG, "Normal Mode.");
            String strGroup = mSpinnerAccount.getSelectedItem().toString();
            String strPassword = mEditPassword.getText().toString();

            if( strPassword.equals("") ) {
                ShowMessage("Please check password.");
                return false;
            }

            int index = mAccountPreference.Confirm(strGroup, strPassword);
            if( 0 > index ) {
                ShowMessage("Please check password.");

                mEditPassword.setText("");

                return false;
            }

            CinemaInfo info = ((CinemaInfo)getApplicationContext());
            info.SetUserGroup(strGroup);
            info.SetUserId(( mAccountPreference.ReadId(strGroup, index) ));
            info.InsertLog( "Login." );
        }
        else {
            Log.i(VD_DTAG, "Rescue Mode.");
            //  Rescue Login Mode
            String strPassword1 = mEditPassword1.getText().toString();
            String strPassword2 = mEditPassword2.getText().toString();
            String strPassword3 = mEditPassword3.getText().toString();

            if( strPassword1.equals("") ||
                strPassword2.equals("") ||
                strPassword3.equals("") ) {
                ShowMessage("Please check password.");
                return false;
            }

            if( 0 > mAccountPreference.Confirm(AccountPreference.GROUP_OPERATOR, strPassword1) ||
                0 > mAccountPreference.Confirm(AccountPreference.GROUP_OPERATOR, strPassword2) ||
                0 > mAccountPreference.Confirm(AccountPreference.GROUP_OPERATOR, strPassword3) ) {
                ShowMessage("Please check password.");

                mEditPassword1.setText(null);
                mEditPassword2.setText(null);
                mEditPassword3.setText(null);

                return false;
            }

            CinemaInfo info = ((CinemaInfo)getApplicationContext());
            info.SetUserGroup(AccountPreference.GROUP_ROOT);
            info.InsertLog( "Login." );
        }

        return true;
    }

    //
    //
    //
    private class AsyncTaskCheckCabinet extends AsyncTask<Void, Void, Void> {
        private CinemaInfo mCinemaInfo = (CinemaInfo)getApplicationContext();

        @Override
        protected Void doInBackground(Void... params) {
            //
            //
            //
            if( mFirstBoot && ((CinemaInfo)getApplicationContext()).IsBootDelay() ) {
                int maxWaitTime = ((CinemaInfo)getApplicationContext()).GetBootDelay();
                try {
                    for( int i = 0; i < maxWaitTime; i++ ) {
                        Log.i(VD_DTAG, String.format(Locale.US, "wait time : %d sec", maxWaitTime - i ));
                        Thread.sleep(1000);
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            //
            //  Detection Cabinet
            //
            for( int i = 0; i < 255; i++ ) {
                if( (i & 0x7F) < 0x10 )
                    continue;

                byte[] result = NxCinemaCtrl.GetInstance().Send(i, NxCinemaCtrl.CMD_TCON_STATUS, null);
                if (result == null || result.length == 0)
                    continue;

                if( 0 > result[0] )
                    continue;

                mCinemaInfo.AddCabinet( (byte)i );
                Log.i(VD_DTAG, String.format(Locale.US, "Add Cabinet ( Cabinet : %d, port : %d, slave : 0x%02x )", (i & 0x7F) - CinemaInfo.OFFSET_TCON, (i & 0x80) >> 7, i & 0x7F ));
            }

            mCinemaInfo.SortCabinet();

            byte[] cabinet = mCinemaInfo.GetCabinet();
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : cabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            //
            //  Check TCON Booting Status
            //
            boolean bTconBooting = true;
            for( byte id : cabinet ) {
                byte[] result;
                result = ctrl.Send(id, NxCinemaCtrl.CMD_TCON_BOOTING_STATUS, null);
                if (result == null || result.length == 0) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Unknown Error. ( cabinet : %d / slave : 0x%02x )", (id & 0x7F) - CinemaInfo.OFFSET_TCON, id));
                    continue;
                }

                if( result[0] == 0 ) {
                    Log.i(VD_DTAG, String.format(Locale.US, "Fail. ( cabinet : %d / slave : 0x%02x / result : %d )", (id & 0x7F) - CinemaInfo.OFFSET_TCON, id, result[0] ));
                    bTconBooting = false;
                }
            }

            if( !bTconBooting ) {
                Log.i(VD_DTAG, "Fail, TCON booting.");
                return null;
            }

            //
            //  Image Quality Function for Test
            //
            String[] resultPath;
            resultPath = CheckFile(LedQualityInfo.PATH_TARGET, LedQualityInfo.NAME);
            for( String file : resultPath ) {
                LedQualityInfo info = new LedQualityInfo();
                if( info.Parse( file ) ) {
                    for( int i = 0; i < info.GetRegister().length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister()[i], NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntToByteArray(info.GetData(0)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_QUALITY, inData);
                        if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_QUALITY, inData);
                    }
                }
            }

            //
            //  Gamma Function for Test
            //
            resultPath = CheckFile(LedGammaInfo.PATH_TARGET, LedGammaInfo.PATTERN_NAME);
            for( String file : resultPath ) {
                LedGammaInfo info = new LedGammaInfo();
                if( info.Parse( file ) ) {
                    int cmd;
                    if( info.GetType() == LedGammaInfo.TYPE_TARGET )
                        cmd = NxCinemaCtrl.CMD_TCON_TGAM_R;
                    else
                        cmd = NxCinemaCtrl.CMD_TCON_DGAM_R;

                    byte[] table = ctrl.IntToByteArray(info.GetTable(), NxCinemaCtrl.FORMAT_INT8);
                    byte[] data = ctrl.IntArrayToByteArray(info.GetData(), NxCinemaCtrl.FORMAT_INT24);
                    byte[] inData = ctrl.AppendByteArray(table, data);

                    if( bValidPort0 ) ctrl.Send( 0x09, cmd + info.GetChannel(), inData );
                    if( bValidPort1 ) ctrl.Send( 0x89, cmd + info.GetChannel(), inData );
                }
            }

            if( mFirstBoot ) {
                if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_INIT, null);
                if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_INIT, null);
            }

            if( mCinemaInfo.IsStandAlone() ) {
                if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MULTI, new byte[] {(byte)0x01} );
                if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_MULTI, new byte[] {(byte)0x01} );
            }

            mFirstBoot = false;
            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            CinemaLoading.Show( LoginActivity.this );
            mCinemaInfo.ClearCabinet();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            startActivity( new Intent(getApplicationContext(), TopActivity.class) );
            overridePendingTransition(0, 0);

            CinemaLoading.Hide();
            finish();
        }
    }

    private String[] CheckFile( String topdir, String regularExpression ) {
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

    //
    //  For Internal Toast Message
    //
    private static Toast mToast;

    private void ShowMessage( String strMsg ) {
        if( mToast == null )
            mToast = Toast.makeText(getApplicationContext(), null, Toast.LENGTH_SHORT);

        mToast.setText(strMsg);
        mToast.show();
    }

    //
    //  For Screen Rotation
    //
    private void SetScreenRotation() {
        String orientation = ((CinemaInfo) getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_ROTATE);
        if( orientation == null ) {
            orientation = String.valueOf(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        switch( Integer.parseInt(orientation) ) {
            case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
            case ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
            default:
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
        }
    }

    private void ChangeScreenRotation() {
        String orientation = ((CinemaInfo) getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_ROTATE);
        if( orientation == null ) {
            orientation = String.valueOf(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        int curRotate;
        int prvRotate = Integer.parseInt(orientation);
        switch (prvRotate) {
            case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
            case ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                break;
            default:
                curRotate = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                break;
        }

        ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_SCREEN_ROTATE, String.valueOf(curRotate));
    }

    //
    //  For ScreenSaver
    //
    private CinemaService mService = null;
    private boolean mServiceRun = false;

    @Override
    protected void onStart() {
        super.onStart();

        Intent intent = new Intent(this, CinemaService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        super.onStop();

        if( mServiceRun ) {
            unbindService(mConnection);
            mServiceRun = false;
        }
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        boolean isOn = mService.IsOn();
        mService.RefreshScreenSaver();

        return !isOn || super.dispatchTouchEvent(ev);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            CinemaService.LocalBinder binder = (CinemaService.LocalBinder)service;
            mService = binder.GetService();
            mServiceRun = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mServiceRun = false;
        }
    };

}
