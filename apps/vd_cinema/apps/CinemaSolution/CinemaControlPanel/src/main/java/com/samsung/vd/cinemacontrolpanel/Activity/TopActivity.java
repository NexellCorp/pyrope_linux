package com.samsung.vd.cinemacontrolpanel.Activity;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import com.samsung.vd.cinemacontrolpanel.InfoClass.Adapters.TextButtonAdapter;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigPfpgaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.ConfigTconInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedDotCorrectInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedGammaInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.LedUniformityInfo;
import com.samsung.vd.cinemacontrolpanel.InfoClass.TextButtonInfo;
import com.samsung.vd.cinemacontrolpanel.R;
import com.samsung.vd.cinemacontrolpanel.Utils.AccountPreference;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaLoading;
import com.samsung.vd.cinemacontrolpanel.Utils.CinemaService;
import com.samsung.vd.cinemacontrolpanel.Utils.FileManager;
import com.samsung.vd.cinemacontrolpanel.Utils.NXAsync;

import java.lang.ref.WeakReference;
import java.util.Locale;

import static com.samsung.vd.cinemacontrolpanel.Utils.CinemaService.*;
import static com.samsung.vd.cinemacontrolpanel.Utils.NXAsync.CMD_TopInitMode;
import static java.lang.Integer.parseInt;

/**
 * Created by doriya on 8/17/16.
 */
public class TopActivity extends BaseActivity {
    private final String VD_DTAG = "TopActivity";

    private UIHandler mUIHandler;
    ConfigTconInfo sourceTconInfo;
    private int mInitMode;
    private Button mBtnInitMode;
    private TextView mTextInitMode;

    private TextButtonAdapter mAdapterMode;

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if( intent.getAction().equals(Intent.ACTION_MEDIA_MOUNTED) ) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                Log.i(VD_DTAG , "ACTION_MEDIA_MOUNTED");
                UpdateInitialValue();
            }
            if( intent.getAction().equals(Intent.ACTION_MEDIA_EJECT) ) {
                mBtnInitMode.setEnabled(false);
                UpdateInitialValue();
            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();

        IntentFilter filter = new IntentFilter();
        filter.addAction( Intent.ACTION_MEDIA_MOUNTED );
        filter.addAction( Intent.ACTION_MEDIA_EJECT );
        filter.addDataScheme("file");

        registerReceiver( mBroadcastReceiver, filter );
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver( mBroadcastReceiver );
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_top);

        mUIHandler = new UIHandler( this );

        mServiceCallback = new CinemaService.ChangeContentsCallback() {
            @Override
            public void onChangeContentsCallback(int mode) {
                mUIHandler.sendEmptyMessage(0);
            }
        };

        // Set Title Bar and Status Bar
        setCommonUI(R.id.titleBarLayoutTop, R.id.statusBarLayoutTop , "Cinema LED Display System - Top Menu" , new Intent(getApplicationContext(), LoginActivity.class) );

        //  Cinema System Information
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);

        //  Alert Message
