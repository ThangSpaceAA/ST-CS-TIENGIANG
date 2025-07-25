cmd_/home/pi/SIM7600_NDIS/Module.symvers := sed 's/ko$$/o/' /home/pi/SIM7600_NDIS/modules.order | scripts/mod/modpost -m -a   -o /home/pi/SIM7600_NDIS/Module.symvers -e -i Module.symvers   -T -
