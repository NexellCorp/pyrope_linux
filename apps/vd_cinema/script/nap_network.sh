#!/system/bin/sh
cmd_cat="busybox cat"
cmd_sed="busybox sed"

network_config="/system/bin/nap_network"

case "$1" in
  start)
        echo "Starting network.."
        ipaddr=`$cmd_cat $network_config | $cmd_sed -n 1p`
        netmask=`$cmd_cat $network_config | $cmd_sed -n 2p`
        gateway=`$cmd_cat $network_config | $cmd_sed -n 3p`
        cidr=`$cmd_cat $network_config | $cmd_sed -n 4p`

        dns1=`$cmd_cat $network_config | $cmd_sed -n 5p`
        dns2=`$cmd_cat $network_config | $cmd_sed -n 6p`

        ifconfig eth0 $ipaddr netmask $netmask
        ndc network create 100 
        ndc network interface add 100 eth0
        ndc network route add 100 eth0 $cidr
        ndc network route add 100 eth0 0.0.0.0/0 $gateway
        if [ -z "$dns1" ] && [ -z "$dns2" ]
        then
            ndc resolver setnetdns 100 localdomain 8.8.8.8
        else
            if [ -n "$dns1" ]; then
                ndc resolver setnetdns 100 localdomain $dns1
            fi
            if [ -n "$dns2" ]; then
                ndc resolver setnetdns 100 localdomain $dns2
            fi
        fi
        ndc network default set 100
        ;;

  stop)
        echo "Stopping network.."
        netcfg eth0 down
        ndc network destroy 100
        ndc interface clearaddrs eth0
        ;;

  restart)
        "$0" stop
        "$0" start
        ;;
  *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
esac

exit $?