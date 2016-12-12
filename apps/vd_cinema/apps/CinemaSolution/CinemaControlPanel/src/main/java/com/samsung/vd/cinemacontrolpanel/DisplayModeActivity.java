package com.samsung.vd.cinemacontrolpanel;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;

import com.samsung.vd.baseutils.VdStatusBar;
import com.samsung.vd.baseutils.VdTitleBar;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Date;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by doriya on 8/16/16.
 */
public class DisplayModeActivity extends AppCompatActivity {
    private final String VD_DTAG = "DisplayModeActivity";

    private LayoutInflater  mInflater;
    private LinearLayout    mParentLayoutMastering;
    private LinearLayout[]  mLayoutMastering;

    private byte[]  mCabinet;

    private String[] mStrModeMastering = {
            "DCI", "HDR", "MASTER"
    };

    private String[][] mTextMastering = {
            { "CC00",           "16383" },
            { "CC01",           "16383" },
            { "CC02",           "16383" },
            { "CC10",           "16383" },
            { "CC11",           "16383" },
            { "CC12",           "16383" },
            { "CC20",           "16383" },
            { "CC21",           "16383" },
            { "CC22",           "16383" },
            { "Global",         "7"     },
            { "R Channel",      "511"   },
            { "G Channel",      "511"   },
            { "B Channel",      "511"   },
            { "R Channel(Low)", "15"    },
            { "G Channel(Low)", "15"    },
            { "B Channel(Low)", "15"    },
    };

    private int[] mRegHideMastering = {
            0x0082, 0x0097,
    };

    private int[][] mDataHideMastering = {
            { 0x0000, 0x0000 },
            { 0x0001, 0x0001 },
            { 0x0000, 0x0000 },
    };

    private int[] mRegMastering = {
            0x0056,     0x0057,     0x0058,     //  REG_CC00,       REG_CC01,       REG_CC02
            0x0059,     0x005A,     0x005B,     //  REG_CC10,       REG_CC11,       REG_CC12
            0x005C,     0x005D,     0x005E,     //  REG_CC20,       REG_CC21,       REG_CC22
            0x00DD,                             //  REG_BC
            0x00B9,     0x00BA,     0x00BB,     //  REG_CC_R01,     REG_CC_G01,     REG_CC_B01
            0x00B6,     0x00B7,     0x00B8,     //  REG_LGSE1_R,    REG_LGSE1_G,    REG_LGSE1_B
    };

    private int[] mDataDci = {
            6361,       74,         269,
            272,        6586,       29,
            0,          128,        6701,
            1,
            145,        98,         67,
            14,         15,         12,
    };

    private int[] mDataHdr = {
            12869,      1989,       507,
            607,        15065,      177,
            0,          0,          16383,
            4,
            252,        234,        111,
            5,          5,          8,
    };

    private int[] mDataMaster = new int[mTextMastering.length];

    private int[][] mDataMastering = {
            mDataDci,
            mDataHdr,
            mDataMaster,
    };

    private Spinner mSpinnerMastering;
    private Button[] mBtnMastering = new Button[mTextMastering.length];
    private SeekBar[] mSeekBarMastering = new SeekBar[mTextMastering.length];
    private TextView[] mValueMastering = new TextView[mTextMastering.length];

    private String mMasteringMode = mStrModeMastering[0];
    private int mMasteringModePos = 0;

    private Spinner mSpinnerImageQuality;
    private Button mBtnImageQuality;

    private StatusSimpleAdapter mAdapterDot;

    private EditText mEditCabinet;

    private String[] strInputEnable = {
            "Enable",   "Disable",
    };

    private String[] mStrInputResolution = {
            "FHD ( 1920 x 1080 )",  "2K ( 2048 x 1080 ",    "4K ( 4096 x 2160 )",
    };

    private String[] mStrInputSource = {
            "IMB",  "HDMI", "3G-SDI",
    };

    private String mInputResolution;
    private String mInputSource;

    private Spinner mSpinnerInputEnable;
    private Spinner mSpinnerInputReoslution;
    private Spinner mSpinnerInputSource;

    private Button mBtnDotCorrection;

    private Spinner mSpinnerYear;
    private Spinner mSpinnerMonth;
    private Spinner mSpinnerDay;
    private Spinner mSpinnerHour;
    private Spinner mSpinnerMin;

