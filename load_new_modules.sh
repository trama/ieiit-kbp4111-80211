#!/bin/bash

ifconfig wlan0 down
#airmon-ng stop wlan0
modprobe -r ath9k_htc
modprobe -r mac80211 cfg80211
#modprobe mac80211
#modprobe ath9

insmod compat/compat.ko
insmod net/wireless/cfg80211.ko
insmod net/mac80211/mac80211.ko
insmod drivers/net/wireless/ath/ath.ko
insmod drivers/net/wireless/ath/ath9k/ath9k_hw.ko
insmod drivers/net/wireless/ath/ath9k/ath9k_common.ko
insmod drivers/net/wireless/ath/ath9k/ath9k_htc.ko

