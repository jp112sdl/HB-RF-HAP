Nach dem Anschluss eines neuen Funkmoduls:
- `enable_rekeying.sh` in `/etc/config/crRFD/data` ablegen
- `cd /etc/config/crRFD/data`
- `chmod 755 enable_rekeying.sh`
- `./enable_rekeying.sh FAKE-SGTIN ORIG-SGTIN` (die Angabe der SGTIN muss ohne Bindestriche erfolgen!)
- Neustart (System oder HMIPServer)
