cmd_/home/pi/SIM7600_NDIS/modules.order := {   echo /home/pi/SIM7600_NDIS/simcom_wwan.ko; :; } | awk '!x[$$0]++' - > /home/pi/SIM7600_NDIS/modules.order