    private Spinner mSpinnerSuspendTime;

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if( intent.getAction().equals(Intent.ACTION_MEDIA_MOUNTED) ) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                String[] resultQuality = CheckFileInUsb(LedQualityInfo.PATH, LedQualityInfo.NAME);
                String[] resultGamma = CheckFileInUsb(LedGammaInfo.PATH, LedGammaInfo.NAME);

                if( (resultQuality != null && resultQuality.length != 0) || (resultGamma != null && resultGamma.length != 0) ) {
                    mBtnImageQuality.setEnabled(true);
                }

                String[] resultDot = CheckFileInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.NAME);
                if( (resultDot != null && resultDot.length != 0) ) {
                    mBtnDotCorrection.setEnabled(true);
                }

            }
            if( intent.getAction().equals(Intent.ACTION_MEDIA_EJECT) ) {
                mBtnImageQuality.setEnabled(false);
                mBtnDotCorrection.setEnabled(false);
            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();

        //
        //  This is called after onCreate().
        //
        mLayoutMastering = new LinearLayout[mTextMastering.length];
        for( int i = 0; i < mTextMastering.length; i++ ) {
            mLayoutMastering[i] = (LinearLayout)mInflater.inflate(R.layout.layout_item_mastering, mParentLayoutMastering, false );
            AddViewMastering( i, mLayoutMastering, mTextMastering );
        }

        UpdateMasteringMode();

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
        setContentView(R.layout.activity_display_mode);

        //
        //  Configuration TitleBar
        //
        VdTitleBar titleBar = new VdTitleBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutTitleBar ));
        titleBar.SetTitle( "Cinema LED Display System - Display Mode" );
        titleBar.SetListener(VdTitleBar.BTN_BACK, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity( new Intent(v.getContext(), TopActivity.class) );
                overridePendingTransition(0, 0);
                finish();
            }
        });
        titleBar.SetListener(VdTitleBar.BTN_EXIT, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mService.TurnOff();
            }
        });

        //
        //  Configuration StatusBar
        //
        new VdStatusBar( getApplicationContext(), (LinearLayout)findViewById( R.id.layoutStatusBar) );

        //
        //  Cinema System Information
        //
        View listViewFooter = ((LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.listview_footer_blank, null, false);
        mCabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();
        AddTabs();


        //
        //  MASTERING
        //
        mInflater =  (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mParentLayoutMastering = (LinearLayout)findViewById(R.id.layoutMastering);

        mMasteringMode = ((CinemaInfo)getApplicationContext()).GetValue( CinemaInfo.KEY_MASTERING_MODE );
        if( mMasteringMode == null ) {
            mMasteringMode = mStrModeMastering[0];
            ((CinemaInfo) getApplicationContext()).SetValue(CinemaInfo.KEY_MASTERING_MODE, mMasteringMode);
        }

        mSpinnerMastering = (Spinner)findViewById(R.id.spinnerMasteringMode);
        mSpinnerMastering.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, mStrModeMastering));
        for( int i = 0; i < mStrModeMastering.length; i++ ) {
            if( mMasteringMode.equals(mStrModeMastering[i]) ) {
                mSpinnerMastering.setSelection(i);
                mMasteringModePos = i;
                break;
            }
        }

        Button btnMasteringMode = (Button)findViewById(R.id.btnMasteringMode);
        btnMasteringMode.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mMasteringMode = mSpinnerMastering.getSelectedItem().toString();
                mMasteringModePos = mSpinnerMastering.getSelectedItemPosition();

                ((CinemaInfo) getApplicationContext()).SetValue(CinemaInfo.KEY_MASTERING_MODE, mMasteringMode);
                if( !IsMasterMode() ) {
                    new AsyncTaskMasteringWrite().execute();
                }
                else {
                    new AsyncTaskMasteringWrite().execute();
                    new AsyncTaskMasteringRead().execute();
                }

                UpdateMasteringMode();

                ((CinemaInfo)getApplicationContext()).InsertLog(String.format( Locale.US, "Change Mastering Mode. ( %s Mode )", mMasteringMode ));
            }
        });


        //
        //  IMAGE QUALITY
        //
        String[] strImageQuality = {
                "1", "2", "3", "4"
        };

        mSpinnerImageQuality = (Spinner)findViewById(R.id.spinnerImageQuality);
        mSpinnerImageQuality.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strImageQuality));

        mBtnImageQuality = (Button)findViewById(R.id.btnImageQuality);
        mBtnImageQuality.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AsyncTaskImageQuality().execute();
            }
        });


        //
        //  DOT CORRECTION
        //
        ListView listViewDot= (ListView)findViewById(R.id.listview_dot_correction);
        listViewDot.addFooterView(listViewFooter);

        mAdapterDot = new StatusSimpleAdapter(this, R.layout.listview_row_status_simple);
        listViewDot.setAdapter( mAdapterDot );

