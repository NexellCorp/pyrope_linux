package com.samsung.vd.cinemacontrolpanel;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.util.Locale;

/**
 * Created by doriya on 8/17/16.
 */
public class TopActivity extends CinemaBaseActivity {
    private final String VD_DTAG = "TopActivity";

    private CinemaInfo mCinemaInfo;

    private int mInitMode;
    private TextView mTextInitMode;
    private Button mBtnScreenOn;

    private Button mBtnUpdate;
    private Button mBtnDelete;

    private TextButtonAdapter mAdapterMode;
    private ConfigTconInfo mTconSrcInfo;
    private ConfigTconInfo mTconUsbInfo;

    private ConfigBehaviorInfo mBehaviorSrcInfo;
    private ConfigBehaviorInfo mBehaviorUsbInfo;

    private TextView mTextOperator;
    private Spinner mSpinnerOperator;

    private boolean mMounted = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_top);

        //
        //  Common Variable
        //
        mCinemaInfo = (CinemaInfo)getApplicationContext();

        final ViewGroup rootGroup = null;
        View listViewFooter = null;
        LayoutInflater inflater = (LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        if( inflater != null ) listViewFooter = inflater.inflate(R.layout.listview_footer_blank, rootGroup, false);

        //
        //  Configuration Title Bar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.titleBarLayoutTop ));
        titleBar.SetTitle( "Cinema LED Display System - Top Menu" );
        titleBar.SetListener(VdTitleBar.BTN_ROTATE, new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ChangeScreenRotation();
            }
        });

        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Launch(v.getContext(), LoginActivity.class);
            }
        });
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TurnOff();
            }
        });

        if( !mCinemaInfo.IsEnableRotate() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_ROTATE, View.GONE);
        }

        if( !mCinemaInfo.IsEnableExit() ) {
            titleBar.SetVisibility(VdTitleBar.BTN_EXIT, View.GONE);
        }

        RegisterBroadcastReceiver(mBroadcastReceiver);

        //
        //  Configuration Status Bar
        //
        VdStatusBar statusBar = new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.statusBarLayoutTop) );
        statusBar.SetListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SetDevelMode();
                mTextOperator.setVisibility( IsDevelMode() ? View.VISIBLE : View.INVISIBLE );
                mSpinnerOperator.setVisibility( IsDevelMode() ? View.VISIBLE : View.INVISIBLE );
                mSpinnerOperator.setEnabled( IsDevelMode() );
            }
        });

        //
        //  Alert Message
        //
        String strCabinetNum = mCinemaInfo.GetValue(CinemaInfo.KEY_CABINET_NUM);
        int configCabinetNum = Integer.parseInt( (strCabinetNum != null) ? strCabinetNum : "0" );
        int detectCabientNum = mCinemaInfo.GetCabinet().length;
        if( configCabinetNum != detectCabientNum && mCinemaInfo.IsCheckCabinetNum() ) {
            String strMessage = String.format(Locale.US, "Please Check Cabinet Number. ( setting: %d, detect: %d )", configCabinetNum, detectCabientNum);
            CinemaAlert.Show( TopActivity.this, "Alert",  strMessage );
        }

        //
        //
        //
        TextView textComment = (TextView)findViewById(R.id.textCommentTop);
        textComment.setText("");
        textComment.append(String.format(Locale.US, "-. TCON Config File\t\t\t\t\t: [USB_TOP]/%s/%s\n", ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME));
        textComment.append(String.format(Locale.US, "-. TCON Behavior Path\t\t\t\t: [USB_TOP]/%s/\n", ConfigBehaviorInfo.PATH_SOURCE));
        textComment.append(String.format(Locale.US, "-. TCON Behavior Extract Path\t: [USB_TOP]/%s/\n", ConfigBehaviorInfo.PATH_EXTRACT));
        textComment.append(String.format(Locale.US, "-. Gamma File\t\t\t\t\t\t: [USB_TOP]/%s/%s\n", LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME));
        textComment.append(String.format(Locale.US, "-. PFPGA Config File\t\t\t\t: [USB_TOP]/%s/%s\n", ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME));
        textComment.append(String.format(Locale.US, "-. Uniformity File\t\t\t\t\t: [USB_TOP]/%s/%s\n", LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME));
        textComment.append(String.format(Locale.US, "-. Pixel Correct Path\t\t\t\t: [USB_TOP]/%s/IDxxx/\n", LedDotCorrectInfo.PATH));
        textComment.append(String.format(Locale.US, "-. Pixel Correct Extract Path\t\t: [USB_TOP]/%s/IDxxx/", LedDotCorrectInfo.PATH_EXTRACT));

        //
        //  EEPROM Status
        //
        TextView textEEPRomStatus = (TextView)findViewById(R.id.textEEPRomStatus);
        textEEPRomStatus.setText( mCinemaInfo.IsValidEEPRom() ? "VALID" : "INVALID"  );

        //
        //  Screen Type
        //
        final TextView textScreenType = (TextView)findViewById(R.id.textScreenType);
        textScreenType.setText( mCinemaInfo.GetScreenType() == CinemaInfo.SCREEN_TYPE_P33 ? "P3.3" : "P2.5" );

        Button btnScreenType = (Button)findViewById(R.id.btnScreenType);
        btnScreenType.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_SCREEN_TYPE,
                        getApplicationContext(),
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                if( !(values instanceof Integer[]) )
                                    return;

                                textScreenType.setText((Integer)values[0] == CinemaInfo.SCREEN_TYPE_P33 ? "P3.3" : "P2.5");
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        //
        //  Screen On
        //
        mBtnScreenOn = (Button)findViewById(R.id.btnScreenOn);
        if( mCinemaInfo.IsScreenOn() ) mBtnScreenOn.setText("SCREEN OFF");
        else mBtnScreenOn.setText("SCREEN ON");

        mBtnScreenOn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                final boolean mute = mBtnScreenOn.getText().toString().equals("SCREEN OFF");

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_SCREEN_MUTE,
                        getApplicationContext(),
                        mute,
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                mBtnScreenOn.setText( mute ? "SCREEN ON" : "SCREEN OFF" );
                                mCinemaInfo.SetScreenOn(mute);
                                HideProgress();
                            }
                        },
                        null
                );
            }
        });

        //
        //
        //
        String[] extPath = FileManager.GetExternalPath();
        mMounted = ((extPath != null) && (extPath.length > 0));

        //
        //  Parse TCON Configuration Files.
        //
        ListView listViewMode = (ListView)findViewById(R.id.listView_mode_apply);
        listViewMode.addFooterView(listViewFooter);

        mAdapterMode = new TextButtonAdapter(this, R.layout.listview_row_text_button);
        listViewMode.setAdapter( mAdapterMode );
        mAdapterMode.clear();
        for( int i = 0; i < 30; i++ ) {
            mAdapterMode.add(new TextButtonInfo(String.format(Locale.US, "mode #%d", i + 1), ""));

            TextButtonInfo info = mAdapterMode.getItem(i);
            if( null == info )
                continue;

            info.SetOnClickListener( new TextButtonAdapter.OnClickListener[]{
                    mUpdateListener,
                    mDeleteListener,
                    mDownloadListener,
                    mApplyListener,
            });

            mAdapterMode.notifyDataSetChanged();
        }

        String strTemp = mCinemaInfo.GetValue(CinemaInfo.KEY_INITIAL_MODE);
        mInitMode = (strTemp == null) ? 0 : Integer.parseInt(strTemp);

        mTextInitMode = (TextView)findViewById(R.id.textInitModeCurrent);
        mTextInitMode.setText( (0 > mInitMode) ?
                "INVALID" :
                String.format(Locale.US, "Mode #%d", mInitMode + 1)
        );

        mBtnUpdate = (Button)findViewById(R.id.btnInitModeUpdate);
        mBtnUpdate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if( mCinemaInfo.IsConfigDevelMode() ) {
                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_COPY_FILE,
                            getApplicationContext(),
                            new String[][]{
                                    {ConfigPfpgaInfo.PATH_SOURCE, ConfigPfpgaInfo.NAME},
                                    {LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME},
                                    {ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME},
                                    {LedGammaInfo.PATH_SOURCE, LedGammaInfo.PATTERN_NAME},
                            },
                            new String[][]{
                                    {ConfigPfpgaInfo.PATH_TARGET, null},
                                    {LedUniformityInfo.PATH_TARGET, null},
                                    {ConfigTconInfo.PATH_TARGET_USB, null},
                                    {LedGammaInfo.PATH_TARGET_USB, null},
                            },
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute(Object[] values) {
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute(Object[] values) {
                                    ShowMessage( "Update Initial Value File.");

                                    UpdateInitialValue();
                                    HideProgress();
                                }
                            },
                            new CinemaTask.ProgressUpdateCallback() {
                                @Override
                                public void onProgressUpdate(Object[] values) {
                                    if( !(values instanceof String[]) )
                                        return;

                                    Log.i(VD_DTAG, String.format( Locale.US, "Copy File. ( %s -> %s )", values[0], values[1]));
                                }
                            }
                    );
                }
                else {
                    for( int i = 10; i < 30; i++ ) {
                        for( int j = 0; j < 3; j++ ) {
                            final int pos = i;
                            final int color = j;

                            if( !mBehaviorUsbInfo.IsValid(pos, color) )
                                continue;

                            CinemaTask.GetInstance().Run(
                                    CinemaTask.CMD_CONFIG_UPLOAD,
                                    getApplicationContext(),
                                    mBehaviorUsbInfo.GetByte(pos, color),
                                    new CinemaTask.PreExecuteCallback() {
                                        @Override
                                        public void onPreExecute(Object[] values) {
                                            ShowProgress();
                                        }
                                    },
                                    new CinemaTask.PostExecuteCallback() {
                                        @Override
                                        public void onPostExecute(Object[] values) {
                                            if( !(values instanceof Integer[]) )
                                                return;

                                            if( 0xFF != (Integer)values[0] ) {
                                                Log.i(VD_DTAG, String.format( Locale.US, ">> Upload Behavior File. ( mode: %d )", (Integer)values[0] ));

                                                mTconSrcInfo.Update(
                                                        String.format( Locale.US, "%s/T_REG_MODE%02d.txt",
                                                                ConfigTconInfo.PATH_TARGET_USB, pos),
                                                        pos
                                                );

                                                mBehaviorSrcInfo.Update(
                                                        String.format( Locale.US, "%s/BEHAVIOR_%s_MODE%02d.txt",
                                                                ConfigBehaviorInfo.PATH_TARGET, (color == 0) ? "R" : ((color==1) ? "G" : "B"), pos)
                                                );

                                                ShowMessage( "Update Initial Value File.");
                                                UpdateInitialValue();
                                            }

                                            HideProgress();
                                        }
                                    },
                                    null
                            );
                        }
                    }
                }
            }
        });

        mBtnDelete = (Button)findViewById(R.id.btnInitModeDelete);
        mBtnDelete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_REMOVE_FILE,
                        getApplicationContext(),
                        new String[][]{
                                {ConfigPfpgaInfo.PATH_TARGET,   ConfigPfpgaInfo.NAME},
                                {ConfigTconInfo.PATH_TARGET_USB,ConfigTconInfo.NAME},
                        },
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                UpdateInitialValue();
                                HideProgress();
                            }
                        },
                        new CinemaTask.ProgressUpdateCallback() {
                            @Override
                            public void onProgressUpdate(Object[] values) {
                                if( !(values[0] instanceof String) )
                                    return;

                                Log.i(VD_DTAG, String.format( Locale.US, "Remove File. ( %s )", values[0]));
                            }
                        }
                );

                if( !mCinemaInfo.IsConfigDevelMode() ) {
                    for( int i = 10; i < 30; i++ ) {
                        if( !mBehaviorSrcInfo.IsValid(i) )
                            continue;

                        final int pos = i;
                        final TextButtonInfo info = mAdapterMode.getItem(pos);
                        if( null == info )
                            continue;

                        CinemaTask.GetInstance().Run(
                                CinemaTask.CMD_CONFIG_DELETE,
                                getApplicationContext(),
                                pos,
                                new CinemaTask.PreExecuteCallback() {
                                    @Override
                                    public void onPreExecute(Object[] values) {
                                        ShowProgress();
                                    }
                                },
                                new CinemaTask.PostExecuteCallback() {
                                    @Override
                                    public void onPostExecute(Object[] values) {
                                        if( !(values instanceof Integer[]) )
                                            return;

                                        if( (Integer)values[0] != CinemaInfo.RET_PASS ) {
                                            mBehaviorSrcInfo.Delete(pos);
                                            mTconSrcInfo.Delete(pos);

                                            info.SetText(mTconSrcInfo.GetDescription(pos));
                                            info.SetTextEnable( IsTextEnable(pos) );
                                            info.SetBtnEnable( 1, IsDelete(pos) );
                                            info.SetBtnEnable( 2, IsDownload(pos) );
                                            info.SetBtnEnable( 3, IsApply(pos) );
                                            mAdapterMode.notifyDataSetChanged();

                                            UpdateBtnDelete();
                                        }
                                        HideProgress();
                                    }
                                },
                                null
                        );
                    }
                }
            }
        });

        Log.i(VD_DTAG, String.format( Locale.US, ">>>> IsDevelMode() : %b, IsConfigDevelMode() : %b", IsDevelMode() , mCinemaInfo.IsConfigDevelMode() ));

        mTextOperator = (TextView)findViewById(R.id.textOperator);
        mTextOperator.setVisibility( IsDevelMode() ? View.VISIBLE : View.INVISIBLE );

        mSpinnerOperator = (Spinner)findViewById(R.id.spinnerOperator);
        mSpinnerOperator.setAdapter(
                new ArrayAdapter<>(
                        this,
                        android.R.layout.simple_spinner_dropdown_item,
                        new String[]{ "User", "Developer" }
                )
        );

        mSpinnerOperator.setVisibility( IsDevelMode() ? View.VISIBLE : View.INVISIBLE );
        mSpinnerOperator.setEnabled( IsDevelMode() );
        mSpinnerOperator.setSelection( mCinemaInfo.IsConfigDevelMode() ? 1 : 0 );

        mSpinnerOperator.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                mCinemaInfo.SetConfigDevelMode( i == 1 );
                UpdateInitialValue();
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });

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

        Log.i(VD_DTAG, "--> Login Group : " + mCinemaInfo.GetUserGroup());
        if( !mCinemaInfo.IsCheckLogin() )
            btnMenuAccount.setEnabled(false);

        if( mCinemaInfo.GetUserGroup().equals(AccountPreference.GROUP_OPERATOR) ) {
            btnMenuDiagnostics.setEnabled(false);
            btnMenuDisplayCheck.setEnabled(false);
            btnMenuSystem.setEnabled(false);
            btnMenuAccount.setEnabled(false);
        }

        if( mCinemaInfo.GetUserGroup().equals(AccountPreference.GROUP_CALIBRATOR) ) {
            btnMenuSystem.setEnabled(false);
        }

        this.RegisterTmsCallback(new CinemaService.TmsEventCallback() {
            @Override
            public void onTmsEventCallback(Object[] values) {
                if( !(values instanceof Integer[]) )
                    return;

                if( 0 > (Integer)values[0] ) {
                    return;
                }

                if( CinemaTask.TMS_MODE_CHANGE <= (Integer)values[0] &&
                    CinemaTask.TMS_MODE_DELETE > (Integer)values[0] ) {
                    mInitMode = (Integer)values[0];
                    mTextInitMode.setText(String.format(Locale.US, "Mode #%d", mInitMode +1));
                }

                if( CinemaTask.TMS_SCREEN_CHANGE <= (Integer)values[0] &&
                    CinemaTask.TMS_SCREEN > (Integer)values[0] ) {
                    UpdateInitialValue();
                }

                if( CinemaTask.TMS_SCREEN_ON  == (Integer)values[0] ||
                    CinemaTask.TMS_SCREEN_OFF == (Integer)values[0] ) {

                    boolean mute = (Integer)values[0] == CinemaTask.TMS_SCREEN_OFF;
                    mBtnScreenOn.setText(mute ? "ON" : "OFF");
                    mCinemaInfo.SetScreenOn(!mute);
                }

                if( CinemaTask.TMS_CONFIG_UPLOAD == (Integer)values[0] ) {
                    ParseTregSrc();
                    ParseBehaviorSrc();
                    UpdateAdapter();
                    UpdateBtnDelete();
                }

                if( CinemaTask.TMS_MODE_DELETE <= (Integer)values[0] &&
                    CinemaTask.TMS_MODE_DELETE_29 >= (Integer)values[0] ) {
                    ParseTregSrc();
                    ParseBehaviorSrc();
                    UpdateAdapter();
                    UpdateBtnDelete();
                }
            }
        });
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if( null == action )
                return;

            mMounted = false;
            mBtnUpdate.setEnabled(false);

            if( action.equals(Intent.ACTION_MEDIA_MOUNTED) ) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                mMounted = true;
            }

            ParseTregUsb();
            ParseBehaviorUsb();
            UpdateAdapter();

            UpdateBtnUpdate();
            UpdateBtnDelete();
        }
    };

    private TextButtonAdapter.OnClickListener mUpdateListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            if( mCinemaInfo.IsConfigDevelMode() ) {
                final int pos = position;

                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_CHECK_FILE,
                        getApplicationContext(),
                        new String[][]{
                                {ConfigTconInfo.PATH_SOURCE,    ConfigTconInfo.NAME},
                        },
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                UpdateBtnDelete();
                                HideProgress();
                            }
                        },
                        new CinemaTask.ProgressUpdateCallback() {
                            @Override
                            public void onProgressUpdate(Object[] values) {
                                if( !(values[0] instanceof String) )
                                    return;

                                mTconSrcInfo.Update( (String)values[0], pos );
                                mTconSrcInfo.Make(
                                        String.format( Locale.US, "%s/%s", ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME ),
                                        10,
                                        30
                                );

                                final TextButtonInfo info = mAdapterMode.getItem(pos);
                                if( null == info )
                                    return;

                                info.SetText(mTconSrcInfo.GetDescription(pos));
                                info.SetTextEnable( IsTextEnable(pos) );
                                info.SetBtnEnable( 0, IsUpdate(pos) );
                                info.SetBtnEnable( 1, IsDelete(pos) );
                                info.SetBtnEnable( 2, IsDownload(pos) );
                                info.SetBtnEnable( 3, IsApply(pos) );

                                mAdapterMode.notifyDataSetChanged();
                            }
                        }
                );
            }
            else {
                for( int i = 0; i < 3; i++ ) {
                    final int pos = position;
                    final int color = i;

                    if( !mBehaviorUsbInfo.IsValid(pos, color) )
                        continue;

                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_CONFIG_UPLOAD,
                            getApplicationContext(),
                            mBehaviorUsbInfo.GetByte(pos, color),
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute(Object[] values) {
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute(Object[] values) {
                                    if( 0xFF != (Integer)values[0] ) {
                                        mTconSrcInfo.Update(
                                                String.format( Locale.US, "%s/T_REG_MODE%02d.txt", ConfigTconInfo.PATH_TARGET_USB, pos),
                                                pos
                                        );

                                        mBehaviorSrcInfo.Update(
                                                String.format( Locale.US, "%s/BEHAVIOR_%s_MODE%02d.txt",
                                                        ConfigBehaviorInfo.PATH_TARGET, (color == 0) ? "R" : ((color==1) ? "G" : "B"), pos)
                                        );

                                        final TextButtonInfo info = mAdapterMode.getItem(pos);
                                        if( null == info )
                                            return;

                                        info.SetText(mTconSrcInfo.GetDescription(pos));
                                        info.SetTextEnable( IsTextEnable(pos) );
                                        info.SetBtnEnable( 0, IsUpdate(pos) );
                                        info.SetBtnEnable( 1, IsDelete(pos) );
                                        info.SetBtnEnable( 2, IsDownload(pos) );
                                        info.SetBtnEnable( 3, IsApply(pos) );

                                        mAdapterMode.notifyDataSetChanged();
                                        UpdateBtnDelete();
                                    }
                                    HideProgress();
                                }
                            },
                            null
                    );
                }
            }
        }
    };


    private TextButtonAdapter.OnClickListener mDeleteListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            final int pos = position;
            final TextButtonInfo info = mAdapterMode.getItem(position);
            if( null == info )
                return;

            if( mCinemaInfo.IsConfigDevelMode() ) {
                mTconSrcInfo.Delete(pos);
                mTconSrcInfo.Make(
                        String.format( Locale.US, "%s/%s", ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME ),
                        10,
                        30
                );

                info.SetText(mTconSrcInfo.GetDescription(position));
                info.SetTextEnable( IsTextEnable(position) );
                info.SetBtnEnable( 1, IsDelete(position) );
                info.SetBtnEnable( 2, IsDownload(position) );
                info.SetBtnEnable( 3, IsApply(position) );
                mAdapterMode.notifyDataSetChanged();

                UpdateBtnDelete();
            }
            else {
                CinemaTask.GetInstance().Run(
                        CinemaTask.CMD_CONFIG_DELETE,
                        getApplicationContext(),
                        pos,
                        new CinemaTask.PreExecuteCallback() {
                            @Override
                            public void onPreExecute(Object[] values) {
                                ShowProgress();
                            }
                        },
                        new CinemaTask.PostExecuteCallback() {
                            @Override
                            public void onPostExecute(Object[] values) {
                                if( !(values instanceof Integer[]) )
                                    return;

                                if( (Integer)values[0] != CinemaInfo.RET_PASS ) {
                                    mBehaviorSrcInfo.Delete(pos);
                                    mTconSrcInfo.Delete(pos);

                                    info.SetText(mTconSrcInfo.GetDescription(pos));
                                    info.SetTextEnable( IsTextEnable(pos) );
                                    info.SetBtnEnable( 1, IsDelete(pos) );
                                    info.SetBtnEnable( 2, IsDownload(pos) );
                                    info.SetBtnEnable( 3, IsApply(pos) );
                                    mAdapterMode.notifyDataSetChanged();

                                    UpdateBtnDelete();
                                }
                                HideProgress();
                            }
                        },
                        null
                );
            }
        }
    };

    private TextButtonAdapter.OnClickListener mDownloadListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            if( mCinemaInfo.IsConfigDevelMode() ) {
                String[] extPath = FileManager.GetExternalPath();
                if (extPath == null || extPath.length == 0)
                    return;

                String szPath = String.format(Locale.US, "%s/%s", extPath[0], ConfigTconInfo.PATH_SOURCE);
                FileManager.MakeDirectory(szPath);

                mTconSrcInfo.Make(
                        String.format( Locale.US, "%s/T_REG_MODE%02d.txt", szPath, position ),
                        position,
                        position
                );
            }
            else {
                String[] extPath = FileManager.GetExternalPath();
                if (extPath == null || extPath.length == 0)
                    return;

                final String szPath = String.format(Locale.US, "%s/%s", extPath[0], ConfigBehaviorInfo.PATH_EXTRACT);
                FileManager.MakeDirectory(szPath);

                for( int i = 0; i < 3; i++ ) {
                    if( !mBehaviorSrcInfo.IsValid( position, i ) )
                        continue;

                    final int pos = position;
                    final int color = i;

                    CinemaTask.GetInstance().Run(
                            CinemaTask.CMD_CONFIG_DOWNLOAD,
                            getApplicationContext(),
                            pos,
                            color,
                            new CinemaTask.PreExecuteCallback() {
                                @Override
                                public void onPreExecute(Object[] values) {
                                    ShowProgress();
                                }
                            },
                            new CinemaTask.PostExecuteCallback() {
                                @Override
                                public void onPostExecute(Object[] values) {
                                    if( !(values instanceof Byte[]) )
                                        return;

                                    Byte[] result = (Byte[])values;
                                    byte[] data = new byte[result.length];
                                    for( int i = 0; i < result.length; i++ )
                                        data[i] = result[i];

                                    FileManager.WriteByte(
                                            String.format( Locale.US, "%s/Mode%02d_%s_%s.txt",
                                                    szPath, pos, mBehaviorSrcInfo.GetDesc(pos, color), (color==0) ? "R" : ((color==1) ? "G" : "B")),
                                            data
                                    );

                                    HideProgress();
                                }
                            },
                            null
                    );
                }
            }
        }
    };

    private TextButtonAdapter.OnClickListener mApplyListener = new TextButtonAdapter.OnClickListener() {
        @Override
        public void onClickListener(int position) {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHANGE_MODE,
                    getApplicationContext(),
                    position,
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            if( !(values instanceof Integer[]) )
                                return;

                            if( 0 > (Integer)values[0] )
                                Log.i(VD_DTAG, "Fail, Change Mode.");
                            else {
                                mInitMode = (Integer)values[0];
                                mTextInitMode.setText(String.format(Locale.US, "Mode #%d", mInitMode + 1));

                                Log.i(VD_DTAG, String.format("Change Mode Done. ( mode = %d )", (Integer)values[0] + 1));
                            }
                            HideProgress();
                        }
                    },
                    null
            );
        }
    };

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch( (v).getId() ) {
                case R.id.btnMenuDiagonostics:
                    Launch(v.getContext(), DiagnosticsActivity.class);
                    break;
                case R.id.btnMenuDisplayCheck:
                    Launch(v.getContext(), DisplayCheckActivity.class);
                    break;
                case R.id.btnMenuDisplayMode:
                    Launch(v.getContext(), DisplayModeActivity.class);
                    break;
                case R.id.btnMenuSystem:
                    Launch(v.getContext(), SystemActivity.class);
                    break;
                case R.id.btnMenuAccount:
                    Launch(v.getContext(), AccountActivity.class);
                    break;
            }
        }
    };

    private void UpdateInitialValue() {
        ParseTregSrc();
        ParseTregUsb();
        ParseBehaviorSrc();
        ParseBehaviorUsb();
        UpdateAdapter();

        UpdateBtnUpdate();
        UpdateBtnDelete();
    }

    private void ParseTregSrc() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_PARSE_TREG,
                getApplicationContext(),
                new String[][]{
                        {ConfigTconInfo.PATH_TARGET_EEPROM, ConfigTconInfo.NAME},
                        {ConfigTconInfo.PATH_TARGET_USB, ConfigTconInfo.NAME},
                },
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        mTconSrcInfo = (ConfigTconInfo)(values[0]);

                        for( int i = 0; i < 30; i++ ) {
                            TextButtonInfo info = mAdapterMode.getItem(i);
                            if( null == info )
                                continue;

                            info.SetText(mTconSrcInfo.GetDescription(i));
                            info.SetTextEnable( IsTextEnable(i) );
                            info.SetBtnEnable( 1, IsDelete(i) );
                            info.SetBtnEnable( 2, IsDownload(i) );
                            info.SetBtnEnable( 3, IsApply(i) );

                            mAdapterMode.notifyDataSetChanged();
                        }

                        HideProgress();
                    }
                },
                null
        );
    }

    private void ParseTregUsb() {
        if( mCinemaInfo.IsConfigDevelMode() ) {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_PARSE_TREG,
                    getApplicationContext(),
                    new String[][]{
                            {ConfigTconInfo.PATH_SOURCE, ConfigTconInfo.NAME},
                    },
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            mTconUsbInfo = (ConfigTconInfo)(values[0]);

                            for( int i = 0; i < 30; i++ ) {
                                TextButtonInfo info = mAdapterMode.getItem(i);
                                if( null == info )
                                    continue;

                                info.SetBtnEnable( 0, IsUpdate(i) );
                                mAdapterMode.notifyDataSetChanged();
                            }

                            HideProgress();
                        }
                    },
                    null
            );
        }
    }

    private void ParseBehaviorSrc() {
        if( !mCinemaInfo.IsConfigDevelMode() ) {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_PARSE_BEHAVIOR,
                    getApplicationContext(),
                    new String[]{ConfigBehaviorInfo.PATH_TARGET},
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            mBehaviorSrcInfo = (ConfigBehaviorInfo)(values[0]);

                            for( int i = 0; i < 30; i++ ) {
                                TextButtonInfo info = mAdapterMode.getItem(i);
                                if( null == info )
                                    continue;

                                info.SetBtnEnable( 1, IsDelete(i) );
                                info.SetBtnEnable( 2, IsDownload(i) );

                                mAdapterMode.notifyDataSetChanged();
                            }

                            HideProgress();
                        }
                    },
                    null
            );
        }
    }

    private void ParseBehaviorUsb() {
        if( !mCinemaInfo.IsConfigDevelMode() ) {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_PARSE_BEHAVIOR,
                    getApplicationContext(),
                    new String[]{ConfigBehaviorInfo.PATH_SOURCE},
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            mBehaviorUsbInfo = (ConfigBehaviorInfo)(values[0]);

                            for( int i = 0; i < 30; i++ ) {
                                TextButtonInfo info = mAdapterMode.getItem(i);
                                if (null == info)
                                    continue;

                                info.SetBtnEnable(0, IsUpdate(i));
                                mAdapterMode.notifyDataSetChanged();
                            }
                            HideProgress();
                        }
                    },
                    null
            );
        }
    }

    private void UpdateAdapter() {
        CinemaTask.GetInstance().Run(
                CinemaTask.CMD_POST_EXECUTE,
                getApplicationContext(),
                0,
                new CinemaTask.PreExecuteCallback() {
                    @Override
                    public void onPreExecute(Object[] values) {
                        ShowProgress();
                    }
                },
                new CinemaTask.PostExecuteCallback() {
                    @Override
                    public void onPostExecute(Object[] values) {
                        for( int i = 0; i < 30; i++ ) {
                            TextButtonInfo info = mAdapterMode.getItem(i);
                            if( null == info )
                                continue;

                            info.SetText(mTconSrcInfo.GetDescription(i));
                            info.SetTextEnable( IsTextEnable(i) );
                            info.SetBtnEnable( 0, IsUpdate(i) );
                            info.SetBtnEnable( 1, IsDelete(i) );
                            info.SetBtnEnable( 2, IsDownload(i) );
                            info.SetBtnEnable( 3, IsApply(i) );

                            mAdapterMode.notifyDataSetChanged();
                        }

                        HideProgress();
                    }
                },
                null
        );
    }

    private void UpdateBtnUpdate() {
        if( mCinemaInfo.IsConfigDevelMode() ) {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHECK_FILE,
                    getApplicationContext(),
                    new String[][]{
                            {ConfigPfpgaInfo.PATH_SOURCE,   ConfigPfpgaInfo.NAME},
                            {LedUniformityInfo.PATH_SOURCE, LedUniformityInfo.NAME},
                            {ConfigTconInfo.PATH_SOURCE,    ConfigTconInfo.NAME},
                            {LedGammaInfo.PATH_SOURCE,      LedGammaInfo.PATTERN_NAME},
                    },
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                            mBtnUpdate.setEnabled(false);
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            HideProgress();
                        }
                    },
                    new CinemaTask.ProgressUpdateCallback() {
                        @Override
                        public void onProgressUpdate(Object[] values) {
                            if( !(values[0] instanceof String) )
                                return;

                            Log.i(VD_DTAG, String.format(Locale.US, "Detection File. ( %s )", values[0] ));
                            mBtnUpdate.setEnabled(true);
                        }
                    }
            );
        }
        else {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHECK_FILE_IN_DIRECTORY,
                    getApplicationContext(),
                    new String[]{
                            ConfigBehaviorInfo.PATH_SOURCE,
                    },
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                            mBtnUpdate.setEnabled(false);
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            HideProgress();
                        }
                    },
                    new CinemaTask.ProgressUpdateCallback() {
                        @Override
                        public void onProgressUpdate(Object[] values) {
                            if( !(values[0] instanceof String) )
                                return;

                            Log.i(VD_DTAG, String.format(Locale.US, "Detection File. ( %s )", values[0] ));
                            mBtnUpdate.setEnabled(true);
                        }
                    }
            );
        }
    }

    private void UpdateBtnDelete() {
        if( mCinemaInfo.IsConfigDevelMode() ) {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHECK_FILE,
                    getApplicationContext(),
                    new String[][]{
                            {ConfigPfpgaInfo.PATH_TARGET,   ConfigPfpgaInfo.NAME},
                            {ConfigTconInfo.PATH_TARGET_USB,ConfigTconInfo.NAME},
                    },
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                            mBtnDelete.setEnabled(false);
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            HideProgress();
                        }
                    },
                    new CinemaTask.ProgressUpdateCallback() {
                        @Override
                        public void onProgressUpdate(Object[] values) {
                            if( !(values[0] instanceof String) )
                                return;

                            Log.i(VD_DTAG, String.format(Locale.US, "Detection File. ( %s )", values[0] ));
                            mBtnDelete.setEnabled(true);
                        }
                    }
            );
        }
        else {
            CinemaTask.GetInstance().Run(
                    CinemaTask.CMD_CHECK_FILE_IN_DIRECTORY,
                    getApplicationContext(),
                    new String[]{
                            ConfigBehaviorInfo.PATH_TARGET,
                    },
                    new CinemaTask.PreExecuteCallback() {
                        @Override
                        public void onPreExecute(Object[] values) {
                            ShowProgress();
                            mBtnDelete.setEnabled(false);
                        }
                    },
                    new CinemaTask.PostExecuteCallback() {
                        @Override
                        public void onPostExecute(Object[] values) {
                            HideProgress();
                        }
                    },
                    new CinemaTask.ProgressUpdateCallback() {
                        @Override
                        public void onProgressUpdate(Object[] values) {
                            if( !(values[0] instanceof String) )
                                return;

                            Log.i(VD_DTAG, String.format(Locale.US, "Detection File. ( %s )", values[0] ));
                            mBtnDelete.setEnabled(true);
                        }
                    }
            );
        }
    }

    private boolean IsTextEnable(int pos) {
        return (mTconSrcInfo.IsValid(pos)) &&
                ( (mTconSrcInfo.GetDataMode(pos) == ConfigTconInfo.MODE_BOTH) ||
                  (mTconSrcInfo.GetDataMode(pos) == ConfigTconInfo.MODE_3D && mCinemaInfo.IsMode3D()));
    }

    private boolean IsUpdate(int pos) {
        if( mCinemaInfo.IsConfigDevelMode() ) {
            return (10 <= pos) && mTconUsbInfo.IsValid(pos);
        }

        return (10 <= pos) && mBehaviorUsbInfo.IsValid(pos);
    }

    private boolean IsDelete(int pos) {
        if( mCinemaInfo.IsConfigDevelMode() ) {
            return (10 <= pos) && (mTconSrcInfo != null) && mTconSrcInfo.IsValid(pos);
        }

        return (10 <= pos) && (mBehaviorSrcInfo != null) && mBehaviorSrcInfo.IsValid(pos);
    }

    private boolean IsDownload(int pos) {
        if( mCinemaInfo.IsConfigDevelMode() ) {
            return (10 <= pos) && ((mBehaviorSrcInfo != null) && mTconSrcInfo.IsValid(pos)) && mMounted;
        }

        return (10 <= pos) && ((mBehaviorSrcInfo != null) && mBehaviorSrcInfo.IsValid(pos)) && mMounted;
    }

    private boolean IsApply(int pos) {
        return (mTconSrcInfo.IsValid(pos)) &&
                ( (mTconSrcInfo.GetDataMode(pos) == ConfigTconInfo.MODE_BOTH) ||
                  (mTconSrcInfo.GetDataMode(pos) == ConfigTconInfo.MODE_3D && mCinemaInfo.IsMode3D()));
    }
}
