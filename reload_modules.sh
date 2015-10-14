
echo "Network IF down"
ifconfig wlan0 down

# this actually disable all ath9k related modules
# and also mac80211, cfg80211 and compat
echo "Unloading all ath9k+mac realted modules"
modprobe -r ath9k ath9k_common rtl8192cu rtlwifi mac80211 

echo "Re-loading compat"
# reload drivers
# first one is the already compiled compat module in the system /lib folder
modprobe compat

echo "Reloading cfg+mac 80211"
# then the currently (just) compiled ones, in the following order
insmod net/wireless/cfg80211.ko
insmod net/mac80211/mac80211.ko

echo "Realoading ath9k"
#insmod drivers/net/wireless/ath/ath.ko
#insmod drivers/net/wireless/ath/ath9k/ath9k_hw.ko
#insmod drivers/net/wireless/ath/ath9k/ath9k_common.ko
#insmod drivers/net/wireless/ath/ath9k/ath9k.ko
modprobe rtl8192cu

echo "Network IF up"
ifconfig wlan0 up
