DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo -e "\E[1;31;43mNetwork IF down\E[0m"
ifconfig wlan0 down

# this actually disable all ath9k related modules
# and also mac80211, cfg80211 and compat
echo "Unloading all ath9k+mac realted modules"
modprobe -r ath9k ath9k_common rtl8192cu rtlwifi mac80211 

echo "Re-loading compat"
# reload drivers
# first one is the already compiled compat module in the system /lib folder
modprobe compat

echo "Reloading (NEW) cfg+mac 80211"
# then the currently (just) compiled ones, in the following order
insmod $DIR/net/wireless/cfg80211.ko
insmod $DIR/net/mac80211/mac80211.ko

if [ -z $1 ]; then 
	echo -e "Realoading (NEW) ath9k"
	insmod $DIR/drivers/net/wireless/ath/ath.ko
	insmod $DIR/drivers/net/wireless/ath/ath9k/ath9k_hw.ko
	insmod $DIR/drivers/net/wireless/ath/ath9k/ath9k_common.ko
	insmod $DIR/drivers/net/wireless/ath/ath9k/ath9k.ko
else
	echo "Realoading (LEGACY) rtl8192cu"
	modprobe rtl8192cu
fi

echo -e "\E[1;31;43mNetwork IF up\E[0m"
ifconfig wlan0 up