//        int configCabinetNum = Integer.parseInt(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM));
//        int detectCabientNum = ((CinemaInfo)getApplicationContext()).GetCabinet().length;
//        if( configCabinetNum != detectCabientNum && ((CinemaInfo)getApplicationContext()).IsCheckCabinet() ) {
//            String strMessage = String.format(Locale.US, "Please Check Cabinet Number. ( setting: %d, detect: %d )", configCabinetNum, detectCabientNum);
//            CinemaAlert.Show( TopActivity.this, "Alert",  strMessage );
//        }

        //
        TextView textComment = (TextView)findViewById(R.id.textCommentTop);
        textComment.setText("");
        textComment.append(String.format(Locale.US, "-. TCON Config File\t\t\t\t: [USB_TOP]/%s/%s\n", ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME));
        textComment.append(String.format(Locale.US, "-. PFPGA Config File\t\t\t\t: [USB_TOP]/%s/%s\n", ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME));
        textComment.append(String.format(Locale.US, "-. Uniformity File\t\t\t\t\t: [USB_TOP]/%s/%s\n", LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME));
        textComment.append(String.format(Locale.US, "-. Gamma File\t\t\t\t\t\t: [USB_TOP]/%s/%s\n", LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME));
        textComment.append(String.format(Locale.US, "-. Dot Correct Path\t\t\t\t: [USB_TOP]/%s/IDxxx/\n", LedDotCorrectInfo.PATH));
        textComment.append(String.format(Locale.US, "-. Dot Correct Extract Path\t: [USB_TOP]/DOT_CORRECTION_IDxxx/\n"));

        //  Parse TCON Configuration Files.
        ListView listViewMode = (ListView)findViewById(R.id.listView_mode_apply);
        listViewMode.addFooterView(listViewFooter);

        mAdapterMode = new TextButtonAdapter(this, R.layout.listview_row_text_button);
        listViewMode.setAdapter( mAdapterMode );

        //
        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE);
        mInitMode = (strTemp == null) ? 0 : parseInt(strTemp);

        mTextInitMode = (TextView)findViewById(R.id.textInitModeCurrent);
        mTextInitMode.setText(String.format(Locale.US, "Index #%d", mInitMode+1));

        mBtnInitMode = (Button)findViewById(R.id.btnInitModeUpdate);
        mBtnInitMode.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String[] resultPfpga = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
                for( String path : resultPfpga ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, ConfigPfpgaInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultUniformity = FileManager.CheckFileInUsb(LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME);
                for( String path : resultUniformity ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, LedUniformityInfo.PATH_TARGET + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultQuality = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
                for( String path : resultQuality ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, ConfigTconInfo.PATH_TARGET_USB + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                String[] resultGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
                for( String path : resultGamma ) {
                    Log.i(VD_DTAG, ">>" + path);
                    FileManager.CopyFile(path, LedGammaInfo.PATH_TARGET_USB + "/" + path.substring(path.lastIndexOf("/") + 1));
                }

                if( (resultPfpga.length != 0) || (resultUniformity.length != 0 ) || (resultQuality.length != 0) || (resultGamma.length != 0) ) {
                    ShowMessage( "Update Initial Value File.\nIndex 11~20 will be changed");
                    UpdateInitialValue();
                }

                //  TCON EEPROM Test Code.
//                new AsyncTaskEEPRomRead().execute();
                //NXAsync.getInstance().Execute(CMD_TopEEPRomRead , baseAsyncCallback);
            }
        });

        UpdateInitialValue();

        //
        //
        //
        Button btnMenuDiagnostics = (Button)findViewById(R.id.btnMenuDiagonostics);
        Button btnMenuDisplayCheck = (Button)findViewById(R.id.btnMenuDisplayCheck);
        Button btnMenuDisplayMode = (Button)findViewById(R.id.btnMenuDisplayMode);
        Button btnMenuSystem = (Button)findViewById(R.id.btnMenuSystem);
        Button btnMenuAccount = (Button)findViewById(R.id.btnMenuAccount);

        btnMenuDiagnostics.setOnClickListener(mClickListener);
        btnMenuDisplayCheck.setOnClickListener(mClickListener);
        btnMenuDisplayMode.setOnClickListener(mClickListener);
        btnMenuSystem.setOnClickListener(mClickListener);
        btnMenuAccount.setOnClickListener(mClickListener);

        CinemaInfo info = ((CinemaInfo)getApplicationContext());
        Log.i(VD_DTAG, "--> Login Group : " + info.GetUserGroup());

        if( !info.IsCheckLogin() )
            btnMenuAccount.setEnabled(false);

        if( info.GetUserGroup().equals(AccountPreference.GROUP_OPERATOR) ) {
            btnMenuDiagnostics.setEnabled(false);
            btnMenuDisplayCheck.setEnabled(false);
            btnMenuSystem.setEnabled(false);
            btnMenuAccount.setEnabled(false);
        }

        if( info.GetUserGroup().equals(AccountPreference.GROUP_CALIBRATOR) ) {
            btnMenuSystem.setEnabled(false);
        }
    }

    private TextButtonAdapter.OnClickListener mTextbuttonAdapterClickListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