//        for( byte cabinet : mCabinet ) {
//            mAdapterDot.add( new StatusSimpleInfo("Cabinet " + String.valueOf(cabinet - CinemaInfo.OFFSET_TCON)) );
//            mAdapterDot.notifyDataSetChanged();
//        }

        mBtnDotCorrection = (Button)findViewById(R.id.btnDotCorrection);
        mBtnDotCorrection.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AsyncTaskDotCorrection(mAdapterDot).execute();
            }
        });

        //
        //  INPUT SOURCE
        //
        mInputResolution = ((CinemaInfo)getApplicationContext()).GetValue( CinemaInfo.KEY_MASTERING_MODE );
        if( mInputResolution == null ) {
            mInputResolution = mStrInputResolution[0];
            ((CinemaInfo) getApplicationContext()).SetValue(CinemaInfo.KEY_INPUT_RESOLUTION, mInputResolution);
        }

        mInputSource = ((CinemaInfo)getApplicationContext()).GetValue( CinemaInfo.KEY_MASTERING_MODE );
        if( mInputSource == null ) {
            mInputSource = mStrInputSource[0];
            ((CinemaInfo) getApplicationContext()).SetValue(CinemaInfo.KEY_INPUT_SOURCE, mInputSource);
        }

        mSpinnerInputEnable = (Spinner)findViewById(R.id.spinnerInputSourceEnable);
        mSpinnerInputReoslution = (Spinner)findViewById(R.id.spinnerInputResolution);
        mSpinnerInputSource = (Spinner)findViewById(R.id.spinnerInputSource);

        mSpinnerInputEnable.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strInputEnable) );
        mSpinnerInputReoslution.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, mStrInputResolution) );
        mSpinnerInputSource.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, mStrInputSource) );

        for( int i = 0; i < mStrInputResolution.length; i++ ) {
            if( mInputResolution.equals(mStrInputResolution[i]) ) {
                mSpinnerInputReoslution.setSelection(i);
                break;
            }
        }

        for( int i = 0; i < mStrInputSource.length; i++ ) {
            if( mInputSource.equals(mStrInputSource[i]) ) {
                mSpinnerInputSource.setSelection(i);
                break;
            }
        }

        Button btnInputSourceEnableApply = (Button)findViewById(R.id.btnInputSourceEnableApply);
        btnInputSourceEnableApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ApplyInputSourceEnable(mSpinnerInputEnable.getSelectedItemPosition());
            }
        });

        Button btnInputSourceSelectApply = (Button)findViewById(R.id.btnInputSourceSelectApply);
        btnInputSourceSelectApply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mInputResolution = mSpinnerInputReoslution.getSelectedItem().toString();
                mInputSource = mSpinnerInputSource.getSelectedItem().toString();

                ((CinemaInfo) getApplicationContext()).SetValue(CinemaInfo.KEY_INPUT_RESOLUTION, mInputResolution);
                ((CinemaInfo) getApplicationContext()).SetValue(CinemaInfo.KEY_INPUT_SOURCE, mInputSource);

                ApplyInputSourceSelect(mSpinnerInputReoslution.getSelectedItemPosition(), mSpinnerInputSource.getSelectedItemPosition() );

                ((CinemaInfo)getApplicationContext()).InsertLog(String.format( Locale.US, "Change Input Source. ( %s / %s )", mInputResolution, mInputSource ));
            }
        });

        //
        //  Spinner Time
        //
        String[] strYear = new String[100];
        String[] strDay = new String[31];
        String[] strHour = new String[24];
        String[] strMin = new String[60];
        final String[] strMonth = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        for( int i = 0; i < strYear.length; i++ )   strYear[i] = String.valueOf(2000 + i);
        for( int i = 0; i < strDay.length; i++ )    strDay[i] = String.valueOf(1 + i);
        for( int i = 0; i < strHour.length; i++ )   strHour[i] = String.valueOf(i);
        for( int i = 0; i < strMin.length; i++ )    strMin[i] = String.valueOf(i);

        mSpinnerYear = (Spinner)findViewById(R.id.spinnerYear);
        mSpinnerMonth = (Spinner)findViewById(R.id.spinnerMonth);
        mSpinnerDay = (Spinner)findViewById(R.id.spinnerDay);
        mSpinnerHour = (Spinner)findViewById(R.id.spinnerHour);
        mSpinnerMin = (Spinner)findViewById(R.id.spinnerMinute);

        mSpinnerYear.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strYear));
        mSpinnerMonth.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strMonth));
        mSpinnerDay.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strDay));
        mSpinnerHour.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strHour));
        mSpinnerMin.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strMin));

        Button btnTime = (Button)findViewById(R.id.btnTimeApply);
        btnTime.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String cmd = String.format( Locale.US, "busybox date -s \"%04d-%02d-%02d %02d:%02d:00\"",
                    mSpinnerYear.getSelectedItemPosition() + 2000,
                    mSpinnerMonth.getSelectedItemPosition() + 1,
                    mSpinnerDay.getSelectedItemPosition() + 1,
                    mSpinnerHour.getSelectedItemPosition(),
                    mSpinnerMin.getSelectedItemPosition()
                );

                try {
                    WriteHelper( cmd );
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });


        //
        //  Spinner Cabinet Number
        //
        String curCabinetNum = ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM);

        mEditCabinet = (EditText)findViewById(R.id.editCabinet);
        mEditCabinet.setText(curCabinetNum);

        Button btnCabinetNum = (Button)findViewById(R.id.btnCabinetNumApply);
        btnCabinetNum.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new AsyncTaskCheckCabinet().execute();
            }
        });

        //
        //  Spinner Suspend Time
        //
        String[] strSuspendTime = {
            "Disable", "1 min", "3 min", "5 min", "10 min", "20 min", "30 min"
        };

        mSpinnerSuspendTime = (Spinner)findViewById(R.id.spinnerSuspendTime);
        mSpinnerSuspendTime.setAdapter( new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, strSuspendTime) );

        Button btnSuspendTime = (Button)findViewById(R.id.btnSuspendTimeApply);
        btnSuspendTime.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ShowMessage("Apply");
                ((CinemaInfo)getApplicationContext()).SetValue(CinemaInfo.KEY_SCREEN_SAVING, CinemaService.OFF_TIME[mSpinnerSuspendTime.getSelectedItemPosition()]);
                mService.RefreshScreenSaver();
            }
        });

        UpdateSetup();

        //
        //  IMM Handler
        //
        LinearLayout parent = (LinearLayout)findViewById(R.id.layoutParent);
        parent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditCabinet.getWindowToken(), 0);
            }
        });
    }

    private void AddTabs() {
        TabHost tabHost = (TabHost) findViewById(R.id.tabHost);
        tabHost.setup();

        TabHost.TabSpec tabSpec0 = tabHost.newTabSpec("TAB0");
        tabSpec0.setIndicator("Mastering Mode");
        tabSpec0.setContent(R.id.tabMasteringMode);

        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec("TAB1");
        tabSpec1.setIndicator("Image Quality");
        tabSpec1.setContent(R.id.tabImageQuality);

        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec("TAB2");
        tabSpec2.setIndicator("Dot Correction");
        tabSpec2.setContent(R.id.tabDotCorrection);

        TabHost.TabSpec tabSpec3 = tabHost.newTabSpec("TAB3");
        tabSpec3.setIndicator("Input Source");
        tabSpec3.setContent(R.id.tabInputSource);

        TabHost.TabSpec tabSpec4 = tabHost.newTabSpec("TAB4");
        tabSpec4.setIndicator("Set up");
        tabSpec4.setContent(R.id.tabSetup);

        tabHost.addTab(tabSpec0);
        tabHost.addTab(tabSpec1);
        tabHost.addTab(tabSpec2);
        tabHost.addTab(tabSpec3);
        tabHost.addTab(tabSpec4);

        tabHost.setOnTabChangedListener(mTabChange);
        tabHost.setCurrentTab(0);

        tabHost.getTabWidget().getChildTabViewAt(3).setEnabled(false);
        ((TextView)tabHost.getTabWidget().getChildAt(3).findViewById(android.R.id.title)).setTextColor(0xFFDCDCDC);
    }

    private TabHost.OnTabChangeListener mTabChange = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            if( tabId.equals("TAB0") ) UpdateMasteringMode();
            if( tabId.equals("TAB1") ) UpdateImageQuality();
            if( tabId.equals("TAB2") ) UpdateDotCorrection();
            if( tabId.equals("TAB3") ) UpdateInputSource();
            if( tabId.equals("TAB4") ) UpdateSetup();
        }
    };

    private boolean IsMasterMode() {
        return (mMasteringMode != null) && mMasteringMode.equals("MASTER");
    }

    private void UpdateMasteringMode() {
        for( int i = 0; i < mStrModeMastering.length; i++ ) {
            if( mMasteringMode.equals(mStrModeMastering[i]) ) {
                mSpinnerMastering.setSelection(i);

                if( !IsMasterMode() ) {
                    for( int j = 0; j < mSeekBarMastering.length; j++ ) {
                        mSeekBarMastering[j].setEnabled(false);
                        mSeekBarMastering[j].setProgress( mDataMastering[mMasteringModePos][j] );
                        mValueMastering[i].setText( String.valueOf(mDataMastering[mMasteringModePos][j]) );
                    }
                }
                else {
                    for( final SeekBar seekBar : mSeekBarMastering ) {
                        seekBar.setEnabled(true);
                    }
                    new AsyncTaskMasteringRead().execute();
                }
                break;
            }
        }
    }

    private void UpdateImageQuality() {
        String[] resultQuality = CheckFileInUsb(LedQualityInfo.PATH, LedQualityInfo.NAME);
        String[] resultGamma = CheckFileInUsb(LedGammaInfo.PATH, LedGammaInfo.NAME);
        if( (resultQuality == null || resultQuality.length == 0) && (resultGamma == null || resultGamma.length == 0) )
            return;

        mBtnImageQuality.setEnabled(true);
    }

    private void UpdateDotCorrection() {
        String[] result = CheckFileInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.NAME);
        if( result == null || result.length == 0 )
            return;

        mBtnDotCorrection.setEnabled(true);
    }

    private void UpdateInputSource() {
        for( int i = 0; i < mStrInputResolution.length; i++ ) {
            if( mInputResolution.equals(mStrInputResolution[i]) ) {
                mSpinnerInputReoslution.setSelection(i);
                break;
            }
        }

        for( int i = 0; i < mStrInputSource.length; i++ ) {
            if( mInputSource.equals(mStrInputSource[i]) ) {
                mSpinnerInputSource.setSelection(i);
                break;
            }
        }
    }

    private void UpdateSetup() {
        Date date = new Date( System.currentTimeMillis() );
        String curYear = new SimpleDateFormat("yyyy", Locale.US).format(date);
        String curMonth = new SimpleDateFormat("MM", Locale.US).format(date);
        String curDay = new SimpleDateFormat("dd", Locale.US).format(date);
        String curHour= new SimpleDateFormat("HH", Locale.US).format(date);
        String curMin = new SimpleDateFormat("mm", Locale.US).format(date);

        mSpinnerYear.setSelection( (Integer.parseInt(curYear, 10) < 2000) ? 0 : Integer.parseInt(curYear, 10) - 2000 );
        mSpinnerMonth.setSelection(Integer.parseInt(curMonth, 10) - 1);
        mSpinnerDay.setSelection(Integer.parseInt(curDay, 10) - 1);
        mSpinnerHour.setSelection(Integer.parseInt(curHour, 10));
        mSpinnerMin.setSelection(Integer.parseInt(curMin, 10));

        mEditCabinet.setText(((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM));

        for( int i = 0; i < CinemaService.OFF_TIME.length; i++ ) {
            if( ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_SCREEN_SAVING).equals(CinemaService.OFF_TIME[i]) ) {
                mSpinnerSuspendTime.setSelection(i);
                break;
            }
        }
    }

    //
    //
    //
    private void WriteHelper(String message) throws IOException {
        LocalSocket sender = new LocalSocket();
        sender.connect(new LocalSocketAddress("cinema.helper"));
        sender.getOutputStream().write(message.getBytes());
        sender.getOutputStream().close();
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
    //
    //
    private void AddViewMastering( int index, View[] childView, String[][] textMastering ) {
        mParentLayoutMastering.addView(childView[index]);

        final int itemIndex = index;
        final LinearLayout layoutText = (LinearLayout)childView[index].findViewById(R.id.layoutTextMastering0);
        final TextView textView0    = (TextView)childView[index].findViewById(R.id.textMastering0);
        final TextView textView1    = (TextView)childView[index].findViewById(R.id.textMastering1);
        mValueMastering[itemIndex]  = (TextView)childView[index].findViewById(R.id.textMastering2);
        mSeekBarMastering[itemIndex]= (SeekBar)childView[index].findViewById(R.id.seekMastering);
        mBtnMastering[itemIndex]    = (Button)childView[index].findViewById(R.id.btnMastering);

        if( textMastering[index][0].equals("CC00") ) {
            textView0.setText("Gamut");
        }
        else if( textMastering[index][0].equals("Global") ) {
            layoutText.setBackgroundResource( R.drawable.draw_line_top_right );
            layoutText.setPadding(0, 15, 0, 15);
            textView0.setText("Brightness");
        }

        textView1.setText( textMastering[index][0] );
        mSeekBarMastering[itemIndex].setMax( Integer.parseInt( textMastering[index][1] ) );

        mSeekBarMastering[itemIndex].setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mValueMastering[itemIndex].setText(String.valueOf(progress));

                int pos = mSpinnerMastering.getSelectedItemPosition();
                if( mDataMastering[pos][itemIndex] == progress && mBtnMastering[itemIndex].isEnabled() ) {
                    mBtnMastering[itemIndex].setEnabled(false);
                }
                if( mDataMastering[pos][itemIndex] != progress && !mBtnMastering[itemIndex].isEnabled() ) {
                    if( IsMasterMode() ) {
                        mBtnMastering[itemIndex].setEnabled(true);
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        mBtnMastering[itemIndex].setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mDataMastering[mMasteringModePos][itemIndex] = Integer.parseInt( mValueMastering[itemIndex].getText().toString() );
                ApplyMasteringMode( itemIndex, mDataMastering[mMasteringModePos][itemIndex] );
                mBtnMastering[itemIndex].setEnabled( false );
            }
        });
    }

    //
    //
    //
    private void ApplyInputSourceEnable( int index ) {
        Log.i(VD_DTAG,  String.format( "Apply( %d )", index ) );
    }

    private void ApplyInputSourceSelect( int indexResolution, int indexSource ) {
        Log.i(VD_DTAG,  String.format( "Apply( %d, %d )", indexResolution, indexSource) );

        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        byte[] inData = new byte[] { (byte)indexResolution, (byte)indexSource };

        if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData );
        if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData );
    }

    private void ApplyMasteringMode( int itemIdx, int value ) {
        boolean bValidPort0 = false, bValidPort1 = false;
        for( byte id : mCabinet ) {
            if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
            if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
        }

        NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
        byte[] inData = ctrl.AppendByteArray( new byte[]{(byte)mRegMastering[itemIdx]}, ctrl.IntToByteArray(value, NxCinemaCtrl.FORMAT_INT16) );

        if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData );
        if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData );
    }

    //
    //
    //
    private class AsyncTaskCheckCabinet extends AsyncTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... params) {
            for( int i = 0; i < 255; i++ ) {
                if( (i & 0x7F) < 0x10 )
                    continue;

                byte[] result = NxCinemaCtrl.GetInstance().Send(i, NxCinemaCtrl.CMD_TCON_STATUS, null);
                if (result == null || result.length == 0)
                    continue;

                if( 0 > result[0] )
                    continue;

                ((CinemaInfo)getApplicationContext()).AddCabinet( (byte)i );
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            CinemaLoading.Show( DisplayModeActivity.this );
            ((CinemaInfo)getApplicationContext()).ClearCabinet();
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            ((CinemaInfo)getApplicationContext()).SortCabinet();
            CinemaLoading.Hide();

            byte[] cabinet = ((CinemaInfo)getApplicationContext()).GetCabinet();
            int curCabinetNum = Integer.parseInt( ((CinemaInfo)getApplicationContext()).GetValue(CinemaInfo.KEY_CABINET_NUM ) );

            if( cabinet.length != curCabinetNum && ((CinemaInfo)getApplicationContext()).IsCheckCabinet() ) {
                ShowMessage( String.format(Locale.US, "Please check cabinet number. ( value: %d, detect: %d )", curCabinetNum, cabinet.length) );
            }
            else {
                String targetCabinetNum = mEditCabinet.getText().toString();

                CinemaInfo info = (CinemaInfo)getApplicationContext();
                info.SetValue(CinemaInfo.KEY_CABINET_NUM, targetCabinetNum);

                String strLog = String.format( "Change cabinet number. ( %s -> %s )", curCabinetNum, targetCabinetNum  );
                info.InsertLog(strLog);
                Log.i(VD_DTAG, strLog);
            }
        }
    }

    //
    //
    //
    private class AsyncTaskMasteringWrite extends AsyncTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            if( IsMasterMode() ) {
                for( int i = 0; i < mRegHideMastering.length; i++ ) {
                    byte[] reg = ctrl.IntToByteArray( mRegHideMastering[i], NxCinemaCtrl.FORMAT_INT8 );
                    byte[] data = ctrl.IntToByteArray( mDataHideMastering[mMasteringModePos][i], NxCinemaCtrl.FORMAT_INT16 );
                    byte[] inData = ctrl.AppendByteArray(reg, data);

                    if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData);
                    if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData);
                }
                return null;
            }

            for( int i = 0; i < mRegHideMastering.length; i++ ) {
                byte[] reg = ctrl.IntToByteArray( mRegHideMastering[i], NxCinemaCtrl.FORMAT_INT8 );
                byte[] data = ctrl.IntToByteArray( mDataHideMastering[mMasteringModePos][i], NxCinemaCtrl.FORMAT_INT16 );
                byte[] inData = ctrl.AppendByteArray(reg, data);

                if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData);
                if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData);
            }

            for( int i = 0; i < mRegMastering.length; i++ ) {
                byte[] reg = ctrl.IntToByteArray( mRegMastering[i], NxCinemaCtrl.FORMAT_INT8 );
                byte[] data = ctrl.IntToByteArray( mDataMastering[mMasteringModePos][i], NxCinemaCtrl.FORMAT_INT16 );
                byte[] inData = ctrl.AppendByteArray(reg, data);

                if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData);
                if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_MASTERING_WR, inData);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            for( int i = 0; i < mSeekBarMastering.length; i++ ) {
                mSeekBarMastering[i].setProgress( mDataMastering[mMasteringModePos][i] );
            }
            CinemaLoading.Hide();
        }
    }

    private class AsyncTaskMasteringRead extends AsyncTask<Void, Void, Void> {
        int[] mValue;

        @Override
        protected Void doInBackground(Void... voids) {
            if( !IsMasterMode() )
                return null;

            if( mCabinet == null || mCabinet.length == 0 )
                return null;

            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();
            for (int i = 0; i < mRegMastering.length; i++) {
                byte[] inData = new byte[] { (byte)mRegMastering[i], (byte)0xFF };
                byte[] result = ctrl.Send( mCabinet[0], NxCinemaCtrl.CMD_TCON_MASTERING_RD, inData );

                if( result == null || result.length == 0 )
                    continue;

                mValue[i] = ctrl.ByteArrayToInt(result);
            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            CinemaLoading.Show(DisplayModeActivity.this);

            mValue = new int[mRegMastering.length];
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            for (int i = 0; i < mSeekBarMastering.length; i++) {
                if( 0 > mValue[i] )
                    continue;

                mSeekBarMastering[i].setProgress(mValue[i]);
                mDataMastering[mMasteringModePos][i] = mValue[i];

                mBtnMastering[i].setEnabled(false);
            }
            CinemaLoading.Hide();
        }
    }

    //
    //
    //
    private String[] CheckFileInUsb( String dir, String regularExpression ) {
        String[] result = new String[0];

        String subdir = (dir == null) ? "" : dir;
        for( int i = 0; i < 10; i++ ) {
            File folder = new File( String.format(Locale.US, "/storage/usbdisk%d/%s", i, subdir) );
            File[] list = folder.listFiles();
            if( list == null || list.length == 0 )
                continue;

            Pattern pattern = Pattern.compile( regularExpression );
            for( File file : list ) {
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

    private class AsyncTaskImageQuality extends AsyncTask<Void, Void, Void> {
        private int mIndexQuality = mSpinnerImageQuality.getSelectedItemPosition();

        @Override
        protected Void doInBackground(Void... voids) {
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            boolean bValidPort0 = false, bValidPort1 = false;
            for( byte id : mCabinet ) {
                if( 0 == ((id >> 7) & 0x01) ) bValidPort0 = true;
                if( 1 == ((id >> 7) & 0x01) ) bValidPort1 = true;
            }

            String[] result;
            result = CheckFileInUsb(LedQualityInfo.PATH, LedQualityInfo.NAME);
            for( String file : result ) {
                LedQualityInfo info = new LedQualityInfo();
                if( info.Parse( file ) ) {
                    for( int i = 0; i < info.GetRegister().length; i++ ) {
                        byte[] reg = ctrl.IntToByteArray(info.GetRegister()[i], NxCinemaCtrl.FORMAT_INT8);
                        byte[] data = ctrl.IntToByteArray(info.GetData(mIndexQuality)[i], NxCinemaCtrl.FORMAT_INT16);
                        byte[] inData = ctrl.AppendByteArray(reg, data);

                        if( bValidPort0 ) ctrl.Send( 0x09, NxCinemaCtrl.CMD_TCON_QUALITY, inData);
                        if( bValidPort1 ) ctrl.Send( 0x89, NxCinemaCtrl.CMD_TCON_QUALITY, inData);
                    }
                }
            }

            result = CheckFileInUsb(LedGammaInfo.PATH, LedGammaInfo.NAME);
            for( String file : result ) {
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
            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
        }
    }

    private class AsyncTaskDotCorrection extends AsyncTask<Void, Integer, Void> {
        private StatusSimpleAdapter mAdapter;

        public AsyncTaskDotCorrection( StatusSimpleAdapter adapter ) {
            mAdapter = adapter;
            mAdapter.clear();
        }

        @Override
        protected Void doInBackground(Void... params) {
            int cnt = 0;
            NxCinemaCtrl ctrl = NxCinemaCtrl.GetInstance();

            String[] result = CheckFileInUsb(LedDotCorrectInfo.PATH, LedDotCorrectInfo.NAME);
            for( String file : result ) {
                LedDotCorrectInfo info = new LedDotCorrectInfo();
                if( info.Parse(file) ) {
                    byte[] sel = ctrl.IntToByteArray( info.GetFlashSel(), NxCinemaCtrl.FORMAT_INT8 );       // size: 1
                    byte[] data = ctrl.IntArrayToByteArray( info.GetData(), NxCinemaCtrl.FORMAT_INT16 );    // size: 61440
                    byte[] inData =  ctrl.AppendByteArray( sel, data );

                    byte[] res = ctrl.Send( info.GetIndex(), NxCinemaCtrl.CMD_TCON_DOT_CORRECTION, inData );

                    if( res == null || res.length == 0 ) {
                        publishProgress(info.GetIndex(), info.GetFlashSel(), 0);
                        continue;
                    }

                    if( res[0] == (byte)0xFF ) {
                        publishProgress(info.GetIndex(), info.GetFlashSel(), 0);
                        continue;
                    }

                    publishProgress(info.GetIndex(), info.GetFlashSel(), 1);
                }
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            mAdapter.add(
                new StatusSimpleInfo(
                String.format(Locale.US, "Cabinet %02d - Module %02d", (values[0] & 0x7F) - CinemaInfo.OFFSET_TCON, values[1]),
                values[2])
            );

            mAdapter.sort(new Comparator<StatusSimpleInfo>() {
                @Override
                public int compare(StatusSimpleInfo t0, StatusSimpleInfo t1) {
                    return (t0.GetTitle().compareTo(t1.GetTitle()) > 0) ? 1 : -1;
                }
            });

            mAdapter.notifyDataSetChanged();
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            CinemaLoading.Show( DisplayModeActivity.this );
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            CinemaLoading.Hide();
        }
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