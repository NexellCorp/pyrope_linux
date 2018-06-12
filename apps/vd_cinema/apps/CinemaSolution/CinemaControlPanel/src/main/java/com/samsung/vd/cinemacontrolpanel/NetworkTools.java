package com.samsung.vd.cinemacontrolpanel;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Locale;

/**
 * Created by doriya on 11/14/16.
 */
public class NetworkTools {
    private static final String VD_DTAG = "NetworkTools";

    private static final String SOCKET_NAME     = "cinema.helper";

    private static final String TAG_ETH_NETWORK = "network";
    private static final String TAG_ETH_LINK    = "link";

    public NetworkTools() {
    }

    public void SetConfig( String ip, String netmask, String gateway, String dns1, String dns2, String host ) {
        String cidr = GetCidr( ip, netmask );

        try {
            LocalSocket sender = new LocalSocket();
            sender.connect(new LocalSocketAddress(SOCKET_NAME));
            sender.getOutputStream().write(String.format("%s%s\n%s\n%s\n%s\n%s\n%s\n%s\n", TAG_ETH_NETWORK, ip, netmask, gateway, cidr, dns1, dns2, host).getBytes());
            sender.getOutputStream().close();
            sender.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public String GetEthLink() throws IOException {
        LocalSocket sender = new LocalSocket();
        sender.connect(new LocalSocketAddress(SOCKET_NAME));

        InputStream inStream = sender.getInputStream();
        OutputStream outStream = sender.getOutputStream();

        outStream.write(TAG_ETH_LINK.getBytes());

        InputStreamReader inStreamReader = new InputStreamReader(inStream);
        BufferedReader bufferedReader = new BufferedReader(inStreamReader);
        StringBuilder strBuilder = new StringBuilder();

        String inLine;
        while (((inLine = bufferedReader.readLine()) != null)) {
            strBuilder.append(inLine);
        }

        String strResult = strBuilder.toString();
        bufferedReader.close();

        inStream.close();
        outStream.close();
        sender.close();

        return strResult;
    }

    public boolean Ping( String host ) {
        NetworkChecker checker = new NetworkChecker(host);
        checker.start();
        return checker.GetResult();
    }

    private String GetCidr( String ip, String mask ) {
        String[] strIp = ip.split("\\.");
        String[] strMask = mask.split("\\.");
        int subnet = 0;

        byte[] byteMask = new byte[4];
        int[] intCidr = new int[4];

        for(int i = 0; i < 4; i++) {
            byteMask[i] = (byte)Integer.parseInt(strMask[i]);
            intCidr[i] = Integer.parseInt(strIp[i]) & Integer.parseInt(strMask[i]);
        }

        for(int i = 0; i < 4; i++)
        {
            int tmp = 0;
            for( int j = 0; j < 8; j++ ) {
                tmp = (byteMask[i] >> ( 7 - j) & 0x01);
                if( 0 == tmp ) break;
                subnet++;
            }
            if( 0 == tmp ) break;
        }

        return String.format(Locale.US, "%d.%d.%d.%d/%d", intCidr[0], intCidr[1], intCidr[2], intCidr[3], subnet);
    }

    private class NetworkChecker extends Thread {
        private String mHost;
        private boolean mResult = false;

        public NetworkChecker( String host ) {
            mHost = host;
        }

        @Override
        public void run() {
            try {
                InetAddress inetAddr = InetAddress.getByName(mHost);
                try {
                    mResult = inetAddr.isReachable(3000);
                    Log.i(VD_DTAG, String.format(Locale.US, "Network Reachable( %s ) : %b", mHost, mResult));
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } catch (UnknownHostException e) {
                e.printStackTrace();
            }
        }

        public boolean GetResult() {
            try {
                join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            return mResult;
        }
    }
}
