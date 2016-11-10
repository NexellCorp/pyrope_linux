package com.samsung.vd.cinemacontrolpanel;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;

/**
 * Created by doriya on 11/3/16.
 */
public class CinemaAlert extends Activity {
    private static final String KEY_TITLE    = "title";
    private static final String KEY_MESSAGE  = "message";

    static public void Show(Context context, String title, String message ) {
        Intent intent;
        intent = new Intent( context, CinemaAlert.class );
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        intent.putExtra( KEY_TITLE, title );
        intent.putExtra( KEY_MESSAGE, message );

        context.startActivity(intent);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String strTitle, strMessage;

        Intent intent = getIntent();
        strTitle = intent.getStringExtra( KEY_TITLE );
        if( null == strTitle ) {
            strTitle = "Your Title";
        }

        strMessage = intent.getStringExtra( KEY_MESSAGE );
        if( null == strMessage ) {
            strMessage = "Your Message";
        }

        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        alertDialogBuilder.setTitle(strTitle);
        alertDialogBuilder
                .setMessage(strMessage)
                .setCancelable(false)
                .setNeutralButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        stopService(getIntent());
                        dialog.cancel();
                        finish();
                    }
                });

        alertDialogBuilder.create().show();
    }
}
