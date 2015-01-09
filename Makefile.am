AUTOMAKE_OPTIONS=foreign

SUBDIRS=CONFIG DES COMPRESS src

#confdir=${prefix}/etc
#conf_DATA=beetalk.conf

#ctldir=${prefix}/bin
#ctl_DATA=serverctl keep_alive time_delete

install-exec-hook:
	chmod -R a+w $(prefix)/etc
	cp beetalk.conf $(prefix)/etc
	cp serverctl keep_alive time_delete $(prefix)/bin
	chmod +x $(prefix)/bin/keep_alive $(prefix)/bin/serverctl $(prefix)/bin/time_delete
