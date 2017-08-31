package com.samsung.vd.baseutils;

import android.content.Context;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.Locale;

/**
 * Created by doriya on 6/2/17.
 */

public class VdSpinCtrl extends LinearLayout {
    private static final String VD_DTAG = "VdSpinCtrl";

    private Button mBtnUp;
    private Button mBtnDn;
    private EditText mEditText;

    private int mMinValue = 0;
    private int mMaxValue = 65535;
    private int mCurValue = 0;

    private OnChangeListener mChangeListener = null;

    public VdSpinCtrl(Context context) {
        super(context);
        InitView();
    }

    public VdSpinCtrl(Context context, AttributeSet attrs) {
        super(context, attrs);
        InitView();
    }

    public VdSpinCtrl(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        InitView();
    }

    public void SetValue( int value ) {
        mCurValue = value;
        mEditText.setText( String.format(Locale.US, "%d", value ) );

//        if( mChangeListener != null ) {
//            mChangeListener.onChange( mCurValue );
//        }
    }

    public int GetValue() {
        return Integer.valueOf( mEditText.getText().toString() );
    }

    public void SetRange( int minValue, int maxValue ) {
        mMinValue = (minValue < 0) ? 0 : minValue;
        mMaxValue = (maxValue > 65535) ? 65535 : maxValue;
    }

    public void setEnabled( boolean bEnable ) {
        mBtnUp.setEnabled( bEnable );
        mBtnDn.setEnabled( bEnable );
        mEditText.setEnabled( bEnable );
    }

    private void InitView() {
        LayoutInflater inflater = (LayoutInflater)getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View view = inflater.inflate( R.layout.vd_spin_ctrl, this, false );
        addView( view );

        mBtnUp = (Button)findViewById(R.id.btnUp);
        mBtnUp.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int value = Integer.parseInt( mEditText.getText().toString(), 10 ) + 1;
                mEditText.setText( String.format(Locale.US, "%d", (value > mMaxValue) ? mMaxValue : value) );

                if( mChangeListener != null ) {
                    mChangeListener.onChange( mCurValue );
                }
            }
        });

        mBtnDn = (Button)findViewById(R.id.btnDn);
        mBtnDn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int value = Integer.parseInt( mEditText.getText().toString(), 10 ) - 1;
                mEditText.setText( String.format(Locale.US, "%d", (value < mMinValue) ? mMinValue : value) );

                if( mChangeListener != null ) {
                    mChangeListener.onChange( mCurValue );
                }
            }
        });

        mEditText = (EditText)findViewById( R.id.editText );
        mEditText.setText("0");
        mEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {
                // Log.i(VD_DTAG, String.format(Locale.US, "beforeTextChanged(). ( %s, %d, %d, %d )", charSequence.toString(), i, i1, i2) );
            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {
                // Log.i(VD_DTAG, String.format(Locale.US, "onTextChanged(). ( %s, %d, %d, %d ) ( %d, %d )", charSequence.toString(), i, i1, i2, Integer.MIN_VALUE, Integer.MAX_VALUE) );
            }

            @Override
            public void afterTextChanged(Editable editable) {
                // Log.i(VD_DTAG, String.format(Locale.US, "afterTextChanged(). ( %s )", editable.toString()) );
                if( editable.toString().length() == 0 ) {
                    mCurValue = 0;
                }
                else {
                    int desiredValue = Integer.parseInt( editable.toString(), 10 );
                    if( desiredValue < mMinValue ) {
                        desiredValue = mMinValue;
                    }
                    if( desiredValue > mMaxValue ) {
                        desiredValue = mMaxValue;
                    }
                    mCurValue = desiredValue;
                }

                if( mCurValue < mMinValue || mCurValue > mMaxValue ) {
                    mCurValue = mMinValue;
                }

                if( !editable.toString().equals(String.format(Locale.US, "%d", mCurValue) )) {
                    mEditText.setText( String.format(Locale.US, "%d", mCurValue) );
                    if( mChangeListener != null ) {
                        mChangeListener.onChange( mCurValue );
                    }
                }
            }
        });
    }

    public interface OnChangeListener {
        public abstract void onChange(int value);
    }

    public void SetOnChangeListener( OnChangeListener listener ) {
        mChangeListener = listener;
    }

    public void SetOnEditorActionListener(TextView.OnEditorActionListener listener) {
        mEditText.setOnEditorActionListener( listener );
    }
}
