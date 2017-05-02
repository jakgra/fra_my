#!/bin/bash

USER="$(id -un)"
P_BASE="$(pwd)"
P="${P_BASE}/mysql_server"

kill $(<${P}/mysqld.pid)
rm -r "${P}/data"
mkdir -p "${P}/data"

mysql_install_db \
	--datadir=${P}/data \
	--user=${USER} \
	--basedir=/usr

mysqld \
	--port=9999 \
	--user=${USER} \
	--datadir=${P}/data \
	--socket=${P}/mysqld.sock \
	--pid-file=${P}/mysqld.pid &

sleep 2

mysql --socket=${P}/mysqld.sock -u root < ${P_BASE}/setup_mysqld.sql