//            new AsyncTaskInitMode(position).execute();
            NXAsync.getInstance().getTconInfoCarrier().setiMode(position);
            NXAsync.getInstance().Execute( CMD_TopInitMode , asyncCallbackInitMode );
        }
    };

    private TextButtonAdapter.OnClickListener mRemovebuttonAdapterClickListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            int iMode = Integer.parseInt(mAdapterMode.getItem(position).GetMode());
            int eepromModeNum = NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetModeNum();

            if(10 > position && position < eepromModeNum) {
                NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().Remove(iMode , TopActivity.this);
            }

            int usbModeNum = NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetModeNum();
            if(10 <= position && (position-10) < usbModeNum ) {
                NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().Remove(iMode , TopActivity.this);
            }
        }
    };
    private TextButtonAdapter.OnClickListener mUpdatebuttonAdapterClickListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            int iMode = Integer.parseInt(mAdapterMode.getItem(position).GetMode());
            int eepromModeNum = NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetModeNum();

            if(10 > position && position < eepromModeNum) {
                NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().Update(iMode , TopActivity.this , sourceTconInfo);
            }

            int usbModeNum = NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetModeNum();
            if(10 <= position && (position-10) < usbModeNum ) {
                NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().Update(iMode , TopActivity.this , sourceTconInfo);
            }
        }
    };

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch( (v).getId() ) {
                case R.id.btnMenuDiagonostics:
                    startActivity( new Intent(v.getContext(), DiagnosticsActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
                case R.id.btnMenuDisplayCheck:
                    startActivity( new Intent(v.getContext(), DisplayCheckActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
                case R.id.btnMenuDisplayMode:
                    startActivity( new Intent(v.getContext(), DisplayModeActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
                case R.id.btnMenuSystem:
                    startActivity( new Intent(v.getContext(), SystemActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
                case R.id.btnMenuAccount:
                    startActivity( new Intent(v.getContext(), AccountActivity.class) );
                    overridePendingTransition(0, 0);
                    finish();
                    break;
            }
        }
    };

    private void UpdateInitialValue() {
        String[] usbPfpga = FileManager.CheckFileInUsb(ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME);
        String[] usbUniformity = FileManager.CheckFileInUsb(LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME);
        String[] usbTcon = FileManager.CheckFileInUsb(ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME);
        String[] usbGamma = FileManager.CheckFileInUsb(LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME);
        String[] internalPfpga = FileManager.CheckFile(ConfigPfpgaInfo.PATH_TARGET, ConfigPfpgaInfo.NAME);
        String[] internalUniformity = FileManager.CheckFile(LedUniformityInfo.PATH_TARGET, LedUniformityInfo.NAME);
        String[] internalTcon = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
        String[] internalGamma = FileManager.CheckFile(LedGammaInfo.PATH_TARGET_USB, LedGammaInfo.PATTERN_NAME);

        if( (usbPfpga != null && usbPfpga.length != 0) ||
            (usbUniformity != null && usbUniformity.length != 0) ||
            (usbTcon != null && usbTcon.length != 0) ||
            (usbGamma != null && usbGamma.length != 0) ) {
            mBtnInitMode.setEnabled(true);
        }

//        if( (internalPfpga != null && internalPfpga.length != 0) ||
//            (internalUniformity != null && internalUniformity.length != 0) ||
//            (internalTcon != null && internalTcon.length != 0) ||
//            (internalGamma != null && internalGamma.length != 0) ) {
//            mBtnInitMode.setEnabled(true);
//        }

        if( usbPfpga != null ) for(String file : usbPfpga) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
        if( usbUniformity != null ) for(String file : usbUniformity) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
        if( usbTcon != null ) for(String file : usbTcon) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));
        if( usbGamma != null ) for(String file : usbGamma) Log.i(VD_DTAG, String.format("Detection File. ( %s )", file ));


        sourceTconInfo = new ConfigTconInfo();
        int[] mMode = null;
        if(usbTcon != null && usbTcon.length != 0) {
          for( String file : usbTcon ) {
              if(sourceTconInfo.Parse(file)) {
                  mMode = sourceTconInfo.GetModeArray();
              }
          }
        }

        String[] resultPath;
        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME);
        mAdapterMode.clear();
        for( String file : resultPath ) {
            if( NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().Parse( file ) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetModeNum() ) {
                        if( null != mMode ){
                            boolean bFound = false;
                            for (int k = 0 ; k < mMode.length ; k++) {
                                if (NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetMode(i) == mMode[k]) {
                                    bFound = true;
                                    mAdapterMode.add(
                                            new TextButtonInfo(
                                                    String.format(Locale.US, "%d", i+1),
                                                    String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetMode(i)),
                                                    String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetDescription(i)),
                                                    mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, mUpdatebuttonAdapterClickListener)
                                    );
                                    break;
                                }
                            }
                            if(!bFound) {
                                mAdapterMode.add(
                                        new TextButtonInfo(
                                                String.format(Locale.US, "%d", i+1),
                                                String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetMode(i)),
                                                String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetDescription(i)),
                                                mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, null)
                                );
                            }
                        }else {
                            mAdapterMode.add(
                                    new TextButtonInfo(
                                            String.format(Locale.US, "%d", i+1),
                                            String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetMode(i)),
                                            String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconEEPRomInfo().GetDescription(i)),
                                            mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, null)
                            );
                        }

                    }
                    else {
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, ""), "")
                        );
                    }
                    mAdapterMode.notifyDataSetChanged();
                }

            }
            else {
                for( int i = 0; i < 10; i++ ) {
                    mAdapterMode.add(
                            new TextButtonInfo(
                                    String.format(Locale.US, ""), "")
                    );
                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }

        resultPath = FileManager.CheckFile(ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME);
        for( String file : resultPath ) {
            if( NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().Parse(file) ) {
                for( int i = 0; i < 10; i++ ) {
                    if( i < NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetModeNum() ) {
                        if( null != mMode ){
                            boolean bFound = false;
                            for (int k = 0 ; k < mMode.length ; k++) {
                                if (NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetMode(i) == mMode[k]) {
                                    bFound = true;
                                    mAdapterMode.add(
                                            new TextButtonInfo(
                                                    String.format(Locale.US, "%d", i+11),
                                                    String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetMode(i)),
                                                    String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetDescription(i)),
                                                    mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, mUpdatebuttonAdapterClickListener)
                                    );
                                    break;
                                }
                            }
                            if(!bFound) {
                                mAdapterMode.add(
                                        new TextButtonInfo(
                                                String.format(Locale.US, "%d", i+11),
                                                String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetMode(i)),
                                                String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetDescription(i)),
                                                mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, null)
                                );
                            }
                        }else {
                            mAdapterMode.add(
                                    new TextButtonInfo(
                                            String.format(Locale.US, "%d", i+11),
                                            String.format(Locale.US, "%d", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetMode(i)),
                                            String.format(Locale.US, "%s", NXAsync.getInstance().getTconInfoCarrier().getTconUsbInfo().GetDescription(i)),
                                            mTextbuttonAdapterClickListener, mRemovebuttonAdapterClickListener, null)
                            );
                        }
                    }
                    else {
                        mAdapterMode.add(
                                new TextButtonInfo(
                                        String.format(Locale.US, ""), "")
                        );
                    }
                    mAdapterMode.notifyDataSetChanged();
                }
            }
            else {
                for( int i = 0; i < 10; i++ ) {
                    mAdapterMode.add(
                            new TextButtonInfo(
                                    String.format(Locale.US, ""), "")
                    );
                    mAdapterMode.notifyDataSetChanged();
                }
            }
        }
    }


    NXAsync.AsyncCallback asyncCallbackInitMode = new NXAsync.AsyncCallback() {
        @Override
        public void onPreExe() {
            TopActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    CinemaLoading.Show( TopActivity.this );
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }

        @Override
        public void onPostExe() {
            TopActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(NXAsync.getInstance().isI2CFailed()) {
                        ShowMessage("I2C Failed.. try again later");
                        CinemaLoading.Hide();
                        NXAsync.getInstance().getAsyncSemaphore().release();
                        return;
                    }

                    mTextInitMode.setText(String.format(Locale.US, "-. Current Mode : Index #%d", NXAsync.getInstance().getTconInfoCarrier().getiMode() + 1));
                    ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_INITIAL_MODE, String.valueOf(NXAsync.getInstance().getTconInfoCarrier().getiMode()));
                    CinemaLoading.Hide();
                    NXAsync.getInstance().getAsyncSemaphore().release();
                }
            });
        }
    };


    private static class UIHandler extends Handler {
        private WeakReference<TopActivity> mActivity;

        public UIHandler( TopActivity activity ) {
            mActivity = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            TopActivity activity = mActivity.get();
            if( activity != null ) {
                activity.handleMessage(msg);
            }
        }
    }

    private void handleMessage( Message msg ) {
        String strTemp = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_INITIAL_MODE);
        mInitMode = (strTemp == null) ? 0 : Integer.parseInt(strTemp);
        mTextInitMode.setText(String.format(Locale.US, "Mode #%d", mInitMode+1));
    }
}//TopActivity class
